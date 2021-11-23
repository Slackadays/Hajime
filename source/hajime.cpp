// (c) 2021 Slackadays on GitHub
#if __cplusplus > 201703L //implementations older than C++20 often have the filesystem library in the experimental category
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#if defined(_WIN64) || defined(_WIN32) //Windows compatibility
#include <Windows.h>
#endif
#include <fstream>
#include <memory>
#include <thread>
#include <iostream>
#include <cstring>
#include <string>

#if (__cplusplus <= 201703L || defined(__APPLE__)) //jthreads are only in C++20 and up and not supported by Apple Clang yet
	#define jthread thread
#endif

#include "output.h"
#include "languages.h"
#include "installer.h"
#include "server.h"
#include "getvarsfromfile.h"
#include "wizard.h"

using std::cin;
using std::cout;
using std::endl;
using std::shared_ptr;
using std::make_shared;
using std::vector;

string defaultServerConfFile = "server0.conf";
string defaultServersFile = "servers.conf";
string sysdService = "/etc/systemd/system/hajime.service"; //systemd service file location
string optFlags = "";
string logFile = "";
string hajConfFile = "";

bool readSettings();

int main(int argc, char *argv[]) {
	#if defined(_WIN64) || defined (_WIN32)
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); //Windows terminal compatibility
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	if (!SetConsoleMode(hOut, (dwMode += ENABLE_VIRTUAL_TERMINAL_PROCESSING))) {
		logObj->noColors = true;
	}
	#endif
	for (int i = 1; i < argc; i++) { //search for the help flag first
		auto flag = [&i, &argv](auto ...fs){return (!strcmp(fs, argv[i]) || ...);}; //compare flags with a parameter pack pattern
		auto helpOut = [](auto ...num){(logObj->out(text.help[num]), ...);}; //print multiple strings pointed to by text.help[] at once by using a parameter pack
		if (flag("-h", "--help")) { //-h = --help = help
			helpOut(0, 1);
			logObj->out(text.help[2] + (string)argv[0] + text.help[3]); //show example of hajime and include its executed file
			helpOut(4, 5, 6, 7, 8, 9, 10, 11, 12); //note: Linux doesn't put an endline at the end upon exit, but Windows does
			return 0; //if someone is asking for help, ignore any other flags and just display the help screen
		}
	}
	for (int i = 1; i < argc; i++)  {//start at i = 1 to improve performance because we will never find a flag at 0
		auto flag = [&i, &argv](auto ...fs){return (!strcmp(fs, argv[i]) || ...);};
		auto assignNextToVar = [&argc, &argv, &i](auto &var){if (i == (argc - 1)) {return false;} else {var = argv[(i + 1)]; i++; return true;}}; //tries to assign the next argv argument to some variable; if it is not valid, then return an error
		if (flag("-f", "--server-file")) {
		  if (!assignNextToVar(defaultServerConfFile)) {
				logObj->out(text.errorNotEnoughArgs, Error);
				return 0;
			}
		}
		if (flag("-hf", "--hajime-file")) {
			if (!assignNextToVar(hajDefaultConfFile)) {
				logObj->out(text.errorNotEnoughArgs, Error);
				return 0;
			}
		}
		if (flag("-ih", "--install-hajime-config")) { //can accept either no added file or an added file
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
		if (flag("-S", "--install-service")) {
			installer.installStartupService(sysdService);
			return 0;
		}
		if (flag("-v", "--verbose")) {
			logObj->verbose = true;
		}
		if (flag("-i", "--install-hajime")) {
			initialHajimeSetup(hajDefaultConfFile, defaultServersFile, defaultServerConfFile, sysdService);
			return 0;
		}
	}
 	if (fs::is_regular_file(hajDefaultConfFile)) {
		readSettings();
		if (logFile == "") {
			logObj->out(text.infoNoLogFile, Info);
		} else {
			logObj->init(logFile);
		}
	} else {
		logObj->out(text.errorConfDoesNotExist1 + hajDefaultConfFile + text.errorConfDoesNotExist2, Error);
		logObj->out("It looks like it's your first time using Hajime. Do you want to do the setup installer?", Question);
		if (logObj->getYN()) {
			initialHajimeSetup(hajDefaultConfFile, defaultServersFile, defaultServerConfFile, sysdService);
			logObj->out("Do you want to start Hajime now? Enter \"n\" to exit.", Question);
			if (!logObj->getYN()) {
				return 0;
			}
		} else {
			return 0;
		}
	}
	if (!fs::is_regular_file(defaultServersFile)) {
		logObj->out("No servers file found", Error);
		return 0;
	}
	vector<Server> serverVec; //create an array of individual server objects
	vector<std::jthread> threadVec; //create an array of thread objects
	Server server(logObj); //create a template object
	for (const auto &serverIt : getVarsFromFile(defaultServersFile)) { //loop through all the server files found
		serverVec.push_back(server); //add a copy of server to use
		threadVec.push_back(std::jthread(&Server::startServer, serverVec.back(), serverIt)); //add a thread that links to startServer and is of the last server object added, use serverIt as parameter
	}
	while(true) { //future command processing
		cout << "Enter a command..." << endl;
		string command;
		cin >> command;
		if (command == "test") {
			cout << "Blah!" << endl;
		} else {
			cout << "This feature isn't implemented yet." << endl;
		}
	}
	return 0;
}

bool readSettings() {
	vector<string> settings{"serversfile", "defserverconf", "logfile", "systemdlocation", "optflags"};
	if (!fs::is_regular_file(hajDefaultConfFile)) {
		logObj->out(text.debugHajDefConfNoExist1 + hajDefaultConfFile + text.debugHajDefConfNoExist2, Debug);
		return 0;
	}
	vector<string> results = getVarsFromFile(hajDefaultConfFile, settings);
	for (vector<string>::iterator firstSetIterator = settings.begin(), secondSetIterator = results.begin(); firstSetIterator != settings.end() && secondSetIterator != results.end(); ++firstSetIterator, ++secondSetIterator) {
		auto setVar = [&](string name, string& tempVar){if (*firstSetIterator == name) {tempVar = *secondSetIterator;}};
		logObj->out(text.debugReadingReadsettings, Debug);
		setVar(settings[0], defaultServersFile);
		setVar(settings[1], defaultServerConfFile);
		setVar(settings[2], logFile);
		setVar(settings[3], sysdService);
		setVar(settings[4], optFlags);
	}
	logObj->out(text.debugReadReadsettings + hajDefaultConfFile, Debug);
	return 1;
}
