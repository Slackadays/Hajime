//Hajime version 1.0.1 R2
//(c) 2020 Slackadays on Github

#include <iostream>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include "server.h"
#include "output.h"

using std::cout;
using std::endl;
using std::shared_ptr;
using std::make_shared;
namespace fs = std::filesystem;

string confFile; // = "server.conf";
string sconfFile = "hajime.conf";
string sysdService = ""; // = "/etc/systemd/system/hajime.service"; //systemd service file location
string logFile;

void makeConfig();
void makeSysd();
void readSettings();

//argNum is the number of arguments from the command line, *args[] is the arguments themselves
int main(int argNum, char *args[]) {
	
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
	
	while (i < argNum) {
		
		
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
			
			makeConfig();
			return 0;
			
		}
		
		if (strcmp(args[i], "-S") == 0) { //-S = systemd install
			
			makeSysd();
			return 0;
			
		}
		
		i++;
	}
	
	Server one;
	one.startServer(confFile, logObj);
	return 0;
	
}

void makeConfig() {
	
	cout << "Installing config file..." << endl;
	
	if (fs::is_regular_file(confFile) == true){
		
		cout << "The file is already here! To make a new one, delete the existing file." << endl;
		
	} else {
		
		ofstream outConf(confFile);
		outConf << "file=SERVER-FILE"<< endl << "path=PATH-TO-DEVICE" << endl << "command=SERVER-EXECUTION-COMMAND" << endl << "debug=1" << endl << "device=DEVICE" << endl;
		outConf << "#" << endl << "This is the comment section. Anything after the # is a comment. \n The first line is the file of the server that needs to be executed. The second line is the path that leads to the home directory of the file nd can't end in a /. The third line is the command that needs to be executed in order to start the server. The fourth line is the debug setting. 0 means most output is disabled. 1 prevents most looped outputs. 2 enables all outputs. I recommend 1, but switch to 2 if there\'s a problem somewhere." << endl;
		cout << "The config file (" << confFile << ") has been created and is now ready for your settings." << endl;
		outConf.close();
		
	}
	
	if (fs::is_regular_file(sconfFile)) {
		cout << "There is a config file here!" << endl;
		
	} else {
		
		ofstream outsConf(sconfFile);
		outsConf <<	"defaultserverconf=server.conf" 
		<< endl << "systemdlocation=/etc/systemd/system/hajime.service"
		<< endl << "logfile="
		<< endl << "#"
		<< endl;
		cout << "Config file made!" << endl;
		outsConf.close();
		
	}
}

void readSettings() {
	

	std::fstream sconf; 	//conjure up a file stream, sconf = settings conf

	sconf.open(sconfFile, std::fstream::in); 	//configuration file open for reading

	int iter = 0;
	int lineNum = 0;
	string var[4], param[4], line;
    string finished = "";
	//checks if there's stuff left to read
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
		//prep var[] for the next line
		lineNum++;
	}
	

	sconf.close(); 	//get rid of the file in memory
}

void makeSysd() {
	if (fs::is_regular_file(sconfFile) == true && sysdService == "") {
		readSettings();
	}
	if (fs::is_regular_file(sconfFile) == false) {
		cout << "Sorry! Please run Hajime with the -I flag to make a config file first." << endl;
		return;
	}
	if (fs::is_directory("/etc/systemd") == true && fs::is_regular_file(sysdService) == true) {
		cout << "The systemd service is already here!" << endl;
	}
	if (fs::is_directory("/etc/systemd") == true && fs::is_regular_file(sysdService) == false) {
		cout << "Making systemd service..." << endl;
		ofstream service(sysdService);
		service << "[Unit]" << endl << "Description=Starts Hajime" << endl << endl << "[Service]\nType=simple\nWorkingDirectory=" << fs::current_path().string() << "\nExecStart=" << fs::current_path().string()  << "/hajime\n\n[Install]\nWantedBy=multi-user.target";
		service.close();
	}
	if (fs::is_directory("/etc/systemd") == false) {
		cout << "Looks like you don't have systemd." << endl;
	}
}
// compile command
// sudo g++ -std=c++20 -o hajime hajime.cpp
