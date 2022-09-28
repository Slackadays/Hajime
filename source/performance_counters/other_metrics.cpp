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

#if defined(_WIN64) || defined(_WIN32)

#elif defined(__APPLE__)

#elif defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#else

#endif

#include <memory>
#include <iterator>
#include <algorithm>
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
//#include <format>
#include <array>

#include "../server.hpp"

namespace fs = std::filesystem;
namespace ch = std::chrono;

void Server::updateCPUusage(std::deque<long long>& CPUreadings) {
	std::lock_guard<std::mutex> lock(perfMutex);
	#if defined(_WIN32) || defined(_WIN64)
	//do stuff here
	//update the values in server.hpp
	#elif defined(__linux__)
	string line;
	double old_pidjiffies;
	double old_cpujiffies;
	double new_pidjiffies;
	double new_cpujiffies;
	long cpuNum = sysconf(_SC_NPROCESSORS_ONLN);
	old_pidjiffies = PIDjiffies;
	old_cpujiffies = CPUjiffies;
	std::fstream pidprocstat("/proc/" + std::to_string(pid) + "/stat", std::fstream::in);
	std::getline(pidprocstat, line);
	std::regex repid("\\S+", std::regex_constants::optimize);
	std::vector<std::string> pidcpuinfo;
	for (auto it = std::sregex_iterator(line.begin(), line.end(), repid); it != std::sregex_iterator(); ++it) {
		std::smatch m = *it;
		pidcpuinfo.push_back(m.str());
	}
	if (pidcpuinfo.size() < 15) {
		term.out<Error, Threadless>("Could not get CPU usage info");
	}
	pidprocstat.close();
	//std::cout << "userjiffies = " << pidcpuinfo[13] << " kerneljiffies = " << pidcpuinfo[14] << std::endl;
	std::fstream procstat("/proc/stat", std::fstream::in);
	std::getline(procstat, line);
	std::regex restat("[0-9]+", std::regex_constants::optimize);
	std::vector<std::string> procstatinfo;
	for (auto it = std::sregex_iterator(line.begin(), line.end(), restat); it != std::sregex_iterator(); ++it) {
		std::smatch m = *it;
		procstatinfo.push_back(m.str());
	}
	try {
		new_pidjiffies = (std::stol(pidcpuinfo.at(13)) + std::stol(pidcpuinfo.at(14)));
	} catch(...) {
		term.out<Error, Threadless>("Failed to add PID jiffies");
	}
	for (new_cpujiffies = 0; const auto& it : procstatinfo) { //add together all the number parameters in procstatinfo
		try {
			new_cpujiffies += std::stol(it); //even though we are adding the PID of the process, it doesn't matter because we will only care about the deltas
		} catch(...) {
			term.out<Error, Threadless>("Failed to add CPU jiffies");
		}
	}
	try {
		long long CPUpercent = (long long)((double)cpuNum * 100.0 * ((new_pidjiffies - old_pidjiffies) / (new_cpujiffies - old_cpujiffies)));
		CPUreadings.push_back(CPUpercent);
		while (CPUreadings.size() > 60) {
			CPUreadings.pop_front();
		}
		PIDjiffies = new_pidjiffies;
		CPUjiffies = new_cpujiffies;
	} catch(...) {
		term.out<Error, Threadless>("Error updating CPU usage");
	}
	procstat.close();
	#else
	//macOS and BSD here
	#endif
}

void Server::updateRAMusage() {
	std::lock_guard<std::mutex> lock(perfMutex);
	auto addReading = [](auto& list, const auto& entry) {
		list.push_back(entry);
		while (list.size() > 240) {
			list.pop_front();
		}
	};
	#if defined(_WIN32) || defined(_WIN64)
	//do stuff here
	//update the values in server.hpp
	#elif defined(__linux__)
	string line;
	std::fstream pidprocstatm("/proc/" + std::to_string(pid) + "/statm", std::fstream::in);
	std::getline(pidprocstatm, line);
	std::regex repid("\\S+", std::regex_constants::optimize);
	std::vector<std::string> pidmeminfo;
	for (auto it = std::sregex_iterator(line.begin(), line.end(), repid); it != std::sregex_iterator(); ++it) {
		std::smatch m = *it;
		pidmeminfo.push_back(m.str());
	}
	try {
		addReading(rambytereadings, stol(pidmeminfo.at(1)) * sysconf(_SC_PAGESIZE)); //calculate the bytes of RAM usage
	} catch(...) {
		term.out<Error, Threadless>("Error adding RAM bytes");
	}
	struct sysinfo info;
	sysinfo(&info);
	try {
		addReading(rampercentreadings, (100.0 * (double)stol(pidmeminfo.at(1)) * (double)sysconf(_SC_PAGESIZE)) / info.totalram);
	} catch(...) {
		term.out<Error, Threadless>("Error adding RAM percent");
	}
	pidprocstatm.close();
	#else
	#endif
}

void Server::trimCounterData() {
	auto bumpAndCull = [this](auto& list) {
		list.emplace_back(0);
		while (list.size() > counterMax) {
			list.pop_front();
		}
	};
	bumpAndCull(cpucyclereadings);
	bumpAndCull(cpuinstructionreadings);
	bumpAndCull(cachemissreadings);
	bumpAndCull(branchinstructionreadings);
	bumpAndCull(branchmissreadings);
	bumpAndCull(cachereferencereadings);
	bumpAndCull(stalledcyclesfrontendreadings);
	bumpAndCull(stalledcyclesbackendreadings);
	bumpAndCull(buscyclereadings);
	bumpAndCull(pagefaultreadings);
	bumpAndCull(contextswitchreadings);
	bumpAndCull(cpumigrationreadings);
	bumpAndCull(alignmentfaultreadings);
	bumpAndCull(emulationfaultreadings);
	bumpAndCull(minorpagefaultreadings);
	bumpAndCull(majorpagefaultreadings);
	bumpAndCull(l1dreadaccessreadings);
	bumpAndCull(l1dreadmissreadings);
	bumpAndCull(l1dprefetchaccessreadings);
	bumpAndCull(l1dprefetchmissreadings);
	bumpAndCull(llreadaccessreadings);
	bumpAndCull(llreadmissreadings);
	bumpAndCull(llwriteaccessreadings);
	bumpAndCull(llwritemissreadings);
	bumpAndCull(llprefetchmissreadings);
	bumpAndCull(dtlbreadaccessreadings);
	bumpAndCull(dtlbreadmissreadings);
	bumpAndCull(dtlbwriteaccessreadings);
	bumpAndCull(dtlbwritemissreadings);
	bumpAndCull(dtlbprefetchaccessreadings);
	bumpAndCull(dtlbprefetchmissreadings);
	bumpAndCull(itlbreadaccessreadings);
	bumpAndCull(itlbreadmissreadings);
	bumpAndCull(bpureadaccessreadings);
	bumpAndCull(bpureadmissreadings);
	bumpAndCull(l1dwriteaccessreadings);
	bumpAndCull(l1dwritemissreadings);
	bumpAndCull(l1ireadaccessreadings);
	bumpAndCull(l1ireadmissreadings);
	bumpAndCull(l1iprefetchaccessreadings);
	bumpAndCull(l1iprefetchmissreadings);
}