#include <iostream>
#include <filesystem>
#include <vector>
#include <random>
#if !defined(_WIN64) && !defined (_WIN32)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include "output.hpp"
#include "installer.hpp"
#include "wizard.hpp"
#include "deduce.hpp"

namespace fs = std::filesystem;

const string aikarFlags = "-XX:+UseG1GC -XX:+ParallelRefProcEnabled -XX:MaxGCPauseMillis=200 -XX:+UnlockExperimentalVMOptions -XX:+DisableExplicitGC -XX:+AlwaysPreTouch -XX:G1NewSizePercent=30 -XX:G1MaxNewSizePercent=40 -XX:G1HeapRegionSize=8M -XX:G1ReservePercent=20 -XX:G1HeapWastePercent=5 -XX:G1MixedGCCountTarget=4 -XX:InitiatingHeapOccupancyPercent=15 -XX:G1MixedGCLiveThresholdPercent=90 -XX:G1RSetUpdatingPauseTimePercent=5 -XX:SurvivorRatio=32 -XX:+PerfDisableSharedMem  -XX:MaxTenuringThreshold=1 -Daikars.new.flags=true -Dusing.aikars.flags=https://mcflags.emc.gs";
const string hillttyFlags = "-XX:+UseLargePages -XX:LargePageSizeInBytes=2M -XX:+UnlockExperimentalVMOptions -XX:+UseShenandoahGC -XX:ShenandoahGCMode=iu -XX:+UseNUMA -XX:+AlwaysPreTouch -XX:-UseBiasedLocking -XX:+DisableExplicitGC -Dfile.encoding=UTF-8";
const string froggeMCFlags = "-XX:+IgnoreUnrecognizedVMOptions -XX:+UnlockExperimentalVMOptions -XX:+UnlockDiagnosticVMOptions -XX:-OmitStackTraceInFastThrow -XX:+ShowCodeDetailsInExceptionMessages -XX:+DisableExplicitGC -XX:-UseParallelGC -XX:-UseParallelOldGC -XX:+PerfDisableSharedMem -XX:+UseZGC -XX:-ZUncommit -XX:ZUncommitDelay=300 -XX:ZCollectionInterval=5 -XX:ZAllocationSpikeTolerance=2.0 -XX:+AlwaysPreTouch -XX:+UseTransparentHugePages -XX:LargePageSizeInBytes=2M -XX:+UseLargePages -XX:+ParallelRefProcEnabled";
const string basicZGCFlags = "-XX:+UnlockExperimentalVMOptions -XX:+DisableExplicitGC -XX:-UseParallelGC -XX:-UseG1GC -XX:+UseZGC";

void Wizard::dividerLine() {
	#if defined(_WIN64) || defined(_WIN32)
	CONSOLE_SCREEN_BUFFER_INFO w;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &w);
	for (int i = 0; i < w.dwSize.X; i++) {
		hjlog.out<None, NoEndline>("─");
	}
	#else
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	for (int i = 0; i < w.ws_col; i++) {
		hjlog.out<None, NoEndline>("─");
	}
	#endif
	std::cout << std::endl;
}

void Wizard::pause(float mean, float stdev) {
	if (doArtificialPauses) {
		std::random_device rand;
		std::normal_distribution<float> normal(mean, stdev);
		int target = (int)abs(normal(rand));
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
	hjlog.out<Question>(text.question.HajimeLanguage);
	switch (hjlog.getYN(string(text.option.CurrentLanguage1 + text.language + text.option.CurrentLanguage2), "English", "Español", "Português", text.option.NoLanguage)) {
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
	hjlog.out<Info>(text.info.wizard.HajimeFile);
	pause(200, 200);
	hjlog.out<Question, NoEndline>(text.question.MakeHajimeConfig);
	if (hjlog.getYN()) {
		pause(400, 400);
		wizardStep(confFile, installer.installDefaultHajConfFile, text.warning.FoundHajConf, text.error.HajFileNotMade, defaultLang);
	}
}

void Wizard::doServerStep() {
	hjlog.out<Info>(text.info.wizard.ServerFile);
	pause(200, 200);
	hjlog.out<Question, KeepEndlines, NoEndline>(text.question.WizardServerFile);
	int choice = hjlog.getYN(text.option.MakeServerFileManually, text.option.LetHajimeDeduce, text.option.SkipStep);
	switch (choice) {
		case 1:
			while (true) {
				pause(400, 400);
				if (!std::regex_match(serverFile, std::regex(".+\\..+", std::regex_constants::optimize))) { //check for lack of file extension
					serverFile += ".server";
				}
				string file = "server.jar";
				string flags;
				hjlog.out<Question>(text.question.ApplyConfigToServerFile);
				switch (hjlog.getYN(text.option.DoManually, text.option.LetHajimeDeduce, text.option.SkipStep)) {
					case 1:
						hjlog.out<Question>(text.question.UseFlags);
						switch (hjlog.getYN(text.option.AikarFlags, text.option.HillttyFlags, text.option.FroggeMCFlags, text.option.BasicZGCFlags, text.option.CustomFlags, text.option.SkipStep)) {
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
								hjlog.out<Question>(text.question.EnterCustomFlags);
								std::getline(std::cin, flags);
							case 6:
								flags = "";
								break;
						}
						hjlog.out<Question>(text.question.UseDefaultServerFile1 + file + text.question.UseDefaultServerFile2);
						switch (hjlog.getYN(text.option.UseDefault, text.option.LetHajimeDeduce, text.option.EnterManually, text.option.SkipStep)) {
							case 1:
								break;
							case 2:
								hjlog.out<Error>(text.error.OptionNotAvailable);
								break;
							case 3:
								hjlog.out<Question>(text.question.EnterNewServerFile);
								std::getline(std::cin, file);
								break;
							case 4:
								file = "";
								break;
						}
						hjlog.out<Info>(text.info.InstallingDefServConf + serverFile + "...");
						if (wizardStep(serverFile, installer.installNewServerConfigFile, text.warning.FoundServerConfPlusFile + serverFile, text.error.ServerConfNotCreated, flags, file)) {
							servers.push_back(serverFile);
						}
						hjlog.out<Info>(text.info.InstallingNewServConf + serverFile + "...");
						break;
					case 2:
						hjlog.out<Error>(text.error.OptionNotAvailable);
						break;
					case 3:
						break;
				}
				hjlog.out<Question>(text.question.CreateAnotherServerFile);
				if (hjlog.getYN()) {
					hjlog.out<Info, NoEndline>(text.info.EnterNewNameForServer1 + std::regex_replace(serverFile, std::regex("\\.server(?!\\w)", std::regex_constants::optimize), "") + text.info.EnterNewNameForServer2);
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
			hjlog.out<Error>(text.error.OptionNotAvailable);
			break;
		case 3:
			break;
	}
}

void Wizard::doStartupStep() {
	hjlog.out<Info>(text.info.wizard.StartupService);
	pause(200, 200);
	hjlog.out<Question>(text.question.WizardStartupService);
	if (hjlog.getYN()) {
		pause(400, 400);
		installer.installStartupService(sysdService);
	}
}

void Wizard::doNextStepStep() {
	if (installedS) {
		if (servers.size() == 1) {
			hjlog.out<Info>(text.info.wizard.NextStepServerFile1 + servers[0] + text.info.wizard.NextStepServerFile2);
		} else if (servers.size() == 2) {
			hjlog.out<Info>(text.info.wizard.NextStepServerFile1 + servers[0] + " & " + servers[1] + text.info.wizard.NextStepServerFile2);
		} else if (servers.size() > 2) {
			hjlog.out<Info, NoEndline>(text.info.wizard.NextStepServerFile1);
			for (int i = 0; i < (servers.size() - 1); i++) {
				hjlog.out<None, NoEndline>(servers[i] + ", ");
			}
			hjlog.out<None>("& " + servers.back() + text.info.wizard.NextStepServerFile2);
		}
	}
}

void Wizard::initialHajimeSetup(string tempConfFile, string tempServerFile, string tempSysdService) {
	servers.clear();
	installedS = false;
	confFile = tempConfFile;
	serverFile = tempServerFile;
	sysdService = tempSysdService;
	pause(400, 400);
	doLanguageStep();
	pause(400, 400);
	dividerLine();
	doHajimeStep();
	pause(400, 400);
	dividerLine();
	doServerStep();
	pause(400, 400);
	dividerLine();
	pause(400, 400);
	doStartupStep();
	pause(400, 400);
	dividerLine();
	hjlog.out<Info>(text.info.wizard.Complete);
	pause(200, 200);
	doNextStepStep();
	pause(400, 400);
}

Wizard wizard;
