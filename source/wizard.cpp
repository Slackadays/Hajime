/*  Hajime, the ultimate startup script.
    Copyright (C) 2022 Slackadays and other contributors to Hajime on GitHub.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.*/

#include <iostream>
#include <filesystem>
#include <vector>
#include <random>
#if !defined(_WIN64) && !defined (_WIN32)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <fmt/format.h>

#include "output.hpp"
#include "installer.hpp"
#include "wizard.hpp"
#include "deduce.hpp"
#include "constants.hpp"

namespace fs = std::filesystem;

const string aikarFlags = "-XX:+UseG1GC -XX:+ParallelRefProcEnabled -XX:MaxGCPauseMillis=200 -XX:+UnlockExperimentalVMOptions -XX:+DisableExplicitGC -XX:+AlwaysPreTouch -XX:G1NewSizePercent=30 -XX:G1MaxNewSizePercent=40 -XX:G1HeapRegionSize=8M -XX:G1ReservePercent=20 -XX:G1HeapWastePercent=5 -XX:G1MixedGCCountTarget=4 -XX:InitiatingHeapOccupancyPercent=15 -XX:G1MixedGCLiveThresholdPercent=90 -XX:G1RSetUpdatingPauseTimePercent=5 -XX:SurvivorRatio=32 -XX:+PerfDisableSharedMem  -XX:MaxTenuringThreshold=1 -Daikars.new.flags=true -Dusing.aikars.flags=https://mcflags.emc.gs";
const string hillttyFlags = "-XX:+UseLargePages -XX:LargePageSizeInBytes=2M -XX:+UnlockExperimentalVMOptions -XX:+UseShenandoahGC -XX:ShenandoahGCMode=iu -XX:+UseNUMA -XX:+AlwaysPreTouch -XX:-UseBiasedLocking -XX:+DisableExplicitGC -Dfile.encoding=UTF-8";
const string froggeMCFlags = "-XX:+IgnoreUnrecognizedVMOptions -XX:+UnlockExperimentalVMOptions -XX:+UnlockDiagnosticVMOptions -XX:-OmitStackTraceInFastThrow -XX:+ShowCodeDetailsInExceptionMessages -XX:+DisableExplicitGC -XX:-UseParallelGC -XX:-UseParallelOldGC -XX:+PerfDisableSharedMem -XX:+UseZGC -XX:-ZUncommit -XX:ZUncommitDelay=300 -XX:ZCollectionInterval=5 -XX:ZAllocationSpikeTolerance=2.0 -XX:+AlwaysPreTouch -XX:+UseTransparentHugePages -XX:LargePageSizeInBytes=2M -XX:+UseLargePages -XX:+ParallelRefProcEnabled";
const string basicZGCFlags = "-XX:+UnlockExperimentalVMOptions -XX:+DisableExplicitGC -XX:-UseParallelGC -XX:-UseG1GC -XX:+UseZGC";

void Wizard::pause(float mean, float stdev) {
	if (doArtificialPauses) {
		std::random_device rand;
		std::mt19937 mt(rand());
		std::normal_distribution<float> normal(mean, stdev);
		int target = (int)abs(normal(mt));
		for (int ms = 0; ms < target; ms += 100) {
			std::cout << "\b|" << std::flush;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (ms += 100; ms >= target) {
				break;
			}
			std::cout << "\b/" << std::flush;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (ms += 100; ms >= target) {
				break;
			}
			std::cout << "\b―" << std::flush; //horizontal bar character
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (ms += 100; ms >= target) {
				break;
			}
			std::cout << "\b\\" << std::flush;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		std::cout << "\b" << std::flush;
	}
}

void Wizard::doLanguageStep() {
	term.out<Question>(text.question.HajimeLanguage);
	switch (term.getYN(fmt::vformat(fmt::to_string_view(text.option.CurrentLanguage), fmt::make_format_args(text.language)), "English", "Español", "Português", text.option.NoLanguage)) {
		case 1:
			defaultLang = text.language;
			break;
		case 2:
			defaultLang = "en";
			text.applyLang(defaultLang);
			break;
		case 3:
			defaultLang = "es";
			text.applyLang(defaultLang);
			break;
		case 4:
			defaultLang = "pt";
			text.applyLang(defaultLang);
			break;
		case 5:
			break;
	}
}

void Wizard::doHajimeStep() {
	term.out<Info>(text.info.wizard.HajimeFile);
	pause(200, 200);
	term.out<Question, NoEndline>(text.question.MakeHajimeConfig);
	if (term.getYN()) {
		pause(400, 400);
		wizardStep(confFile, installer.installDefaultHajConfFile, text.warning.FoundHajConf, text.error.HajFileNotMade, defaultLang);
	}
}

void Wizard::doServerStep() {
	term.out<Info>(text.info.wizard.ServerFile);
	pause(200, 200);
	term.out<Question, KeepEndlines, NoEndline>(text.question.WizardServerFile);
	int choice = term.getYN(text.option.MakeServerFileManually, text.option.LetHajimeDeduce, text.option.SkipStep);
	switch (choice) {
		case 1:
			while (true) {
				pause(400, 400);
				if (!std::regex_match(serverFile, std::regex(".+\\..+", std::regex_constants::optimize))) { //check for lack of file extension
					serverFile += ".server";
				}
				string file = "server.jar";
				string flags;
				term.out<Question>(text.question.ApplyConfigToServerFile);
				switch (term.getYN(text.option.DoManually, text.option.LetHajimeDeduce, text.option.SkipStep)) {
					case 1: {
						term.out<Question>(text.question.UseFlags);
						switch (term.getYN(text.option.AikarFlags, text.option.HillttyFlags, text.option.FroggeMCFlags, text.option.BasicZGCFlags, text.option.CustomFlags, text.option.SkipStep)) {
							case 1:
								flags = aikarFlags;
								break;
							case 2:
								flags = hillttyFlags;
								break;
							case 3:
								flags = froggeMCFlags;
								break;
							case 4:
								flags = basicZGCFlags;
							case 5:
								term.out<Question>(text.question.EnterCustomFlags);
								std::getline(std::cin, flags);
							case 6:
								flags = "";
								break;
						}
						term.out<Question>(fmt::vformat(fmt::to_string_view(text.question.UseDefaultServerFile), fmt::make_format_args(file)));
						switch (term.getYN(text.option.UseDefault, text.option.LetHajimeDeduce, text.option.EnterManually, text.option.SkipStep)) {
							case 1:
								break;
							case 2:
								term.out<Error>(text.error.OptionNotAvailable);
								break;
							case 3:
								term.out<Question>(text.question.EnterNewServerFile);
								std::getline(std::cin, file);
								break;
							case 4:
								file = "";
								break;
						}
						doAdvancedServerStep();
						term.out<Info>(text.info.InstallingDefServConf + serverFile + "...");
						ServerConfigFile tempConfig;
						tempConfig.serverFile = file;
						tempConfig.skipFileCheck = false;
						tempConfig.flags = flags;
						tempConfig.fileLocation = defaultServerConfFile;
						tempConfig.serverName = "MyServer";
						installer.installNewServerConfigFile(tempConfig);
						//if (wizardStep(serverFile, installer.installNewServerConfigFile, text.warning.FoundServerConfPlusFile + serverFile, text.error.ServerConfNotCreated, flags, file)) {
							servers.push_back(serverFile);
						//}
						term.out<Info>(text.info.InstallingNewServConf + serverFile + "...");
						break;
					}
					case 2:
						term.out<Error>(text.error.OptionNotAvailable);
						break;
					case 3:
						break;
				}
				term.out<Question>(text.question.CreateAnotherServerFile);
				if (term.getYN()) {
					term.out<Info, NoEndline>(fmt::vformat(fmt::to_string_view(text.info.EnterNewNameForServer), fmt::make_format_args(std::regex_replace(serverFile, std::regex("\\.server(?!\\w)", std::regex_constants::optimize), ""))));
					std::getline(std::cin, serverFile);
					std::cout << "\033[0m";
					pause(200, 200);
				} else {
					break;
				}
			}
			installedS = true;
			break;
		case 2:
			term.out<Error>(text.error.OptionNotAvailable);
			break;
		case 3:
			break;
	}
}

void Wizard::doAdvancedServerStep() {
	term.out<Question>("Would you like to apply an advanced configuration to this server?");
	if (term.getYN()) {
		doStartupStep();
	}
}

void Wizard::doAdvancedHajimeStep() {
	term.out<Question>("Would you like to apply an advanced configuration to Hajime?");
	if (term.getYN()) {
		pause(400, 400);
		doStartupStep();
	}
}

void Wizard::doStartupStep() {
	term.out<Info>(text.info.wizard.StartupService);
	pause(200, 200);
	term.out<Question>(text.question.WizardStartupService);
	if (term.getYN()) {
		pause(400, 400);
		installer.installStartupService("/etc/systemd/system/hajime.service");
	}
}

void Wizard::doNextStepStep() {
	if (installedS) {
		if (servers.size() == 1) {
			term.out<Info>(fmt::vformat(fmt::to_string_view(text.info.wizard.NextStepServerFile), fmt::make_format_args(servers[0])));
		} else if (servers.size() == 2) {
			term.out<Info>(fmt::vformat(fmt::to_string_view(text.info.wizard.NextStepServerFile), fmt::make_format_args((servers[0] + " & " + servers[1]))));
		} else if (servers.size() > 2) {
			term.out<Info, NoEndline>(text.info.wizard.NextStepServerFile);
			for (int i = 0; i < (servers.size() - 1); i++) {
				term.out<None, NoEndline>(servers[i] + ", ");
			}
			term.out<None>("& " + servers.back() + ".");
		}
	}
}

void Wizard::initialHajimeSetupUnattended(string tempConfFile, string tempServerFile) {
	//std::cout << deduce.hajimeFile() << std::endl;
	//std::cout << deduce.serverConfig() << std::endl;
	for (const auto& it : deduce.serverFiles(fs::current_path())) {
		std::cout << "found server jar " << it << std::endl;
	}
	installer.installDefaultHajConfFile(deduce.hajimeFile(), true, text.filterLanguage(text.getUserLanguage()));
	ServerConfigFile myConfig;
	myConfig.fileLocation = deduce.serverConfig();
	myConfig.skipFileCheck = true;
	myConfig.flags = aikarFlags;
	myConfig.serverFile = deduce.serverFiles(fs::current_path()).at(0);
	installer.installNewServerConfigFile(myConfig);
	/*for (auto dir = fs::recursive_directory_iterator{fs::current_path()}; dir != fs::recursive_directory_iterator(); ++dir) {
		if (dir->is_directory()) {
			for (const auto& it : deduce.serverFiles(dir->path())) {
				std::cout << "found server jar " << it << std::endl;
			}
		}
	}*/
}

void Wizard::applySteps() {
	term.out<Info>("This part is currently not implemented yet");
}

void Wizard::initialHajimeSetupAttended(string tempConfFile, string tempServerFile) {
	servers.clear();
	installedS = false;
	confFile = tempConfFile;
	serverFile = tempServerFile;
	pause(400, 400);
	doLanguageStep();
	pause(400, 400);
	term.dividerLine();
	doHajimeStep();
	pause(400, 400);
	term.dividerLine();
	doServerStep();
	pause(400, 400);
	term.dividerLine();
	pause(400, 400);
	doAdvancedHajimeStep();
	pause(400, 400);
	term.dividerLine();
	term.out<Info>(text.info.wizard.Complete);
	term.out<Info>("Finalizing your installation");
	applySteps();
	pause(200, 200);
	doNextStepStep();
	pause(400, 400);
}

Wizard wizard;
