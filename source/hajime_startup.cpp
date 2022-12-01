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
    
#if defined(_WIN64) || defined(_WIN32) //Windows compatibility
#include <Windows.h>
#include <shlobj.h>
#else
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#if defined(__linux__)
#include <sensors/sensors.h>
#endif

#include <vector>
#include <sstream>
#include <filesystem>
#include <signal.h>

#include <boost/algorithm/string.hpp>
#include "nlohmann/json.hpp"

#define FMT_HEADER_ONLY
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
#include "flexi_format.hpp"

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
		shutdownServers();
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
		exit(1);
	});
	signal(SIGABRT, [](int sig){
		std::cout << "Hajime ending execution abnormally; exiting Hajime" << std::endl;
		exit(1);
	});
	signal(SIGILL, [](int sig){
		std::cout << "Illegal instruction; try recompiling Hajime" << std::endl;
		exit(1);
	});
	signal(SIGFPE, [](int sig){
		std::cout << "Illegal math operation; exiting Hajime" << std::endl;
		exit(1);
	});
	signal(SIGTERM, [](int sig){
		std::cout << "Termination requested; exiting Hajime" << std::endl;
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
void setupTUI() {
	if (useTUI) {
		initscr();
		start_color();
		init_pair(1, COLOR_BLACK, COLOR_CYAN);
		attron(COLOR_PAIR(1));
		wbkgd(stdscr, COLOR_PAIR(1));
		attron(A_BOLD);
		printw("Hajime, the ultimate startup script.");
		getch();
		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	}
}

void setupRLimits() {
	struct rlimit rlimits;
	if (getrlimit(RLIMIT_NOFILE, &rlimits) == -1) {
		term.out<Error, Threadless>(flexi_format("Error getting resource limits; errno = {}", std::to_string(errno)));
	}
	rlimits.rlim_cur = rlimits.rlim_max; //resize soft limit to max limit; the max limit is a ceiling for the soft limit
	if (setrlimit(RLIMIT_NOFILE, &rlimits) == -1) {
		term.out<Error, Threadless>("Error changing resource limits; errno = " + std::to_string(errno));
	}
	term.out<Debug>("New soft file descriptor soft limit = " + std::to_string(rlimits.rlim_cur));
}
#endif

#if defined(__linux__)
void setupSensors() {
	if (sensors_init(nullptr) == 0) {
		term.out<Debug>("libsensors initialized, library version " + std::string(libsensors_version));
	} else {
		term.out<Error, Threadless>("Error initializing sensors");
	}
}
#endif

void setupHajimeDirectory() {
	auto setupSubdirectory = []() {
		if (!fs::is_directory(hajimePath + serverSubpath)) {
			term.out<Info>(text.info.MakingHajimeDirectory + hajimePath + serverSubpath);
			try {
				fs::create_directory(hajimePath + serverSubpath);
			} catch (fs::filesystem_error& e) {
				term.out<Error>("Could not create server subdirectory");
				term.out<Error>(e.what());
				exit(1);
			}
		}
	};
	if (!fs::is_directory(hajimePath)) {
		term.out<Info>(text.info.MakingHajimeDirectory + hajimePath);
		try {
			fs::create_directory(hajimePath);
			setupSubdirectory();
		} catch (fs::filesystem_error& e) {
			term.out<Error>("Could not create Hajime directory");
			term.out<Error>(e.what());
			exit(1);
		}
	} else {
		setupSubdirectory();
	}
}

bool readSettings() {
	std::vector<std::string> settings{"version", "logfile", "debug", "threadcolors", "stoponexit"};
	if (!fs::is_regular_file(hajDefaultConfFile)) {
		term.out<Debug>(flexi_format(text.debug.HajDefConfNoExist, hajDefaultConfFile));
		return 0;
	}
	//read the entire contents of hajDefaultConfFile into a variable
	std::ifstream file(hajDefaultConfFile);
	std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	term.out<Debug>(text.debug.ReadingReadsettings);
	try {
		nlohmann::json content = nlohmann::json::parse(contents);
		for (const auto& setting : settings) {
			version = content["version"];
			logFile = content["logfile"];
			term.debug = content["debug"];
			term.showThreadsAsColors = content["threadcolors"];
			stopOnExit = content["stoponexit"];
		}
	} catch (std::exception& e) {
		term.out<Error, Threadless>(flexi_format("Error parsing Hajime JSON: {}", e.what()));
		return 0;
	}
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
	for (const auto& file : fs::directory_iterator{fs::current_path() /= (hajimePath + serverSubpath)}) {
		if (std::regex_match(file.path().filename().string(), std::regex(".+\\.json(?!.+)", std::regex_constants::optimize | std::regex_constants::icase))) {
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
		term.out<Error>("No server files found (all server files end with .json and are in the hajime.d/servers subdirectory)");
		exit(0);
	}
	for (const auto& serverIt : serverFiles) { //loop through all the server files found
		serverVec.emplace_back(std::make_shared<Server>()); //add a copy of server to use
		threadVec.emplace_back(std::thread(&Server::startServer, serverVec.back(), hajimePath + serverSubpath + serverIt)); //add a thread that links to startServer and is of the last server object added, use serverIt as parameter
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
	if (input.at(0) == "term" || input.at(0) == "t") {
		if (input.size() >= 2) {
			try {
				std::shared_ptr<Server> server = getServerObject(input.at(1));
				server->terminalAccessWrapper();
			} catch(...) {
				term.out<Error>(text.error.ServerSelectionInvalid);
			}
		} else {
			term.out<Error>(text.error.NotEnoughArgs);
		}
	} else if (input.at(0) == "info" || input.at(0) == "i") {
		if (input.size() >= 2) {
			try {
				std::shared_ptr<Server> server = getServerObject(input.at(1));
				std::lock_guard<std::mutex> lock(server->serverSettings.mutex);
				term.out<Info>("Version: " + server->serverSettings.version);
				term.out<Info>("Name: " + server->serverSettings.name);
				term.out<Info>("Path: " + server->serverSettings.path);
				term.out<Info>("Exec: " + server->serverSettings.exec);
				term.out<Info>("Flags: " + server->serverSettings.flags);
				term.out<Info>("File: " + server->serverSettings.file);
				term.out<Info>("Device: " + server->serverSettings.device);
				term.out<Info>("Restart minute interval: " + std::to_string(server->serverSettings.restartMins));
				term.out<Info>("Commands: " + std::to_string(server->serverSettings.doCommands));
				term.out<Info>("Custom message: " + server->serverSettings.customMessage);
				term.out<Info>("Chat kick regex: " + server->serverSettings.chatKickRegex);
				if (server->serverSettings.counterLevel == CounterLevel::All) {
					term.out<Info>("Counter level: All");
				} else if (server->serverSettings.counterLevel == CounterLevel::Medium) {
					term.out<Info>("Counter level: Medium");
				} else if (server->serverSettings.counterLevel == CounterLevel::Low) {
					term.out<Info>("Counter level: Low");
				} else if (server->serverSettings.counterLevel == CounterLevel::Off) {
					term.out<Info>("Counter level: Off");
				}
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
	} else if (input.at(0) == "set" || input.at(0) == "s") {
		if (input.size() >= 4) {
			try {
				std::shared_ptr<Server> server = getServerObject(input.at(1));
				std::lock_guard<std::mutex> lock(server->serverSettings.mutex);
				try {
					if (input.at(2) == "name") {
						server->serverSettings.name = input.at(3);
						term.out<Info>(flexi_format("Server {}'s name set to {}", input.at(1), server->serverSettings.name));
					} else if (input.at(2) == "path") {
						server->serverSettings.path = input.at(3);
						term.out<Info>(flexi_format("Server {}'s path set to {}", input.at(1), server->serverSettings.path));
					} else if (input.at(2) == "exec") {
						server->serverSettings.exec = input.at(3);
						term.out<Info>(flexi_format("Server {}'s execution file set to {}", input.at(1), server->serverSettings.exec));
					} else if (input.at(2) == "flags") {
						server->serverSettings.flags = input.at(3);
						term.out<Info>(flexi_format("Server {}'s flags set to {}", input.at(1), server->serverSettings.flags));
					} else if (input.at(2) == "file") {
						server->serverSettings.file = input.at(3);
						term.out<Info>(flexi_format("Server {}'s server file set to {}", input.at(1), server->serverSettings.file));
					} else if (input.at(2) == "device") {
						server->serverSettings.device = input.at(3);
						term.out<Info>(flexi_format("Server {}'s device set to {}", input.at(1), server->serverSettings.device));
					} else if (input.at(2) == "restartmins") {
						server->serverSettings.restartMins = stol(input.at(3));
						term.out<Info>(flexi_format("Server {}'s restart minute interval set to {}", input.at(1), server->serverSettings.restartMins));
					} else if (input.at(2) == "docommands") {
						server->serverSettings.doCommands = stoi(input.at(3));
						term.out<Info>(flexi_format("Server {}'s command preference set to {}", input.at(1), server->serverSettings.doCommands));
					} else if (input.at(2) == "custommsg") {
						server->serverSettings.customMessage = input.at(3);
						term.out<Info>(flexi_format("Server {}'s custom message set to {}", input.at(1), server->serverSettings.customMessage));
					} else if (input.at(2) == "chat") {
						server->serverSettings.chatKickRegex = input.at(3);
						term.out<Info>(flexi_format("Server {}'s chat kick regex set to {}", input.at(1), server->serverSettings.chatKickRegex));
					} else if (input.at(2) == "counterlevel") {
						if (input.at(3) == "all") {
							server->serverSettings.counterLevel = CounterLevel::All;
							term.out<Info>(flexi_format("Server {}'s counter level set to All", input.at(1)));
						} else if (input.at(3) == "medium") {
							server->serverSettings.counterLevel = CounterLevel::Medium;
							term.out<Info>(flexi_format("Server {}'s counter level set to Medium", input.at(1)));
						} else if (input.at(3) == "low") {
							server->serverSettings.counterLevel = CounterLevel::Low;
							term.out<Info>(flexi_format("Server {}'s counter level set to Low", input.at(1)));
						} else if (input.at(3) == "off") {
							server->serverSettings.counterLevel = CounterLevel::Off;
							term.out<Info>(flexi_format("Server {}'s counter level set to Off", input.at(1)));
						} else {
							term.out<Error>("Invalid counter level");
						}
					} else if (input.at(2) == "autoupdatename") {
						server->serverSettings.autoUpdateName = input.at(3);
						term.out<Info>(flexi_format("Server {}'s auto update name set to {}", input.at(1), server->serverSettings.autoUpdateName));
					} else if (input.at(2) == "autoupdateversion") {
						server->serverSettings.autoUpdateVersion = input.at(3);
						term.out<Info>(flexi_format("Server {}'s auto update version set to {}", input.at(1), server->serverSettings.autoUpdateVersion));
					} else if (input.at(2) == "counterinterval") {
						server->serverSettings.counterInterval = stol(input.at(3));
						term.out<Info>(flexi_format("Server {}'s performance counter interval set to {}", input.at(1), server->serverSettings.counterInterval));
					} else if (input.at(2) == "countermax") {
						server->serverSettings.counterMax = stol(input.at(3));
						term.out<Info>(flexi_format("Server {}'s performance counter max set to {}", input.at(1), server->serverSettings.counterMax));
					} else {
						term.out<Error>(flexi_format("Invalid option {}", input.at(2)));
					}
				} catch (std::exception& e) {
					term.out<Error>(flexi_format("Error setting server option: {}", e.what()));
				}
			} catch(...) {
				term.out<Error>(text.error.ServerSelectionInvalid);
			}
		} else {
			term.out<Error>("Not enough arguments provided");
		}
	} else if (input.at(0) == "save") {
		if (input.size() >= 2) {
			try {
				std::shared_ptr<Server> server = getServerObject(input.at(1));
				std::lock_guard<std::mutex> lock(server->serverSettings.mutex);
				try {
					std::fstream fileIn(server->serverAttributes.configFileLocation, std::ios::in);
					std::string contents((std::istreambuf_iterator<char>(fileIn)), std::istreambuf_iterator<char>());
					fileIn.close();
					nlohmann::json content = nlohmann::json::parse(contents);
					content["name"] = server->serverSettings.name;
					content["exec"] = server->serverSettings.exec;
					content["file"] = server->serverSettings.file;
					content["path"] = server->serverSettings.path;
					content["flags"] = server->serverSettings.flags;
					content["device"] = server->serverSettings.device;
					content["restartmins"] = server->serverSettings.restartMins;
					content["docommands"] = server->serverSettings.doCommands;
					content["custommsg"] = server->serverSettings.customMessage;
					content["chatkickregex"] = server->serverSettings.chatKickRegex;
					content["autoupdate"] = server->serverSettings.autoUpdateName + ' ' + server->serverSettings.autoUpdateVersion;
					content["counterintervalmax"] = std::to_string(server->serverSettings.counterInterval) + ' ' + std::to_string(server->serverSettings.counterMax);
					content["version"] = server->serverSettings.version;
					std::fstream fileOut(server->serverAttributes.configFileLocation, std::ios::out);
					fileOut << content.dump(4);
					fileOut.close();
					term.out<Info>(flexi_format("Saved server {}'s settings", input.at(1)));
				} catch (std::exception& e) {
					term.out<Error>(flexi_format("Error saving server settings: {}", e.what()));
				}
			} catch(...) {
				term.out<Error>(text.error.ServerSelectionInvalid);
			}
		} else {
			term.out<Error>("Not enough arguments provided");
		}
	} else if (input.at(0) == "exit") {
		term.out<Warning, NoEndline>("Are you sure you want to exit Hajime?");
		if (term.getYN()) {
			exit(0);
		}
	} else if (input.at(0) == "ee" && ee && text.language == "en") {
		term.out("https://www.youtube.com/watch?v=ccY25Cb3im0");
	} else if (input.at(0) == "ee" && ee && text.language == "es") {
		term.out("https://www.youtube.com/watch?v=iFClTRUnmKc");
	} else {
		term.out<Error>(text.error.InvalidCommand);
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
