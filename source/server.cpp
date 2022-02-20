#if defined(_WIN64) || defined(_WIN32)
#include <Windows.h>
#include <shellapi.h>
#pragma comment (lib, "Shell32")
#else
#include <unistd.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <fcntl.h>
#include <fstream>
#include <thread>
#include <list>
#include <atomic>
#include <cstring>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>
#include <errno.h>
#include <regex>
#include <ctime>
//#include <format>
#include <random>

#ifdef _MSC_VER
#if (_MSC_VER < 1928 || _MSVC_LANG <= 201703L) // msvc usually doesn't define __cplusplus to the correct value
	#define jthread thread
#endif
#elif (__cplusplus <= 201703L || defined(__APPLE__) || defined(__clang__)) //jthreads are only in C++20 and up and not supported by Clang yet
	#define jthread thread
#endif

#include "getvarsfromfile.hpp"
#include "server.hpp"
#include "output.hpp"

using std::shared_ptr;
using std::string;
using std::fstream;
using std::to_string;
using std::ofstream;
using std::ios;
using std::vector;
using std::cout;
using namespace std::chrono;

namespace fs = std::filesystem;
namespace ch = std::chrono;

void Server::startServer(string confFile) {
	hjlog.hajimeTerminal = false;
	try {
		if (fs::is_regular_file(confFile, ec)) {
			hjlog.out<Info>(text.info.ReadingServerSettings);
			readSettings(confFile);
		} else {
			hjlog.out<Error>(text.error.ServerFileNotPresent1 + confFile + text.error.ServerFileNotPresent2);
			return;
		}
		hjlog.out<Info>("----" + name + "----");
		hjlog.out<Info, NoEndline>(text.info.ServerFile + file + " | ");
		hjlog.out<None>(text.info.ServerPath + path);
		hjlog.out<Info, NoEndline>(text.info.ServerCommand + command + " | ");
		hjlog.out<None>(text.info.ServerMethod + method);
		hjlog.out<Info, NoEndline>(text.info.ServerDebug + to_string(hjlog.debug) + " | "); // ->out wants a string so we convert the debug int (converted from a string) back to a string
		hjlog.out<None>(text.info.ServerDevice + device);
		hjlog.out<Info, NoEndline>("Restart interval: " + to_string(restartMins) + " | ");
		hjlog.out<None>("Silent commands: " + to_string(silentCommands));
		hjlog.hajimeTerminal = true;
		if (!fs::is_regular_file(file)) {
			hjlog.out<Warning>(file + text.warning.FileDoesntExist);
		}
		while (true) {
			try {
				fs::current_path(path);
			} catch(...) {
				hjlog.out<Error>(text.error.CouldntSetPath);
			}
			#if !defined(_WIN64) && !defined(_WIN32)
			ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
			#endif
			if (((fs::current_path() == path) || (fs::current_path().string() == std::regex_replace(fs::current_path().string(), std::regex("^(.*)(?=(/|\\\\)" + path + "$)", std::regex_constants::optimize), ""))) && !isRunning) { //checks if we're in the right place and if the server file is there
				hjlog.out<Info>(text.info.StartingServer);
				startProgram(method);
				hjlog.out<Info>(text.info.ServerStartCompleted);
			}
			std::this_thread::sleep_for(std::chrono::seconds(2));
			if (!fs::is_directory(path, ec) && !fs::is_directory(fs::current_path().string() + '/' + path, ec) && !fs::is_directory(fs::current_path().string() + '\\' + path, ec)) { //if the desired path doesn't exist, make it
				makeDir();
			}
			fs::current_path(path, ec);
			if (!hasMounted) {
				mountDrive();
			}
			#if defined(_WIN64) || defined(_WIN32)
			DWORD code;
			//if (GetExitCodeProcess(pi.hProcess, &code); code == STILL_ACTIVE) { //alternative method
			if (WAIT_TIMEOUT == WaitForSingleObject(pi.hProcess, 0)) {
			#else
			if (getPID() != 0) { //getPID looks for a particular keyword in /proc/PID/cmdline that signals the presence of a server
			#endif
				std::this_thread::sleep_for(std::chrono::seconds(3));
				if (!isRunning) {
					hjlog.out<Info>(text.info.ServerIsRunning);
					isRunning = true;
					hasMounted = true;
				}
			} else {
				isRunning = false;
				hjlog.out<Warning>(text.warning.IsRunningFalse);
				#if defined(_WIN64) || defined(_WIN32)
				// close handles
				// the order MATTERS here, you need to close the one given to the child first (i think)
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				CloseHandle(inputread);
				CloseHandle(inputwrite);
				CloseHandle(outputwrite);
				CloseHandle(outputread);
				#endif
			}
			updateUptime();
			processAutoRestart();
		}
	} catch(string& s) {
		hjlog.out<>(s);
	}
 catch(...) { //error handling
		hjlog.out<Error>(text.error.Generic);
	}
}

vector<string> Server::toArray(string input) {
	vector<string> flagVector;
	vector<string> addToEndVector;
	string temp = "";
	string execFile;
	execFile = path + '/' + exec;
	flagVector.push_back(execFile.c_str()); //convert the execFile string to a c-style string that the exec command will understand
	for (int i = 0; i < input.length(); temp = "") {
		while (i < input.length() && input[i] == ' ') { //skip any leading whitespace
			i++;
		}
		while (i < input.length() && input[i] != ' ') { //add characters to a temp variable that will go into the vector
			temp += input[i];
			i++;
		}
		while (i < input.length() && input[i] == ' ') { //skip any trailing whitespace
			i++;
		}
		flagVector.push_back(temp); //add the finished flag to the vector of flags
		hjlog.out<Debug>(text.debug.flag.VecInFor + flagVector[0]);
	}
	flagVector.push_back(file.c_str()); //add the file that we want to execute by exec to the end
	flagVector.push_back("--nogui");
	hjlog.out<Debug>(text.debug.flag.VecOutFor + flagVector[0]);
	return flagVector;
}

auto Server::toPointerArray(vector<string> &strings) {
	vector<char*> pointers; //the pointer array that we will pass to the exec command
	for (auto &string : strings) { //loop over the whole flag string vector
		pointers.push_back(string.data()); //give the pointer array an address to a c-style string from the flag array
	}
	pointers.push_back(nullptr); //add a null pointer to the end because the exec command is from c
	return pointers;
}

void Server::startProgram(string method = "new") {
	uptime = 0;
	said15MinRestart = false;
	said5MinRestart = false;
	timeStart = std::chrono::steady_clock::now();
	if (!isRunning) {
		hjlog.out<Info>(text.info.TryingToStartProgram);
		fs::current_path(path);
		fs::remove("world/session.lock"); //session.lock will be there if the server didn't shut down properly
		lines.clear(); // clear output from previous session

		if (method == "old") {
			hjlog.out<Debug>(text.debug.UsingOldMethod);
			int returnVal = system(command.c_str()); //convert the command to a c-style string, execute the command
			if (returnVal == 0 || returnVal == -1) {
				hjlog.out<Error>("system() error");
			}
		} else if (method == "new") {
			hjlog.out<Debug>(text.debug.UsingNewMethod);
			#if defined(_WIN64) || defined (_WIN32)
			SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
			saAttr.bInheritHandle = TRUE;
			saAttr.lpSecurityDescriptor = NULL;
			if (!CreatePipe(&outputread, &outputwrite, &saAttr, 0) || !CreatePipe(&inputread, &inputwrite, &saAttr, 0))
			{
				hjlog.out<Error>(text.error.CreatingPipe);
				return;
			}
			ZeroMemory(&si, sizeof(si)); //ZeroMemory fills si with zeroes
			si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
			si.hStdInput = inputread;
			si.hStdOutput = outputwrite;
			si.hStdError = outputwrite;
			si.cb = sizeof(si); //si.cb = size of si
			ZeroMemory(&pi, sizeof(pi));
			// createprocessa might cause an error if commandline is const
			char* tempflags = new char[flags.size() + 1]; // +1 for null character at the end
			strncpy_s(tempflags, flags.size() + 1, flags.c_str(), flags.size() + 1); //save flags.c_str() to tempflags so that CreateProcessA can modify the variable
			CreateProcessA(NULL, tempflags, NULL, NULL, TRUE, CREATE_NO_WINDOW | BELOW_NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi); // create process with new console
			delete[] tempflags; //we don't need tempflags any more, so free memory and prevent a memory leak (maybe :)
			if (!startedRfdThread) {
				std::jthread rfd(&Server::processServerTerminal, this);
				rfd.detach();
				startedRfdThread = true;
			}
			if (!startedPerfThread) {
				std::jthread perfThread(&Server::processPerfStats, this);
				perfThread.detach();
				startedPerfThread = true;
			}
			#else
			hjlog.out<Debug>(text.debug.Flags + flags);
			auto flagTemp = toArray(flags);
			auto flagArray = toPointerArray(flagTemp);
			hjlog.out<Debug>(text.debug.flag.Array0 + (string)flagArray[0]);
			hjlog.out<Debug>(text.debug.flag.Array1 + (string)flagArray[1]);
			wantsLiveOutput = false;
			fd = posix_openpt(O_RDWR);
			grantpt(fd);
			unlockpt(fd);
			slave_fd = open(ptsname(fd), O_RDWR);
			pid = fork();
			if (pid == 0) { //this is the child
				hjlog.out<Debug>("fork() = 0");
				close(fd);
				struct termios old_sets; //something to save the old settings to
				struct termios new_sets;
				tcgetattr(slave_fd, &old_sets); //save current temrinal settings to old_sets
				new_sets = old_sets;
				cfmakeraw (&new_sets); //set terminal to raw mode (disable character preprocessing)
				tcsetattr (slave_fd, TCSANOW, &new_sets); //assign the new settings to the terminal
				close(0); //get rid of the old cin
				close(1); //get rid of the old cout
				close(2); //get rid of the old cerr
				dup2(slave_fd, 0); //assign the slave fd to cin
				dup2(slave_fd, 1); //ditto, cout
				dup2(slave_fd, 2); //ditto, cerr
				close(slave_fd); //close out the fd we used just for assigning to new fds
				setsid(); //create a new session without a terminal
				ioctl(slave_fd, TIOCSCTTY, 0); //assign the terminal of to the current program
				//ioctl(0, TIOCSCTTY, 0); etc
				execvp(exec.c_str(), flagArray.data());
				//execlp("bc", "/bc", NULL); //use this for testing
				exit(0);
			} else { //this is the parent
				hjlog.out<Debug>("fork() != 0");
				if (!startedRfdThread) {
					std::jthread rfd(&Server::processServerTerminal, this);
					rfd.detach();
					startedRfdThread = true;
				}
				if (!startedPerfThread) {
					std::jthread perfThread(&Server::processPerfStats, this);
					perfThread.detach();
					startedPerfThread = true;
				}
				close(slave_fd);
				std::this_thread::sleep_for(std::chrono::seconds(4));
				std::fstream cmdl;
				cmdl.open("/proc/" + to_string(pid) + "/cmdline", std::fstream::in);
				//std::cout << "opening cmdline file for pid " << pid << " at /proc/" << to_string(pid) << "/cmdline" << std::endl;
				getline(cmdl, cmdline);
				//std::cout << "cmdline = " << cmdline << std::endl;
				cmdl.close();
			}
			#endif
		} else {
			hjlog.out<Error>(text.error.MethodNotValid);
		}
			hasMounted = true;
	}
}

void Server::makeDir() {
	hjlog.out<Info>(text.info.CreatingDirectory);
	if (!fs::create_directory(path, ec)) {
		hjlog.out<Error>(text.error.CreatingDirectory);
	}
}

void Server::mountDrive() {
	#if defined(_WIN64) || defined(_WIN32) //Windows doesn't need drives to be mounted manually
	hjlog.out<Info>(text.info.POSIXdriveMount);
	hasMounted = true;
	#else
	hjlog.out<Info>(text.info.TryingMount);
	if (!fs::is_empty(path, ec)) { //if there are files, then we don't want to mount there
		hjlog.out<Error>(text.error.FilesInPath);
		return;
	} else {
		string error;
		#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
		//BSDs have different mount() parameters
		if (!mount(systems[systemi].c_str(), path.c_str(), 0, const_cast<char*>(device.c_str()))) { //cast a c-style device string to a constant char*
		#else
		if (!mount(device.c_str(), path.c_str(), systems[systemi].c_str(), 0, "")) {
		//brute-forces every possible filesystem because mount() depends on it being the right one
		#endif
			hjlog.out<Info>(text.info.DeviceMounted);
			hasMounted = true;
			systemi = 0; //reset in case it needs to mount again
		} else {
			int errsv = errno; //errno is the POSIX error code, save errno to a dummy variable to stop it from getting tainted
			if (systemi == 6) {
				switch (errsv) {
					case 1:
						error = text.eno.NotPermitted;
						break;
					case 2:
						error = text.eno.NoFileOrDir;
						break;
					case 13:
						error = text.eno.PermissionDenied;
						break;
					case 5:
						error = text.eno.InOut;
						break;
					case 12:
						error = text.eno.Memory;
						break;
					case 11:
						error = text.eno.Unavailable;
						break;
					case 14:
						error = text.eno.Address;
						break;
					case 15:
						error = text.eno.BlockDev;
						break;
					case 16:
						error = text.eno.Busy;
						break;
					case 21:
						error = text.eno.Directory;
						break;
					case 22:
						error = text.eno.BadArgs;
						break;
					case 19:
						error = text.eno.UnknownDev;
						break;
					default:
						error = text.eno.UnknownGeneric;
				}
				if (!hasOutputUSB) {
					hjlog.out<Error>(text.error.Mount + error);
					hasOutputUSB = true;
					systemi = 0;
				}
				hjlog.out<Error>(text.error.Code + to_string(errsv));
			}
		}
		if (systemi < 6) {
			hjlog.out<Info>(text.info.TryingFilesystem1 + systems[systemi] + text.info.TryingFilesystem2);
			systemi++; //increment the filesystem
		}
	}
	#endif
}

void Server::removeSlashesFromEnd(string& var) {
	while (!var.empty() && ((var[var.length() - 1] == '/') || (!var.empty() && var[var.length() - 1] == '\\'))) {
		var.pop_back();
	}
}

void Server::readSettings(const string confFile) {
	auto eliminateSpaces = [&](auto& ...var) {
		((var = std::regex_replace(var, std::regex("\\s+(?![^#])", std::regex_constants::optimize), "")), ...);
	};
	vector<string> settings {"name", "exec", "file", "path", "command", "flags", "method", "device", "restartmins", "silentcommands", "commands", "custommsg", "chatkickregex"};
	vector<string> results = getVarsFromFile(confFile, settings);
	for (const auto& it : results) {
		hjlog.out<Debug>(it);
	}
	for (vector<string>::iterator firstSetIterator = settings.begin(), secondSetIterator = results.begin(); firstSetIterator != settings.end(); ++firstSetIterator, ++secondSetIterator) {
		auto setVar = [&](auto name, auto& tempVar) {
			if (*firstSetIterator == name) {
				tempVar = *secondSetIterator;
			}
		};
		auto setVari = [&](auto name, auto& tempVar) {
			if (*firstSetIterator == name) {
				try {
					eliminateSpaces(*secondSetIterator);
					tempVar = stoi(*secondSetIterator);
				} catch(...) {
					tempVar = 0;
				}
			}
		};
		setVar(settings[0], name);
		setVar(settings[1], exec);
		setVar(settings[2], file);
		setVar(settings[3], path);
		setVar(settings[4], command);
		setVar(settings[5], flags);
		setVar(settings[6], method);
		setVar(settings[7], device);
		setVari(settings[8], restartMins);
		setVari(settings[9], silentCommands);
		setVari(settings[10], doCommands);
		setVar(settings[11], customMessage);
		setVar(settings[12], chatKickRegex);
		hjlog.out<Debug>(text.debug.ReadingReadsettings);
	}
	hjlog.addServerName(name); //send the name of the server name to hjlog so that it can associate a name with a thread id
	if (device == "") {
		hjlog.out<Info>(text.info.NoMount);
		hasMounted = true;
	}
	hjlog.out<Debug>(text.debug.ValidatingSettings);
	auto remSlash = [&](auto& ...var) {
		(removeSlashesFromEnd(var), ...);
	};
	remSlash(file, path, device, exec);
	eliminateSpaces(file, path, device, exec, method, flags, name);
	#if defined(_WIN64) || defined(_WIN32)
	flags = exec + ' ' + flags + " -Dfile.encoding=UTF-8 " + file + " nogui";
	#endif
}

int Server::getPID() {
	#if defined(_WIN64) || defined(_WIN32)
	hjlog.out<Warning>(text.warning.TestingWindowsSupport);
	return pi.dwProcessId; // honestly I don't think this is necessary but whatever
	#else
	if (method == "new") {
		if (!kill(pid, 0)) {
			std::fstream cmdl;
			cmdl.open("/proc/" + to_string(pid) + "/cmdline", std::fstream::in);
			string temp = "";
			getline(cmdl, temp);
			//std::cout << "temp is " << temp << std::endl;
			cmdl.close();
			if (temp == cmdline) {
				return 1;
			} else {
				return 0;
			}
		} else {
			//int errnum = errno;
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
			if (str.length() > 0) { //if a cmdline is not used, there will be nothing
				if (str.find("SCREEN") != string::npos) { //look for a keyword in cmdline, string::npos is a special value (-1) that needs to be used
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
