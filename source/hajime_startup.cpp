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
#if defined(_WIN64) || defined(_WIN32) //Windows compatibility
#include <Windows.h>
#include <shlobj.h>
#else
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <vector>
#include <sstream>
#include <signal.h>

#include <boost/algorithm/string.hpp>

#if !defined(_WIN64) && !defined(_WIN32)
#include <ncurses.h>
#endif

#include "hajime_startup.hpp"
#include "output.hpp"
#include "deduce.hpp"
#include "constants.hpp"
#include "installer.hpp"
#include "wizard.hpp"
#include "flags.hpp"

void setupSignals() {
	atexit([]{
		term.dividerLine("Exiting", true);
		#if !defined(_WIN64) && !defined(_WIN32)
		endwin();
		#endif
		#if defined(__APPLE__)
		exit(0);
		#else
		quick_exit(0);
		#endif
	});
	signal(SIGINT, hajimeUserExit);
	signal(SIGSEGV, [](int sig){
		std::cout << "Segmentation fault; exiting Hajime" << std::endl;
		#if !defined(_WIN64) && !defined(_WIN32)
		endwin();
		#endif
		exit(1);
	});
	signal(SIGABRT, [](int sig){
		std::cout << "Hajime ending execution abnormally; exiting Hajime" << std::endl;
		#if !defined(_WIN64) && !defined(_WIN32)
		endwin();
		#endif
		exit(1);
	});
	signal(SIGILL, [](int sig){
		std::cout << "Illegal instruction; try recompiling Hajime" << std::endl;
		#if !defined(_WIN64) && !defined(_WIN32)
		endwin();
		#endif
		exit(1);
	});
	signal(SIGFPE, [](int sig){
		std::cout << "Illegal math operation; exiting Hajime" << std::endl;
		#if !defined(_WIN64) && !defined(_WIN32)
		endwin();
		#endif
		exit(1);
	});
	signal(SIGTERM, [](int sig){
		std::cout << "Termination requested; exiting Hajime" << std::endl;
		#if !defined(_WIN64) && !defined(_WIN32)
		endwin();
		#endif
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

void hajimeUserExit(int sig) {
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
	for (const auto& file : fs::directory_iterator{fs::current_path() /= hajimePath}) {
		if (std::regex_match(file.path().filename().string(), std::regex(".+\\.server(?!.+)", std::regex_constants::optimize | std::regex_constants::icase))) {
			results.emplace_back(file.path().filename().string());
		}
	}
	return results;
}

std::vector<std::string> splitToVec(std::string input) {
	std::vector<std::string> output;
	std::istringstream iss(input);
	std::string temp;
	//split the string into a vector
	while (std::getline(iss, temp, ' ')) {
		output.emplace_back(temp);
	}
	return output;
}

void setupServers() {
	std::vector<std::string> serverFiles = getServerFiles();
	if (serverFiles.size() == 0) {
		term.out<Error>("No server files found (all server files end with .server)");
		exit(0);
	}
	for (const auto& serverIt : serverFiles) { //loop through all the server files found
		serverVec.emplace_back(std::make_shared<Server>()); //add a copy of server to use
		threadVec.emplace_back(std::thread(&Server::startServer, serverVec.back(), hajimePath + serverIt)); //add a thread that links to startServer and is of the last server object added, use serverIt as parameter
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
	if (input.at(0) == "term" || input.at(0) == "t") {
		if (input.size() >= 2) {
			try {
				if (stoi(input[1]) > serverVec.size() || stoi(input[1]) < 1) {
					term.out<Error>(text.error.InvalidServerNumber);
				} else {
					serverVec.at(stoi(input[1]) - 1)->terminalAccessWrapper();
				}
			} catch (...) {
				bool attachSuccess = false;
				for (auto& it : serverVec) {
					if (it->serverSettings.name == input[1]) {
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
	return !geteuid();
	#else
	return IsUserAnAdmin();
	#endif
}

void doHajimeTerminal() {
	term.hajimeTerminal = true;
	std::string command;
	while(true) {
		std::getline(std::cin, command);
		boost::trim(command); //remove leading and trailing whitespace
		std::cout << "\033[0m" << std::flush;
		if (command != "") {
			processHajimeCommand(splitToVec(command));
		} else {
			std::cout << std::endl;
		}
	}
}
