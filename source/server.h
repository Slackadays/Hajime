#if defined(_win64) || defined (_WIN32)
#include <Windows.h>
#define sleep(int) Sleep(1000 * int) //convert sleep statements to Windows-style
#else
#include <unistd.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <signal.h>
#endif
#include <iostream>
#include <filesystem>
#include <stdlib.h>
#include <fstream>
#include <cstring>
#include <string>
#include <errno.h>
#include <vector>

using std::shared_ptr;
using std::string;
using std::fstream;
using std::to_string;
using std::ofstream;
using std::ios;
using std::vector;
using std::cout;

class Server {
	bool hasOutput, hasOutputUSB, hasMounted = false;

	int systemi = 0;

	std::error_code ec;

	shared_ptr<Output> logObj;

	const string systems[8] = {"ext2", "ext3", "ext4", "vfat", "msdos", "f2fs", "ntfs", "fuseblk"};

	void mountDrive();
	void makeDir();
	void startProgram(string method);
	void readSettings(string confFile, vector<string> settings);
	int getPID(int pid = 0, string method = "new");
	vector<string> toArray(string input);
	auto toPointerArray(vector<string> &strings);

	string file, path, command, flags, confFile, device = "";
	string method = "new";

	vector<string> serverConfigParams{"file", "path", "command", "flags", "method", "device"};

	public:
		Server(shared_ptr<Output> tempObj);
		bool isRunning = false;
		void startServer(string confFile);
};

Server::Server(shared_ptr<Output> tempObj) {
	logObj = tempObj;
}

void Server::startServer(string confFile) {
	try {
		if (fs::is_regular_file(confFile, ec)) {
			logObj->out("Reading server settings...", "info");
			readSettings(confFile, serverConfigParams);
		} else {
			logObj->out("The server's config file (" + confFile + ") doesn't exist", "error");
			return;
		}
			logObj->out("Server file: " + file, "info");
			logObj->out("Server path: " + path, "info");
			logObj->out("Server command: " + command, "info");
			//logObj->out("Debug value: " + to_string(debug)); // ->out wants a string so we convert the debug int (converted from a string) back to a string
			logObj->out("Device: " + device, "info");
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
				startProgram(method);
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

vector<string> Server::toArray(string input) {
	vector<string> flagVector;
	string temp = "";
	string execFile = path + '/' + file;
	flagVector.push_back(execFile.c_str());
	for (int i = 0; i < input.length(); temp = "") {
    while (input[i] == ' ' && i < input.length()) { //blah
      i++;
    }
		while (input[i] != ' ' && i < input.length()) {
			temp += input[i];
			i++;
		}
		while (input[i] == ' ' && i < input.length()) {
			i++;
		}
		flagVector.push_back(temp);
		logObj->out("flagVector[0] in For loop =" + flagVector[0], "debug");
	}
	logObj->out("flagVector[0] outside of For loop =" + flagVector[0], "debug");
	return flagVector;
}

auto Server::toPointerArray(vector<string> &strings) {
	vector<char*> pointers;
	for (auto &string : strings) {
		pointers.push_back(string.data());
	}
	pointers.push_back(nullptr);
	return pointers;
}

void Server::startProgram(string method = "new") {
	if (!isRunning) {
		logObj->out("Starting server!", "info");
		fs::current_path(path);
		fs::remove("world/session.lock"); //session.lock will be there if the server didn't shut down properly
		if (method == "old") {
			logObj->out("Using the old method", "debug");
			int returnVal = system(command.c_str()); //convert the command to a c-style string, execute the command
		} else if (method == "new") {
			logObj->out("Using the new method", "debug");
			#if defined(_WIN64) || defined (_WIN32)
			ShellExecuteA(NULL, "open", file.c_str(), flags.c_str(), NULL, SW_NORMAL);
			#else
			logObj->out("Flags =" + flags, "debug");
			auto flagTemp = toArray(flags);
      auto flagArray = toPointerArray(flagTemp);
			int pid = fork();
			if (pid == 0) {
				logObj->out("flagArray[0] =" + (string)flagArray[0], "debug");
				logObj->out("flagArray[1] =" + (string)flagArray[1], "debug");
				execv(file.c_str(), flagArray.data());
			} else {
				sleep(1);
				if (getPID() != 0) { //check for the PID of the program we just started
      		isRunning = true; //isRunning disables a lot of checks
          hasMounted = true;
		    }
			}
			#endif
		} else {
			logObj->out("The method isn't a valid type", "error");
		}
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
	#if defined(_WIN64) || defined(_WIN32)
	logObj->out("Drive mounting is only needed on Linux", "info");
	hasMounted = true;
	#else
	logObj->out("Trying to mount", "info");
	if (!fs::is_empty(path, ec)) { //if there are files, then we don't want to mount there
		logObj->out("There are files in the path", "error");
		return;
	} else {
		string error;
		if (!mount(device.c_str(), path.c_str(), systems[systemi].c_str(), 0, "")) { //brute-forces every possible filesystem because mount() depends on it being the right one
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
					logObj->out("An error occurred, but the script will keep trying to mount. Error: " + error, "error");
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
	#endif
}

void Server::readSettings(string confFile, vector<string> settings) {
	vector<string> results = getVarsFromFile(confFile, settings);
    for (vector<string>::iterator firstSetIterator = settings.begin(), secondSetIterator = results.begin(); firstSetIterator != settings.end(); ++firstSetIterator, ++secondSetIterator) {
    	auto setVar = [&](string name, string& tempVar){if (*firstSetIterator == name) {tempVar = *secondSetIterator;}};
        setVar(settings[0], file);
        setVar(settings[1], path);
        setVar(settings[2], command);
	    	setVar(settings[3], flags);
	    	setVar(settings[4], device);
    }
	if (device == "") {
		logObj->out("No device requested; no mounting this time!", "info");
		hasMounted = true;
	}
}

int Server::getPID(int pid, string method) {
#if defined(_WIN64) || defined(_WIN32)
logObj->out("Testing Windows support!", "warning");
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
