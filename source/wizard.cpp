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

void Wizard::dividerLine() {
	#if defined(_WIN64) || defined(_WIN32)
	CONSOLE_SCREEN_BUFFER_INFO w;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &w);
	for (int i = 0; i < w.dwSize.X; i++) {
		logObj->out("―", None, 0, 0);
	}
	#else
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	for (int i = 0; i < w.ws_col; i++) {
		logObj->out("―", None, 0, 0);
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

void Wizard::doHajimeStep(string &confFile) {
	logObj->out(text.infoWizardHajimeFile, Info);
	pause(200, 200);
	logObj->out(text.questionMakeHajimeConfig, Question, 0, 0);
	if (logObj->getYN()) {
		pause(400, 400);
		wizardStep(confFile, installer.installDefaultHajConfFile, text.warningFoundHajConf, text.errorHajFileNotMade);
	}
}

void Wizard::doServerStep(bool &installedS, string &serverFile, std::vector<string> &servers) {
	logObj->out(text.infoWizardServerFile, Info);
	pause(200, 200);
	logObj->out(text.questionWizardServerFile, Question, 1, 1);
	int choice = logObj->getYN(text.optionMakeServerFileManually, text.optionLetHajimeDeduce, text.optionSkipStep);
	switch (choice) {
		case 1:
			while (true) {
				pause(400, 400);
				if (!std::regex_match(serverFile, std::regex(".+\\..+", std::regex_constants::optimize))) { //check for lack of file extension
					serverFile += ".conf";
				}
				string file = "server.jar";
				string flags;
				logObj->out(text.questionApplyConfigToServerFile, Question);
				switch (logObj->getYN(text.optionDoManually, text.optionLetHajimeDeduce, text.optionSkipStep)) {
					case 1:
						logObj->out(text.questionUseFlags, Question);
						switch (logObj->getYN(text.optionAikarFlags, text.optionHillttyFlags, "Use custom flags", text.optionSkipStep)) {
							case 1:
								flags = aikarFlags;
								break;
							case 2:
								flags = hillttyFlags;
								break;
							case 3:
								logObj->out("Enter your custom flags here: ", Question);
								std::getline(std::cin, flags);
							case 4:
								flags = "";
								break;
						}
						logObj->out(text.questionUseDefaultServerFile1 + file + text.questionUseDefaultServerFile2, Question);
						switch (logObj->getYN(text.optionUseDefault, text.optionLetHajimeDeduce, text.optionEnterManually, text.optionSkipStep)) {
							case 1:
								break;
							case 2:
								logObj->out(text.errorOptionNotAvailable, Error);
								break;
							case 3:
								logObj->out(text.questionEnterNewServerFile, Question);
								std::getline(std::cin, file);
								break;
							case 4:
								file = "";
								break;
						}
						logObj->out(text.infoInstallingDefServConf + serverFile + "...", Info);
						if (wizardStep(serverFile, installer.installNewServerConfigFile, text.warningFoundServerConfPlusFile + serverFile, text.errorServerConfNotCreated, flags, file)) {
							servers.push_back(serverFile);
						}
						logObj->out(text.infoInstallingNewServConf + serverFile + "...", Info);
						break;
					case 2:
						logObj->out(text.errorOptionNotAvailable, Error);
						break;
					case 3:
						break;
				}
				logObj->out(text.questionCreateAnotherServerFile, Question);
				if (logObj->getYN()) {
					logObj->out(text.infoEnterNewNameForServer1 + std::regex_replace(serverFile, std::regex("\\.conf(?!\\w)", std::regex_constants::optimize), "") + text.infoEnterNewNameForServer2, Info, 0, 0);
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
			logObj->out(text.errorOptionNotAvailable, Error);
			break;
		case 3:
			break;
	}
}

void Wizard::doServersStep(string &serversFile, std::vector<string> &servers) {
	logObj->out(text.infoWizardServersFile, Info);
	logObj->out(text.questionWizardServersFile, Question);
	if (logObj->getYN()) {
		pause(500, 800);
		wizardStep(serversFile, installer.installDefaultServersFile, text.errorServersFilePresent, text.errorServersFileNotCreated, servers);
	}
}

void Wizard::doStartupStep(string &sysdService) {
	logObj->out(text.infoWizardStartupService, Info);
	pause(200, 200);
	logObj->out(text.questionWizardStartupService, Question);
	if (logObj->getYN()) {
		pause(400, 400);
		installer.installStartupService(sysdService);
	}
}

void Wizard::doNextStepStep(bool &installedS, std::vector<string> &servers) {
	if (installedS) {
		if (servers.size() == 1) {
			logObj->out(text.infoWizardNextStepServerFile1 + servers[0] + text.infoWizardNextStepServerFile2, Info);
		} else if (servers.size() == 2) {
			logObj->out(text.infoWizardNextStepServerFile1 + servers[0] + " & " + servers[1] + text.infoWizardNextStepServerFile2, Info);
		} else if (servers.size() > 2) {
			logObj->out(text.infoWizardNextStepServerFile1, Info, 0, 0);
			for (int i = 0; i < (servers.size() - 1); i++) {
				logObj->out(servers[i] + ", ", None, 0, 0);
			}
			logObj->out("& " + servers.back() + text.infoWizardNextStepServerFile2, None);
		}
	}
}

void Wizard::initialHajimeSetup(string confFile, string serversFile, string serverFile, string sysdService) {
	std::vector<string> servers;
	bool installedS = false;
	pause(400, 400);
	doHajimeStep(confFile);
	pause(400, 400);
	dividerLine();
	doServerStep(installedS, serverFile, servers);
	pause(400, 400);
	dividerLine();
	doServersStep(serversFile, servers);
	pause(400, 400);
	dividerLine();
	doStartupStep(sysdService);
	pause(400, 400);
	dividerLine();
	logObj->out(text.infoWizardComplete, Info);
	pause(200, 200);
	doNextStepStep(installedS, servers);
	pause(400, 400);
}

Wizard wizard;
