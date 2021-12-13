module Hajime:Wizard;

#include <iostream>
#include <filesystem>
#include <optional>

import Output;
import Installer;

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
    bool installedS = false;
    logObj->out(text.infoWizardHajimeFile, Info);
    logObj->out(text.questionMakeHajimeConfig, Question, 0, 0);
    if (logObj->getYN()) {
      wizardStep(confFile, installer.installDefaultHajConfFile, text.warningFoundHajConf, "Hajime config file not created");
    }
    logObj->out(text.infoWizardServersFile, Info);
    logObj->out(text.questionWizardServersFile, Question);
    if (logObj->getYN()) {
      wizardStep(serversFile, installer.installDefaultServersFile, "Found an existing servers file", "Servers file not created");
    }
    logObj->out(text.infoWizardServerFile, Info);
    logObj->out(text.questionWizardServerFile, Question);
    if (logObj->getYN()) {
      wizardStep(serverFile, installer.installDefaultServerConfFile, "Found an existing server file with name " + serverFile, "Server config file not created");
      installedS = true;
    }
    logObj->out(text.infoWizardStartupService, Info);
    logObj->out(text.questionWizardStartupService, Question);
    if (logObj->getYN()) {
      installer.installStartupService(sysdService);
    }
    logObj->out(text.infoWizardComplete, Info);
    if (installedS) {
	    logObj->out(text.infoWizardNextStepServerFile1 + serverFile + text.infoWizardNextStepServerFile2, Info);
    }
}
