// (c) 2021 Slackadays on GitHub
// compile command
// sudo g++ -Ofast -std=c++17 -o hajime hajime.cpp -lstdc++fs
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

void readSettings(vector<string> settings);
bool getYN();

shared_ptr<Output> logObj = make_shared<Output>(); // make this pointer global
 
int main(int argn, char *args[]) {
	Installer installer;
	for (int i = 0; i < argn; i++) {
		auto flag = [&i, &args](auto ...fs){return (!strcmp(fs, args[i]) || ...);}; //compare flags with a parameter pack pattern
		if (flag("-f", "--server-file")) {
			defaultServerConfFile = args[(i + 1)];
		}
		if (flag("-h", "--help")) { //-h = --help = help
			logObj->out("Hajime is a high-performance startup script that can start a Minecraft server from an external device.");
			logObj->out("\033[1;1m\033[1;32mUsage:\033[1;0m " + (string)args[0] + " [the following flags]");
			logObj->out("-f configuration-file \033[3mor\033[0m --server-file configuration-file \033[1;1m|\033[1;0m  Specify a server configuration file to use manually.");
			logObj->out("-h  \033[3mor\033[0m --help \033[1;1m|\033[1;0m  Show this help message.");
			logObj->out("-I  \033[3mor\033[0m --install \033[1;1m|\033[1;0m  Create a default server configuration file.");
			logObj->out("-S  \033[3mor\033[0m --systemd \033[1;1m|\033[1;0m  Install a systemd service file to start Hajime automatically.");
			logObj->out("\033[1;1m\033[1;32mNotes:\033[1;0m\nUse -f in conjunction with a custom config file. A plain filename is treated as being in the same directory Hajime is located in, so use a \033[1m/\033[0m to specify otherwise.", "none", 1);
			return 0;
		}
		if (flag("-I", "--install")) { //-I , --install = install a default server configuration file
			installer.mainconfig(defaultServerConfFile);
			return 0;
		}
		if (flag("-S", "--systemd")) { //-S = systemd install
			if (!fs::is_regular_file(hajDefaultConfFile)) {
				cout << "Looks like there isn't a Hajime configuation file. Would you like to make one? [y/n] ";
				if (getYN()){
					installer.installDefaultHajConfFile(hajDefaultConfFile);
					return 0;
				}
			}
			if (fs::is_regular_file(hajDefaultConfFile) && sysdService == "") {
				readSettings(hajimeConfParams);
				installer.systemd(sysdService);
				return 0;
			}
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
		logObj->out("Looks like there isn't a Hajime configuation file. Would you like to make one? [y/n] ", "info", 0, 0);
                	if (getYN()) {
                	installer.installDefaultHajConfFile(hajDefaultConfFile);
                }
        }
	Server serverOne(logObj);
	serverOne.startServer(defaultServerConfFile);
	return 0;
}

void readSettings(vector<string> settings) {
	vector<string> results = getVarsFromFile(hajDefaultConfFile, settings);
	for (vector<string>::iterator firstSetIterator = settings.begin(), secondSetIterator = results.begin(); firstSetIterator != settings.end(); ++firstSetIterator, ++secondSetIterator) {
		auto setVar = [&](string name, string& tempVar){if (*firstSetIterator == name) {tempVar = *secondSetIterator;}};
		setVar(settings[0], defaultServerConfFile);
		setVar(settings[1], logFile);
		setVar(settings[2], sysdService);
	}
}

