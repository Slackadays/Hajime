//Hajime version 1.0.1 R3
//(c) 2021 Slackadays on Github

#include <iostream>
#include <cstring>
#include <string>
#include <filesystem>
#include <fstream>
#include <memory>

#include "server.h"
#include "output.h"
#include "installer.h"

using std::cout;
using std::endl;
using std::shared_ptr;
using std::make_shared;

namespace fs = std::filesystem;

string confFile = ""; // = "server.conf";
string sconfFile = "hajime.conf";
string sysdService = ""; // = "/etc/systemd/system/hajime.service"; //systemd service file location
string logFile;

void readSettings();

int main(int argn, char *args[]) {
	
	Installer install;
	
	shared_ptr<Output> logObj = make_shared<Output>(); //smart pointer to the file output object
	
	if (fs::is_regular_file(sconfFile)) {
		
		readSettings();
		
		if (logFile == "") {
			
			cout << "No log file to be made!" << endl;
			
		} else {
			
			logObj->init(logFile);
			
		}

	} else {
		
		cout << "Config file doesn't exist!" << endl;
		
	}
	
	int i = 0;
	
	while (i < argn) {
		
		
		if (strcmp(args[i], "-f") == 0) { //allow the user to choose a file preceded by -f, strcmp() compares a C pointer and a primitive type
			
			confFile = args[(i + 1)];
			
		}
		
		if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0) { //-h = --help = help
			
			cout << "Hajime is a high-performance startup script designed to start a Minecraft server from an external device. Usage: \n" << 
			args[0] << " [-f configuration-file] [-h] [-I] [-S] \n" <<
			"-f is used in conjunction with a custom config file. A plain filename is interpreted as the same directory the script is located in, so use a / to specify otherwise." << endl;
			return 0;
			
		}
		
		if (strcmp(args[i], "-I") == 0) { //-I = install
			
			install.mainconfig(confFile);
			return 0;
			
		}
		
		if (strcmp(args[i], "-S") == 0) { //-S = systemd install
			if (fs::is_regular_file(sconfFile) == true && sysdService == "") {
		readSettings();
		install.systemd(sysdService);
		}
	}
		i++;
	}
	
	Server one;
	one.startServer(confFile, logObj);
	return 0;
	
}



void readSettings() {
	

	std::fstream sconf; 	//conjure up a file stream, sconf = settings conf

	sconf.open(sconfFile, std::fstream::in); 	//configuration file open for reading

	int iter = 0;
	int lineNum = 0;
	
	string var[4], param[4], line;
	string finished = "";
    
	while (sconf.good() && lineNum < 3) { //linenum < 6 because otherwise, we get a segmentation fault
		getline(sconf, line); //get a line and save it to line
		
		if (line == ""){
			throw "Whoops! The config file doesn't have anything in it.";
		}
		//if we've reachd the end of the config section (#) then get out of the loop!
		if (line[iter] == '#') {
					break;
		}
		
		param[lineNum] = "";
		//skips past anything that isn't in a quote
		//single quotes mean a char, and escape the double quote with a backslash
		while (line[iter] != '=') {
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
		if (param[lineNum] == "defaultserverconf") {confFile = var[lineNum];}
		if (param[lineNum] == "logfile") {logFile = var[lineNum];}
		if (param[lineNum] == "systemdlocation") {sysdService = var[lineNum];}
		lineNum++; 		//prep var[] for the next line
	}
	

	sconf.close(); 	//get rid of the file in memory
}


// compile command
// sudo g++ -std=c++20 -o hajime hajime.cpp
