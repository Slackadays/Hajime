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
#include <mutex>
#include <array>

#if defined(_OPENMP)
#include <omp.h>
#endif

#include "../output.hpp"
#include "../languages.hpp"

class Server {
	template<typename T>
	T averageVal(std::deque<T> myList, unsigned int minutes) {
		minutes *= 20; //convert to 3-second intervals
		unsigned int readings = 0;
		T temp = 0;
		if (myList.size() < minutes) {
			minutes = myList.size();
		}
		reverse(myList.begin(), myList.end());
		#pragma omp parallel for reduction(+:temp, readings)
		for (long long i = 0; i < minutes; i++) {
			if (myList[i] != 0) {
				temp += myList[i];
				readings++;
			}
		}
		if (readings > 0) {
			temp /= readings;
		} else {
			return 0;
		}
		return temp;
	}

	const std::vector<std::string> systems{"ext2", "ext3", "ext4", "vfat", "msdos", "f2fs", "ntfs", "fuseblk"};

	#if defined(__linux__)
	std::vector<long> getProcessChildPids(long pid);
	void setupCounter(auto& s);
	void createCounters(std::vector<struct pcounter*>& counters, const std::vector<long>& pids);
	void cullCounters(std::vector<struct pcounter*>& counters, const std::vector<long>& pids);
	void resetAndEnableCounters(const auto& counters);
	void disableCounters(const auto& counters);
	void readCounters(auto& counters);
	#elif defined(__APPLE__)
	#elif defined(_WIN64) || defined (_WIN32)
	#elif defined(__FreeBSD__)
	#endif

	std::string generateSecret();
	std::string formatWrapper(std::string input);
	std::string readFromServer();
	void checkHajimeHelper(std::string input);
	void processTerminalBuffer(std::string input);
	void processServerCommand(std::string input);
	void processChatKicks(std::string input);
	void processPerfStats();
	void updateCPUusage(std::deque<long long>& CPUreadings);
	void updateRAMusage();
	void trimCounterData();
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

	std::string getOS();
	std::string getCPU();
	std::string getRAM();
	std::string getSwap();
	std::string getProcesses();
	std::string formatReadingsLIB(const std::deque<long long>& little, const std::deque<long long>& big);
	std::string formatReadingsLIB(const std::deque<long long>& readings);
	std::string formatReadingsHIB(const std::deque<long long>& readings);
	std::string getUptime();
	std::string getLoadavg();
	std::string getCPUusage();
	std::string getCPUmigs();
	std::string getRAMusage();
	std::string getStorage();
	std::string getTemps();

	bool areCountersAvailable();

	std::string getIPC();
	std::string getIPS();
	std::string getCPS();
	std::string getContextSwitches();
	std::string getStalledCyclesBackend();
	std::string getStalledCyclesFrontend();
	std::string getBusCycles();
	std::string getBranchMisses();
	std::string getCacheMisses();
	std::string getAlignmentFaults();
	std::string getEmulationFaults();
	std::string getMinorPagefaults();
	std::string getMajorPagefaults();
	std::string getL1dReadMisses();
	std::string getL1dPrefetchMisses();
	std::string getL1dWriteMisses();
	std::string getL1iReadMisses();
	std::string getL1iPrefetchMisses();
	std::string getLLReadMisses();
	std::string getLLWriteMisses();
	std::string getLLPrefetchMisses();
	std::string getdTLBReadMisses();
	std::string getdTLBWriteMisses();
	std::string getdTLBPrefetchMisses();
	std::string getiTLBReadMisses();
	std::string getBPUReadMisses();
	std::string addNumberColors(std::string input);

	void processRestartAlert(std::string input);
	void mountDrive();
	void makeDir();
	void updateUptime();
	void processAutoUpdate(bool force = false);
	void processAutoRestart();
	void startProgram();
	void readSettings(std::string confFile);
	void removeSlashesFromEnd(std::string& var);
	void processServerTerminal();
	void startBackgroundThreads();

	int getPID();

	std::vector<std::string> toArray(std::string input);
	auto toPointerArray(std::vector<std::string> &strings);

	inline static std::vector<long long> knownBadEvents = {};
	inline static std::atomic<int> performanceCounterCompat = 0; // 0 = unknown, 1 = yes, -1 = no

	struct CounterData {
		std::mutex mutex;

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
	};
	CounterData counterData;

	std::deque<std::string> lines;

	public:
		struct ServerSettings {
			std::mutex mutex; // mutex for settings

			std::string version;
			std::string name;
			std::string exec;
			std::string path;
			std::string file;
			std::string flags;
			std::string confFile;
			std::string cmdline;
			std::string device;
			std::string customMessage;
			std::string autoUpdateName;
			std::string autoUpdateVersion;
			std::string chatKickRegex;

			bool doCommands = true;
			int counterLevel = 0;
			long restartMins;
			long long counterInterval = defaultCounterInterval;
			long long counterMax = defaultCounterMax;
		};
		ServerSettings serverSettings;

		struct ServerAttributes {
			std::mutex mutex;

			std::chrono::time_point<std::chrono::steady_clock> timeStart;
			std::chrono::time_point<std::chrono::steady_clock> timeCurrent;

			std::string secret;

			bool hasOutputUSB = false;
			bool hasMounted = false;
			bool usesHajimeHelper = false;
			bool isRunning = false;
			bool startedRfdThread = false;
			bool startedPerfThread = false;
			bool wantsLiveOutput = false;
			bool said15MinRestart = false;
			bool said5MinRestart = false;
			bool saidHajimeHelperMessage = false;

			int systemi = 0;

			int runstate = 1;

			long long uptime = 0;
			long long CPUjiffies = 0;
			long long PIDjiffies = 0;

			std::string lastCommandUser;
		};
		ServerAttributes serverAttributes;

		#if defined(_WIN64) || defined(_WIN32)
		STARTUPINFO si; // a variable that can specify parameters for windows created with it
		PROCESS_INFORMATION pi; // can get process handle and pid from this
		HANDLE inputread, inputwrite, outputread, outputwrite; // pipes for reading/writing
		#else
		int slave_fd, fd, pid;
		struct winsize w;
		#endif
	
		void startServer(std::string confFile);
		void terminalAccessWrapper();
		void writeToServerTerminal(std::string input);
};
