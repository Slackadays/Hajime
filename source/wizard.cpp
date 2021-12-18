#include <iostream>
#include <filesystem>
#include <vector>

#include "output.hpp"
#include "installer.hpp"
#include "wizard.hpp"

namespace fs = std::filesystem;

void initialHajimeSetup(string confFile, string serversFile, string serverFile, string sysdService) {
    auto pause = [](int ms){std::this_thread::sleep_for(std::chrono::milliseconds(ms));};
    bool installedS = false;
    pause(500);
    logObj->out(text.infoWizardHajimeFile, Info);
    logObj->out(text.questionMakeHajimeConfig, Question, 0, 0);
    if (logObj->getYN()) {
      pause(500);
      wizardStep(confFile, installer.installDefaultHajConfFile, text.warningFoundHajConf, text.errorHajFileNotMade);
    }
    pause(500);
    logObj->out("--------------------");
    std::vector<string> servers;
    logObj->out(text.infoWizardServerFile, Info);
    logObj->out(text.questionWizardServerFile, Question);
    if (logObj->getYN()) {
      while (true) {
        pause(500);
        if (!std::regex_match(serverFile, std::regex(".+\\..+", std::regex_constants::optimize))) { //check for lack file extension
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
    pause(500);
    logObj->out("--------------------");
    logObj->out(text.infoWizardServersFile, Info);
    logObj->out(text.questionWizardServersFile, Question);
    if (logObj->getYN()) {
      pause(500);
      wizardStep(serversFile, installer.installDefaultServersFile, text.errorServersFilePresent, text.errorServersFileNotCreated, servers);
    }
    pause(500);
    logObj->out("--------------------");
    logObj->out(text.infoWizardStartupService, Info);
    logObj->out(text.questionWizardStartupService, Question);
    if (logObj->getYN()) {
      pause(500);
      installer.installStartupService(sysdService);
    }
    pause(500);
    logObj->out("--------------------");
    logObj->out(text.infoWizardComplete, Info);
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
