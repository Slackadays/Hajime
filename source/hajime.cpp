//(c) 2021 Slackadays on Github

#include <iostream>
#include <cstring>
#include <string>
#include <filesystem>
#include <fstream>
#include <memory>

namespace fs = std::filesystem;

#include "server.h"
#include "output.h"
#include "installer.h"

using std::cout;
using std::endl;
using std::shared_ptr;
using std::make_shared;

string defaultServerConfFile = ""; // = "server.conf";
string sconfFile = "hajime.conf";
string sysdService = ""; // = "/etc/systemd/system/hajime.service"; //systemd service file location
string logFile;

void readSettings();

int main(int argn, char *args[]) {
	
	Installer install;
	
	shared_ptr<Output> logObj = make_shared<Output>(); //smart pointer to the file output object
	
	int i = 0;
	while (i < argn) {
		
		if (!strcmp(args[i], "-f")) { //allow the user to choose a file preceded by -f, strcmp() compares a C pointer and a primitive type
			
			defaultServerConfFile = args[(i + 1)];
			
		}
		
		if (!strcmp(args[i], "-h") || !strcmp(args[i], "--help")) { //-h = --help = help
			cout << "Hajime is a high-performance startup script designed to start a Minecraft server from an external device. \n\e[1;32mUsage:\e[1;0m " << 
			args[0] << " [any of the following flags]\n-f configuration-file  \e[1;1m|\e[1;0m  Specify a server configuration file to use manually.\n-h  \e[1;1m|\e[1;0m  Show this help message.\n-I  \e[1;1m|\e[1;0m  Create a default server configuration file.\n-S  \e[1;1m|\e[1;0m  Install a systemd service file to start Hajime automatically.\n" <<
			"\e[1;32mNotes:\e[1;0m\n-f is used in conjunction with a custom config file. A plain filename is interpreted as the same directory the script is located in, so use a / to specify otherwise." << endl;
			return 0;
		}
		
		if (!strcmp(args[i], "-I")) { //-I = install
			install.mainconfig(defaultServerConfFile);
			return 0;
		}
		
		if (!strcmp(args[i], "-S")) { //-S = systemd install
			if (!fs::is_regular_file(sconfFile)) {
				cout << "Looks like there isn't a Hajime configuation file. Would you like to make one? [y/n]";
				string response;
				std::cin >> response;
			if (response == "y"){cout << "Testing" << endl;}
		}
			if (fs::is_regular_file(sconfFile) && sysdService == "") {
				readSettings();
				install.systemd(sysdService);
		}
	}
		i++;
	}
 	if (fs::is_regular_file(sconfFile)) {

                readSettings();

                if (logFile == "") {

                        cout << "\e[1;46m[Info]\e[1;0mNo log file to be made; sending messages to console." << endl;

                } else {

                        logObj->init(logFile);

                }

        	} else {

                cout << "\e[1;41m\e[1;33m[Error]\e[1;0m Config file doesn't exist!" << endl;

        }
	Server one;
	one.startServer(defaultServerConfFile, logObj);
	return 0;
}

void readSettings() {

	std::fstream sconf; 	//conjure up a file stream, sconf = settings conf

	sconf.open(sconfFile, std::fstream::in); 	//configuration file open for reading

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


// compile command
// sudo g++ -std=c++20 -o hajime hajime.cpp
