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

using std::cin;
using std::cout;
using std::endl;
using std::shared_ptr;
using std::make_shared;

string defaultServerConfFile = "server.conf";
string hajDefaultConfFile = "hajime.conf";
string sysdService = "/etc/systemd/system/hajime.service"; //systemd service file location
string logFile;

void readSettings();
bool getYN();
 
int main(int argn, char *args[]) {
	Installer installer;
	shared_ptr<Output> logObj = make_shared<Output>(); //smart pointer to the file output object
	for (int i = 0; i < argn; i++) {
		auto flag = [&i, &args](string f1, string f2 = "#"){return (f1 == args[i]) || (f2 == args[i]);}; // # is purely a dummy variable so flag() can have 1 or 2 parameters
		if (flag("-f", "--server-file")) {
			defaultServerConfFile = args[(i + 1)];
		}
		if (flag("-h", "--help")) { //-h = --help = help
			logObj->out("Hajime is a high-performance startup script that can start a Minecraft server from an external device.");
			logObj->out("\e[1;1m\e[1;32mUsage:\e[1;0m " + (string)args[0] + " [the following flags]");
			logObj->out("-f configuration-file \e[3mor\e[0m --server-file configuration-file \e[1;1m|\e[1;0m  Specify a server configuration file to use manually.");
			logObj->out("-h  \e[3mor\e[0m --help \e[1;1m|\e[1;0m  Show this help message.");
			logObj->out("-I  \e[3mor\e[0m --install \e[1;1m|\e[1;0m  Create a default server configuration file.");
			logObj->out("-S  \e[3mor\e[0m --systemd \e[1;1m|\e[1;0m  Install a systemd service file to start Hajime automatically.");
			logObj->out("\e[1;1m\e[1;32mNotes:\e[1;0m\nUse -f in conjunction with a custom config file. A plain filename is treated as being in the same directory Hajime is located in, so use a \e[1m/\e[0m to specify otherwise.", "none", 1);
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
				readSettings();
				installer.systemd(sysdService);
				return 0;
			}
		}
	}
 	if (fs::is_regular_file(hajDefaultConfFile)) {
                readSettings();
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
	Server serverOne;
	serverOne.startServer(defaultServerConfFile, logObj);
	return 0;
}

void readSettings() {
	std::fstream sconf; 	//conjure up a file stream, sconf = settings conf
	sconf.open(hajDefaultConfFile, std::fstream::in); 	//configuration file open for reading
	int iter = 0;
	int lineNum = 0;
	string var[4], param[4], line;
	string finished = "";
	while (sconf.good() && lineNum < 3) { //this value is higher than the number of lines = segmentation fault!
		getline(sconf, line); //get a line and save it to line
		if (line == ""){
			throw "Whoops! The config file doesn't have anything in it.";
		}
		//if we've reachd the end of the config section (#) then get out of the loop!
		if (line[iter] == '#') {
			break;
		}
		param[lineNum] = "";
		//single quotes mean a char, and escape the double quote with a backslash
		while (line[iter] != '=') { //skips past anything that isn't in a quote
			param[lineNum] = param[lineNum] + line[iter];
			iter++;
		}
		iter++; //the current position is that of a quote, so increment it 1

		while ((uint)iter < line.length()) {		//cast to a uint to prevent a warning
			finished = finished + line[iter]; 		//append the finished product
			iter++;
		}
		var[lineNum] = finished; 	//make the var[] what the finished product is
		iter = 0; 	//reset for the next loop 
		finished = "";
		if (param[lineNum] == "defaultserverconf") {defaultServerConfFile = var[lineNum];}
		if (param[lineNum] == "logfile") {logFile = var[lineNum];}
		if (param[lineNum] == "systemdlocation") {sysdService = var[lineNum];}
		lineNum++; 		//prep var[] for the next line
	}
	sconf.close(); 	//get rid of the file in memory
}

