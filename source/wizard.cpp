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
		hjlog->out("─", None, 0, 0);
	}
	#else
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	for (int i = 0; i < w.ws_col; i++) {
		hjlog->out("─", None, 0, 0);
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

void Wizard::doHajimeStep() {
	hjlog->out(text.infoWizardHajimeFile, Info);
	pause(200, 200);
	hjlog->out(text.questionMakeHajimeConfig, Question, 0, 0);
	if (hjlog->getYN()) {
		pause(400, 400);
		wizardStep(confFile, installer.installDefaultHajConfFile, text.warningFoundHajConf, text.errorHajFileNotMade);
	}
}

void Wizard::doServerStep() {
	hjlog->out(text.infoWizardServerFile, Info);
	pause(200, 200);
	hjlog->out(text.questionWizardServerFile, Question, 1, 1);
	int choice = hjlog->getYN(text.optionMakeServerFileManually, text.optionLetHajimeDeduce, text.optionSkipStep);
	switch (choice) {
		case 1:
			while (true) {
				pause(400, 400);
				if (!std::regex_match(serverFile, std::regex(".+\\..+", std::regex_constants::optimize))) { //check for lack of file extension
					serverFile += ".conf";
				}
				string file = "server.jar";
				string flags;
				hjlog->out(text.questionApplyConfigToServerFile, Question);
				switch (hjlog->getYN(text.optionDoManually, text.optionLetHajimeDeduce, text.optionSkipStep)) {
					case 1:
						hjlog->out(text.questionUseFlags, Question);
						switch (hjlog->getYN(text.optionAikarFlags, text.optionHillttyFlags, text.optionFroggeMCFlags, text.optionBasicZGCFlags, text.optionCustomFlags, text.optionSkipStep)) {
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
								hjlog->out(text.questionEnterCustomFlags, Question);
								std::getline(std::cin, flags);
							case 6:
								flags = "";
								break;
						}
						hjlog->out(text.questionUseDefaultServerFile1 + file + text.questionUseDefaultServerFile2, Question);
						switch (hjlog->getYN(text.optionUseDefault, text.optionLetHajimeDeduce, text.optionEnterManually, text.optionSkipStep)) {
							case 1:
								break;
							case 2:
								hjlog->out(text.errorOptionNotAvailable, Error);
								break;
							case 3:
								hjlog->out(text.questionEnterNewServerFile, Question);
								std::getline(std::cin, file);
								break;
							case 4:
								file = "";
								break;
						}
						hjlog->out(text.infoInstallingDefServConf + serverFile + "...", Info);
						if (wizardStep(serverFile, installer.installNewServerConfigFile, text.warningFoundServerConfPlusFile + serverFile, text.errorServerConfNotCreated, flags, file)) {
							servers.push_back(serverFile);
						}
						hjlog->out(text.infoInstallingNewServConf + serverFile + "...", Info);
						break;
					case 2:
						hjlog->out(text.errorOptionNotAvailable, Error);
						break;
					case 3:
						break;
				}
				hjlog->out(text.questionCreateAnotherServerFile, Question);
				if (hjlog->getYN()) {
					hjlog->out(text.infoEnterNewNameForServer1 + std::regex_replace(serverFile, std::regex("\\.conf(?!\\w)", std::regex_constants::optimize), "") + text.infoEnterNewNameForServer2, Info, 0, 0);
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
			hjlog->out(text.errorOptionNotAvailable, Error);
			break;
		case 3:
			break;
	}
}

void Wizard::doServersStep() {
	hjlog->out(text.infoWizardServersFile, Info);
	hjlog->out(text.questionWizardServersFile, Question);
	if (hjlog->getYN()) {
		pause(500, 800);
		wizardStep(serversFile, installer.installDefaultServersFile, text.errorServersFilePresent, text.errorServersFileNotCreated, servers);
	}
}

void Wizard::doStartupStep() {
	hjlog->out(text.infoWizardStartupService, Info);
	pause(200, 200);
	hjlog->out(text.questionWizardStartupService, Question);
	if (hjlog->getYN()) {
		pause(400, 400);
		installer.installStartupService(sysdService);
	}
}

void Wizard::doNextStepStep() {
	if (installedS) {
		if (servers.size() == 1) {
			hjlog->out(text.infoWizardNextStepServerFile1 + servers[0] + text.infoWizardNextStepServerFile2, Info);
		} else if (servers.size() == 2) {
			hjlog->out(text.infoWizardNextStepServerFile1 + servers[0] + " & " + servers[1] + text.infoWizardNextStepServerFile2, Info);
		} else if (servers.size() > 2) {
			hjlog->out(text.infoWizardNextStepServerFile1, Info, 0, 0);
			for (int i = 0; i < (servers.size() - 1); i++) {
				hjlog->out(servers[i] + ", ", None, 0, 0);
			}
			hjlog->out("& " + servers.back() + text.infoWizardNextStepServerFile2, None);
		}
	}
}

void Wizard::initialHajimeSetup(string tempConfFile, string tempServersFile, string tempServerFile, string tempSysdService) {
	servers.clear();
	installedS = false;
	confFile = tempConfFile;
	serversFile = tempServersFile;
	serverFile = tempServerFile;
	sysdService = tempSysdService;
	pause(400, 400);
	doHajimeStep();
	pause(400, 400);
	dividerLine();
	doServerStep();
	pause(400, 400);
	dividerLine();
	doServersStep();
	pause(400, 400);
	dividerLine();
	doStartupStep();
	pause(400, 400);
	dividerLine();
	hjlog->out(text.infoWizardComplete, Info);
	pause(200, 200);
	doNextStepStep();
	pause(400, 400);
}

Wizard wizard;
