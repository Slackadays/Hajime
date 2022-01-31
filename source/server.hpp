#if defined(_WIN64) || defined (_WIN32)
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
#include <chrono>

#include "getvarsfromfile.hpp"
#include "output.hpp"
#include "languages.hpp"

using std::shared_ptr;
using std::string;
using std::fstream;
using std::to_string;
using std::ofstream;
using std::ios;
using std::vector;
using std::cout;

class Server {
	bool hasOutput, hasOutputUSB, hasMounted = false;

	int systemi = 0;

	std::error_code ec;

	shared_ptr<Output> hjlog;

	const string systems[8] = {"ext2", "ext3", "ext4", "vfat", "msdos", "f2fs", "ntfs", "fuseblk"};

	string formatWrapper(string input);
	string readFromServer();
	void writeToServerTerminal(string input);
	void processTerminalBuffer(string input);
	void processServerCommand(string input);
	void commandHajime();
	void commandTime();
	void commandHelp();
	void commandDie();
	void commandCoinflip();
	void commandDiscord();
	void commandName();
	void commandUptime();
	void commandRestart();
	void commandSystem();
	string getOS();
	string getCPU();
	string addNumberColors(string input);
	void processRestartAlert(string input);
	void mountDrive();
	void makeDir();
	void updateUptime();
	void processAutoRestart();
	void startProgram(string method);
	void readSettings(string confFile);
	void removeSlashesFromEnd(string& var);
	void processServerTerminal();
	int getPID();
	vector<string> toArray(string input);
	auto toPointerArray(vector<string> &strings);

	#if defined(_WIN64) || defined(_WIN32)
	inline static STARTUPINFO si; // a variable that can specify parameters for windows created with it
	inline static PROCESS_INFORMATION pi; // can get process handle and pid from this
	inline static HANDLE inputread, inputwrite, outputread, outputwrite; // pipes for reading/writing
	#else
	inline static std::atomic<int> slave_fd, fd, pid;
	inline static struct winsize w;
	#endif

	inline static std::atomic<long int> restartMins;
	inline static std::atomic<long int> uptime;
	inline static std::chrono::time_point<std::chrono::steady_clock> timeStart;
	inline static std::chrono::time_point<std::chrono::steady_clock> timeCurrent;

	inline static bool said15MinRestart;
	inline static bool said5MinRestart;

	bool startedRfdThread = false;

	inline static std::atomic<bool> wantsLiveOutput; //you can't assign a value to this yet, so we give it a value before we use it

	inline static std::list<string> lines; //make this inline static so the program only has one copy of lines available
	//super duper important!!
	public:
		inline static string name, exec, file, path, command, flags, confFile, device, method, cmdline = "";
		Server(shared_ptr<Output> tempObj);
		bool isRunning = false;
		void startServer(string confFile);
		void terminalAccessWrapper();
};
