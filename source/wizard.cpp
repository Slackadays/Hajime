#include <iostream>
#include <filesystem>

#include "output.hpp"
#include "installer.hpp"

namespace fs = std::filesystem;

void initialHajimeSetup(string confFile, string serversFile, string serverFile, string sysdService) {
    bool installedS = false;
    logObj->out(text.infoWizardHajimeFile, Info);
    logObj->out(text.questionMakeHajimeConfig, Question, 0, 0);
    if (logObj->getYN()) {
      installer.installDefaultHajConfFile(confFile);
    }
    logObj->out(text.infoWizardServersFile, Info);
    logObj->out(text.questionWizardServersFile, Question);
    if (logObj->getYN()) {
      installer.installDefaultServersFile(serversFile);
    }
    logObj->out(text.infoWizardServerFile, Info);
    logObj->out(text.questionWizardServerFile, Question);
    if (logObj->getYN()) {
      if (installer.installDefaultServerConfFile(serverFile)) {
      	installedS = true;
      }
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
