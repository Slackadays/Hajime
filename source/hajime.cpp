// (c) 2021 Slackadays on GitHub
#include <filesystem>
namespace fs = std::filesystem;
#if defined(_WIN64) || defined(_WIN32) //Windows compatibility
#include <Windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include <fstream>
#include <memory>
#include <thread>
#include <iostream>
#include <cstring>
#include <string>
#include <stdlib.h>

#ifdef _MSC_VER
#if (_MSC_VER < 1928 || _MSVC_LANG <= 201703L) // msvc usually doesn't define __cplusplus to the correct value
#define jthread thread
#endif
#elif (__cplusplus <= 201703L || defined(__APPLE__) || defined(__MINGW32__) || defined(__MINGW64__)) //jthreads are only in C++20 and up and not supported by Apple Clang yet
#define jthread thread
#endif

#include "output.hpp"
#include "languages.hpp"
#include "installer.hpp"
#include "server.hpp"
#include "getvarsfromfile.hpp"
#include "wizard.hpp"

using std::cin;
using std::cout;
using std::endl;
using std::shared_ptr;
using std::make_shared;
using std::vector;

string defaultServerConfFile = "MyServer.conf";
string defaultServersFile = "servers.conf";
string sysdService = "/etc/systemd/system/hajime.service"; //systemd service file location
string logFile = "";
string hajConfFile = "";

bool readSettings();
void dividerLine();

int main(int argc, char *argv[]) {
	atexit(dividerLine);
	#if defined(_WIN64) || defined (_WIN32)
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); //Windows terminal color compatibility
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	if (!SetConsoleMode(hOut, (dwMode += ENABLE_VIRTUAL_TERMINAL_PROCESSING))) {
		logObj->noColors = true;
	}
	SetConsoleOutputCP(CP_UTF8); //fix broken accents on Windows
	#endif
	dividerLine();
	for (int i = 1; i < argc; i++) { //search for the help flag first
		auto flag = [&i, &argv](auto ...fs){return (!strcmp(fs, argv[i]) || ...);}; //compare flags with a parameter pack pattern
		auto helpOut = [](auto ...num){(logObj->out(text.help[num]), ...);}; //print multiple strings pointed to by text.help[] at once by using a parameter pack
		auto assignNextToVar = [&argc, &argv, &i](auto &var){if (i == (argc - 1)) {return false;} else {var = argv[(i + 1)]; i++; return true;}};
		if (flag("-h", "--help")) { //-h = --help = help
			helpOut(0, 1);
			logObj->out(text.help[2] + (string)argv[0] + text.help[3]); //show example of hajime and include its executed file
			helpOut(4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19); //note: Linux doesn't put an endline at the end upon exit, but Windows does
			return 0; //if someone is asking for help, ignore any other flags and just display the help screen
		}
		if (flag("-l", "--language")) {
			if ((i < (argc - 1)) && string(argv[i + 1]).front() != '-') {
				text.applyLang(argv[i + 1]);
			} else {
				logObj->out(text.errorNotEnoughArgs, Error);
				return 0;
			}
		}
	}
	for (int i = 1; i < argc; i++) {//start at i = 1 to improve performance because we will never find a flag at 0
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
			wizard.wizardStep(hajDefaultConfFile, installer.installDefaultHajConfFile, text.warningFoundHajConf, text.errorHajFileNotMade);
			return 0;
		}
		if (flag("-ss", "--install-servers-file")) {
			wizard.wizardStep(defaultServersFile, installer.installDefaultServersFile, text.errorServersFilePresent, text.errorServersFileNotCreated, std::vector<string>{defaultServerConfFile});
			return 0;
		}
		if (flag("-s", "--install-default-server")) {
			wizard.wizardStep(defaultServerConfFile, installer.installDefaultServerConfFile, text.warningFoundServerConfPlusFile + defaultServerConfFile, text.errorServerConfNotCreated);
			return 0;
		}
		if (flag("-S", "--install-service")) {
			installer.installStartupService(sysdService);
			return 0;
		}
		if (flag("-v", "--verbose")) {
			logObj->verbose = true;
		}
		if (flag("-m", "--monochrome", "--no-colors")) {
			logObj->noColors = true;
		}
		if (flag("-d", "--debug")) {
			logObj->debug = true;
		}
		if (flag("-i", "--install-hajime")) {
			wizard.initialHajimeSetup(hajDefaultConfFile, defaultServersFile, defaultServerConfFile, sysdService);
			return 0;
		}
		if (flag("-np", "--no-pauses")) {
			wizard.doArtificialPauses = false;
		}
		if (flag("-tc", "--thread-colors")) {
			logObj->showThreadsAsColors = true;
		}
		if (flag("-it", "--show-info-type")) {
			logObj->showExplicitInfoType = true;
		}
	}
	if (fs::is_regular_file(hajDefaultConfFile)) {
		readSettings();
		empty(logFile) ? logObj->out(text.infoNoLogFile, Info) : logObj->init(logFile);
	} else {
		logObj->out(text.errorConfDoesNotExist1 + hajDefaultConfFile + text.errorConfDoesNotExist2, Error);
		logObj->out(text.questionDoSetupInstaller, Question);
		if (logObj->getYN()) {
			dividerLine();
			wizard.initialHajimeSetup(hajDefaultConfFile, defaultServersFile, defaultServerConfFile, sysdService);
			logObj->out(text.questionStartHajime, Question);
			if (!logObj->getYN()) {
				return 0;
			}
		} else {
			return 0;
		}
	}
	if (!fs::is_regular_file(defaultServersFile)) {
		logObj->out(text.errorNoServersFile, Error);
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
		string command = "";
		std::getline(std::cin, command);
		if (command == "watch") {
			#if !defined(_WIN64) && !defined (_WIN32)
			logObj->normalDisabled = true;
			serverVec[0].terminalAccessWrapper();
			logObj->normalDisabled = false;
			#else
			cout << "Windows doesn't support this feature." << endl;
			#endif
		} else {
			cout << "Invalid command; try \"watch\" to access the server terminal." << endl;
		}
	}
	return 0;
}

bool readSettings() {
	vector<string> settings{"serversfile", "defserverconf", "logfile", "systemdlocation", "debug"};
	if (!fs::is_regular_file(hajDefaultConfFile)) {
		logObj->out(text.debugHajDefConfNoExist1 + hajDefaultConfFile + text.debugHajDefConfNoExist2, Debug);
		return 0;
	}
	vector<string> results = getVarsFromFile(hajDefaultConfFile, settings);
	for (vector<string>::iterator firstSetIterator = settings.begin(), secondSetIterator = results.begin(); firstSetIterator != settings.end() && secondSetIterator != results.end(); ++firstSetIterator, ++secondSetIterator) {
		auto setVar = [&](string name, string& tempVar){if (*firstSetIterator == name) {tempVar = *secondSetIterator;}};
		auto setVari = [&](string name, int& tempVar){if (*firstSetIterator == name) {try {tempVar = stoi(*secondSetIterator);} catch(...) {tempVar = 0;}}};
		logObj->out(text.debugReadingReadsettings, Debug);
		setVar(settings[0], defaultServersFile);
		setVar(settings[1], defaultServerConfFile);
		setVar(settings[2], logFile);
		setVar(settings[3], sysdService);
		setVari(settings[4], logObj->debug);
	}
	logObj->out(text.debugReadReadsettings + hajDefaultConfFile, Debug);
	return 1;
}

void dividerLine() {
	#if defined(_WIN64) || defined(_WIN32)
	CONSOLE_SCREEN_BUFFER_INFO w;
	int ret;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &w);
	for (int i = 0; i < w.dwSize.X; i++) {
		logObj->out("―", None, 0, 0);
	}
	std::cout << std::endl;
	#else
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	for (int i = 0; i < w.ws_col; i++) {
		logObj->out("―", None, 0, 0);
	}
	std::cout << std::endl;
	#endif
}
