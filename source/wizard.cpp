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
        logObj->out("Do you want to install a new one?", Question);
        if (logObj->getYN()) {
          skipFileCheck = true;
        } else {
          break;
        }
      } else if (i == 1) {
        logObj->out(fileNotMade, Error);
        logObj->out("Do you want to try creating one again?", Question);
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
      wizardStep(confFile, installer.installDefaultHajConfFile, text.warningFoundHajConf, "Hajime config file not created");
    }
    pause(500);
    logObj->out("--------------------");
    logObj->out(text.infoWizardServersFile, Info);
    logObj->out(text.questionWizardServersFile, Question);
    if (logObj->getYN()) {
      pause(500);
      wizardStep(serversFile, installer.installDefaultServersFile, "Found an existing servers file", "Servers file not created");
    }
    pause(500);
    logObj->out("--------------------");
    logObj->out(text.infoWizardServerFile, Info);
    logObj->out(text.questionWizardServerFile, Question);
    if (logObj->getYN()) {
      pause(500);
      wizardStep(serverFile, installer.installDefaultServerConfFile, "Found an existing server file with name " + serverFile, "Server config file not created");
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
