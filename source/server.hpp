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
using std::list;
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

	template<typename T>
	T averageVal(list<T> myList, unsigned int minutes) {
		int readings = 0.0;
		T temp = 0;
		myList.reverse();
		for (const auto& value : myList) {
			temp += value;
			readings++;
			if (readings >= minutes) {
				break;
			}
		}
		if (readings > 0) {
			temp /= readings;
		} else {
			std::cout << "/0 error" << std::endl;
		}
		return temp;
	}

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
	string getBusCycles();
	string getBranchMisses();
	string getCacheMisses();
	string getAlignmentFaults();
	string getEmulationFaults();
	string getMinorPagefaults();
	string getMajorPagefaults();
	string getL1dReadMisses();
	string getLLReadMisses();
	string getLLWriteMisses();
	string getdTLBReadMisses();
	string getdTLBWriteMisses();
	string getiTLBReadMisses();
	string getBPUReadMisses();
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

	std::list<long long> cpuusagereadings;
	std::list<double> rampercentreadings;
	std::list<unsigned long long> rambytereadings, cpucyclereadings, cpuinstructionreadings, cachemissreadings, branchinstructionreadings, branchmissreadings, cachereferencereadings, stalledcyclesfrontendreadings, stalledcyclesbackendreadings, buscyclereadings;
	std::list<unsigned long long> pagefaultreadings, contextswitchreadings, cpumigrationreadings, alignmentfaultreadings, emulationfaultreadings, minorpagefaultreadings, majorpagefaultreadings;
	std::list<unsigned long long> l1dreadaccessreadings, l1dreadmissreadings, llreadaccessreadings, llreadmissreadings, dtlbreadaccessreadings, dtlbreadmissreadings, dtlbwriteaccessreadings, dtlbwritemissreadings, itlbreadaccessreadings, itlbreadmissreadings, bpureadaccessreadings, bpureadmissreadings, llwriteaccessreadings, llwritemissreadings;

	double RAMpercent1m, RAMpercent5m, RAMpercent15m;
	long long CPUjiffies, PIDjiffies;

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
