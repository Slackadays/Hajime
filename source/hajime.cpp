// (c) 2021 Slackadays on GitHub
#include <iostream>
#include <cstring>
#include <string>
#include <filesystem>
#include <fstream>
#include <memory>
#include <thread>

namespace fs = std::filesystem;

#include "getyn.h"
#include "server.h"
#include "output.h"
#include "installer.h"
#include "getvarsfromfile.h"
#include "languages.h"

using std::cin;
using std::cout;
using std::endl;
using std::shared_ptr;
using std::make_shared;
using std::vector;
using std::thread;

string defaultServerConfFile = "server0.conf";
string defaultServersFile = "servers.conf";
string hajDefaultConfFile = "hajime.conf";
string sysdService = "/etc/systemd/system/hajime.service"; //systemd service file location
string logFile = "";
string lang = "";

vector<string> hajimeConfParams{"serversfile", "logfile", "systemdlocation", "lang"};

bool readSettings(vector<string> settings);
bool readLanguage(vector<string> settings);

shared_ptr<Output> logObj = make_shared<Output>(); // make this pointer global

int main(int argc, char *argv[]) {
	Installer installer;
	readLanguage(hajimeConfParams);
	Text text(lang);
	for (int i = 1; i < argc; i++) { //search for the help flag first
		auto flag = [&i, &argv](auto ...fs){return (!strcmp(fs, argv[i]) || ...);}; //compare flags with a parameter pack pattern
                if (flag("-h", "--help")) { //-h = --help = help
                        logObj->out(text.help[0]);
                        logObj->out(text.help[1] + (string)argv[0] + text.help[2]);
                        logObj->out(text.help[3]);
                        logObj->out(text.help[4]);
			logObj->out(text.help[5]);
                        logObj->out(text.help[6]);
                        logObj->out(text.help[7]);
			logObj->out(text.help[8]);
                        logObj->out(text.help[9], "none", 1);
			logObj->out(text.help[10]);
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
		auto assignNextToVar = [&argc, &argv, &i](auto &var){if (i == (argc - 1)) {return false;} else {var = argv[(i + 1)]; i++; return true;}};
                if (flag("-f", "--server-file")) {
                       	if (!assignNextToVar(defaultServerConfFile)) {
				logObj->out("Not enough arguments provided", "error");
				return 0;
			}
                }
		if (flag("-hf", "--hajime-file")) {
			if (!assignNextToVar(hajDefaultConfFile)) {
				logObj->out("Not enough arguments provided", "error");
				return 0;
			}
		}
		if (flag("-i", "--install-hajime")) { //can accept either no added file or an added file
			if (string var = "-"; assignNextToVar(var) && var[0] != '-') { //compare the next flag if present and check if it is a filename
				hajDefaultConfFile = var;
			}
			installer.installDefaultHajConfFile(hajDefaultConfFile);
			return 0;
		}
		if (flag("-ss", "--install-servers-file")) {
			installer.installDefaultServersFile(defaultServersFile);
			return 0;
		}
		if (flag("-s", "--install-default-server")) {
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
                logObj->out("Config file " + hajDefaultConfFile + " doesn't exist!", "error");
		logObj->out("Looks like there isn't a Hajime configuation file. Would you like to make one?", "info", 0, 0);
                	if (getYN()) {
                	installer.installDefaultHajConfFile(hajDefaultConfFile);
                }
        }
	vector<Server> serverVec;
	vector<thread> threadVec;
	Server server(logObj); //create a template object
	for (const auto &serverIt : getVarsFromFile(defaultServersFile)) { //loop through all the server files found
		serverVec.push_back(server); //add a copy of server to use
		threadVec.push_back(thread(&Server::startServer, serverVec.back(), serverIt)); //add a thread that links to startServer and is of the last server object added, use serverIt as parameter
	}
	while(true) { //dummy loop for thereads
		cout << "Blah." << endl;
		sleep(1);
	}
	return 0;
}

bool readSettings(vector<string> settings) {
	if (!fs::is_regular_file(hajDefaultConfFile)) {
		return 0;
	}
	vector<string> results = getVarsFromFile(hajDefaultConfFile, settings);
	for (vector<string>::iterator firstSetIterator = settings.begin(), secondSetIterator = results.begin(); firstSetIterator != settings.end(); ++firstSetIterator, ++secondSetIterator) {
		auto setVar = [&](string name, string& tempVar){if (*firstSetIterator == name) {tempVar = *secondSetIterator;}};
		setVar(settings[0], defaultServersFile);
		setVar(settings[1], logFile);
		setVar(settings[2], sysdService);
	}
	return 1;
}

bool readLanguage(vector<string> settings) {
        if (!fs::is_regular_file(hajDefaultConfFile)) {
                return 0;
        }
        vector<string> results = getVarsFromFile(hajDefaultConfFile, settings);
        for (vector<string>::iterator firstSetIterator = settings.begin(), secondSetIterator = results.begin(); firstSetIterator != settings.end(); ++firstSetIterator, ++secondSetIterator) {
                auto setVar = [&](string name, string& tempVar){if (*firstSetIterator == name) {tempVar = *secondSetIterator;}};
                setVar(settings[3], lang);
        }
        return 1;
}


