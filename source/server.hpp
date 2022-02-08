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
	bool hasOutputUSB = false, hasMounted = false;

	int systemi = 0;

	std::error_code ec;

	const string systems[8] = {"ext2", "ext3", "ext4", "vfat", "msdos", "f2fs", "ntfs", "fuseblk"};

	#if defined(__linux__)
	vector<long> getProcessChildPids(long pid);
	void setupCounter(auto& s);
	void createCounters(vector<struct pcounter*>& counters, const vector<long>& pids);
	void cullCounters(vector<struct pcounter*>& counters, const vector<long>& pids);
	#endif

	string formatWrapper(string input);
	string readFromServer();
	void writeToServerTerminal(string input);
	void processTerminalBuffer(string input);
	void processServerCommand(string input);
	void processPerfStats();
	void updateCPUusage(std::list<long long>& CPUreadings);
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
	string getRAMusage();
	string getIPC();
	string getIPS();
	string getCPS();
	string getContextSwitches();
	string getStalledCyclesBackend();
	string getStalledCyclesFrontend();
	string getBranchMisses();
	string getCacheMisses();
	string getAlignmentFaults();
	string getEmulationFaults();
	string getMinorPagefaults();
	string getMajorPagefaults();
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
	STARTUPINFO si; // a variable that can specify parameters for windows created with it
	PROCESS_INFORMATION pi; // can get process handle and pid from this
	HANDLE inputread, inputwrite, outputread, outputwrite; // pipes for reading/writing
	#else
	int slave_fd, fd, pid;
	struct winsize w;
	#endif

	long int restartMins;
	long int uptime;
	std::chrono::time_point<std::chrono::steady_clock> timeStart;
	std::chrono::time_point<std::chrono::steady_clock> timeCurrent;

	bool said15MinRestart;
	bool said5MinRestart;
	bool doCommands;
	bool silentCommands;

	double RAMpercent1m, RAMpercent5m, RAMpercent15m;
	long long CPUjiffies, PIDjiffies;
	long long CPUcycles1m, CPUcycles5m, CPUcycles15m;
	long long CPUinstructions1m, CPUinstructions5m, CPUinstructions15m;
	long long CPUpercent1m, CPUpercent5m, CPUpercent15m;
	long long CPUmigrations1m, CPUmigrations5m, CPUmigrations15m;
	long long RAMbytes1m, RAMbytes5m, RAMbytes15m;
	long long contextSwitches1m, contextSwitches5m, contextSwitches15m;
	long long pageFaults1m, pageFaults5m, pageFaults15m;
	long long branchInstructions1m, branchInstructions5m, branchInstructions15m;
	long long branchMisses1m, branchMisses5m, branchMisses15m;
	long long cacheMisses1m, cacheMisses5m, cacheMisses15m;
	long long cacheReferences1m, cacheReferences5m, cacheReferences15m;
	long long stalledCyclesFrontend1m, stalledCyclesFrontend5m, stalledCyclesFrontend15m;
	long long stalledCyclesBackend1m, stalledCyclesBackend5m, stalledCyclesBackend15m;
	long long alignmentFaults1m, alignmentFaults5m, alignmentFaults15m;
	long long emulationFaults1m, emulationFaults5m, emulationFaults15m;
	long long minorPagefaults1m, minorPagefaults5m, minorPagefaults15m;
	long long majorPagefaults1m, majorPagefaults5m, majorPagefaults15m;

	string lastCommandUser;

	bool startedRfdThread = false;
	bool startedPerfThread = false;

	bool wantsLiveOutput;

	std::list<string> lines; //make this so the program only has one copy of lines available

	public:
		string name, exec, file, path, command, flags, confFile, device, method, cmdline;
		bool isRunning = false;
		void startServer(string confFile);
		void terminalAccessWrapper();
};
