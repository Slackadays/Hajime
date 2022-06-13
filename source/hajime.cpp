/*  Hajime, the ultimate startup script.
    Copyright (C) 2022 Slackadays and other contributors to Hajime on GitHub.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.*/

#include <filesystem>
namespace fs = std::filesystem;
#if defined(_WIN64) || defined(_WIN32) //Windows compatibility
#include <Windows.h>
#include <shlobj.h>
#else
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <signal.h>
#include <fstream>
#include <memory>
#include <thread>
#include <iostream>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <regex>
#include <chrono>
#ifdef _MSC_VER
#if (_MSC_VER < 1928 || _MSVC_LANG <= 201703L) // msvc usually doesn't define __cplusplus to the correct value
#define jthread thread
#endif
#elif (__cplusplus <= 201703L || defined(__APPLE__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__clang__)) //jthreads are only in C++20 and up and not supported by Apple Clang yet
#define jthread thread
#endif

#include "constants.hpp"
#include "output.hpp"
#include "languages.hpp"
#include "installer.hpp"
#include "server.hpp"
#include "getvarsfromfile.hpp"
#include "wizard.hpp"
#include "deduce.hpp"

bool ee = false;
bool bypassPriviligeCheck = false;
std::vector<std::shared_ptr<Server>> serverVec = {}; //create an array of individual server objects
std::vector<std::jthread> threadVec = {}; //create an array of thread objects
std::string logFile = "";
std::string hajConfFile = "";
std::string version;

#if defined(_WIN64) || defined (_WIN32)
void setupTerminal();
#else
void setupRLimits();
#endif
void doPreemptiveFlags(std::vector<std::string> flags);
void doRegularFlags(std::vector<std::string> flags);
void setupSignals();
bool readSettings();
void hajimeExit(int sig);
std::vector<std::string> getServerFiles();
std::vector<std::string> toVec(std::string input);
void setupServers();
void setupFirstTime();
void processHajimeCommand(std::vector<std::string> input);
bool isUserPrivileged();
void doHajimeTerminal();

int main(int argc, char *argv[]) {
	//auto then = std::chrono::high_resolution_clock::now();
	setupSignals();
	#if defined(_WIN64) || defined (_WIN32)
	setupTerminal();
	#endif
	term.dividerLine();
	std::vector<std::string> flags;
	for (int i = 0; i < argc; i++) {
		flags.push_back(argv[i]);
	}
	doPreemptiveFlags(flags);
	doRegularFlags(flags);
	term.out<Info>("Starting Hajime...");
	if (fs::is_regular_file(hajDefaultConfFile)) {
		readSettings();
		empty(logFile) ? term.out<Info>(text.info.NoLogFile) : term.init(logFile);
	} else {
		setupFirstTime();
	}
	if (!bypassPriviligeCheck && isUserPrivileged()) {
		term.out<Error>("Hajime must not be run by a privileged user");
		return 1;
	}
	#if !defined(_WIN64) && !defined(_WIN32)
	setupRLimits();
	#endif
	//std::cout << "This took " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - then).count() << " microseconds" << std::endl;
	//exit(0);
	setupServers();
	doHajimeTerminal();
	return 0;
}

void setupSignals() {
	atexit([]{
		term.dividerLine("Exiting", true);
		#if defined(__APPLE__)
		exit(0);
		#else
		quick_exit(0);
		#endif
	});
	signal(SIGINT, hajimeExit);
	signal(SIGSEGV, [](int sig){
		std::cout << "Segmentation fault detected; exiting Hajime now" << std::endl;
		exit(0);
	});
	signal(SIGABRT, [](int sig){
		std::cout << "Hajime ending execution abnormally; exiting Hajime now" << std::endl;
		exit(0);
	});
	signal(SIGILL, [](int sig){
		std::cout << "Illegal instruction detected; try recompiling Hajime" << std::endl;
		exit(0);
	});
	signal(SIGFPE, [](int sig){
		std::cout << "Illegal math operation; exiting Hajime now" << std::endl;
		exit(0);
	});
	signal(SIGTERM, [](int sig){
		std::cout << "Termination requested; exiting Hajime now" << std::endl;
		exit(0);
	});
}

#if defined(_WIN64) || defined (_WIN32)
void setupTerminal() {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); //Windows terminal color compatibility
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	if (!SetConsoleMode(hOut, (dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT))) {
		term.noColors = true;
	}
	SetConsoleOutputCP(CP_UTF8); //fix broken accents on Windows
}
#else
void setupRLimits() {
	struct rlimit rlimits;
	if (getrlimit(RLIMIT_NOFILE, &rlimits) == -1) {
		term.out<Error, Threadless>("Error getting resource limits; errno = " + std::to_string(errno));
	}
	rlimits.rlim_cur = rlimits.rlim_max; //resize soft limit to max limit; the max limit is a ceiling for the soft limit
	if (setrlimit(RLIMIT_NOFILE, &rlimits) == -1) {
		term.out<Error, Threadless>("Error changing resource limits; errno = " + std::to_string(errno));
	}
	term.out<Debug>("New soft file descriptor soft limit = " + std::to_string(rlimits.rlim_cur));
}
#endif

void doPreemptiveFlags(std::vector<std::string> flags) {
	if (getenv("NO_COLOR") != NULL) {
		term.noColors = true;
	}
	for (int i = 1; i < flags.size(); i++) { //search for the help flag first
		auto flag = [&flags, &i](auto ...fs){
			return ((fs == flags.at(i)) || ...);
		}; //compare flags with a parameter pack pattern
		auto assignNextToVar = [&flags, &i](auto &var){
			if (i == (flags.size() - 1)) {
				return false;
			} else {
				var = flags.at(i + 1);
				i++;
				return true;
			}
		};
		if (flag("-h", "--help")) { //-h = --help = help
			for (auto it : text.help) {
				it = std::regex_replace(it, std::regex("^-(?=\\w+)", std::regex_constants::optimize), "\033[1m$&");
				it = std::regex_replace(it, std::regex(" (?=\\w+ \\w+ --)", std::regex_constants::optimize), "$&\033[3m");
				it = std::regex_replace(it, std::regex(" (?=\\w+ --)", std::regex_constants::optimize), "$&\033[0m");
				it = std::regex_replace(it, std::regex("--(?=\\w+)", std::regex_constants::optimize), "$&\033[1m");
				it = std::regex_replace(it, std::regex(" (?=\\w+ \\|)", std::regex_constants::optimize), "$&\033[3m");
				it = std::regex_replace(it, std::regex("\\|", std::regex_constants::optimize), "\033[0m\033[1m$&\033[0m");
				term.out<Border>(it);
			}
			exit(0); //if someone is asking for help, ignore any other flags and just display the help screen
		}
		if (flag("-l", "--language")) {
			if ((i < (flags.size() - 1)) && flags.at(i + 1).front() != '-') {
				text.applyLang(flags.at(i + 1));
			} else {
				term.out<Error>(text.error.NotEnoughArgs);
				exit(0);
			}
		}
	}
}

void doRegularFlags(std::vector<std::string> flags) {
	if (getenv("NO_COLOR") != NULL) {
		term.noColors = true;
	}
	for (int i = 1; i < flags.size(); i++) {//start at i = 1 to improve performance because we will never find a flag at 0
		auto flag = [&flags, &i](auto ...fs){
			return ((fs == flags.at(i)) || ...);
		};
		auto assignNextToVar = [&flags, &i](auto &var){
			if (i == (flags.size() - 1)) {
				return false;
			} else {
				var = flags.at(i + 1);
				i++;
				return true;
			}
		}; //tries to assign the next argv argument to some variable; if it is not valid, then return an error
		if (flag("-f", "--server-file")) {
			if (!assignNextToVar(defaultServerConfFile)) {
				term.out<Error>(text.error.NotEnoughArgs);
				exit(0);
			}
		}
		if (flag("-hf", "--hajime-file")) {
			if (!assignNextToVar(hajDefaultConfFile)) {
				term.out<Error>(text.error.NotEnoughArgs);
				exit(0);
			}
		}
		if (flag("-ih", "--install-hajime-config")) { //can accept either no added file or an added file
			std::string tempHajConfFile;
			if (std::string var = "-"; assignNextToVar(var) && var[0] != '-') { //compare the next flag if present and check if it is a filename
				tempHajConfFile = var;
			} else {
				tempHajConfFile = hajDefaultConfFile;
			}
			wizard.wizardStep(tempHajConfFile, installer.installDefaultHajConfFile, text.warning.FoundHajConf, text.error.HajFileNotMade, text.language);
			exit(0);
		}
		if (flag("-p", "--privileged")) {
			bypassPriviligeCheck = true;
		}
		if (flag("-s", "--install-default-server")) {
			wizard.wizardStep(defaultServerConfFile, installer.installNewServerConfigFile, text.warning.FoundServerConfPlusFile + defaultServerConfFile, text.error.ServerConfNotCreated, "", "server.jar");
			exit(0);
		}
		if (flag("-S", "--install-service")) {
			installer.installStartupService("/etc/systemd/system/hajime.service");
			exit(0);
		}
		if (flag("-v", "--verbose")) {
			term.verbose = true;
		}
		if (flag("-m", "--monochrome", "--no-colors")) {
			term.noColors = true;
		}
		if (flag("-d", "--debug")) {
			term.debug = true;
		}
		if (flag("-ee")) {
			ee = true;
		}
		if (flag("-i", "--install-hajime")) {
			wizard.initialHajimeSetupAttended(hajDefaultConfFile, defaultServerConfFile);
			exit(0);
		}
		if (flag("-np", "--no-pauses")) {
			wizard.doArtificialPauses = false;
		}
		if (flag("-tc", "--thread-colors")) {
			term.showThreadsAsColors = true;
		}
		if (flag("-ntc", "--no-thread-colors")) {
			term.showThreadsAsColors = false;
		}
		if (flag("-it", "--show-info-type")) {
			term.showExplicitInfoType = true;
		}
	}
}

bool readSettings() {
	std::vector<std::string> settings{"version", "logfile", "debug", "threadcolors"};
	if (!fs::is_regular_file(hajDefaultConfFile)) {
		term.out<Debug>(text.debug.HajDefConfNoExist1 + hajDefaultConfFile + text.debug.HajDefConfNoExist2);
		return 0;
	}
	std::vector<std::string> results = getVarsFromFile(hajDefaultConfFile, settings);
	for (std::vector<std::string>::iterator firstSetIterator = settings.begin(), secondSetIterator = results.begin(); firstSetIterator != settings.end() && secondSetIterator != results.end(); ++firstSetIterator, ++secondSetIterator) {
		auto setVar = [&](std::string name, std::string& tempVar){
			if (*firstSetIterator == name) {
				tempVar = *secondSetIterator;
			}
		};
		auto setVari = [&](std::string name, int& tempVar){
			if (*firstSetIterator == name) {
				try {
					tempVar = stoi(*secondSetIterator);
				} catch(...) {
					tempVar = 0;
				}
			}
		};
		term.out<Debug>(text.debug.ReadingReadsettings);
		setVar(settings[0], version);
		setVar(settings[1], logFile);
		if (!term.debug) {
			setVari(settings[2], term.debug);
		}
		setVari(settings[3], term.showThreadsAsColors);
	}
	term.out<Debug>(text.debug.ReadReadsettings + hajDefaultConfFile);
	return 1;
}

void hajimeExit(int sig) {
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	static std::chrono::time_point<std::chrono::system_clock> then;
	if (std::chrono::duration_cast<std::chrono::seconds>(now - then).count() <= 3) {
		std::cout << "\b\b  " << std::endl;
		exit(0);
	} else {
		std::cout << "\b\b  " << std::endl;
		term.out<None, KeepEndlines, NoEndline, Border>("\033[1mTry again within 3 seconds to exit Hajime");
	}
	then = std::chrono::system_clock::now();
}

std::vector<std::string> getServerFiles() {
	std::vector<std::string> results;
	for (const auto& file : fs::directory_iterator{fs::current_path()}) {
		if (std::regex_match(file.path().filename().string(), std::regex(".+\\.server(?!.+)", std::regex_constants::optimize | std::regex_constants::icase))) {
			results.emplace_back(file.path().filename().string());
		}
	}
	return results;
}

std::vector<std::string> toVec(std::string input) {
	std::vector<std::string> output;
	std::string temp = "";
	for (int i = 0; i < input.length(); temp = "") {
		while (input[i] == ' ' && i < input.length()) { //skip any leading whitespace
			i++;
		}
		while (input[i] != ' ' && i < input.length()) { //add characters to a temp variable that will go into the std::vector
			temp += input[i];
			i++;
		}
		while (input[i] == ' ' && i < input.length()) { //skip any trailing whitespace
			i++;
		}
		output.push_back(temp); //add the finished flag to the std::vector of flags
	}
	return output;
}

void setupServers() {
	std::vector<std::string> serverFiles = getServerFiles();
	if (serverFiles.size() == 0) {
		term.out<Error>("No server files found (Hint: all server files end with .server)");
		exit(0);
	}
	for (const auto& serverIt : serverFiles) { //loop through all the server files found
		serverVec.emplace_back(std::make_shared<Server>()); //add a copy of server to use
		threadVec.emplace_back(std::jthread(&Server::startServer, serverVec.back(), serverIt)); //add a thread that links to startServer and is of the last server object added, use serverIt as parameter
	}
}

void setupFirstTime() {
	term.out<Question>(text.question.DoSetupInstaller);
	switch (term.getYN(text.option.AttendedInstallation, text.option.UnattendedInstallation, text.option.SkipSetup)) {
		case 1:
			term.dividerLine();
			wizard.initialHajimeSetupAttended(hajDefaultConfFile, defaultServerConfFile);
			term.out<Question, NoEndline>(text.question.StartHajime);
			if (!term.getYN()) {
				exit(0);
			}
			break;
		case 2:
			wizard.initialHajimeSetupUnattended(hajDefaultConfFile, defaultServerConfFile);
			term.out<Error>(text.error.OptionNotAvailable);
			break;
		case 3:
			exit(0);
	}
}

void processHajimeCommand(std::vector<std::string> input) {
	if (input[0] == "term" || input[0] == "t") {
		if (input.size() >= 2) {
			try {
				if (stoi(input[1]) > serverVec.size() || stoi(input[1]) < 1) {
					term.out<Error>(text.error.InvalidServerNumber);
				} else {
					term.hajimeTerminal = false;
					serverVec[stoi(input[1]) - 1]->terminalAccessWrapper();
					term.hajimeTerminal = true;
				}
			} catch (...) {
				bool attachSuccess = false;
				for (auto& it : serverVec) {
					if (it->name == input[1]) {
						term.hajimeTerminal = false;
						it->terminalAccessWrapper();
						term.hajimeTerminal = true;
						attachSuccess = true;
						break;
					}
				}
				if (!attachSuccess) {
					term.out<Error>(text.error.ServerSelectionInvalid);
				}
			}
		} else {
			term.out<Error>(text.error.NotEnoughArgs);
		}
	} else if (input[0] == "ee" && ee && text.language == "en") {
		term.out("https://www.youtube.com/watch?v=ccY25Cb3im0");
	} else if (input[0] == "ee" && ee && text.language == "es") {
		term.out("https://www.youtube.com/watch?v=iFClTRUnmKc");
	} else {
		term.out<Error>(text.error.InvalidCommand);
		term.out<Error>(text.error.InvalidHajCommand1);
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

void doHajimeTerminal() {
	term.hajimeTerminal = true;
	while(true) {
		std::string command = "";
		std::getline(std::cin, command);
		std::cout << "\033[0m" << std::flush;
		if (command != "") {
			processHajimeCommand(toVec(command));
		} else {
			term.out<Error>("Command must not be empty");
		}
	}
}
