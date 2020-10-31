//Hajime class and function files

#include <filesystem>
#include <stdlib.h>
#include <fstream>
#include <sys/mount.h>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <string>
#include "output.h"

#ifdef _WIN32
	#include <Windows.h> //Windows library
#else
	#include <unistd.h> //Linux version of the library
#endif

using namespace std;
namespace fs = std::filesystem;

class Server {
	bool hasOutput = false;
	bool hasOutputUSB = false; //if one variable is used, then the debug action will interfere wihh the USB mount function
	bool hasMounted = false;
	int debug = 2; //set to 0 to get rid of most messages, 1 for 1 message per action, 2 for all messages available
	int systemi = 0;
	int maxPID = 0;

	const string systems[7] = {"ext2", "ext3", "ext4", "vfat", "msdos", "f2fs", "fuseblk"};
	
	void mountDrive();
	void makeDir();
	void startProgram();
	void readSettings(string confFile);
	void printMessage(string message, int = 1);

	string file, path, command, confFile, device;
	shared_ptr<Output> fileObj;
	
	public:
		Server();
		bool isRunning = false;
		void startServer(string confFile, shared_ptr<Output> fileObj);
		int getPID();
};

Server::Server() {
string oldPath = fs::current_path();
int i = 0; //start from beginning
fs::current_path("/proc"); //proc holds all the processes
while (i < 20000) { //20000 is the highest likely process PID
	if (fs::exists(to_string(i))){ //i needs to be a string to use with the fs functions
			maxPID = i;
	}
	i++; //go up 1 PID
}
fs::current_path(oldPath);
}

void Server::startServer(string confFile, std::shared_ptr<Output> fileObj) {
	try {
		if (fs::is_regular_file(confFile)) {
			fileObj->out("Reading settings...");
			readSettings(confFile);
		} else {
			fileObj->out("The server's config file doesn't exist");
			return;
		}
		
		switch(debug) {
			
		case 0:
			break;
			
		case 1:
			fileObj->out("The file is: " + file);
			fileObj->out("The path is: " + path);
			fileObj->out("Command: " + command);
			fileObj->out("Debug value: " + debug);
			fileObj->out("Device: " + device);
			break;
			
		case 2: 
			fileObj->out("The file is: " + file);
			fileObj->out("The path is: " + path);
			fileObj->out("Command: " + command);
			fileObj->out("Debug value: " + debug);
			fileObj->out("Device: " + device);
			break;
			
		}
		
		while(true) {
			if (getPID() != 0) { //getPID looks for a particular keyword in /proc/PID/cmdline that signals the presence of a server
				sleep(3);
				fileObj->out("Program is running!");
				isRunning = true;
				hasMounted = true;
				} else {
					isRunning = false;
				}
				
			fs::current_path(path);

			//checks if we're in the right place and if the server file is there
			if (fs::current_path() == path && fs::is_regular_file(file) && isRunning == false) {
				startProgram();
			}

			sleep(2);
			
			//if the desired path doesn't exist, make it
			if (!fs::is_directory(path)) {
				makeDir();
			}
			
			fs::current_path(path);
			
			if (!hasMounted) {
				mountDrive();
			}
		}
	} catch(string mes){
		printMessage(mes, 1);
	} catch(...) { //error handling
		fileObj->out("Whoops! An error occurred.");
	}
}

void Server::startProgram() {
	if (!isRunning) {
		
		printMessage("Starting program!", 1);
		
		fs::current_path(path);

		//session.lock will be there if the server didn't shut down properly
		fs::remove("world/session.lock");

		//system() is a C command too
		system(command.c_str()); //execute the command
		
		sleep(1);
		
		if (getPID() != 0) { //check for the PID of the program we just started
			isRunning = true; //isRunning disables a lot of checks
			hasMounted = true;
		}
	}
}

void Server::makeDir() {
	printMessage("No directory!", 2);
	if (!fs::create_directory(path)) {
		fileObj->out("Error creating directory!");
	}
}

void Server::mountDrive() {
	fileObj->out("Trying to mount.");
	//sda1 is the first external mass storage device
	if (!fs::is_empty(path)) { //if there are files, then we don't want to mount there
			fileObj->out("There are files in the path");
			return;
	} else {
		string error;
	if (mount(device.c_str(), path.c_str(), systems[systemi].c_str(), 0, "") == 0) { //brute-forces every possible filesystem because mount() depends on it being the right one
		fileObj->out("Device mounted!");
		hasMounted = true;
		systemi = 0; //reset in case it needs to mount again
	} else {
		int errsv = errno; //errno is the POSIX error code
		if (systemi == 6) {
			switch (errsv) {
			case 1 : error = "Not permitted. Is the device correct?"; break;
			case 2 : error = "No such file or directory."; break;
			case 13: error = "Permission denied. Is Hajime being run under root?"; break;
			case 5 : error = "Input/output error. Is the drive OK?"; break;
			case 12: error = "Not enough memory. Is there a shortage of it?"; break;
			case 11: error = "Resource unavailable."; break;
			case 14: error = "Bad address."; break;
			case 15: error = "Not a block device. Did you make sure you're mounting a mass storage device?"; break;
			case 16: error = "Busy. Is the device being accessed right now?"; break;
			case 21: error = "It's a directory. Did you make sure you're mounting a mass stoage device?"; break;
			case 22: error = "Bad arguments. Is the configuration set correctly?"; break;
			case 19: error = "Unknown device. Is the filesystem supported?"; break;
			default: error = "Unknown error.";
			}
			if (hasOutputUSB == false){
				fileObj->out("An error occurred, but the script will keep trying to mount. Error: " + error);
				hasOutputUSB = true;
				systemi = 0;
			}
			fileObj->out("Error code: " + to_string(errsv));
			}
		}
		if (systemi < 6) {
			fileObj->out("Trying " + systems[systemi] + " filesystem");
			systemi++; //increment the filesystem
		}
	}
}

void Server::readSettings(string confFile) {
	
	//conjure up a file stream
	std::fstream conf;

	//configuration file open for reading
	conf.open(confFile, std::fstream::in);

	int iter = 0;
	int lineNum = 0;
	string var[6], param[6], line;
    string finished = "";
	//checks if there's stuff left to read
	while (conf.good() && lineNum < 6) { //linenum < 6 because otherwise, we get a segmentation fault
		getline(conf, line); //get a line and save it to line
		
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
		//the current position is that of a quote, so increment it 1
		iter++;
		//append the finished product
		while ((uint)iter < line.length()) {
			finished = finished + line[iter];
			iter++;
		}

		//make the var[] what the finished product is
		var[lineNum] = finished;
		//reset for the next loop
		iter = 0;
		finished = "";
		if (param[lineNum] == "File") {file = var[lineNum];}
		if (param[lineNum] == "Path") {path = var[lineNum];}
		if (param[lineNum] == "Command") {command = var[lineNum];}
		//stoi() converts the string result into an int
		if (param[lineNum] == "Debug") {debug = stoi(var[lineNum]);}
		if (param[lineNum] == "Device") {device = var[lineNum];}
		//prep var[] for the next line
		lineNum++;
	}
	
	if (device == "") {
		fileObj->out("No device requested: No mounting this time!");
		hasMounted = true;
	}

	//get rid of the file in memory
	conf.close();
}

int Server::getPID() {
string oldPath = fs::current_path();
int i = maxPID; //start from beginning
fs::current_path("/proc"); //proc holds all the processes
while (i < 20000) { //20000 is the highest likely process PID
	if (fs::exists(to_string(i))){ //i needs to be a string to use with the fs functions
			fstream file; //create a file object
			file.open(to_string(i) + "/cmdline", ios::in); //open the file of /proc/PID/cmdline for reading
			string str = ""; //reset string
			getline(file, str); //read cmdline (it is only 1 line)
			if (str.length() > 0){ //if a cmdline is not used, there will be nothing
				if (str.find("SCREEN") != string::npos){ //look for a keyword in cmdline, string::npos is a special value (-1) that needs to be used
					file.close(); //erase from memory
					fs::current_path(oldPath);
					return i; //return the PID of the known good process
				
				}
			}
			file.close(); //erase the file from memory
	}
	i++; //go up 1 PID
}
fs::current_path(oldPath);
return 0; //0 is the signal for "doesn't exist"
}

void Server::printMessage(string message, int v) { //v = verbosity
	switch (debug) {
		case 0: 
			break;
		case 1:
			if (v == 1) {cout << message << endl; break;} 
			if (v == 2) {break;}
		case 2:
			if (v == 1 || v == 2) {cout << message << endl; break;}
		default: //if debug isn't set, we don't want to miss anything
			cout << message << endl;
	}
}


