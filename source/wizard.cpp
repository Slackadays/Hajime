#include <iostream>
#include <filesystem>
#include <vector>
#include <random>

#include "output.hpp"
#include "installer.hpp"
#include "wizard.hpp"

namespace fs = std::filesystem;

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
	logObj->out(text.questionWizardServerFile, Question);
	if (logObj->getYN()) {
		while (true) {
			pause(400, 400);
			if (!std::regex_match(serverFile, std::regex(".+\\..+", std::regex_constants::optimize))) { //check for lack of file extension
				serverFile += ".conf";
			}
			if (wizardStep(serverFile, installer.installDefaultServerConfFile, text.warningFoundServerConfPlusFile + serverFile, text.errorServerConfNotCreated)) {
				servers.push_back(serverFile);
			}
			logObj->out(text.questionCreateAnotherServerFile, Question);
			if (logObj->getYN()) {
				logObj->out(text.infoEnterNewNameForServer1 + std::regex_replace(serverFile, std::regex("\\.conf(?!\\w)", std::regex_constants::optimize), "") + text.infoEnterNewNameForServer2, Info, 0, 0);
				std::cin >> serverFile;
				std::cout << "\033[0m";
				pause(200, 200);
			} else {
				break;
			}
		}
		installedS = true;
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
	logObj->out("--------------------");
	doServerStep(installedS, serverFile, servers);
	pause(400, 400);
	logObj->out("--------------------");
	doServersStep(serversFile, servers);
	pause(400, 400);
	logObj->out("--------------------");
	doStartupStep(sysdService);
	pause(400, 400);
	logObj->out("--------------------");
	logObj->out(text.infoWizardComplete, Info);
	pause(200, 200);
	doNextStepStep(installedS, servers);
	pause(400, 400);
}

Wizard wizard;
