#include <iostream>
#include <filesystem>
#include <optional>

#include "output.hpp"
#include "installer.hpp"

namespace fs = std::filesystem;

template<typename Fn>
void wizardStep(string filename, Fn func, string foundFile, string fileNotMade) {
  for (bool skipFileCheck = false; true;) {
    try {
      func(filename, skipFileCheck);
      break;
    }
    catch (int i) {
      if (i == 0) {
        logObj->out(foundFile, Warning);
        logObj->out(text.questionInstallNewOne, Question);
        if (logObj->getYN()) {
          skipFileCheck = true;
        } else {
          break;
        }
      } else if (i == 1) {
        logObj->out(fileNotMade, Error);
        logObj->out(text.questionInstallNewOneAgain, Question);
        if (!logObj->getYN()) {
          break;
        }
      }
    }
  }
}

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
    logObj->out(text.infoWizardServersFile, Info);
    logObj->out(text.questionWizardServersFile, Question);
    if (logObj->getYN()) {
      pause(500);
      wizardStep(serversFile, installer.installDefaultServersFile, text.errorServersFilePresent, text.errorServersFileNotCreated);
    }
    pause(500);
    logObj->out("--------------------");
    logObj->out(text.infoWizardServerFile, Info);
    logObj->out(text.questionWizardServerFile, Question);
    if (logObj->getYN()) {
      while (true) {
        pause(500);
        wizardStep(serverFile, installer.installDefaultServerConfFile, "Found an existing server file with name " + serverFile, "Server config file not created");
        logObj->out("Do you want to create another server file?", Question);
        if (logObj->getYN()) {
          logObj->out("Enter a new name for the next server file (the previous one was " + std::regex_replace(serverFile, std::regex("\\.conf(?!\\w)", std::regex_constants::optimize), "") + "): \033[1m", Info, 0, 0);
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
	    logObj->out(text.infoWizardNextStepServerFile1 + serverFile + text.infoWizardNextStepServerFile2, Info);
    }
}
