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
#include <deque>
#include <atomic>
#include <cstring>
#include <string>
#include <vector>
#include <deque>
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
using std::deque;
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
	void resetAndEnableCounters(const auto& counters);
	void disableCounters(const auto& counters);
	void readCounters(auto& counters);
	#endif

	string formatWrapper(string input);
	string readFromServer();
	void writeToServerTerminal(string input);
	void processTerminalBuffer(string input);
	void processServerCommand(string input);
	void processChatKicks(string input);
	void processPerfStats();
	void updateCPUusage(std::deque<long long>& CPUreadings);
	void updateRAMusage();
	void commandHajime();
	void commandTime();
	void commandHelp();
	void commandDie();
	void commandD20();
	void commandCoinflip();
	void commandDiscord();
	void commandName();
	void commandInfo();
	void commandUptime();
	void commandRestart();
	void commandSystem();
	void commandPerf();
	void commandHWPerf();
	void commandSWPerf();
	void commandCAPerf();

	template<typename T>
	T averageVal(deque<T> myList, unsigned int minutes) {
		minutes *= 12; //convert to 5-second intervals
		int readings = 0;
		T temp = 0;
		reverse(myList.begin(), myList.end());
		for (const auto& value : myList) {
			if (value != 0) {
				temp += value;
				if (readings++; readings >= minutes) {
					break;
				}
			}
		}
		if (readings > 0) {
			temp /= readings;
		} else {
			return 0;
		}
		return temp;
	}

	string getOS();
	string getCPU();
	string getRAM();
	string getSwap();
	string getProcesses();
	bool areCountersAvailable();
	string formatReadingsLIB(const std::deque<long long>& little, const std::deque<long long>& big);
	string formatReadingsLIB(const std::deque<long long>& readings);
	string formatReadingsHIB(const std::deque<long long>& readings);
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
	string getL1dPrefetchMisses();
	string getL1dWriteMisses();
	string getL1iReadMisses();
	string getL1iPrefetchMisses();
	string getLLReadMisses();
	string getLLWriteMisses();
	string getLLPrefetchMisses();
	string getdTLBReadMisses();
	string getdTLBWriteMisses();
	string getdTLBPrefetchMisses();
	string getiTLBReadMisses();
	string getBPUReadMisses();
	string addNumberColors(string input);
	void processRestartAlert(string input);
	void mountDrive();
	void makeDir();
	void updateUptime();
	void processAutoUpdate(bool force = false);
	void processAutoRestart();
	void startProgram(string method);
	void readSettings(string confFile);
	void removeSlashesFromEnd(string& var);
	void processServerTerminal();
	int getPID();
	vector<string> toArray(string input);
	auto toPointerArray(vector<string> &strings);

	inline static std::atomic<int> performanceCounterCompat = 0;

	int counterLevel = 0;

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

	bool said15MinRestart = false;
	bool said5MinRestart = false;
	bool doCommands = true;
	bool silentCommands = false;

	inline static std::vector<long long> knownBadEvents = {};

	std::deque<long long> cpuusagereadings{0};
	std::deque<double> rampercentreadings{0.0};
	std::deque<long long> rambytereadings{0};

	std::deque<long long> cpucyclereadings{0};
	std::deque<long long> cpuinstructionreadings{0};
	std::deque<long long> cachemissreadings{0};
	std::deque<long long> branchinstructionreadings{0};
	std::deque<long long> branchmissreadings{0};
	std::deque<long long> cachereferencereadings{0};
	std::deque<long long> stalledcyclesfrontendreadings{0};
	std::deque<long long> stalledcyclesbackendreadings{0};
	std::deque<long long> buscyclereadings{0};

	std::deque<long long> pagefaultreadings{0};
	std::deque<long long> contextswitchreadings{0};
	std::deque<long long> cpumigrationreadings{0};
	std::deque<long long> alignmentfaultreadings{0};
	std::deque<long long> emulationfaultreadings{0};
	std::deque<long long> minorpagefaultreadings{0};
	std::deque<long long> majorpagefaultreadings{0};

	std::deque<long long> l1dreadaccessreadings{0};
	std::deque<long long> l1dreadmissreadings{0};
	std::deque<long long> l1dprefetchaccessreadings{0};
	std::deque<long long> l1dprefetchmissreadings{0};
	std::deque<long long> llreadaccessreadings{0};
	std::deque<long long> llreadmissreadings{0};
	std::deque<long long> dtlbreadaccessreadings{0};
	std::deque<long long> dtlbreadmissreadings{0};
	std::deque<long long> dtlbwriteaccessreadings{0};
	std::deque<long long> dtlbwritemissreadings{0};
	std::deque<long long> dtlbprefetchaccessreadings{0};
	std::deque<long long> dtlbprefetchmissreadings{0};
	std::deque<long long> itlbreadaccessreadings{0};
	std::deque<long long> itlbreadmissreadings{0};
	std::deque<long long> bpureadaccessreadings{0};
	std::deque<long long> bpureadmissreadings{0};
	std::deque<long long> llwriteaccessreadings{0};
	std::deque<long long> llwritemissreadings{0};
	std::deque<long long> llprefetchmissreadings{0};
	std::deque<long long> l1dwriteaccessreadings{0};
	std::deque<long long> l1dwritemissreadings{0};
	std::deque<long long> l1ireadaccessreadings{0};
	std::deque<long long> l1ireadmissreadings{0};
	std::deque<long long> l1iprefetchaccessreadings{0};
	std::deque<long long> l1iprefetchmissreadings{0};

	long long CPUjiffies, PIDjiffies;

	string lastCommandUser;
	string chatKickRegex;

	bool startedRfdThread = false;
	bool startedPerfThread = false;

	bool wantsLiveOutput;

	std::deque<string> lines;

	public:
		string name, exec, file, path, command, flags, confFile, device, method, cmdline, customMessage, autoUpdateName, autoUpdateVersion;
		bool isRunning = false;
		void startServer(string confFile);
		void terminalAccessWrapper();
};
