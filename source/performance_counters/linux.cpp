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
	
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>

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

#include "../server/server.hpp"

namespace fs = std::filesystem;
namespace ch = std::chrono;

struct read_format {
	unsigned long long nr = 0; //how many events there are
	struct {
		unsigned long long value; //the value of event nr
		unsigned long long id; //the id of event nr
	} values[];
};

struct pcounter {
	long pid;

	std::array<std::array<struct perf_event_attr, 4>, 17> perfstruct;
	std::array<std::array<unsigned long long, 4>, 17> group_id;
	std::array<std::array<unsigned long long, 4>, 17> group_value;
	std::array<std::array<int, 4>, 17> group_fd;

	char buf[96];
	struct read_format* data = reinterpret_cast<struct read_format*>(buf);
};

std::vector<long> Server::getProcessChildPids(long pid) {
	std::vector<long> pids;
	std::regex re("/proc/\\d+/task/", std::regex_constants::optimize);
	for (const auto& dir : fs::directory_iterator{"/proc/" + std::to_string(pid) + "/task"}) {
		try {
			pids.emplace_back(stol(std::regex_replace(dir.path().string(), re, "")));
		} catch(...) {
			term.out<Error, Threadless>("Could not add PID to list");
		}
	}
	return pids;
}

void Server::setupCounter(auto& s) {
	if (performanceCounterCompat == -1) {
		return;
	}
	auto initArrays = [](auto& arr) {
		for (auto& outergroup : arr) {
			for (auto& innerelement : outergroup) {
				innerelement = 0;
			}
		}
	};
	initArrays(s->group_id);
	initArrays(s->group_value);
	initArrays(s->group_fd);
	auto configureStruct = [&](auto& st, const auto perftype, const auto config) {
		memset(&(st), 0, sizeof(struct perf_event_attr)); //fill the struct with 0s
		st.type = perftype; //the type of event
		st.size = sizeof(struct perf_event_attr);
		st.config = config; //the event we want to measure
		st.disabled = true;
		st.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID; //format the result in our all-in-one data struct
	};

	auto setupEvent = [&](auto& fd, auto& id, auto& st, auto group_fd) {
		if (std::find(knownBadEvents.begin(), knownBadEvents.end(), st.config) != knownBadEvents.end() || performanceCounterCompat == -1) {
			return;
		}
		fd = syscall(__NR_perf_event_open, &(st), s->pid, -1, group_fd, 0);
		//std::cout << "fd = " << std::to_string(fd) << std::endl;
		if (fd > 0) {
			performanceCounterCompat = 1;
			ioctl(fd, PERF_EVENT_IOC_ID, &(id));
		} else if (fd == -1) {
			switch(errno) {
				case E2BIG:
					term.out<Debug, Threadless>(text.debug.counters.perfstructTooSmall);
					return;
				case EACCES:
					term.out<Warning, Threadless>("Performance counters not permitted or available; try using a newer Linux kernel or assigning Hajime the CAP_PERFMON capability");
					performanceCounterCompat = -1;
					return;
				case EBADF:
					if (group_fd > -1) {
						term.out<Debug, Threadless>(text.debug.counters.groupfdNotValid);
					}
					return;
				case EBUSY:
					term.out<Debug, Threadless>(text.debug.counters.exclusiveAccess);
					performanceCounterCompat = -1;
					return;
				case EFAULT:
					term.out<Debug, Threadless>(text.debug.counters.invalidMemoryAddress);
					return;
				case EINVAL:
					term.out<Debug, Threadless>(text.debug.counters.invalidEvent);
					knownBadEvents.push_back(st.config);
					return;
				case EMFILE:
					term.out<Error, Threadless>("Not enough file descriptors available");
					performanceCounterCompat = -1;
					return;
				case ENODEV:
					term.out<Debug, Threadless>(text.debug.counters.eventNotSupported);
					knownBadEvents.push_back(st.config);
					return;
				case ENOENT:
					term.out<Debug, Threadless>(text.debug.counters.invalidEventType);
					knownBadEvents.push_back(st.config);
					return;
				case ENOSPC:
					term.out<Debug, Threadless>(text.debug.counters.tooManyHardwareBreakpoints);
					return;
				case EOPNOTSUPP:
					term.out<Debug, Threadless>(text.debug.counters.hardwareSupport);
					knownBadEvents.push_back(st.config);
					return;
				case EPERM:
					term.out<Debug, Threadless>(text.debug.counters.unsupportedEventExclusion);
					return;
				case ESRCH:
					term.out<Debug, Threadless>(text.debug.counters.invalidPID + std::to_string(s->pid));
					return;
				default:
					term.out<Debug, Threadless>(text.debug.counters.otherError + std::to_string(errno));
					return;
			}
		}
	};
	//std::cout << "setting up counters for pid " << s->pid << std::endl;
	//std::cout << "cpu cycles" << std::endl;
	//group 1: hardware
	//std::cout << "cpu cycles" << std::endl;
	errno = 0;
	configureStruct(s->perfstruct[0][0], PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES);
	setupEvent(s->group_fd[0][0], s->group_id[0][0], s->perfstruct[0][0], -1);
	//std::cout << "Setting up the event and doing the check" << std::endl;
	//std::cout << "errno 1 = " << errno << std::endl;
	if (errno != 0 || std::find(knownBadEvents.begin(), knownBadEvents.end(), s->perfstruct[0][0].config) != knownBadEvents.end()) {
		//std::cout << "Using the alternative event" << std::endl;
		configureStruct(s->perfstruct[0][0], PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
		setupEvent(s->group_fd[0][0], s->group_id[0][0], s->perfstruct[0][0], -1);
	}
	configureStruct(s->perfstruct[0][1], PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
	setupEvent(s->group_fd[0][1], s->group_id[0][1], s->perfstruct[0][1], s->group_fd[0][0]);
	//std::cout << "errno 2 = " << errno << std::endl;
	//std::cout << "cache misses" << std::endl;
	if (serverSettings.counterLevel >= 3) {
		configureStruct(s->perfstruct[0][2], PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND); //this event creates another group within the [0] group because we needed to separate these from the first group
		setupEvent(s->group_fd[0][2], s->group_id[0][2], s->perfstruct[0][2], -1);

		configureStruct(s->perfstruct[0][3], PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND);
		setupEvent(s->group_fd[0][3], s->group_id[0][3], s->perfstruct[0][3], s->group_fd[0][2]);
	}
	configureStruct(s->perfstruct[1][0], PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES);
	setupEvent(s->group_fd[1][0], s->group_id[1][0], s->perfstruct[1][0], -1);

	configureStruct(s->perfstruct[1][1], PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES);
	setupEvent(s->group_fd[1][1], s->group_id[1][1], s->perfstruct[1][1], s->group_fd[1][0]);
	if (serverSettings.counterLevel >= 2) {
		configureStruct(s->perfstruct[2][0], PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS);
		setupEvent(s->group_fd[2][0], s->group_id[2][0], s->perfstruct[2][0], -1);
		//std::cout << "branch misses" << std::endl;
		configureStruct(s->perfstruct[2][1], PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES);
		setupEvent(s->group_fd[2][1], s->group_id[2][1], s->perfstruct[2][1], s->group_fd[2][0]);
	}
	//std::cout << "stalled cycles" << std::endl;
	if (serverSettings.counterLevel >= 3) {
		//std::cout << "bus cycles" << std::endl;
		configureStruct(s->perfstruct[2][2], PERF_TYPE_HARDWARE, PERF_COUNT_HW_BUS_CYCLES);
		setupEvent(s->group_fd[2][2], s->group_id[2][2], s->perfstruct[2][2], s->group_fd[2][0]);
	}

	if (serverSettings.counterLevel >= 2) {
		configureStruct(s->perfstruct[3][0], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS);
		setupEvent(s->group_fd[3][0], s->group_id[3][0], s->perfstruct[3][0], -1);

		configureStruct(s->perfstruct[3][1], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MIN);
		setupEvent(s->group_fd[3][1], s->group_id[3][1], s->perfstruct[3][1], s->group_fd[3][0]);

		configureStruct(s->perfstruct[3][2], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MAJ);
		setupEvent(s->group_fd[3][2], s->group_id[3][2], s->perfstruct[3][2], s->group_fd[3][0]);
	}
	//group 2: software
	if (serverSettings.counterLevel >= 3) {
		configureStruct(s->perfstruct[4][0], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES);
		setupEvent(s->group_fd[4][0], s->group_id[4][0], s->perfstruct[4][0], -1);

		configureStruct(s->perfstruct[4][1], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_MIGRATIONS);
		setupEvent(s->group_fd[4][1], s->group_id[4][1], s->perfstruct[4][1], s->group_fd[4][0]);

		configureStruct(s->perfstruct[4][2], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_ALIGNMENT_FAULTS);
		setupEvent(s->group_fd[4][2], s->group_id[4][2], s->perfstruct[4][2], s->group_fd[4][0]);
		//std::cout << "emu faults" << std::endl;
		configureStruct(s->perfstruct[4][3], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_EMULATION_FAULTS);
		setupEvent(s->group_fd[4][3], s->group_id[4][3], s->perfstruct[4][3], s->group_fd[4][0]);
	}
	//group 3: cache
	//std::cout << "l1d cache" << std::endl;
	if (serverSettings.counterLevel >= 2) {
		configureStruct(s->perfstruct[5][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
		setupEvent(s->group_fd[5][0], s->group_id[5][0], s->perfstruct[5][0], -1);
	 	//we need to bitshift the second and third enums by 8 and 16 bits respectively, and we do that with <<
		configureStruct(s->perfstruct[5][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
		setupEvent(s->group_fd[5][1], s->group_id[5][1], s->perfstruct[5][1], s->group_fd[5][0]);

		configureStruct(s->perfstruct[6][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
		setupEvent(s->group_fd[6][0], s->group_id[6][0], s->perfstruct[6][0], -1);

		configureStruct(s->perfstruct[6][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
		setupEvent(s->group_fd[6][1], s->group_id[6][1], s->perfstruct[6][1], s->group_fd[6][0]);
	}

	if (serverSettings.counterLevel >= 3) {
		configureStruct(s->perfstruct[7][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
		setupEvent(s->group_fd[7][0], s->group_id[7][0], s->perfstruct[7][0], -1);

		configureStruct(s->perfstruct[7][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
		setupEvent(s->group_fd[7][1], s->group_id[7][1], s->perfstruct[7][1], s->group_fd[7][0]);
		//std::cout << "dtlb write" << std::endl;
		configureStruct(s->perfstruct[8][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
		setupEvent(s->group_fd[8][0], s->group_id[8][0], s->perfstruct[8][0], -1);

		configureStruct(s->perfstruct[8][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
		setupEvent(s->group_fd[8][1], s->group_id[8][1], s->perfstruct[8][1], s->group_fd[8][0]);
		//std::cout << "itlb read" << std::endl;
		configureStruct(s->perfstruct[9][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_ITLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
		setupEvent(s->group_fd[9][0], s->group_id[9][0], s->perfstruct[9][0], -1);

		configureStruct(s->perfstruct[9][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_ITLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
		setupEvent(s->group_fd[9][1], s->group_id[9][1], s->perfstruct[9][1], s->group_fd[9][0]);

		configureStruct(s->perfstruct[10][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_BPU | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
		setupEvent(s->group_fd[10][0], s->group_id[10][0], s->perfstruct[10][0], -1);

		configureStruct(s->perfstruct[10][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_BPU | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
		setupEvent(s->group_fd[10][1], s->group_id[10][1], s->perfstruct[10][1], s->group_fd[10][0]);
	}

	if (serverSettings.counterLevel >= 2) {
		configureStruct(s->perfstruct[11][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
		setupEvent(s->group_fd[11][0], s->group_id[11][0], s->perfstruct[11][0], -1);

		configureStruct(s->perfstruct[11][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
		setupEvent(s->group_fd[11][1], s->group_id[11][1], s->perfstruct[11][1], s->group_fd[11][0]);

		configureStruct(s->perfstruct[11][2], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
		setupEvent(s->group_fd[11][2], s->group_id[11][2], s->perfstruct[11][2], s->group_fd[11][0]);
	}

	if (serverSettings.counterLevel >= 3) {
		configureStruct(s->perfstruct[12][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
		setupEvent(s->group_fd[12][0], s->group_id[12][0], s->perfstruct[12][0], -1);

		configureStruct(s->perfstruct[12][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
		setupEvent(s->group_fd[12][1], s->group_id[12][1], s->perfstruct[12][1], s->group_fd[12][0]);
	}

	if (serverSettings.counterLevel >= 2) {
		configureStruct(s->perfstruct[13][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
		setupEvent(s->group_fd[13][0], s->group_id[13][0], s->perfstruct[13][0], -1);

		configureStruct(s->perfstruct[13][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
		setupEvent(s->group_fd[13][1], s->group_id[13][1], s->perfstruct[13][1], s->group_fd[13][0]);

		configureStruct(s->perfstruct[14][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
		setupEvent(s->group_fd[14][0], s->group_id[14][0], s->perfstruct[14][0], -1);
	 	//we need to bitshift the second and third enums by 8 and 16 bits respectively, and we do that with <<
		configureStruct(s->perfstruct[14][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
		setupEvent(s->group_fd[14][1], s->group_id[14][1], s->perfstruct[14][1], s->group_fd[14][0]);

		configureStruct(s->perfstruct[15][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1I | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
		setupEvent(s->group_fd[15][0], s->group_id[15][0], s->perfstruct[15][0], -1);
	 	//we need to bitshift the second and third enums by 8 and 16 bits respectively, and we do that with <<
		configureStruct(s->perfstruct[15][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1I | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
		setupEvent(s->group_fd[15][1], s->group_id[15][1], s->perfstruct[15][1], s->group_fd[15][0]);

		configureStruct(s->perfstruct[16][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1I | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
		setupEvent(s->group_fd[16][0], s->group_id[16][0], s->perfstruct[16][0], -1);
	 	//we need to bitshift the second and third enums by 8 and 16 bits respectively, and we do that with <<
		configureStruct(s->perfstruct[16][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1I | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
		setupEvent(s->group_fd[16][1], s->group_id[16][1], s->perfstruct[16][1], s->group_fd[16][0]);
	}
	//std::cout << "end" << std::endl;
}

void Server::createCounters(std::vector<struct pcounter*>& counters, const std::vector<long>& pids) {
	for (const auto& it : pids) {
		counters.emplace_back(new pcounter);
		counters.back()->pid = it;
		//std::cout << "creating counter for pid " << counters.back()->pid << std::endl;
		setupCounter(counters.back());
	}
}

void Server::cullCounters(std::vector<struct pcounter*>& counters, const std::vector<long>& pids) {
	for (const auto culledpid : pids) {
		for (auto& s : counters) {
			if (s->pid == culledpid) {
				for (const auto group : s->group_fd) {
					for (const auto filedescriptor : group) {
						if (filedescriptor > 2) { //check that we are not closing a built-in file descriptor for stdout, stdin, or stderr
							//std::cout << "closing fd " << filedescriptor << std::endl;
							close(filedescriptor);
						}
					}
				}
				//std::cout << "culling counter for pid " << s->pid << std::endl;
				counters.erase(std::find(begin(counters), end(counters), s));
			}
		}
	}
}

void Server::resetAndEnableCounters(const auto& counters) {
	for (const auto& s : counters) {
		for (const auto& group : s->group_fd) {
			ioctl(group[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP); //reset the counters for ALL the events that are members of group 1 (g1fd1)
			ioctl(group[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
		}
	}
}

void Server::disableCounters(const auto& counters) {
	for (const auto& s : counters) {
		for (const auto& group : s->group_fd) {
			ioctl(group[0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP); //disable all counters in the groups
		}
	}
}

void Server::readCounters(auto& counters) {
	long size;
	for (auto& s : counters) {
		if (s->group_fd[0][0] > 2) {
			size = read(s->group_fd[0][0], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size for g1 = " << size << std::endl;
			if (size >= 40) {
				for (int i = 0; i < s->data->nr; i++) { //read data from all the events in the struct pointed to by data
					if (s->data->values[i].id == s->group_id[0][0]) { //data->values[i].id points to an event id, and we want to match this id to the one belonging to event 1
						s->group_value[0][0] = s->data->values[i].value; //store the counter value in g1v1
					} else if (s->data->values[i].id == s->group_id[0][1]) {
						s->group_value[0][1] = s->data->values[i].value;
					}
				}
			}
		}
		if (s->group_fd[0][2] > 2) {
			size = read(s->group_fd[0][2], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size for g1 = " << size << std::endl;
			if (size >= 40) {
				for (int i = 0; i < s->data->nr; i++) { //read data from all the events in the struct pointed to by data
					if (s->data->values[i].id == s->group_id[0][2]) { //data->values[i].id points to an event id, and we want to match this id to the one belonging to event 1
						s->group_value[0][2] = s->data->values[i].value; //store the counter value in g1v1
					} else if (s->data->values[i].id == s->group_id[0][3]) {
						s->group_value[0][3] = s->data->values[i].value;
					}
				}
			}
		}
		//memset(&(s->buf), 0, sizeof(s->buf));
		if (s->group_fd[1][0] > 2) {
			size = read(s->group_fd[1][0], s->buf, sizeof(s->buf));
			//std::cout << "size = " << size << std::endl;
			if (size >= 40) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[1][0]) {
						s->group_value[1][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[1][1]) {
						s->group_value[1][1] = s->data->values[i].value;
					}
				}
			}
		}
		if (s->group_fd[2][0] > 2) {
			size = read(s->group_fd[2][0], s->buf, sizeof(s->buf));
			//std::cout << "size = " << size << std::endl;
			if (size >= 56) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[2][0]) {
						s->group_value[2][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[2][1]) {
						s->group_value[2][1] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[2][2]) {
						s->group_value[2][2] = s->data->values[i].value;
					}
				}
			}
		}
		if (s->group_fd[3][0] > 2) {
			size = read(s->group_fd[3][0], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size = " << size << std::endl;
			if (size >= 56) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[3][0]) {
						s->group_value[3][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[3][1]) {
						s->group_value[3][1] = s->data->values[i].value;
					}  else if (s->data->values[i].id == s->group_id[3][2]) {
						s->group_value[3][2] = s->data->values[i].value;
					}
				}
			}
		}
		if (s->group_fd[4][0] > 2) {
			size = read(s->group_fd[4][0], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size = " << size << std::endl;
			if (size >= 72) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[4][0]) {
						s->group_value[4][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[4][1]) {
						s->group_value[4][1] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[4][2]) {
						s->group_value[4][2] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[4][3]) {
						s->group_value[4][3] = s->data->values[i].value;
					}
				}
			}
		}

		if (s->group_fd[5][0] > 2) {
			size = read(s->group_fd[5][0], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size = " << size << std::endl;
			if (size >= 40) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[5][0]) {
						s->group_value[5][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[5][1]) {
						s->group_value[5][1] = s->data->values[i].value;
					}
				}
			}
		}
		if (s->group_fd[6][0] > 2) {
			size = read(s->group_fd[6][0], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size for g1 = " << size << std::endl;
			if (size >= 40) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[6][0]) {
						s->group_value[6][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[6][1]) {
						s->group_value[6][1] = s->data->values[i].value;
					}
				}
			}
		}

		if (s->group_fd[7][0] > 2) {
			size = read(s->group_fd[7][0], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size for g1 = " << size << std::endl;
			if (size >= 40) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[7][0]) {
						s->group_value[7][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[7][1]) {
						s->group_value[7][1] = s->data->values[i].value;
					}
				}
			}
		}
		if (s->group_fd[8][0] > 2) {
			size = read(s->group_fd[8][0], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size for g1 = " << size << std::endl;
			if (size >= 40) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[8][0]) {
						s->group_value[8][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[8][1]) {
						s->group_value[8][1] = s->data->values[i].value;
					}
				}
			}
		}

		if (s->group_fd[9][0] > 2) {
			size = read(s->group_fd[9][0], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size for g1 = " << size << std::endl;
			if (size >= 40) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[9][0]) {
						s->group_value[9][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[9][1]) {
						s->group_value[9][1] = s->data->values[i].value;
					}
				}
			}
		}

		if (s->group_fd[10][0] > 2) {
			size = read(s->group_fd[10][0], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size for g1 = " << size << std::endl;
			if (size >= 40) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[10][0]) {
						s->group_value[10][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[10][1]) {
						s->group_value[10][1] = s->data->values[i].value;
					}
				}
			}
		}

		if (s->group_fd[11][0] > 2) {
			size = read(s->group_fd[11][0], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size for g1 = " << size << std::endl;
			if (size >= 56) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[11][0]) {
						s->group_value[11][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[11][1]) {
						s->group_value[11][1] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[11][2]) {
						s->group_value[11][2] = s->data->values[i].value;
					}
				}
			}
		}


		if (s->group_fd[12][0] > 2) {
			size = read(s->group_fd[12][0], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size for g1 = " << size << std::endl;
			if (size >= 40) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[12][0]) {
						s->group_value[12][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[12][1]) {
						s->group_value[12][1] = s->data->values[i].value;
					}
				}
			}
		}

		if (s->group_fd[13][0]) {
			size = read(s->group_fd[13][0], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size for g1 = " << size << std::endl;
			if (size >= 40) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[13][0]) {
						s->group_value[13][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[13][1]) {
						s->group_value[13][1] = s->data->values[i].value;
					}
				}
			}
		}

		if (s->group_fd[14][0] > 2) {
			size = read(s->group_fd[14][0], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size for g1 = " << size << std::endl;
			if (size >= 40) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[14][0]) {
						s->group_value[14][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[14][1]) {
						s->group_value[14][1] = s->data->values[i].value;
					}
				}
			}
		}

		if (s->group_fd[15][0] > 2) {
			size = read(s->group_fd[15][0], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size for g1 = " << size << std::endl;
			if (size >= 40) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[15][0]) {
						s->group_value[15][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[15][1]) {
						s->group_value[15][1] = s->data->values[i].value;
					}
				}
			}
		}

		if (s->group_fd[16][0] > 2) {
			size = read(s->group_fd[16][0], s->buf, sizeof(s->buf)); //get information from the counters
			//std::cout << "size for g1 = " << size << std::endl;
			if (size >= 40) {
				for (int i = 0; i < s->data->nr; i++) {
					if (s->data->values[i].id == s->group_id[16][0]) {
						s->group_value[16][0] = s->data->values[i].value;
					} else if (s->data->values[i].id == s->group_id[16][1]) {
						s->group_value[16][1] = s->data->values[i].value;
					}
				}
			}
		}
	}
}

void Server::processPerfStats() {
	std::this_thread::sleep_for(std::chrono::seconds(5));
	std::vector<struct pcounter*> MyCounters = {};
	std::vector<long> newPids = {};
	std::vector<long> diffPids = {};
	std::vector<long> currentPids;
	if (serverSettings.counterLevel > 0) {
		term.out<Debug>("Making performance counters");
		currentPids = getProcessChildPids(pid);
		createCounters(MyCounters, currentPids);
		term.out<Debug>("Done making performance counters");
	}
	//auto then = std::chrono::high_resolution_clock::now();
	while (true) {
		if (serverSettings.counterLevel > 0 && performanceCounterCompat != -1) {
			resetAndEnableCounters(MyCounters);
		}
		updateCPUusage(counterData.cpuusagereadings);
		updateRAMusage();
		//std::cout << "This took " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - then).count() << " microseconds" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(serverSettings.counterInterval));
		//then = std::chrono::high_resolution_clock::now();
		if (serverSettings.counterLevel > 0 && performanceCounterCompat != -1) {
			std::lock_guard<std::mutex> lock(perfMutex);
			trimCounterData();
			disableCounters(MyCounters);
			readCounters(MyCounters);
			for (const auto& s : MyCounters) {
				counterData.cpucyclereadings.back() += s->group_value[0][0];
				counterData.cpuinstructionreadings.back() += s->group_value[0][1];
				counterData.cachemissreadings.back() += s->group_value[1][0];
				counterData.branchinstructionreadings.back() += s->group_value[2][0];
				counterData.branchmissreadings.back() += s->group_value[2][1];
				counterData.cachereferencereadings.back() += s->group_value[1][1];
				counterData.stalledcyclesfrontendreadings.back() += s->group_value[0][2];
				counterData.stalledcyclesbackendreadings.back() += s->group_value[0][3];
				counterData.buscyclereadings.back() += s->group_value[2][2];
				counterData.pagefaultreadings.back() += s->group_value[3][0];
				counterData.contextswitchreadings.back() += s->group_value[4][0];
				counterData.cpumigrationreadings.back() += s->group_value[4][1];
				counterData.alignmentfaultreadings.back() += s->group_value[4][2];
				counterData.emulationfaultreadings.back() += s->group_value[4][3];
				counterData.minorpagefaultreadings.back() += s->group_value[3][1];
				counterData.majorpagefaultreadings.back() += s->group_value[3][2];
				counterData.l1dreadaccessreadings.back() += s->group_value[5][0];
				counterData.l1dreadmissreadings.back() += s->group_value[5][1];
				counterData.llreadaccessreadings.back() += s->group_value[6][0];
				counterData.llreadmissreadings.back() += s->group_value[6][1];
				counterData.dtlbreadaccessreadings.back() += s->group_value[7][0];
				counterData.dtlbreadmissreadings.back() += s->group_value[7][1];
				counterData.dtlbwriteaccessreadings.back() += s->group_value[8][0];
				counterData.dtlbwritemissreadings.back() += s->group_value[8][1];
				counterData.itlbreadaccessreadings.back() += s->group_value[9][0];
				counterData.itlbreadmissreadings.back() += s->group_value[9][1];
				counterData.bpureadaccessreadings.back() += s->group_value[10][0];
				counterData.bpureadmissreadings.back() += s->group_value[10][1];
				counterData.llwriteaccessreadings.back() += s->group_value[11][0];
				counterData.llwritemissreadings.back() += s->group_value[11][1];
				counterData.llprefetchmissreadings.back() += s->group_value[11][2];
				counterData.dtlbprefetchaccessreadings.back() += s->group_value[12][0];
				counterData.dtlbprefetchmissreadings.back() += s->group_value[12][1];
				counterData.l1dprefetchaccessreadings.back() += s->group_value[13][0];
				counterData.l1dprefetchmissreadings.back() += s->group_value[13][1];
				counterData.l1dwriteaccessreadings.back() += s->group_value[14][0];
				counterData.l1dwritemissreadings.back() += s->group_value[14][1];
				counterData.l1ireadaccessreadings.back() += s->group_value[15][0];
				counterData.l1ireadmissreadings.back() += s->group_value[15][1];
				counterData.l1iprefetchaccessreadings.back() += s->group_value[16][0];
				counterData.l1iprefetchmissreadings.back() += s->group_value[16][1];
			}
			//std::cout << "cache readings: " << L1dReadAccesses << " and " << DTLBReadMisses << std::endl;
			newPids = getProcessChildPids(pid);
			diffPids.clear();
			std::set_difference(newPids.begin(), newPids.end(), currentPids.begin(), currentPids.end(), std::inserter(diffPids, diffPids.begin())); //calculate what's in newPids that isn't in oldPids
			createCounters(MyCounters, diffPids);
			diffPids.clear();
			std::set_difference(currentPids.begin(), currentPids.end(), newPids.begin(), newPids.end(), std::inserter(diffPids, diffPids.begin())); //calculate what's in OldPids that isn't in NewPids
			cullCounters(MyCounters, diffPids);
			currentPids = newPids;
		}
	}
}
