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

#include "httplib/httplib.h"
#if defined(_WIN64) || defined(_WIN32)
#include <Windows.h>
#include <shellapi.h>
#include <VersionHelpers.h>
#include <intrin.h>
#include <powerbase.h>
#pragma comment (lib, "Shell32")
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <signal.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <termios.h>
#else
#include <unistd.h>
#include <termios.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <memory>
#include <iterator>
#include <algorithm>
#include <sstream>
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
#include <chrono>
#include <filesystem>
#include <errno.h>
#include <regex>
#include <ctime>
//#include <format>
#include <random>
#include <array>

namespace fs = std::filesystem;

#include "getvarsfromfile.hpp"
#include "server.hpp"

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

void Server::processChatKicks(string input) {
	try {
		std::regex kickreg("\\[.+\\]: <.+> " + chatKickRegex, std::regex_constants::optimize | std::regex_constants::icase);
		if (std::regex_search(input, kickreg)) {
			writeToServerTerminal("kick " + lastCommandUser + " §4§LForbidden word in chat; please do not say that!");
			writeToServerTerminal(formatWrapper("[Hajime] Kicked " + lastCommandUser + " for a chat infraction"));
		}
	} catch(...) {
		term.out<Error>("Invalid chat kick regex");
	}
}

void Server::processServerCommand(string input) {
	std::smatch m;
	std::regex_search(input, m, std::regex("\\[.+\\]: <(.+)> .+", std::regex_constants::optimize));
	lastCommandUser = m[1];
	if (std::regex_search(input, std::regex("\\" + text.server.command.hajime.regex + "?(?![\\w])", std::regex_constants::optimize))) {
		commandHajime();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.time.regex + "?(?!\\w)", std::regex_constants::optimize))) {
		commandTime();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.help.regex + "?(?!\\w)", std::regex_constants::optimize))) {
		commandHelp();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.die.regex + "?(?!\\w)", std::regex_constants::optimize))) {
		commandDie();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.d20.regex + "(?!\\w)", std::regex_constants::optimize))) {
		commandD20();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.coinflip.regex + "?(?!\\w)", std::regex_constants::optimize))) {
		commandCoinflip();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.discord.regex + "?(?!\\w)", std::regex_constants::optimize))) {
		commandDiscord();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.name.regex + "?(?!\\w)", std::regex_constants::optimize))) {
		commandName();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.info.regex + "?(?!\\w)", std::regex_constants::optimize))) {
		commandInfo();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.uptime.regex + "?(?!\\w)", std::regex_constants::optimize))) {
		commandUptime();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.restart.regex + "?(?!\\w)", std::regex_constants::optimize))) {
		commandRestart();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.system.regex + "?(?!\\w)", std::regex_constants::optimize))) {
		commandSystem();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.perf.regex + "?(?!\\w)", std::regex_constants::optimize))) {
		commandPerf();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.hwperf.regex + "?(?!\\w)", std::regex_constants::optimize))) {
		commandHWPerf();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.swperf.regex + "?(?!\\w)", std::regex_constants::optimize))) {
		commandSWPerf();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.caperf.regex + "?(?!\\w)", std::regex_constants::optimize))) {
		commandCAPerf();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\.ee(?!.\\w)", std::regex_constants::optimize))) {
		writeToServerTerminal(formatWrapper("[Hajime] https://www.youtube.com/watch?v=kjPD_H81hDc"));
	}
}

string Server::getOS() {
	#if defined(__linux__) && !defined(__FreeBSD__)
	std::fstream proc;
	proc.open("/proc/version", std::fstream::in);
	std::ostringstream temp;
	temp << proc.rdbuf();
	string out = temp.str();
	out.erase(std::remove(out.begin(), out.end(), '\n'), out.end());
	return out;
	#elif defined(_WIN32) || defined (_WIN64)
	std::string name;
	if (IsWindows10OrGreater()) {
		name = "Windows 10+";
	}
	else if (IsWindows8Point1OrGreater()) {
		name = "Windows 8.1+";
	}
	else if (IsWindows8OrGreater()) {
		name = "Windows 8+";
	}
	else if (IsWindows7OrGreater()) {
		name = "Windows 7+";
	}
	else if (IsWindowsVistaOrGreater()) {
		name = "Windows Vista+";
	}
	else if (IsWindowsXPOrGreater()) {
		name = "Windows XP+";
	} else {
		name = "Unknown Windows Version";
	}
	if (IsWindowsServer()) {
		name += " (Server)";
	}
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	switch (sys_info.wProcessorArchitecture) {
	case PROCESSOR_ARCHITECTURE_INTEL:
		name += " x86";
		break;
	case PROCESSOR_ARCHITECTURE_AMD64:
		name += " x64";
		break;
	case PROCESSOR_ARCHITECTURE_ARM:
		name += " arm";
		break;
	case PROCESSOR_ARCHITECTURE_ARM64:
		name += " arm64";
		break;
	}
	return name;
	#elif defined(__APPLE__)
	size_t len;
	sysctlbyname("kern.version", NULL, &len, NULL, 0);
	string result(len, '\0');
	sysctlbyname("kern.version", result.data(), &len, NULL, 0);
	sysctlbyname("kern.osproductversion", NULL, &len, NULL, 0);
       	string macosresult(len, '\0');
        sysctlbyname("kern.osproductversion", macosresult.data(), &len, NULL, 0);
	return result + " (macOS " + macosresult + ")";
	#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
	return "Blah";
	#endif
	return "Not available on your platform";
}

string Server::getCPU() {
	#if defined(__linux__) && !defined(__FreeBSD__)
	std::smatch m;
	std::fstream proc;
	proc.open("/proc/cpuinfo", std::fstream::in);
	std::ostringstream temp;
	temp << proc.rdbuf();
	string temp2 = temp.str();
	std::regex_search(temp2, m, std::regex("(?:model name\\s*:\\s*)(.*)", std::regex_constants::optimize));
	return m[1];
	#elif defined (_WIN32)
	std::string brandname;
	#if defined(_M_IX86) || defined(_M_X64)
	std::array<int, 4> cpui = {};
	std::array<char, 16> cpui_c = {};
	std::vector<char> brand;
	__cpuid(cpui.data(), 0x80000000);
	unsigned int maxid = cpui[0];
	if (maxid < 0x80000004U) {
		brandname = "Unknown CPU";
	}	else {
		for (unsigned int i = 0x80000002U; i <= 0x80000004U; i++) {
			// here it actually gives chars, but cpuidex only accepts int*
			__cpuidex(reinterpret_cast<int*>(cpui_c.data()), i, 0);
			brand.insert(brand.end(), cpui_c.begin(), cpui_c.end());
		}
		brandname = std::string(brand.begin(), brand.end() - 1);
		// erase trailing whitespaces
		while (brandname.back() == ' ') {
			brandname.pop_back();
		}
	}
	#else
	brandname = "Unknown or ARM CPU";
	#endif
	// number of threads
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	DWORD number_of_processors = sys_info.dwNumberOfProcessors;
	brandname.insert(0, std::to_string(number_of_processors) + "x ");
	return brandname;
	#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
	return "Blah";
	#elif defined(__APPLE__)
	size_t len;
  sysctlbyname("machdep.cpu.brand_string", NULL, &len, NULL, 0);
  string cpuname(len, '\0');
  sysctlbyname("machdep.cpu.brand_string", cpuname.data(), &len, NULL, 0);
	sysctlbyname("hw.ncpu", NULL, &len, NULL, 0);
  int cpucount;
  sysctlbyname("hw.ncpu", &cpucount, &len, NULL, 0);
  return std::to_string(cpucount) + "x " + cpuname;
	#endif
	return "Only available on Linux or Windows";
}

string Server::getRAM() {
	#if defined(__linux__)
	std::fstream proc;
	proc.open("/proc/meminfo", std::fstream::in);
	std::ostringstream temp;
	temp << proc.rdbuf();
	string temp2 = temp.str();
	std::regex re("\\d+", std::regex_constants::optimize);
	std::vector<std::string> meminfo;
	for (auto it = std::sregex_iterator(temp2.begin(), temp2.end(), re); it != std::sregex_iterator(); ++it) {
		std::smatch m = *it;
		meminfo.push_back(m.str());
	}
	if (meminfo.size() < 3) {
		return string("Could not get memory info");
	}
	string result = meminfo[0] + "kB total, " + meminfo[1] + "kB free, " + meminfo[2] + "kB available";
	return result;
	#elif defined(_WIN32)
	MEMORYSTATUSEX mem;
	mem.dwLength = sizeof(mem);
	GlobalMemoryStatusEx(&mem);
	constexpr double div = 1024 * 1024 * 1024;
	const auto roundto2 = [](double d) -> std::string {
		std::stringstream ss;
		ss << std::fixed << std::setprecision(2) << d;
		return ss.str();
	};
	DWORDLONG total_totalmem = mem.ullTotalPhys + mem.ullTotalPageFile;
	DWORDLONG total_availmem = mem.ullAvailPhys + mem.ullAvailPageFile;
	DWORDLONG total_usedmem = total_totalmem - total_availmem;
	std::string result = roundto2(total_usedmem / div) + '/' + roundto2(total_totalmem / div) + " GB Total, " + std::to_string(std::lround((total_usedmem * 100.0) / total_totalmem)) + "% Used (" +
		roundto2((mem.ullTotalPhys - mem.ullAvailPhys) / div) + '/' + roundto2(mem.ullTotalPhys / div) + " GB Physical, " + std::to_string(mem.dwMemoryLoad) + "% Used)";
	return result;
	#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
	return "Blah";
	#elif defined(__APPLE__)
	size_t len;
  sysctlbyname("hw.memsize", NULL, &len, NULL, 0);
  long int memtotal;
  sysctlbyname("hw.memsize", &memtotal, &len, NULL, 0);
  return std::to_string(memtotal) + "B total";
	#else
	return "Not available yet";
	#endif
}

string Server::getUptime() {
	#if defined(__linux__)
	std::smatch m;
	std::fstream proc;
	proc.open("/proc/uptime", std::fstream::in);
	std::ostringstream temp;
	temp << proc.rdbuf();
	string temp2 = temp.str();
	std::regex_search(temp2, m, std::regex("[0-9]+(\\.[0-9]+)?", std::regex_constants::optimize));
	try {
		return string(m[0]) + " seconds (" + std::to_string(stoi(m[0]) / 60) + string(" minutes, ") + std::to_string(stoi(m[0]) / 3600) + " hours)";
	} catch (...) {
		return "Error parsing memory";
	}
	#else
	return string("Not available yet");
	#endif
}

string Server::getLoadavg() {
	#if defined(__linux__)
	std::fstream proc;
	proc.open("/proc/loadavg", std::fstream::in);
	std::ostringstream temp;
	temp << proc.rdbuf();
	string temp2 = temp.str();
	std::regex re("[0-9.]+", std::regex_constants::optimize);
	std::vector<std::string> loadinfo;
	for (auto it = std::sregex_iterator(temp2.begin(), temp2.end(), re); it != std::sregex_iterator(); ++it) {
		std::smatch m = *it;
		loadinfo.push_back(m.str());
	}
	if (loadinfo.size() < 3) {
		return string("Could not get load average info");
	}
	string result = "last 1 minute: " + loadinfo[0] + ", last 5 minutes: " + loadinfo[1] + ", last 10 minutes: " + loadinfo[2];
	return result;
	#else
	return "Not available yet";
	#endif
}

string Server::getSwap() {
	#if defined(__linux__)
	struct sysinfo info;
	sysinfo(&info);
	string result = std::to_string(info.totalswap / 1024) + "kB total, " + std::to_string(info.freeswap / 1024) + "kB available";
	return result;
	#else
	return "Not available yet";
	#endif
}

string Server::getProcesses() {
	#if defined(__linux__)
	struct sysinfo info;
	sysinfo(&info);
	string result = std::to_string(info.procs) + " total";
	return result;
	#else
	return "Not available yet";
	#endif
}

bool Server::areCountersAvailable() {
	#if defined(_WIN32) || defined(_WIN64)
	return false;
	#elif defined(__linux__)
	if (performanceCounterCompat == -1) {
		return false;
	} else {
		return true;
	}
	#else
	return false;
	#endif
}

string Server::formatReadingsLIB(const std::deque<long long>& little, const std::deque<long long>& big) {
	if (areCountersAvailable()) {
		return std::to_string(little.back()) + " (" + std::to_string(100.0 * (double)little.back() / (double)big.back()) + "% of total) last 5 seconds, " + std::to_string(averageVal(little, 1)) + " (" + std::to_string(100.0 * (double)averageVal(little, 1) / (double)averageVal(big, 1)) + "%) last 1 minute, " + std::to_string(averageVal(little, 5)) + " (" + std::to_string(100.0 * (double)averageVal(little, 5) / (double)averageVal(big, 5)) + "%) last 5, " + std::to_string(averageVal(little, 15)) + " (" + std::to_string(100.0 * (double)averageVal(little, 15) / (double)averageVal(big, 15)) + "%) last 15, " + std::to_string(averageVal(little, 60)) + " (" + std::to_string(100.0 * (double)averageVal(little, 60) / (double)averageVal(big, 60)) + "%) last 1 hour (Lower is better)";
	} else {
		return "Currently not available on this server";
	}
}

string Server::formatReadingsLIB(const std::deque<long long>& readings) {
	if (areCountersAvailable()) {
		return std::to_string(readings.back()) + " last 5 seconds, " + std::to_string(averageVal(readings, 1)) + " last 1 minute, " + std::to_string(averageVal(readings, 5)) + " last 5, " + std::to_string(averageVal(readings, 15)) + " last 15, " + std::to_string(averageVal(readings, 60)) + " last 1 hour (Lower is better)";
	} else {
		return "Currently not available on this server";
	}
}

string Server::formatReadingsHIB(const std::deque<long long>& readings) {
	if (areCountersAvailable()) {
		return std::to_string(readings.back()) + " last 5 seconds, " + std::to_string(averageVal(readings, 1)) + " last 1 minute, " + std::to_string(averageVal(readings, 5)) + " last 5, " + std::to_string(averageVal(readings, 15)) + " last 15, " + std::to_string(averageVal(readings, 60)) + " last 1 hour (Higher is better)";
	} else {
		return "Currently not available on this server";
	}
}

string Server::getCPUusage() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return std::to_string(averageVal(cpuusagereadings, 1)) + "% last 1 minute, " + std::to_string(averageVal(cpuusagereadings, 5)) + "% last 5, " + std::to_string(averageVal(cpuusagereadings, 15)) + "% last 15 (Lower is better)";
}

string Server::getCPUmigs() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return std::to_string(averageVal(cpumigrationreadings, 1)) + " last 1 minute, " + std::to_string(averageVal(cpumigrationreadings, 5)) + " last 5, " + std::to_string(averageVal(cpumigrationreadings, 15)) + " last 15";
}

string Server::getRAMusage() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return std::to_string(averageVal(rampercentreadings, 1)) + "% last 1 minute, " + std::to_string(averageVal(rampercentreadings, 5)) + "% last 5, " + std::to_string(averageVal(rampercentreadings, 15)) + "% last 15 (" + std::to_string((averageVal(rambytereadings, 1) / 1024) / 1024) + "MB/" + std::to_string((averageVal(rambytereadings, 5) / 1024) / 1024) + "MB/" + std::to_string((averageVal(rambytereadings, 15) / 1024) / 1024) + "MB) (Lower is better)";
}

string Server::getIPC() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return std::to_string((double)averageVal(cpuinstructionreadings, 1) / (double)averageVal(cpucyclereadings, 1)) + " last 1 minute, " + std::to_string((double)averageVal(cpuinstructionreadings, 5) / (double)averageVal(cpucyclereadings, 5)) + " last 5, " + std::to_string((double)averageVal(cpuinstructionreadings, 15) / (double)averageVal(cpucyclereadings, 15)) + " last 15 (Higher is better)";
}

string Server::getIPS() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return std::to_string((averageVal(cpuinstructionreadings, 1) / 60) / 1000000) + "M/s last 1 minute, " + std::to_string((averageVal(cpuinstructionreadings, 5) / 60) / 1000000) + "M/s last 5, " + std::to_string((averageVal(cpuinstructionreadings, 15) / 60) / 1000000) + "M/s last 15";
}

string Server::getCPS() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return std::to_string((averageVal(cpucyclereadings, 1) / 60) / 1000000) + "M/s last 1 minute, " + std::to_string((averageVal(cpucyclereadings, 5) / 60) / 1000000) + "M/s last 5, " + std::to_string((averageVal(cpucyclereadings, 15) / 60) / 1000000) + "M/s last 15";
}

string Server::getContextSwitches() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(contextswitchreadings, cpuinstructionreadings);
}

string Server::getStalledCyclesFrontend() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(stalledcyclesfrontendreadings);
}

string Server::getStalledCyclesBackend() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(stalledcyclesbackendreadings);
}

string Server::getBusCycles() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsHIB(buscyclereadings);
}

string Server::getBranchMisses() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(branchmissreadings, branchinstructionreadings);
}

string Server::getCacheMisses() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(cachemissreadings, cachereferencereadings);
}

string Server::getAlignmentFaults() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(alignmentfaultreadings);
}

string Server::getEmulationFaults() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(emulationfaultreadings);
}

string Server::getMinorPagefaults() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(minorpagefaultreadings, pagefaultreadings);
}

string Server::getMajorPagefaults() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(majorpagefaultreadings, pagefaultreadings);
}

string Server::getL1dReadMisses() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(l1dreadmissreadings, l1dreadaccessreadings);
}

string Server::getL1dPrefetchMisses() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(l1dprefetchmissreadings, l1dprefetchaccessreadings);
}

string Server::getL1dWriteMisses() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(l1dwritemissreadings, l1dwriteaccessreadings);
}

string Server::getL1iReadMisses() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(l1ireadmissreadings, l1ireadaccessreadings);
}

string Server::getL1iPrefetchMisses() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(l1iprefetchmissreadings, l1iprefetchaccessreadings);
}

string Server::getLLReadMisses() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(llreadmissreadings, llreadaccessreadings);
}

string Server::getLLWriteMisses() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(llwritemissreadings, llwriteaccessreadings);
}

string Server::getLLPrefetchMisses() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(llprefetchmissreadings);
}

string Server::getdTLBReadMisses() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(dtlbreadmissreadings, dtlbreadaccessreadings);
}

string Server::getdTLBWriteMisses() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(dtlbwritemissreadings, dtlbwriteaccessreadings);
}

string Server::getdTLBPrefetchMisses() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(dtlbprefetchmissreadings, dtlbprefetchaccessreadings);
}

string Server::getiTLBReadMisses() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(itlbreadmissreadings, itlbreadaccessreadings);
}

string Server::getBPUReadMisses() {
	std::lock_guard<std::mutex> lock(perfMutex);
	return formatReadingsLIB(bpureadmissreadings, bpureadaccessreadings);
}

void Server::processRestartAlert(string input) {
	std::smatch m;
	if (restartMins > 0 && uptime >= (restartMins - 5) && std::regex_search(input, m, std::regex("\\[.+\\]: ([\\w\\d]+)\\[.+\\] .+", std::regex_constants::optimize))) {
		string hajInfo = "tellraw " + string(m[1]) + text.server.restart.alert1 + std::to_string(restartMins - uptime) + text.server.restart.alert2;
		writeToServerTerminal(addNumberColors(hajInfo));
	}
}

string Server::addNumberColors(string input) {
	return std::regex_replace(input, std::regex("(?: |\\()\\d+\\.?\\d*", std::regex_constants::optimize), "§b$&§f");
}

string Server::formatWrapper(string input) {
	string output;
	if (!silentCommands) {
		if (input.front() == '[' && input.back() == ']') {
			output = "tellraw @a " + input;
		} else {
			output = "tellraw @a \"" + input + "\"";
		}
	} else {
		if (input.front() == '[' && input.back() == ']') {
			output = "tellraw " + lastCommandUser + " " + input;
		} else {
			output = "tellraw " + lastCommandUser + " \"" + input + "\"";
		}
	}
	output = std::regex_replace(output, std::regex("\\[Hajime\\]", std::regex_constants::optimize), "§6$&§f");
	return output;
}

void Server::writeToServerTerminal(string input) {
	input += "\n"; //this is the delimiter of the server command
	#if defined(_WIN64) || defined(_WIN32)
	DWORD byteswritten;
	if (!WriteFile(inputwrite, input.c_str(), input.size(), &byteswritten, NULL)) {// write to input pipe
		term.out<Error>("Unable to write to pipe");
	} else if (byteswritten != input.size()) {
		std::cout << "Wrote " + std::to_string(byteswritten) + "bytes, expected " + std::to_string(input.size()) << std::endl;
	}
	#else
	int len = write(fd, input.c_str(), input.length());
	if (len == -1) {
		term.out<Error>("Error writing to server terminal");
	}
	#endif
}

void Server::processServerTerminal() {
	string terminalOutput;
	while (true) {
		terminalOutput = readFromServer();
		if (doCommands) {
			processServerCommand(terminalOutput);
		}
		if (chatKickRegex != "") {
			processChatKicks(terminalOutput);
		}
		processRestartAlert(terminalOutput);
		processTerminalBuffer(terminalOutput);
	}
}

string Server::readFromServer() {
	char input[1000];
	#if defined(_WIN32) || defined (_WIN64)
	DWORD length = 0;
	if (!ReadFile(outputread, input, 1000, &length, NULL)) {
		term.out<Error, Threadless>("ReadFile failed (unable to read from pipe)");
		return std::string();
	}
	#else
	ssize_t length = -1;
	while (length == -1) {
		length = read(fd, input, sizeof(input));
		if (length == -1) {
			switch(errno) {
				case EAGAIN:
					term.out<Error, Threadless>("Error reading file descriptor; not a socket and is nonblocking");
					break;
				case EINTR:
					term.out<Error, Threadless>("Error reading file descriptor; interrupted before read could complete");
					break;
				case EBADF:
					term.out<Error, Threadless>("Error reading file descriptor; not valid or not available for reading");
					break;
				case EIO:
					term.out<Error, Threadless>("Error reading file descriptor; I/O error (the server is likely down)");
					break;
				default:
					term.out<Error, Threadless>("Other error reading file descriptor; errno = " + std::to_string(errno));
					break;
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
	#endif
	std::string output;
	for (int i = 0; i < length; i++) {
		output.push_back(input[i]);
	}
	return output;
}

void Server::updateUptime() {
	timeCurrent = std::chrono::steady_clock::now();
	auto tempUptime = std::chrono::duration_cast<std::chrono::minutes>(timeCurrent - timeStart);
	uptime = tempUptime.count();
	//std::cout << "uptime = " + std::to_string(uptime) << std::endl;
}

void Server::processAutoUpdate(bool force) {
	if ((restartMins > 0 && uptime >= restartMins) || force) {
		std::string content;
		auto makeFile = [&content](std::string filename) {
			std::fstream file(filename, std::fstream::out);
			file << content;
			if (fs::file_size(filename) > 0) {
				term.out<Info>("Downloaded server file " + filename + " with file size " + std::to_string(fs::file_size(filename)));
			} else if (fs::file_size(filename) == 0) {
				term.out<Error>("There was an error downloading or saving the server file");
			} else if (fs::file_size(filename) == -1) {
				term.out<Error>("There was an error reading the server file");
			}
			file.close();
		};
		term.out<Info>("Updating server software with name " + autoUpdateName + " and version " + autoUpdateVersion);
		if (autoUpdateName == "purpur") {
			std::string target = "/v2/purpur/" + autoUpdateVersion + "/latest/download";
			#if defined(CPPHTTPLIB_OPENSSL_SUPPORT)
			httplib::SSLClient cli("api.purpurmc.org");
			cli.enable_server_certificate_verification(false);
			#else
			httplib::Client cli("http://api.purpurmc.org");
			#endif
			httplib::Headers headers = { {"Accept-Encoding", "gzip, deflate"} };
			auto res = cli.Get(target.c_str(), headers, [&](const char * data, size_t length) {
				content.append(data, length);
				return true;
			});
			makeFile("purpur.jar");
		} else if (autoUpdateName == "paper") {
			term.out<Error>("Paper is not yet supported");
		} else if (autoUpdateName == "fabric") {
			std::string target = "/v2/versions/loader/" + autoUpdateVersion;
			httplib::Headers headers = { {"Accept-Encoding", "gzip, deflate"} };

			#if defined(CPPHTTPLIB_OPENSSL_SUPPORT)
			httplib::SSLClient cli("meta.fabricmc.net");
			cli.enable_server_certificate_verification(false);
			#else
			httplib::Client cli("http://meta.fabricmc.net");
			#endif
			auto res = cli.Get(target.c_str(), headers, [&](const char * data, size_t length) {
				content.append(data, length);
				return true;
			});
			std::smatch matches;
			if(std::regex_search(content, matches, std::regex("\"loader\"(.+\n){4}.+(\"version\": \")([0-9.]+)"))) {
				target += "/" + string(matches[3]);
			}

			content = "";

			#if defined(CPPHTTPLIB_OPENSSL_SUPPORT)
			httplib::SSLClient cli2("meta.fabricmc.net");
			cli2.enable_server_certificate_verification(false);
			#else
			httplib::Client cli2("http://meta.fabricmc.net");
			#endif
			auto res2 = cli2.Get("/v2/versions/installer", headers, [&](const char * data, size_t length) {
				content.append(data, length);
				return true;
			});
			if(std::regex_search(content, matches, std::regex("(\"version\": \")([0-9.]+)"))) {
				target += "/" + string(matches[2]);
			}

			content = "";
			target += "/server/jar";

			#if defined(CPPHTTPLIB_OPENSSL_SUPPORT)
			httplib::SSLClient cli3("meta.fabricmc.net");
			cli3.enable_server_certificate_verification(false);
			#else
			httplib::Client cli3("http://meta.fabricmc.net");
			#endif
			auto res3 = cli3.Get(target.c_str(), headers, [&](const char * data, size_t length) {
				content.append(data, length);
				return true;
			});

			makeFile("fabric.jar");
		} else {
			term.out<Error>("Invalid auto update server name");
		}
	}
}

void Server::processAutoRestart() {
	if (restartMins > 0 && uptime >= restartMins) {
		writeToServerTerminal("stop");
	}	else if (restartMins > 0 && uptime >= (restartMins - 5) && !said5MinRestart) {
		writeToServerTerminal(formatWrapper(addNumberColors(text.server.restart.minutes5)));
		said5MinRestart = true;
	} else if (restartMins > 0 && uptime >= (restartMins - 15) && !said15MinRestart) {
		writeToServerTerminal(formatWrapper(addNumberColors(text.server.restart.minutes15)));
		said15MinRestart = true;
	}
}

void Server::terminalAccessWrapper() {
	term.dividerLine(name);
	term.normalDisabled = true;
	wantsLiveOutput = true;
	for (const auto& it : lines) {
		std::cout << it << std::flush;
	}
	std::string user_input = "";
	while (true) {
		std::getline(std::cin, user_input); //getline allows for spaces
		if (user_input == ".d") {
			wantsLiveOutput = false;
			break;
		} else if (user_input[0] == '.') {
			std::cout << text.error.InvalidCommand << std::endl;
			std::cout << text.error.InvalidServerCommand1 << std::endl;
		} else {
			//std::cout << "sending to server" << std::endl;
			writeToServerTerminal(user_input); //write to the master side of the pterminal with user_input converted into a c-style string
		}
	}
	term.normalDisabled = false;
	term.dividerLine("Hajime");
}
