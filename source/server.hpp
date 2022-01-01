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

	shared_ptr<Output> logObj;

	const string systems[8] = {"ext2", "ext3", "ext4", "vfat", "msdos", "f2fs", "ntfs", "fuseblk"};

	string readServerTerminal();
	void writeToServerTerminal(string input);
	void processTerminalBuffer(string input);
	void processServerCommand(string input);
	void mountDrive();
	void makeDir();
	void startProgram(string method);
	void readSettings(string confFile);
	void removeSlashesFromEnd(string& var);
	void processServerTerminal();
	int getPID();
	vector<string> toArray(string input);
	auto toPointerArray(vector<string> &strings);

	inline static string name, exec, file, path, command, flags, confFile, device, method, cmdline = "";

	inline static int slave_fd, fd, pid, uptime;

	#if defined(_WIN64) || defined(_WIN32)
	inline static STARTUPINFO si; // a variable that can specify parameters for windows created with it
	inline static PROCESS_INFORMATION pi; // can get process handle and pid from this
	#else
	inline static struct winsize w;
	#endif

	bool startedRfdThread = false;

	inline static std::atomic<bool> wantsLiveOutput; //you can't assign a value to this yet, so we give it a value before we use it

	inline static std::list<string> lines; //make this inline static so the program only has one copy of lines available
	//super duper important!!

	public:
		Server(shared_ptr<Output> tempObj);
		bool isRunning = false;
		void startServer(string confFile);
		void terminalAccessWrapper();
};
