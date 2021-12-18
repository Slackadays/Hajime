#include <iostream>
#include <filesystem>
#include <vector>

#include "output.hpp"
#include "installer.hpp"
#include "wizard.hpp"

namespace fs = std::filesystem;

void doHajimeStep(string &confFile) {
  auto pause = [](int ms){std::this_thread::sleep_for(std::chrono::milliseconds(ms));};
  logObj->out(text.infoWizardHajimeFile, Info);
  logObj->out(text.questionMakeHajimeConfig, Question, 0, 0);
  if (logObj->getYN()) {
    pause(500);
    wizardStep(confFile, installer.installDefaultHajConfFile, text.warningFoundHajConf, text.errorHajFileNotMade);
  }
}

void doServerStep(bool &installedS, string &serverFile, std::vector<string> &servers) {
  auto pause = [](int ms){std::this_thread::sleep_for(std::chrono::milliseconds(ms));};
  logObj->out(text.infoWizardServerFile, Info);
  logObj->out(text.questionWizardServerFile, Question);
  if (logObj->getYN()) {
    while (true) {
      pause(500);
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
      } else {
        break;
      }
    }
    installedS = true;
  }
}

void doServersStep(string &serversFile, std::vector<string> &servers) {
  logObj->out(text.infoWizardServersFile, Info);
  logObj->out(text.questionWizardServersFile, Question);
  if (logObj->getYN()) {
    pause(500);
    wizardStep(serversFile, installer.installDefaultServersFile, text.errorServersFilePresent, text.errorServersFileNotCreated, servers);
  }
}

void doStartupStep(string &sysdService) {
  logObj->out(text.infoWizardStartupService, Info);
  logObj->out(text.questionWizardStartupService, Question);
  if (logObj->getYN()) {
    pause(500);
    installer.installStartupService(sysdService);
  }
}

void doNextStepStep (bool &installedS, std::vector<string> &servers) {
  if (installedS) {
    if (servers.size() == 1) {
      logObj->out(text.infoWizardNextStepServerFile1 + servers[0] + text.infoWizardNextStepServerFile2, Info);
    } else if (servers.size() == 2) {
      logObj->out(text.infoWizardNextStepServerFile1 + servers[0] + " and " + servers[1] + text.infoWizardNextStepServerFile2, Info);
    } else if (servers.size() > 2) {
      logObj->out(text.infoWizardNextStepServerFile1, Info, 0, 0);
      for (int i = 0; i < servers.size(); i++) {
        logObj->out(servers[i] + ", ", None, 0, 0);
      }
      logObj->out("and " + servers.back() + text.infoWizardNextStepServerFile2, None);
    }
  }
}

void initialHajimeSetup(string confFile, string serversFile, string serverFile, string sysdService) {
    auto pause = [](int ms){std::this_thread::sleep_for(std::chrono::milliseconds(ms));};
    std::vector<string> servers;
    bool installedS = false;
    pause(500);
    doHajimeStep(confFile);
    pause(500);
    logObj->out("--------------------");
    doServerStep(installedS, serverFile, servers);
    pause(500);
    logObj->out("--------------------");
    doServersStep(serversFile, servers);
    pause(500);
    logObj->out("--------------------");
    doStartupStep(sysdService);
    pause(500);
    logObj->out("--------------------");
    logObj->out(text.infoWizardComplete, Info);
    doNextStepStep(installedS, servers);
    pause(500);
}
