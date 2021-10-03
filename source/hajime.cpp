// (c) 2021 Slackadays on GitHub
#include <iostream>
#include <cstring>
#include <string>
#include <filesystem>
#include <fstream>
#include <memory>

namespace fs = std::filesystem;

#include "getyn.h"
#include "server.h"
#include "output.h"
#include "installer.h"
#include "getvarsfromfile.h"

using std::cin;
using std::cout;
using std::endl;
using std::shared_ptr;
using std::make_shared;
using std::vector;

string defaultServerConfFile = "server.conf";
string hajDefaultConfFile = "hajime.conf";
string sysdService = "/etc/systemd/system/hajime.service"; //systemd service file location
string logFile;

vector<string> hajimeConfParams{"defaultserverconf", "logfile", "systemdlocation"};

bool readSettings(vector<string> settings);

shared_ptr<Output> logObj = make_shared<Output>(); // make this pointer global

int main(int argc, char *argv[]) {
	Installer installer;
	for (int i = 1; i < argc; i++) { //search for the help flag first
		auto flag = [&i, &argv](auto ...fs){return (!strcmp(fs, argv[i]) || ...);}; //compare flags with a parameter pack pattern
                if (flag("-h", "--help")) { //-h = --help = help
                        logObj->out("Hajime is a high-performance startup script that can start a Minecraft server from an external device.");
                        logObj->out("\033[1m\033[32mUsage:\033[1;0m " + (string)argv[0] + " [the following flags]");
                        logObj->out("\033[1m-f \033[3mfile\033[0m or \033[1m--server-file \033[3mfile \033[0m\033[1;1m|\033[1;0m  Specify a server configuration file to use manually.");
                        logObj->out("\033[1m-h \033[0mor\033[1m --help |\033[1;0m  Show this help message.");
			logObj->out("\033[1m--hajime-file\033[0m \033[1m\033[3mfile \033[0m \033[1m|\033[0m Manually specify the configuration file that Hajime uses.");
                        logObj->out("\033[1m-s  \033[0mor\033[1m --install-server \033[1m|\033[0m  Create a default server configuration file.");
                        logObj->out("\033[1m-S  \033[0mor\033[1m --systemd \033[1;1m|\033[1;0m  Install a systemd service file to start Hajime automatically.");
                        logObj->out("\033[1;1m\033[1;32mNotes:\033[1;0m\nUse -f in conjunction with a custom config file. A plain filename is treated as being in the same directory Hajime is located in, so use a \033[1m/\033[0m to specify otherwise.", "none", 1);
                        return 0;
                }

	}
	if (!readSettings(hajimeConfParams)) {
		logObj->out("Default Hajime config file not found", "error");
		logObj->out("Would you like to make it now?", "info", 0, 0);
		if (getYN()) {
			installer.installDefaultHajConfFile(hajDefaultConfFile);
		}
	}
	for (int i = 1; i < argc; i++)  {//start at i = 1 to improve performance because we will never find a flag at 0
		auto flag = [&i, &argv](auto ...fs){return (!strcmp(fs, argv[i]) || ...);};
		auto assignNextToVar = [&argc, &argv, &i](auto &var){if (i == (argc - 1)) {logObj->out("Not enough arguments provided", "error"); return false;} else {var = argv[(i + 1)]; i++; return true;}};
                if (flag("-f", "--server-file")) {
                       	if (!assignNextToVar(defaultServerConfFile)) {
				return 0;
			}
                }
		if (flag("--hajime-file")) {
			if (!assignNextToVar(hajDefaultConfFile)) {
				return 0;
			}
		}
		if (flag("-i", "--install-hajime")) {
			installer.installDefaultHajConfFile(hajDefaultConfFile);
			return 0;
		}
		if (flag("-s", "--install-server")) {
			installer.installDefaultServerConfFile(defaultServerConfFile);
			return 0;
		}
		if (flag("-S", "--install-systemd")) {
			installer.installSystemdService(sysdService);
			return 0;
		}
	}
 	if (fs::is_regular_file(hajDefaultConfFile)) {
                readSettings(hajimeConfParams);
                if (logFile == "") {
                        logObj->out("No log file to be made; sending messages to console.", "info");
                } else {
                        logObj->init(logFile);
                }
        	} else {
                logObj->out("Config file doesn't exist!", "error");
		logObj->out("Looks like there isn't a Hajime configuation file. Would you like to make one?", "info", 0, 0);
                	if (getYN()) {
                	installer.installDefaultHajConfFile(hajDefaultConfFile);
                }
        }
	Server serverOne(logObj);
	serverOne.startServer(defaultServerConfFile);
	return 0;
}

bool readSettings(vector<string> settings) {
	if (!fs::is_regular_file(hajDefaultConfFile)) {
		return 0;
	}
	vector<string> results = getVarsFromFile(hajDefaultConfFile, settings);
	for (vector<string>::iterator firstSetIterator = settings.begin(), secondSetIterator = results.begin(); firstSetIterator != settings.end(); ++firstSetIterator, ++secondSetIterator) {
		auto setVar = [&](string name, string& tempVar){if (*firstSetIterator == name) {tempVar = *secondSetIterator;}};
		setVar(settings[0], defaultServerConfFile);
		setVar(settings[1], logFile);
		setVar(settings[2], sysdService);
	}
	return 1;
}

