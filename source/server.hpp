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
	bool hasOutput = false, hasOutputUSB = false, hasMounted = false;

	int systemi = 0;

	std::error_code ec;

	shared_ptr<Output> hjlog;

	const string systems[8] = {"ext2", "ext3", "ext4", "vfat", "msdos", "f2fs", "ntfs", "fuseblk"};

	string formatWrapper(string input);
	string readFromServer();
	void writeToServerTerminal(string input);
	void processTerminalBuffer(string input);
	void processServerCommand(string input);
	void processPerfStats();
	void commandHajime();
	void commandTime();
	void commandHelp();
	void commandDie();
	void commandD20();
	void commandCoinflip();
	void commandDiscord();
	void commandName();
	void commandUptime();
	void commandRestart();
	void commandSystem();
	void commandPerf();
	string getOS();
	string getCPU();
	string getRAM();
	string getUptime();
	string getLoadavg();
	string getCPUusage();
	string getCPUmigs();
	string getLastCPU();
	string getRAMusage();
	string getIPC();
	string getIPS();
	string getContextSwitches();
	string getPagefaults();
	string getBranchInstructions();
	string getBranchMisses();
	string getCacheMisses();
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
	inline static bool doCommands;
	inline static bool silentCommands;

	inline static std::atomic<long long> CPUinstructions1m, CPUinstructions5m, CPUinstructions15m;
	inline static std::atomic<double> CPUpercent1m, CPUpercent5m, CPUpercent15m;
	inline static std::atomic<long long> CPUmigrations1m, CPUmigrations5m, CPUmigrations15m;
	inline static std::atomic<int> lastseenCPU;
	inline static std::atomic<double> RAMpercent1m, RAMpercent5m, RAMpercent15m;
	inline static std::atomic<long long> RAMbytes1m, RAMbytes5m, RAMbytes15m;
	inline static std::atomic<double> IPC1m, IPC5m, IPC15m;
	inline static std::atomic<double> IPS1m, IPS5m, IPS15m;
	inline static std::atomic<long long> contextSwitches1m, contextSwitches5m, contextSwitches15m;
	inline static std::atomic<long long> pageFaults1m, pageFaults5m, pageFaults15m;
	inline static std::atomic<long long> branchInstructions1m, branchInstructions5m, branchInstructions15m;
	inline static std::atomic<long long> branchMisses1m, branchMisses5m, branchMisses15m;
	inline static std::atomic<long long> cacheMisses1m, cacheMisses5m, cacheMisses15m;

	inline static string lastCommandUser;

	bool startedRfdThread = false;
	bool startedPerfThread = false;

	inline static std::atomic<bool> wantsLiveOutput; //you can't assign a value to this yet, so we give it a value before we use it

	inline static std::list<string> lines; //make this inline static so the program only has one copy of lines available
	//super duper important!!
	public:
		inline static string name, exec, file, path, command, flags, confFile, device, method, cmdline = "";
		explicit Server(shared_ptr<Output> tempObj);
		bool isRunning = false;
		void startServer(string confFile);
		void terminalAccessWrapper();
};
