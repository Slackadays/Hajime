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
#include <fmt/format.h>

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

std::vector<std::shared_ptr<Server>> serverVec = {}; //create an array of individual server objects
std::vector<std::thread> threadVec = {}; //create an array of thread objects
std::string hajConfFile = "";
std::string version;
int stopOnExit = 1;

void shutdownServers() {
	stopOnExit = 2;
	if (stopOnExit == 1) {
		for (auto& server : serverVec) {
			#if !defined(_WIN32) && !defined(_WIN64)
			kill(server->pid, SIGINT);
			#else
			server->writeToServerTerminal("stop");
			#endif
		}
	} else if (stopOnExit == 2) {
		for (auto& server : serverVec) {
			#if !defined(_WIN32) && !defined(_WIN64)
			kill(server->pid, SIGKILL);
			#else
			server->writeToServerTerminal("stop");
			#endif
		}
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

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
	signal(SIGINT, [](int sig){
		hajimeUserExit(sig);
	});
	signal(SIGSEGV, [](int sig){
		std::cout << "Segmentation fault; exiting Hajime" << std::endl;
		shutdownServers();
		exit(1);
	});
	signal(SIGABRT, [](int sig){
		std::cout << "Hajime ending execution abnormally; exiting Hajime" << std::endl;
		shutdownServers();
		exit(1);
	});
	signal(SIGILL, [](int sig){
		std::cout << "Illegal instruction; try recompiling Hajime" << std::endl;
		shutdownServers();
		exit(1);
	});
	signal(SIGFPE, [](int sig){
		std::cout << "Illegal math operation; exiting Hajime" << std::endl;
		shutdownServers();
		exit(1);
	});
	signal(SIGTERM, [](int sig){
		std::cout << "Termination requested; exiting Hajime" << std::endl;
		shutdownServers();
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
		term.out<Error, Threadless>(fmt::vformat("Error getting resource limits; errno = {}", fmt::make_format_args(std::to_string(errno))));
	}
	rlimits.rlim_cur = rlimits.rlim_max; //resize soft limit to max limit; the max limit is a ceiling for the soft limit
	if (setrlimit(RLIMIT_NOFILE, &rlimits) == -1) {
		term.out<Error, Threadless>("Error changing resource limits; errno = " + std::to_string(errno));
	}
	term.out<Debug>("New soft file descriptor soft limit = " + std::to_string(rlimits.rlim_cur));
}
#endif

bool readSettings() {
	std::vector<std::string> settings{"version", "logfile", "debug", "threadcolors", "stoponexit"};
	if (!fs::is_regular_file(hajDefaultConfFile)) {
		term.out<Debug>(fmt::vformat(fmt::to_string_view(text.debug.HajDefConfNoExist), fmt::make_format_args(hajDefaultConfFile)));
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
		setVari(settings[4], stopOnExit);
	}
	term.out<Debug>(text.debug.ReadReadsettings + hajDefaultConfFile);
	return 1;
}

void hajimeUserExit(int sig) {
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	static std::chrono::time_point<std::chrono::system_clock> then;
	if (std::chrono::duration_cast<std::chrono::seconds>(now - then).count() <= 3) {
		std::cout << "\b\b  " << std::endl;
		shutdownServers();
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
	while (std::getline(iss, temp, ' ')) {
		if (temp != "") {
			output.emplace_back(temp);
		}
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
	auto getServerObject = [&](const std::string& serverName) {
		try {
			return std::shared_ptr<Server>(serverVec.at(stoi(serverName) - 1));
		} catch (...) {
			for (auto& it : serverVec) {
				if (it->serverSettings.name == serverName) {
					return std::shared_ptr<Server>(it);
				}
			}
		}
		throw std::invalid_argument("Server not found");
	};
	if (input.size() == 0) {
		return;
	}
	if (input[0] == "term" || input.at(0) == "t") {
		if (input.size() >= 2) {
			try {
				std::shared_ptr<Server> server = getServerObject(input[1]);
				server->terminalAccessWrapper();
			} catch(...) {
				term.out<Error>(text.error.ServerSelectionInvalid);
			}
		} else {
			term.out<Error>(text.error.NotEnoughArgs);
		}
	} else if (input[0] == "info" || input[0] == "i") {
		if (input.size() >= 2) {
			try {
				std::shared_ptr<Server> server = getServerObject(input[1]);
				term.out<Info>("Version: " + server->serverSettings.version);
				term.out<Info>("Name: " + server->serverSettings.name);
				term.out<Info>("Path: " + server->serverSettings.path);
				term.out<Info>("Exec: " + server->serverSettings.exec);
				term.out<Info>("Flags: " + server->serverSettings.flags);
				term.out<Info>("File: " + server->serverSettings.file);
				term.out<Info>("Device: " + server->serverSettings.device);
				term.out<Info>("Restart minute interval: " + std::to_string(server->serverSettings.restartMins));
				term.out<Info>("Commands: " + server->serverSettings.doCommands);
				term.out<Info>("Custom message: " + server->serverSettings.customMessage);
				term.out<Info>("Chat kick regex: " + server->serverSettings.chatKickRegex);
				term.out<Info>("Counters: " + std::to_string(server->serverSettings.counterLevel));
				term.out<Info>("Auto update name: " + server->serverSettings.autoUpdateName);
				term.out<Info>("Auto update version: " + server->serverSettings.autoUpdateVersion);
				term.out<Info>("Counter interval: " + std::to_string(server->serverSettings.counterInterval));
				term.out<Info>("Counter max: " + std::to_string(server->serverSettings.counterMax));
			} catch(...) {
				term.out<Error>(text.error.ServerSelectionInvalid);
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
		}
		term.terminalDispatch("\r \033[92m\033[1m# \033[0m", None, 0);
	}
}
