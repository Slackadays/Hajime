// (c) 2021 Slackadays on GitHub
#include <filesystem>
namespace fs = std::filesystem;
#if defined(_WIN64) || defined(_WIN32) //Windows compatibility
#include <Windows.h>
#include <shlobj.h>
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
#include <regex>

#ifdef _MSC_VER
#if (_MSC_VER < 1928 || _MSVC_LANG <= 201703L) // msvc usually doesn't define __cplusplus to the correct value
#define jthread thread
#endif
#elif (__cplusplus <= 201703L || defined(__APPLE__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__clang__)) //jthreads are only in C++20 and up and not supported by Apple Clang yet
#define jthread thread
#endif

#include "output.hpp"
#include "languages.hpp"
#include "installer.hpp"
#include "server.hpp"
#include "getvarsfromfile.hpp"
#include "wizard.hpp"
#include "deduce.hpp"

using std::cin;
using std::cout;
using std::endl;
using std::shared_ptr;
using std::make_shared;
using std::vector;

bool ee = false;
vector<Server*> serverVec = {}; //create an array of individual server objects
vector<std::jthread> threadVec = {}; //create an array of thread objects
string defaultServerConfFile = "MyServer.server";
string defaultServersFile = "servers.conf";
string sysdService = "/etc/systemd/system/hajime.service"; //systemd service file location
string logFile = "";
string hajConfFile = "";
string version;

bool readSettings();
void dividerLine();
vector<string> toVec(string input);
void processHajimeCommand(vector<string> input);
bool isUserPrivileged();

int main(int argc, char *argv[]) {
	atexit(dividerLine);
	#if defined(_WIN64) || defined (_WIN32)
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); //Windows terminal color compatibility
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	if (!SetConsoleMode(hOut, (dwMode += ENABLE_VIRTUAL_TERMINAL_PROCESSING))) {
		hjlog.noColors = true;
	}
	SetConsoleOutputCP(CP_UTF8); //fix broken accents on Windows
	#endif
	dividerLine();
	for (int i = 1; i < argc; i++) { //search for the help flag first
		auto flag = [&i, &argv](auto ...fs){return (!strcmp(fs, argv[i]) || ...);}; //compare flags with a parameter pack pattern
		auto assignNextToVar = [&argc, &argv, &i](auto &var){if (i == (argc - 1)) {return false;} else {var = argv[(i + 1)]; i++; return true;}};
		if (flag("-h", "--help")) { //-h = --help = help
			for (auto it : text.help) {
				it = std::regex_replace(it, std::regex("^-(?=\\w+)", std::regex_constants::optimize), "\033[1m$&");
				it = std::regex_replace(it, std::regex(" (?=\\w+ \\w+ --)", std::regex_constants::optimize), "$&\033[3m");
				it = std::regex_replace(it, std::regex(" (?=\\w+ --)", std::regex_constants::optimize), "$&\033[0m");
				it = std::regex_replace(it, std::regex("--(?=\\w+)", std::regex_constants::optimize), "$&\033[1m");
				it = std::regex_replace(it, std::regex(" (?=\\w+ \\|)", std::regex_constants::optimize), "$&\033[3m");
				it = std::regex_replace(it, std::regex("\\|", std::regex_constants::optimize), "\033[0m\033[1m$&\033[0m");
				hjlog.out(it);
			}
			return 0; //if someone is asking for help, ignore any other flags and just display the help screen
		}
		if (flag("-l", "--language")) {
			if ((i < (argc - 1)) && string(argv[i + 1]).front() != '-') {
				text.applyLang(argv[i + 1]);
			} else {
				hjlog.out(text.error.NotEnoughArgs, Error);
				return 0;
			}
		}
	}
	bool bypassPriviligeCheck = false;
	for (int i = 1; i < argc; i++) {//start at i = 1 to improve performance because we will never find a flag at 0
		auto flag = [&i, &argv](auto ...fs){return (!strcmp(fs, argv[i]) || ...);};
		auto assignNextToVar = [&argc, &argv, &i](auto &var){if (i == (argc - 1)) {return false;} else {var = argv[(i + 1)]; i++; return true;}}; //tries to assign the next argv argument to some variable; if it is not valid, then return an error
		if (flag("-f", "--server-file")) {
			if (!assignNextToVar(defaultServerConfFile)) {
				hjlog.out(text.error.NotEnoughArgs, Error);
				return 0;
			}
		}
		if (flag("-hf", "--hajime-file")) {
			if (!assignNextToVar(hajDefaultConfFile)) {
				hjlog.out(text.error.NotEnoughArgs, Error);
				return 0;
			}
		}
		if (flag("-ih", "--install-hajime-config")) { //can accept either no added file or an added file
			if (string var = "-"; assignNextToVar(var) && var[0] != '-') { //compare the next flag if present and check if it is a filename
				hajDefaultConfFile = var;
			}
			wizard.wizardStep(hajDefaultConfFile, installer.installDefaultHajConfFile, text.warning.FoundHajConf, text.error.HajFileNotMade, text.language);
			return 0;
		}
		if (flag("-p", "--privileged")) {
			bypassPriviligeCheck = true;
		}
		if (flag("-ss", "--install-servers-file")) {
			wizard.wizardStep(defaultServersFile, installer.installDefaultServersFile, text.error.ServersFilePresent, text.error.ServersFileNotCreated, std::vector<string>{defaultServerConfFile});
			return 0;
		}
		if (flag("-s", "--install-default-server")) {
			wizard.wizardStep(defaultServerConfFile, installer.installNewServerConfigFile, text.warning.FoundServerConfPlusFile + defaultServerConfFile, text.error.ServerConfNotCreated, "", "server.jar");
			return 0;
		}
		if (flag("-S", "--install-service")) {
			installer.installStartupService(sysdService);
			return 0;
		}
		if (flag("-v", "--verbose")) {
			hjlog.verbose = true;
		}
		if (flag("-m", "--monochrome", "--no-colors")) {
			hjlog.noColors = true;
		}
		if (flag("-d", "--debug")) {
			hjlog.debug = true;
		}
		if (flag("-ee")) {
			ee = true;
		}
		if (flag("-i", "--install-hajime")) {
			wizard.initialHajimeSetup(hajDefaultConfFile, defaultServersFile, defaultServerConfFile, sysdService);
			return 0;
		}
		if (flag("-np", "--no-pauses")) {
			wizard.doArtificialPauses = false;
		}
		if (flag("-tc", "--thread-colors")) {
			hjlog.showThreadsAsColors = true;
		}
		if (flag("-ntc", "--no-thread-colors")) {
			hjlog.showThreadsAsColors = false;
		}
		if (flag("-it", "--show-info-type")) {
			hjlog.showExplicitInfoType = true;
		}
	}
	if (fs::is_regular_file(hajDefaultConfFile)) {
		readSettings();
		empty(logFile) ? hjlog.out(text.info.NoLogFile, Info) : hjlog.init(logFile);
	} else {
		hjlog.out(text.error.ConfDoesNotExist1 + hajDefaultConfFile + text.error.ConfDoesNotExist2, Error);
		hjlog.out(text.question.DoSetupInstaller, Question);
		switch (hjlog.getYN(text.option.AttendedInstallation, text.option.UnattendedInstallation, text.option.SkipSetup)) {
			case 1:
				dividerLine();
				wizard.initialHajimeSetup(hajDefaultConfFile, defaultServersFile, defaultServerConfFile, sysdService);
				hjlog.out(text.question.StartHajime, Question);
				if (!hjlog.getYN()) {
					return 0;
				}
				break;
			case 2:
				hjlog.out(text.error.OptionNotAvailable, Error);
				break;
			case 3:
				return 0;
		}
	}
	if (!fs::is_regular_file(defaultServersFile)) {
		hjlog.out(text.error.NoServersFile, Error);
		return 0;
	}
	for (int i = 1; i < argc; i++) {
		auto flag = [&i, &argv](auto ...fs){return (!strcmp(fs, argv[i]) || ...);};
		if (flag("-tc", "--thread-colors")) {
			hjlog.showThreadsAsColors = true;
		}
		if (flag("-ntc", "--no-thread-colors")) {
			hjlog.showThreadsAsColors = false;
		}
	}
	if (!bypassPriviligeCheck && isUserPrivileged()) {
		hjlog.out("Hajime must not be run by a privileged user", Error);
		return 1;
	}
	for (const auto& serverIt : getVarsFromFile(defaultServersFile)) { //loop through all the server files found
		serverVec.emplace_back(new Server); //add a copy of server to use
		threadVec.emplace_back(std::jthread(&Server::startServer, serverVec.back(), serverIt)); //add a thread that links to startServer and is of the last server object added, use serverIt as parameter
	}
	hjlog.hajimeTerminal = true;
	while(true) {
		string command = "";
		std::getline(std::cin, command);
		if (command != "") {
			processHajimeCommand(toVec(command));
		} else {
			hjlog.out("Command must not be empty", Error);
		}
	}
	return 0;
}

bool readSettings() {
	vector<string> settings{"version", "serversfile", "defserverconf", "logfile", "systemdlocation", "debug", "threadcolors"};
	if (!fs::is_regular_file(hajDefaultConfFile)) {
		hjlog.out(text.debug.HajDefConfNoExist1 + hajDefaultConfFile + text.debug.HajDefConfNoExist2, Debug);
		return 0;
	}
	vector<string> results = getVarsFromFile(hajDefaultConfFile, settings);
	for (vector<string>::iterator firstSetIterator = settings.begin(), secondSetIterator = results.begin(); firstSetIterator != settings.end() && secondSetIterator != results.end(); ++firstSetIterator, ++secondSetIterator) {
		auto setVar = [&](string name, string& tempVar){if (*firstSetIterator == name) {tempVar = *secondSetIterator;}};
		auto setVari = [&](string name, int& tempVar){if (*firstSetIterator == name) {try {tempVar = stoi(*secondSetIterator);} catch(...) {tempVar = 0;}}};
		hjlog.out(text.debug.ReadingReadsettings, Debug);
		setVar(settings[0], version);
		setVar(settings[1], defaultServersFile);
		setVar(settings[2], defaultServerConfFile);
		setVar(settings[3], logFile);
		setVar(settings[4], sysdService);
		setVari(settings[5], hjlog.debug);
		setVari(settings[6], hjlog.showThreadsAsColors);
	}
	hjlog.out(text.debug.ReadReadsettings + hajDefaultConfFile, Debug);
	return 1;
}

void dividerLine() {
	#if defined(_WIN64) || defined(_WIN32)
	CONSOLE_SCREEN_BUFFER_INFO w;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &w);
	for (int i = 0; i < w.dwSize.X; i++) {
		hjlog.out("─", None, 0, 0);
	}
	#else
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	for (int i = 0; i < w.ws_col; i++) {
		hjlog.out("─", None, 0, 0);
	}
	#endif
	std::cout << std::endl;
}

vector<string> toVec(string input) {
	vector<string> output;
	string temp = "";
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
		output.push_back(temp); //add the finished flag to the vector of flags
	}
	return output;
}

void processHajimeCommand(vector<string> input) {
	if (input[0] == "term" || input[0] == "t") {
		if (input.size() >= 2) {
			try {
				if (stoi(input[1]) > serverVec.size() || stoi(input[1]) < 1) {
					hjlog.out(text.error.InvalidServerNumber, Error);
				} else {
					hjlog.hajimeTerminal = false;
					serverVec[stoi(input[1]) - 1]->terminalAccessWrapper();
					hjlog.hajimeTerminal = true;
				}
			} catch (...) {
				bool attachSuccess = false;
				for (auto& it : serverVec) {
					if (it->name == input[1]) {
						hjlog.hajimeTerminal = false;
						it->terminalAccessWrapper();
						hjlog.hajimeTerminal = true;
						attachSuccess = true;
						break;
					}
				}
				if (!attachSuccess) {
					hjlog.out(text.error.ServerSelectionInvalid, Error);
				}
			}
		} else {
			hjlog.out(text.error.NotEnoughArgs, Error);
		}
	} else if (input[0] == "ee" && ee && text.language == "en") {
		hjlog.out("https://www.youtube.com/watch?v=ccY25Cb3im0");
	} else if (input[0] == "ee" && ee && text.language == "es") {
		hjlog.out("https://www.youtube.com/watch?v=iFClTRUnmKc");
	} else {
		hjlog.out(text.error.InvalidCommand, Error);
		hjlog.out(text.error.InvalidHajCommand1, Error);
	}
}

bool isUserPrivileged() {
	#if !defined(_WIN32) && !defined(_WIN64)
	if (!geteuid()) {
		return true;
	} else {
		return false;
	}
	#else
	if (IsUserAnAdmin()) {
		return true;
	} else {
		return false;
	}
	#endif
}
