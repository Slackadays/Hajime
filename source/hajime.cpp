// (c) 2021 Slackadays on GitHub
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

using std::cin;
using std::cout;
using std::endl;
using std::shared_ptr;
using std::make_shared;
using std::vector;

bool ee = false;
vector<std::shared_ptr<Server>> serverVec = {}; //create an array of individual server objects
vector<std::jthread> threadVec = {}; //create an array of thread objects
string logFile = "";
string hajConfFile = "";
string version;

bool readSettings();
void dividerLine();
void hajimeExit(int sig);
vector<string> getServerFiles();
vector<string> toVec(string input);
void processHajimeCommand(vector<string> input);
bool isUserPrivileged();

int main(int argc, char *argv[]) {
	//auto then = std::chrono::high_resolution_clock::now();
	atexit([]{
		dividerLine();
		#if defined(__APPLE__)
		exit(0);
		#else
		quick_exit(0);
		#endif
	});
	signal(SIGINT, hajimeExit);
	signal(SIGSEGV, [](int sig){
		cout << "Segmentation fault detected; exiting Hajime now" << endl;
		exit(0);
	});
	signal(SIGABRT, [](int sig){
		cout << "Hajime ending execution abnormally; exiting Hajime now" << endl;
		exit(0);
	});
	signal(SIGILL, [](int sig){
		cout << "Illegal instruction detected; try recompiling Hajime" << endl;
		exit(0);
	});
	signal(SIGFPE, [](int sig){
		cout << "Illegal math operation; exiting Hajime now" << endl;
		exit(0);
	});
	signal(SIGTERM, [](int sig){
		cout << "Termination requested; exiting Hajime now" << endl;
		exit(0);
	});
	#if defined(_WIN64) || defined (_WIN32)
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); //Windows terminal color compatibility
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	if (!SetConsoleMode(hOut, (dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT))) {
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
				hjlog.out<Error>(text.error.NotEnoughArgs);
				return 0;
			}
		}
	}
	bool bypassPriviligeCheck = false;
	for (int i = 1; i < argc; i++) {//start at i = 1 to improve performance because we will never find a flag at 0
		auto flag = [&i, &argv](auto ...fs){
			return (!strcmp(fs, argv[i]) || ...);
		};
		auto assignNextToVar = [&argc, &argv, &i](auto &var){
			if (i == (argc - 1)) {
				return false;
			} else {
				var = argv[(i + 1)];
				i++;
				return true;
			}
		}; //tries to assign the next argv argument to some variable; if it is not valid, then return an error
		if (flag("-f", "--server-file")) {
			if (!assignNextToVar(defaultServerConfFile)) {
				hjlog.out<Error>(text.error.NotEnoughArgs);
				return 0;
			}
		}
		if (flag("-hf", "--hajime-file")) {
			if (!assignNextToVar(hajDefaultConfFile)) {
				hjlog.out<Error>(text.error.NotEnoughArgs);
				return 0;
			}
		}
		if (flag("-ih", "--install-hajime-config")) { //can accept either no added file or an added file
			string tempHajConfFile;
			if (string var = "-"; assignNextToVar(var) && var[0] != '-') { //compare the next flag if present and check if it is a filename
				tempHajConfFile = var;
			} else {
				tempHajConfFile = hajDefaultConfFile;
			}
			wizard.wizardStep(tempHajConfFile, installer.installDefaultHajConfFile, text.warning.FoundHajConf, text.error.HajFileNotMade, text.language);
			return 0;
		}
		if (flag("-p", "--privileged")) {
			bypassPriviligeCheck = true;
		}
		if (flag("-s", "--install-default-server")) {
			wizard.wizardStep(defaultServerConfFile, installer.installNewServerConfigFile, text.warning.FoundServerConfPlusFile + defaultServerConfFile, text.error.ServerConfNotCreated, "", "server.jar");
			return 0;
		}
		if (flag("-S", "--install-service")) {
			installer.installStartupService("/etc/systemd/system/hajime.service");
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
			wizard.initialHajimeSetupAttended(hajDefaultConfFile, defaultServerConfFile);
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
		empty(logFile) ? hjlog.out<Info>(text.info.NoLogFile) : hjlog.init(logFile);
	} else {
		hjlog.out<Question>(text.question.DoSetupInstaller);
		switch (hjlog.getYN(text.option.AttendedInstallation, text.option.UnattendedInstallation, text.option.SkipSetup)) {
			case 1:
				dividerLine();
				wizard.initialHajimeSetupAttended(hajDefaultConfFile, defaultServerConfFile);
				hjlog.out<Question, NoEndline>(text.question.StartHajime);
				if (!hjlog.getYN()) {
					return 0;
				}
				break;
			case 2:
				wizard.initialHajimeSetupUnattended(hajDefaultConfFile, defaultServerConfFile);
				hjlog.out<Error>(text.error.OptionNotAvailable);
				break;
			case 3:
				return 0;
		}
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
		hjlog.out<Error>("Hajime must not be run by a privileged user");
		return 1;
	}
	#if !defined(_WIN64) && !defined(_WIN32)
	struct rlimit rlimits;
	if (getrlimit(RLIMIT_NOFILE, &rlimits) == -1) {
		hjlog.out<Error, Threadless>("Error getting resource limits; errno = " + std::to_string(errno));
	}
	rlimits.rlim_cur = rlimits.rlim_max; //resize soft limit to max limit; the max limit is a ceiling for the soft limit
	if (setrlimit(RLIMIT_NOFILE, &rlimits) == -1) {
		hjlog.out<Error, Threadless>("Error changing resource limits; errno = " + std::to_string(errno));
	}
	hjlog.out<Debug>("New soft file descriptor soft limit = " + to_string(rlimits.rlim_cur));
	#endif
	//std::cout << "This took " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - then).count() << " microseconds" << std::endl;
	//exit(0);
	for (const auto& serverIt : getServerFiles()) { //loop through all the server files found
		serverVec.emplace_back(std::make_shared<Server>()); //add a copy of server to use
		threadVec.emplace_back(std::jthread(&Server::startServer, serverVec.back(), serverIt)); //add a thread that links to startServer and is of the last server object added, use serverIt as parameter
	}
	hjlog.hajimeTerminal = true;
	while(true) {
		string command = "";
		std::getline(std::cin, command);
		std::cout << "\033[0m" << std::flush;
		if (command != "") {
			processHajimeCommand(toVec(command));
		} else {
			hjlog.out<Error>("Command must not be empty");
		}
	}
	return 0;
}

bool readSettings() {
	vector<string> settings{"version", "logfile", "debug", "threadcolors"};
	if (!fs::is_regular_file(hajDefaultConfFile)) {
		hjlog.out<Debug>(text.debug.HajDefConfNoExist1 + hajDefaultConfFile + text.debug.HajDefConfNoExist2);
		return 0;
	}
	vector<string> results = getVarsFromFile(hajDefaultConfFile, settings);
	for (vector<string>::iterator firstSetIterator = settings.begin(), secondSetIterator = results.begin(); firstSetIterator != settings.end() && secondSetIterator != results.end(); ++firstSetIterator, ++secondSetIterator) {
		auto setVar = [&](string name, string& tempVar){
			if (*firstSetIterator == name) {
				tempVar = *secondSetIterator;
			}
		};
		auto setVari = [&](string name, int& tempVar){
			if (*firstSetIterator == name) {
				try {
					tempVar = stoi(*secondSetIterator);
				} catch(...) {
					tempVar = 0;
				}
			}
		};
		hjlog.out<Debug>(text.debug.ReadingReadsettings);
		setVar(settings[0], version);
		setVar(settings[1], logFile);
		if (!hjlog.debug) {
			setVari(settings[2], hjlog.debug);
		}
		setVari(settings[3], hjlog.showThreadsAsColors);
	}
	hjlog.out<Debug>(text.debug.ReadReadsettings + hajDefaultConfFile);
	return 1;
}

void hajimeExit(int sig) {
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	static std::chrono::time_point<std::chrono::system_clock> then;
	if (std::chrono::duration_cast<std::chrono::seconds>(now - then).count() <= 3) {
		std::cout << "\b\b  " << std::endl;
		exit(0);
	} else {
		std::cout << "\b\b  " << std::endl << "\033[1mTry again within 3 seconds to exit Hajime" << std::flush;
	}
	then = std::chrono::system_clock::now();
}

void dividerLine() {
	#if defined(_WIN64) || defined(_WIN32)
	CONSOLE_SCREEN_BUFFER_INFO w;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &w);
	for (int i = 0; i < w.dwSize.X; i++) {
		std::cout << "─" << std::flush;
	}
	#else
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	for (int i = 0; i < w.ws_col; i++) {
		std::cout << "─" << std::flush;
	}
	#endif
	std::cout << std::endl;
}

vector<string> getServerFiles() {
	vector<string> results;
	for (const auto& file : fs::directory_iterator{fs::current_path()}) {
		if (std::regex_match(file.path().filename().string(), std::regex(".+\\.server(?!.+)", std::regex_constants::optimize | std::regex_constants::icase))) {
			results.emplace_back(file.path().filename().string());
		}
	}
	return results;
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
					hjlog.out<Error>(text.error.InvalidServerNumber);
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
					hjlog.out<Error>(text.error.ServerSelectionInvalid);
				}
			}
		} else {
			hjlog.out<Error>(text.error.NotEnoughArgs);
		}
	} else if (input[0] == "ee" && ee && text.language == "en") {
		hjlog.out("https://www.youtube.com/watch?v=ccY25Cb3im0");
	} else if (input[0] == "ee" && ee && text.language == "es") {
		hjlog.out("https://www.youtube.com/watch?v=iFClTRUnmKc");
	} else {
		hjlog.out<Error>(text.error.InvalidCommand);
		hjlog.out<Error>(text.error.InvalidHajCommand1);
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
