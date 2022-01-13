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
#elif (__cplusplus <= 201703L || defined(__APPLE__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__clang__)) //jthreads are only in C++20 and up and not supported by Clang yet
	#define jthread thread
#endif

#include "getvarsfromfile.hpp"
#include "server.hpp"

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

Server::Server(shared_ptr<Output> tempObj) {
	hjlog = tempObj;
}

void Server::processTerminalBuffer(string input) {
	while (lines.size() >= 100000) {
		//std::cout << "Popping, ws.row = " << w.ws_row << std::endl;
		lines.pop_front();
		//std::cout << "lines size = " << (unsigned short)lines.size() << w.ws_row << std::endl;
	}
	input = std::regex_replace(input, std::regex(">\\.\\.\\.\\.", std::regex_constants::optimize), ">"); //replace ">...." with ">" because this shows up in the temrinal output
	//std::cout << "Pushing back" << std::endl;
	lines.push_back(input);
	if (wantsLiveOutput) {
		std::cout << input << std::flush;
	}
}

void Server::processServerCommand(string input) {
	if (std::regex_search(input, std::regex("\\.hajime(?![\\w])", std::regex_constants::optimize))) {
		string hajInfo = "tellraw @a [\"§6[Hajime] §fThis server is using \",{\"text\":\"Hajime 0.1.9\",\"underlined\":true,\"color\":\"aqua\",\"clickEvent\":{\"action\":\"open_url\",\"value\":\"https:\/\/hajime.sh\"}}]";
		writeToServerTerminal(hajInfo);
		return;
	}
	if (std::regex_search(input, std::regex("\\.time(?![\\w])", std::regex_constants::optimize))) {
		std::time_t timeNow = std::time(nullptr);
		string stringTimeNow = std::asctime(std::localtime(&timeNow));
		stringTimeNow.erase(std::remove(stringTimeNow.begin(), stringTimeNow.end(), '\n'), stringTimeNow.end());
		string hajInfo = "tellraw @a \"§6[Hajime]§f This server's local time is §b" + stringTimeNow + '\"';
		writeToServerTerminal(hajInfo);
		return;
	}
	if (std::regex_search(input, std::regex("\\.h(elp){0,1}(?![\\w])", std::regex_constants::optimize))) {
		writeToServerTerminal(tellrawWrapper("§6[Hajime]§f Roll over a command to show its action."));
		writeToServerTerminal(tellrawWrapper("[{\"text\":\".coinflip, \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§bFlip a coin.\"}},{\"text\":\".die, \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§bRoll a die.\"}},{\"text\":\".discord, \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§bShow the Hajime Discord invite.\"}},{\"text\":\".hajime, \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§bShow the Hajime version.\"}},{\"text\":\".h, help, \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§bShow this help message.\"}},{\"text\":\".time\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§bShow the server's local time and date.\"}}]"));
		return;
	}
	if (std::regex_search(input, std::regex("\\.die(?![\\w])", std::regex_constants::optimize))) {
		std::random_device rand;
		std::uniform_int_distribution<int> die(1, 6);
		writeToServerTerminal(tellrawWrapper("§6[Hajime]§f Rolled a die and got §b" + std::to_string(die(rand))));
		//switch this to C++20 format when it becomes supported
		return;
	}
	if (std::regex_search(input, std::regex("\\.coinflip(?![\\w])", std::regex_constants::optimize))) {
		std::random_device rand;
		std::uniform_int_distribution<int> flip(1, 2);
		if (flip(rand) == 1) {
			writeToServerTerminal(tellrawWrapper("§6[Hajime]§f Flipped a coin and got §bheads"));
		} else {
			writeToServerTerminal(tellrawWrapper("§6[Hajime]§f Flipped a coin and got §btails"));
		}
		return;
	}
	if (std::regex_search(input, std::regex("\\.discord(?![\\w])", std::regex_constants::optimize))) {
		string hajInfo = "[\"§6[Hajime] §fJoin the official Hajime Discord at \",{\"text\":\"https://discord.gg/J6asnc3pEG\",\"underlined\":true,\"color\":\"aqua\",\"clickEvent\":{\"action\":\"open_url\",\"value\":\"https://discord.gg/J6asnc3pEG\"}}]";
		writeToServerTerminal(tellrawWrapper(hajInfo));
		return;
	}
	if (std::regex_search(input, std::regex("\\.name(?![\\w])", std::regex_constants::optimize))) {
		string hajInfo = "§6[Hajime]§f This server's name is §b" + name;
		writeToServerTerminal(tellrawWrapper(hajInfo));
		return;
	}
	if (std::regex_search(input, std::regex("\\.uptime(?![\\w])", std::regex_constants::optimize))) {
		string hajInfo = "§6[Hajime]§f This server's uptime is §b" + std::to_string(uptime) + "§f minutes (§b" + std::to_string(uptime / 60.0) + "§f hours)";
		writeToServerTerminal(tellrawWrapper(hajInfo));
		return;
	}
}

string Server::tellrawWrapper(string input) {
	string output;
	if (input.front() == '[' && input.back() == ']') {
		output = "tellraw @a " + input;
	} else {
		output = "tellraw @a \"" + input + "\"";
	}
	return output;
}

void Server::writeToServerTerminal(string input) {
	input += "\n"; //this is the delimiter of the server command
	#if defined(_WIN64) || defined(_WIN32)
	DWORD byteswritten;
	if (!WriteFile(inputwrite, input.c_str(), input.size(), &byteswritten, NULL)) {// write to input pipe
		hjlog->out("Unable to write to pipe", Warning);
	} else if (byteswritten != input.size()) {
		hjlog->out("Wrote " + std::to_string(byteswritten) + "bytes, expected " + std::to_string(input.size()), Warning);
	}
	#else
	write(fd, input.c_str(), input.length());
	#endif
}

void Server::processServerTerminal() {
	while (true) {
		string terminalOutput = readFromServer();
		processServerCommand(terminalOutput);
		processTerminalBuffer(terminalOutput);
	}
}

string Server::readFromServer() {
	char input[1000];
	#ifdef _WIN32
	DWORD length = 0;
	if (!ReadFile(outputread, input, 1000, &length, NULL))
	{
		hjlog->out("ReadFile failed (unable to read from pipe)", Warning);
		return std::string();
	}
	#else
	int length;
	length = read(fd, input, sizeof(input));
	if (length == -1) {
		hjlog->out("read() errno = " + to_string(errno), Debug);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	#endif
	std::string output;
	for (int i = 0; i < length; i++) {
		output.push_back(input[i]);
	}
	return output;
}

void Server::terminalAccessWrapper() {
	hjlog->normalDisabled = true;
	std::cout << "----->" << name << std::endl;
	wantsLiveOutput = true;
	for (const auto& it : lines) {
		std::cout << it << std::flush;
	}
	while (true) {
		std::string user_input = "";
		std::getline(std::cin, user_input); //getline allows for spaces
		if (user_input == ".d") {
			wantsLiveOutput = false;
			break;
		} else if (user_input[0] == '.') {
			std::cout << text.error.InvalidCommand << std::endl;
			std::cout << text.error.InvalidServerCommand1 << std::endl;
		} else {
			writeToServerTerminal(user_input); //write to the master side of the pterminal with user_input converted into a c-style string
		}
	}
	std::cout << "Hajime<-----" << std::endl;
	hjlog->normalDisabled = false;
}

void Server::startServer(string confFile) {
	try {
		if (fs::is_regular_file(confFile, ec)) {
			hjlog->out(text.info.ReadingServerSettings, Info);
			readSettings(confFile);
		} else {
			hjlog->out(text.error.ServerFileNotPresent1 + confFile + text.error.ServerFileNotPresent2, Error);
			return;
		}
		hjlog->out("----" + name + "----", Info);
		hjlog->out(text.info.ServerFile + file + " | ", Info, 0, 0);
		hjlog->out(text.info.ServerPath + path, None);
		hjlog->out(text.info.ServerCommand + command + " | ", Info, 0, 0);
		hjlog->out(text.info.ServerMethod + method, None);
		hjlog->out(text.info.ServerDebug + to_string(hjlog->debug) + " | ", Info, 0, 0); // ->out wants a string so we convert the debug int (converted from a string) back to a string
		hjlog->out(text.info.ServerDevice + device, None);
		if (!fs::is_regular_file(file)) {
			hjlog->out(file + " doesn't exist.", Warning);
		}
		while (true) {
			try {
				fs::current_path(path);
			} catch(...) {
				hjlog->out(text.error.CouldntSetPath, Error);
			}
			#if !defined(_WIN64) && !defined(_WIN32)
			ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
			#endif
			if (((fs::current_path() == path) || (fs::current_path().string() == std::regex_replace(fs::current_path().string(), std::regex("^(.*)(?=(\/||\\\\)" + path + "$)", std::regex_constants::optimize), ""))) && !isRunning) { //checks if we're in the right place and if the server file is there
				hjlog->out(text.info.StartingServer, Info);
				startProgram(method);
				hjlog->out(text.info.ServerStartCompleted, Info);
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
					hjlog->out(text.info.ServerIsRunning, Info);
					isRunning = true;
					hasMounted = true;
				}
			} else {
				isRunning = false;
				hjlog->out(text.warning.IsRunningFalse, Warning);
				#if defined(_WIN64) || defined(_WIN32)
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				//CloseHandle(inputread); //commented these out because they mess up server restarting
				//CloseHandle(inputwrite);
				//CloseHandle(outputread);
				//CloseHandle(outputwrite);
				#endif
			}
		}
	} catch(string s) {
		hjlog->out(s);
	}
 catch(...) { //error handling
		hjlog->out(text.error.Generic, Error);
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
		if (!std::regex_search(temp, std::regex("nogui", std::regex_constants::optimize))) { //--nogui has to come at the end
			flagVector.push_back(temp); //add the finished flag to the vector of flags
		} else {
			addToEndVector.push_back(temp); //add an end-dependent flag to this special vector
		}
		hjlog->out(text.debug.FlagVecInFor + flagVector[0], Debug);
	}
	flagVector.push_back(file.c_str()); //add the file that we want to execute by exec to the end
	for (const auto& it : addToEndVector) { //tack on the end-dependent flags that have to come after the file we want to run
		flagVector.push_back(it);
	}
	hjlog->out(text.debug.FlagVecOutFor + flagVector[0], Debug);
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
	uptime = 70; //placeholder
	if (!isRunning) {
		hjlog->out(text.info.TryingToStartProgram, Info);
		fs::current_path(path);
		fs::remove("world/session.lock"); //session.lock will be there if the server didn't shut down properly
		if (method == "old") {
			hjlog->out(text.debug.UsingOldMethod, Debug);
			int returnVal = system(command.c_str()); //convert the command to a c-style string, execute the command
		} else if (method == "new") {
			hjlog->out(text.debug.UsingNewMethod, Debug);
			#if defined(_WIN64) || defined (_WIN32)
			SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
			saAttr.bInheritHandle = TRUE;
			saAttr.lpSecurityDescriptor = NULL;
			if (!CreatePipe(&outputread, &outputwrite, &saAttr, 0) || !CreatePipe(&inputread, &inputwrite, &saAttr, 0))
			{
				hjlog->out("Error creating pipe", Error);
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
			#else
			hjlog->out(text.debug.Flags + flags, Debug);
			auto flagTemp = toArray(flags);
			auto flagArray = toPointerArray(flagTemp);
			hjlog->out(text.debug.FlagArray0 + (string)flagArray[0], Debug);
			hjlog->out(text.debug.FlagArray1 + (string)flagArray[1], Debug);
			wantsLiveOutput = false;
			fd = posix_openpt(O_RDWR);
			grantpt(fd);
			unlockpt(fd);
			slave_fd = open(ptsname(fd), O_RDWR);
			pid = fork();
			if (pid == 0) {
				hjlog->out("This is the child.", Debug);
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
			} else {
				hjlog->out("This is the parent.", Debug);
				int length = 0;
				if (!startedRfdThread) {
					std::jthread rfd(&Server::processServerTerminal, this);
					rfd.detach();
					startedRfdThread = true;
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
			hjlog->out(text.error.MethodNotValid, Error);
		}
			hasMounted = true;
	}
}

void Server::makeDir() {
	hjlog->out(text.info.CreatingDirectory, Info);
	if (!fs::create_directory(path, ec)) {
		hjlog->out(text.error.CreatingDirectory, Error);
	}
}

void Server::mountDrive() {
	#if defined(_WIN64) || defined(_WIN32) //Windows doesn't need drives to be mounted manually
	hjlog->out(text.info.POSIXdriveMount, Info);
	hasMounted = true;
	#else
	hjlog->out(text.info.TryingMount, Info);
	if (!fs::is_empty(path, ec)) { //if there are files, then we don't want to mount there
		hjlog->out(text.error.FilesInPath, Error);
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
			hjlog->out(text.info.DeviceMounted, Info);
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
					hjlog->out(text.error.Mount + error, Error);
					hasOutputUSB = true;
					systemi = 0;
				}
				hjlog->out(text.error.Code + to_string(errsv), Error);
			}
		}
		if (systemi < 6) {
			hjlog->out(text.info.TryingFilesystem1 + systems[systemi] + text.info.TryingFilesystem2, Info);
			systemi++; //increment the filesystem
		}
	}
	#endif
}

void Server::removeSlashesFromEnd(string& var) {
	while (!var.empty() && ((var[var.length() - 1] == '/') || (var[var.length() - 1] == '\\'))) {
		var.pop_back();
	}
}

void Server::readSettings(string confFile) {
	vector<string> settings {"name", "exec", "file", "path", "command", "flags", "method", "device"};
	vector<string> results = getVarsFromFile(confFile, settings);
	for (const auto& it : results) {
		hjlog->out(it, Debug);
	}
	for (vector<string>::iterator firstSetIterator = settings.begin(), secondSetIterator = results.begin(); firstSetIterator != settings.end(); ++firstSetIterator, ++secondSetIterator) {
		auto setVar = [&](string name, string& tempVar){if (*firstSetIterator == name) {tempVar = *secondSetIterator;}};
		setVar(settings[0], name);
		setVar(settings[1], exec);
		setVar(settings[2], file);
		setVar(settings[3], path);
		setVar(settings[4], command);
		setVar(settings[5], flags);
		setVar(settings[6], method);
		setVar(settings[7], device);
			hjlog->out(text.debug.ReadingReadsettings, Debug);
	}
	hjlog->addServerName(name); //send the name of the server name to hjlog so that it can associate a name with a thread id
	if (device == "") {
		hjlog->out(text.info.NoMount, Info);
		hasMounted = true;
	}
	hjlog->out(text.debug.ValidatingSettings, Debug);
	auto remSlash = [&](auto& ...var){(removeSlashesFromEnd(var), ...);};
	remSlash(file, path, device, exec);
	#if defined(_WIN64) || defined(_WIN32)
	flags = exec + ' ' + flags + ' ' + file + " nogui";
	#endif
}

int Server::getPID() {
	#if defined(_WIN64) || defined(_WIN32)
	hjlog->out(text.warning.TestingWindowsSupport, Warning);
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
			int errnum = errno;
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
