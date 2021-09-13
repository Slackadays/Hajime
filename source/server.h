#include <filesystem>
#include <stdlib.h>
#include <fstream>
#include <sys/mount.h>
#include <cstring>
#include <iostream>
#include <string>
#include <errno.h>
#include <vector>

#if defined(_win64) || defined (_WIN32)
#else
#include <unistd.h>
#endif

#include "output.h"

using std::shared_ptr;
using std::string;
using std::fstream;
using std::to_string;
using std::ofstream;
using std::ios;

class Server {
	bool hasOutput, hasOutputUSB, hasMounted = false;
	
	int systemi = 0;

	std::error_code ec;

	shared_ptr<Output> logObj;

	const string systems[8] = {"ext2", "ext3", "ext4", "vfat", "msdos", "f2fs", "ntfs", "fuseblk"};
	
	void mountDrive();
	void makeDir();
	void startProgram();
	void readSettings(string confFile);

	string file, path, command, confFile, device = "";
	
	public:
		Server(shared_ptr<Output> tempObj);
		bool isRunning = false;
		void startServer(string confFile);
		int getPID();
};

Server::Server(shared_ptr<Output> tempObj) {
	logObj = tempObj;
}

void Server::startServer(string confFile) {
	try {
		if (fs::is_regular_file(confFile, ec)) {
			logObj->out("Reading server settings...", "info");
			readSettings(confFile);
		} else {
			logObj->out("The server's config file doesn't exist", "error");
			return;
		}
			cout << "The file is: " << file << endl;
			logObj->out("The path is: " + path);
			logObj->out("Command: " + command);
			//logObj->out("Debug value: " + to_string(debug)); // ->out wants a string so we convert the debug int (converted from a string) back to a string
			logObj->out("Device: " + device);
		while(true) {
			if (getPID() != 0) { //getPID looks for a particular keyword in /proc/PID/cmdline that signals the presence of a server
				sleep(3);
				logObj->out("Program is running!", "info");
				isRunning = true;
				hasMounted = true;
			} else {
				isRunning = false;
				logObj->out("isRunning is now false", "warning");
			}
			try {
				fs::current_path(path);
			} catch(...) {
				logObj->out("Couldn't set the path.", "error");
			}
			if (fs::current_path() == path && fs::is_regular_file(file) && !isRunning) { //checks if we're in the right place and if the server file is there
				logObj->out("Trying to start program", "info");
				startProgram();
				logObj->out("Program start completed", "info");
			}
			sleep(2);
			if (!fs::is_directory(path, ec)) { //if the desired path doesn't exist, make it
				makeDir();
			}
			fs::current_path(path, ec);
			if (!hasMounted) {
				mountDrive();
			}
		}
	} catch(...) { //error handling
		logObj->out("Whoops! An unknown error occurred.", "error");
	}
}

void Server::startProgram() {
	if (!isRunning) {
		logObj->out("Starting server!", "info");
		fs::current_path(path);
		fs::remove("world/session.lock"); //session.lock will be there if the server didn't shut down properly
		system(command.c_str()); //convert the command to a c-style string, execute the command
		sleep(1);
		if (getPID() != 0) { //check for the PID of the program we just started
			isRunning = true; //isRunning disables a lot of checks
			hasMounted = true;
		}
	}
}

void Server::makeDir() {
	logObj->out("No directory!");
	if (!fs::create_directory(path, ec)) {
		logObj->out("Error creating directory!", "error");
	}
}

void Server::mountDrive() {
	logObj->out("Trying to mount", "info");
	if (!fs::is_empty(path, ec)) { //if there are files, then we don't want to mount there
		logObj->out("There are files in the path", "error");
		return;
	} else {
		string error;
		if (mount(device.c_str(), path.c_str(), systems[systemi].c_str(), 0, "") == 0) { //brute-forces every possible filesystem because mount() depends on it being the right one
			logObj->out("Device mounted!", "info");
			hasMounted = true;
			systemi = 0; //reset in case it needs to mount again
		} else {
			#if defined(_WIN64)
			#else
			int errsv = errno; //errno is the POSIX error code, save errno to a dummy variable to stop it from getting tainted
			#endif
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
				if (!hasOutputUSB){
					logObj->out("An error occurred, but the script will keep trying to mount. Error: " + error);
					hasOutputUSB = true;
					systemi = 0;
				}
				logObj->out("Error code: " + to_string(errsv), "error");
				}
			}
			if (systemi < 6) {
				logObj->out("Trying " + systems[systemi] + " filesystem", "info");
				systemi++; //increment the filesystem
			}
	}
}

void Server::readSettings(string confFile) {
	std::fstream conf; 	//conjure up a file stream
	conf.open(confFile, std::fstream::in); //configuration file open for reading
	string line = "";
	std::vector <string> param, var;
    	string finished = "";
	bool skipLine = false;
	//checks if there's stuff left to read
	for (int lineNum = 0, iter = 0; conf.good() && !conf.eof(); lineNum++) { //linenum < 6 because otherwise, we get a segmentation fault
		getline(conf, line); //get a line and save it to line
		if (line == ""){
			break;
		}
		//skip lines when you get to a # character
		if (line[iter] == '#') {
			skipLine = true;
		}
		//skips past anything that isn't in a quote
		//single quotes mean a char, and escape the double quote with a backslash
		if (line[iter] == '\n' || line[iter] == '\0') {
			skipLine = true;
		}
		if (!skipLine) {
			while (line[iter] != '=') {
				finished += line[iter];
				iter++;
				if (line[iter] == '#') {break;}
			}
			iter++;
			param.push_back(finished);
			finished = "";
			while ((uint)iter < line.length()) {
				if (line[iter] == '#') {break;}
				finished += line[iter]; //append the finished product
				iter++;
			}
			//make the var[] what the finished product is
			var.push_back(finished);
			finished = "";
			if (param[lineNum] == "file") {file = var[lineNum];}
			if (param[lineNum] == "path") {path = var[lineNum];}
			if (param[lineNum] == "command") {command = var[lineNum];}
			//if (param[lineNum] == "debug") {debug = stoi(var[lineNum]);} //stoi() converts the string result into an int
			if (param[lineNum] == "device") {device = var[lineNum];}
		}
		iter = 0;
	}
	if (device == "") {
		logObj->out("No device requested: No mounting this time!", "info");
		hasMounted = true;
	}
	conf.close(); //get rid of the file in memory
}

int Server::getPID() {
#if defined(_WIN64) || defined(_WIN32)
	cout << "Testing Windows support!" << endl;
#else
	fs::directory_iterator Directory("/proc/"); //search /proc/
	fs::directory_iterator End; //a dummy object to compare to
	for (string dir = ""; Directory != End; Directory++) { 
		dir = Directory->path(); //assigns a formatted directory string to dir
		fstream file; //create a file object
		file.open(dir + "/cmdline", ios::in); //open the file of /proc/PID/cmdline for reading
		string str = ""; //reset string
		getline(file, str); //read cmdline (it is only 1 line)
		if (str.length() > 0){ //if a cmdline is not used, there will be nothing
			if (str.find("SCREEN") != string::npos){ //look for a keyword in cmdline, string::npos is a special value (-1) that needs to be used
				file.close(); //erase from memory
				return stoi(dir.erase(0, 6)); 	//return the PID of the known good process
			}
		}
	file.close(); //erase the file from memory
	}
	return 0; //doesn't exist
#endif
}
