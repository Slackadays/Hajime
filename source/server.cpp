#if defined(_win64) || defined (_WIN32)
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <cstring>
#include <string>
#include <errno.h>
#include <vector>
#include <chrono>
#include <filesystem>

#include "getvarsfromfile.hpp"
#include "server.hpp"

using std::shared_ptr;
using std::string;
using std::fstream;
using std::to_string;
using std::ofstream;
using std::ios;
using std::vector;
using std::cout;

namespace fs = std::filesystem;

Server::Server(shared_ptr<Output> tempObj) {
	logObj = tempObj;
}

void Server::startServer(string confFile) {
	try {
		if (fs::is_regular_file(confFile, ec)) {
			logObj->out(text.infoReadingServerSettings, Info);
			readSettings(confFile);
		} else {
			logObj->out(text.errorServerFileNotPresent1 + confFile + text.errorServerFileNotPresent2, Error);
			return;
		}
			logObj->out(text.infoServerFile + file, Info);
			logObj->out(text.infoServerPath + path, Info);
			logObj->out(text.infoServerCommand + command, Info);
			logObj->out(text.infoServerMethod + method, Info);
			logObj->out(text.infoServerDebug + to_string(debug), Info); // ->out wants a string so we convert the debug int (converted from a string) back to a string
			logObj->out(text.infoServerDevice + device, Info);
		while(true) {
			if (getPID() != 0) { //getPID looks for a particular keyword in /proc/PID/cmdline that signals the presence of a server
				std::this_thread::sleep_for(std::chrono::seconds(3));
				logObj->out(text.infoServerIsRunning, Info);
				isRunning = true;
				hasMounted = true;
			} else {
				isRunning = false;
				logObj->out(text.warningIsRunningFalse, Warning);
			}
			try {
				fs::current_path(path);
			} catch(...) {
				logObj->out(text.errorCouldntSetPath, Error);
			}
			if (fs::current_path() == path && fs::is_regular_file(file) && !isRunning) { //checks if we're in the right place and if the server file is there
				logObj->out(text.infoStartingServer, Info);
				startProgram(method);
				logObj->out(text.infoServerStartCompleted, Info);
			}
			std::this_thread::sleep_for(std::chrono::seconds(2));
			if (!fs::is_directory(path, ec)) { //if the desired path doesn't exist, make it
				makeDir();
			}
			fs::current_path(path, ec);
			if (!hasMounted) {
				mountDrive();
			}
		}
	} catch(...) { //error handling
		logObj->out(text.errorGeneric, Error);
	}
}

vector<string> Server::toArray(string input) {
	vector<string> flagVector;
	string temp = "";
	string execFile = path + '/' + file; //make an absolute executable path for the thing we're executing
	flagVector.push_back(execFile.c_str()); //convert the execFile string to a c-style string that the exec command will understand
	for (int i = 0; i < input.length(); temp = "") {
  	while (input[i] == ' ' && i < input.length()) { //skip any leading whitespace
    	i++;
    }
		while (input[i] != ' ' && i < input.length()) { //add characters to a temp variable that will go into the vector
			temp += input[i];
			i++;
		}
		while (input[i] == ' ' && i < input.length()) { //skip any trailing whitespace
			i++;
		}
		flagVector.push_back(temp); //add the finished flag to the vector of flags
		logObj->out(text.debugFlagVecInFor + flagVector[0], Debug);
	}
	logObj->out(text.debugFlagVecOutFor + flagVector[0], Debug);
	return flagVector;
}

auto Server::toPointerArray(vector<string> &strings) {
	vector<char*> pointers; //the pointer array that we will pass to the exec command
	for (auto &string : strings) { //loop over the whole flag string vector
		pointers.push_back(string.data()); //give the pointer array an address to a c-style string from the flag array
	}
	pointers.push_back(nullptr); //add a null pointer to the end because the exec command is from c
	return pointers;
}

void Server::startProgram(string method = "new") {
	if (!isRunning) {
		logObj->out(text.infoTryingToStartProgram, Info);
		fs::current_path(path);
		fs::remove("world/session.lock"); //session.lock will be there if the server didn't shut down properly
		if (method == "old") {
			logObj->out(text.debugUsingOldMethod, Debug);
			int returnVal = system(command.c_str()); //convert the command to a c-style string, execute the command
		} else if (method == "new") {
			logObj->out(text.debugUsingNewMethod, Debug);
			#if defined(_WIN64) || defined (_WIN32)
			ShellExecuteA(NULL, "open", file.c_str(), flags.c_str(), NULL, SW_NORMAL);
			#else
			logObj->out(text.debugFlags + flags, Debug);
			auto flagTemp = toArray(flags);
     			auto flagArray = toPointerArray(flagTemp);
			int pid = fork();
			if (pid == 0) {
				logObj->out(text.debugFlagArray0 + (string)flagArray[0], Debug);
				logObj->out(text.debugFlagArray1 + (string)flagArray[1], Debug);
				execv(file.c_str(), flagArray.data());
			} else {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (getPID() != 0) { //check for the PID of the program we just started
      		isRunning = true; //isRunning disables a lot of checks
          hasMounted = true;
		    }
			}
			#endif
		} else {
			logObj->out(text.errorMethodNotValid, Error);
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if (getPID() != 0) { //check for the PID of the program we just started
			isRunning = true; //isRunning disables a lot of checks
			hasMounted = true;
		}
	}
}

void Server::makeDir() {
	logObj->out(text.infoCreatingDirectory, Info);
	if (!fs::create_directory(path, ec)) {
		logObj->out(text.errorCreatingDirectory, Error);
	}
}

void Server::mountDrive() {
	#if defined(_WIN64) || defined(_WIN32) //Windows doesn't need drives to be mounted manually
	logObj->out(text.infoPOSIXdriveMount, Info);
	hasMounted = true;
	#else
	logObj->out(text.infoTryingMount, Info);
	if (!fs::is_empty(path, ec)) { //if there are files, then we don't want to mount there
		logObj->out(text.errorFilesInPath, Error);
		return;
	} else {
		string error;
		#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
		//BSDs have different mount() parameters
		if (!mount(systems[systemi].c_str(), path.c_str(), 0, const_cast<char*>(device.c_str()))) { //cast a c-style device string to a constant char*
		#else
		if (!mount(device.c_str(), path.c_str(), systems[systemi].c_str(), 0, "")) {
		//brute-forces every possible filesystem because mount() depends on it being the right one
		#endif
			logObj->out(text.infoDeviceMounted, Info);
			hasMounted = true;
			systemi = 0; //reset in case it needs to mount again
		} else {
			#if defined(_WIN64)
			#else
			int errsv = errno; //errno is the POSIX error code, save errno to a dummy variable to stop it from getting tainted
			#endif
			if (systemi == 6) {
				switch (errsv) {
				case 1 : error = text.errnoNotPermitted; break;
				case 2 : error = text.errnoNoFileOrDir; break;
				case 13: error = text.errnoPermissionDenied; break;
				case 5 : error = text.errnoInOut; break;
				case 12: error = text.errnoMemory; break;
				case 11: error = text.errnoUnavailable; break;
				case 14: error = text.errnoAddress; break;
				case 15: error = text.errnoBlockDev; break;
				case 16: error = text.errnoBusy; break;
				case 21: error = text.errnoDirectory; break;
				case 22: error = text.errnoBadArgs; break;
				case 19: error = text.errnoUnknownDev; break;
				default: error = text.errnoUnknownGeneric;
				}
				if (!hasOutputUSB){
					logObj->out(text.errorMount + error, Error);
					hasOutputUSB = true;
					systemi = 0;
				}
				logObj->out(text.errorCode + to_string(errsv), Error);
				}
			}
			if (systemi < 6) {
				logObj->out(text.infoTryingFilesystem1 + systems[systemi] + text.infoTryingFilesystem2, Info);
				systemi++; //increment the filesystem
			}
	}
	#endif
}

void Server::removeSlashesFromEnd(string& var) {
	while ((var[var.length() - 1] == '/') || (var[var.length() - 1] == '\\')) {
		var.pop_back();
	}
}

void Server::readSettings(string confFile) {
	vector<string> settings {"file", "path", "command", "flags", "method", "device", "debug"};
	vector<string> results = getVarsFromFile(confFile, settings);
	for (const auto& it : results) {
		logObj->out(it, Debug);
	}
    for (vector<string>::iterator firstSetIterator = settings.begin(), secondSetIterator = results.begin(); firstSetIterator != settings.end(); ++firstSetIterator, ++secondSetIterator) {
			auto setVar = [&](string name, string& tempVar){if (*firstSetIterator == name) {tempVar = *secondSetIterator;}};
			auto setVari = [&](string name, int& tempVar){if (*firstSetIterator == name) {tempVar = stoi(*secondSetIterator);}};
      setVar(settings[0], file);
      setVar(settings[1], path);
      setVar(settings[2], command);
	    setVar(settings[3], flags);
			setVar(settings[4], method);
	    setVar(settings[5], device);
			setVari(settings[6], debug);
			logObj->out(text.debugReadingReadsettings, Info);
    }
	if (device == "") {
		logObj->out(text.infoNoMount, Info);
		hasMounted = true;
	}
	logObj->out(text.debugValidatingSettings, Debug);
	auto remSlash = [&](auto& ...var){(removeSlashesFromEnd(var), ...);};
	remSlash(file, path, device);
}

int Server::getPID(int pid, string method) {
#if defined(_WIN64) || defined(_WIN32)
logObj->out(text.warningTestingWindowsSupport, Warning);
return 0;
#else
if (method == "new") {
		if (!kill(pid, 0)) {
			return pid;
		} else {
			int errnum = errno;
			return 0;
		}
} else {
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
}
#endif
}
