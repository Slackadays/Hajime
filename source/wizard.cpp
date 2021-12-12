#include <iostream>
#include <filesystem>

#include "output.hpp"
#include "installer.hpp"

namespace fs = std::filesystem;

std::string optFlags = "-XX:+UseG1GC -XX:+ParallelRefProcEnabled -XX:MaxGCPauseMillis=200 -XX:+UnlockExperimentalVMOptions -XX:+DisableExplicitGC -XX:+AlwaysPreTouch -XX:G1NewSizePercent=30 -XX:G1MaxNewSizePercent=40 -XX:G1HeapRegionSize=8M -XX:G1ReservePercent=20 -XX:G1HeapWastePercent=5 -XX:G1MixedGCCountTarget=4 -XX:InitiatingHeapOccupancyPercent=15 -XX:G1MixedGCLiveThresholdPercent=90 -XX:G1RSetUpdatingPauseTimePercent=5 -XX:SurvivorRatio=32 -XX:+PerfDisableSharedMem  XX:MaxTenuringThreshold=1 -Daikars.new.flags=true -Dusing.aikars.flags=https://mcflags.emc.gs";

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
      for (bool skipFileCheck, useFlags, askedForFlags = false; true;) {
        try {
          if (!askedForFlags) {
            logObj->out("Would you like to apply Aikar's Flags to the server?", Question);
            useFlags = logObj->getYN();
            askedForFlags = true;
          }
          useFlags ? installer.installDefaultServerConfFile(serverFile, skipFileCheck, optFlags) : installer.installDefaultServerConfFile(serverFile, skipFileCheck);
          installedS = true;
          break;
        }
        catch (int i) {
          if (i == 0) {
            logObj->out("Found an existing server file with name " + serverFile, Warning);
            logObj->out("Do you want to install a new one?", Question);
            if (logObj->getYN()) {
              skipFileCheck = true;
            } else {
              break;
            }
          } else if (i == 1) {
            logObj->out("Server config file not created", Error);
            logObj->out("Do you want to try creating one again?", Question);
            if (!logObj->getYN()) {
              break;
            }
          }
        }
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
