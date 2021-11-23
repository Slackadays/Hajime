#include <iostream>
#include <filesystem>

#include "output.h"
#include "installer.h"

namespace fs = std::filesystem;

void initialHajimeSetup(string confFile, string serversFile, string serverFile, string sysdService) {
    bool installedS = false;
    logObj->out("Let's start with creating the file that Hajime will use for its basic settings.", Info);
    logObj->out(text.questionMakeHajimeConfig, Question, 0, 0);
    if (logObj->getYN()) {
      installer.installDefaultHajConfFile(confFile);
    }
    logObj->out("Next, we'll need to install the \"servers file,\" or where Hajime looks for servers to start up.", Info);
    logObj->out("Do you want to install it now?", Question);
    if (logObj->getYN()) {
      installer.installDefaultServersFile(serversFile);
    }
    logObj->out("Now we need a server file to define one of your servers to run.", Info);
    logObj->out("Do you want to make a server file now?", Question);
    if (logObj->getYN()) {
      if (installer.installDefaultServerConfFile(serverFile)) {
      	installedS = true;
      }
    }
    logObj->out("Finally, we need to make Hajime start upon the host booting.", Info);
    logObj->out("Do you want to install a startup service?", Question);
    if (logObj->getYN()) {
      installer.installStartupService(sysdService);
    }
    logObj->out("Setup complete!", Info);
    if (installedS) {
	    logObj->out("Next Steps: Enter your server settings in " + serverFile + ".", Info);
    }
}
