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

#include "server.hpp"

using std::shared_ptr;
using std::string;
using std::fstream;
using std::to_string;
using std::ofstream;
using std::ios;
using std::vector;
using std::array;
using std::cout;
using namespace std::chrono;

namespace fs = std::filesystem;
namespace ch = std::chrono;

#if defined(__linux__)

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
	std::array<std::array<unsigned long long, 4>, 17> gid{0, 0, 0, 0};
	std::array<std::array<unsigned long long, 4>, 17> gv{0, 0, 0, 0};
	std::array<std::array<int, 4>, 17> gfd{0, 0, 0, 0};

	char buf[1024];
	struct read_format* data = (struct read_format*)buf;
};

vector<long> Server::getProcessChildPids(long pid) {
	std::vector<long> pids;
	std::regex re("/proc/\\d+/task/", std::regex_constants::optimize);
	for (const auto& dir : fs::directory_iterator{"/proc/" + std::to_string(pid) + "/task"}) {
		try {
			pids.emplace_back(stol(std::regex_replace(dir.path().string(), re, "")));
		} catch(...) {
			hjlog.out<Error, Threadless>("Could not add PID to list");
		}
	}
	return pids;
}

void Server::setupCounter(auto& s) {
	if (performanceCounterCompat == -1) {
		return;
	}

	auto configureStruct = [&](auto& st, const auto perftype, const auto config) {
		memset(&(st), 0, sizeof(struct perf_event_attr)); //fill the struct with 0s
		st.type = perftype; //the type of event
		st.size = sizeof(struct perf_event_attr);
		st.config = config; //the event we want to measure
		st.disabled = true;
		st.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID; //format the result in our all-in-one data struct
	};

	auto setupEvent = [&](auto& fd, auto& id, auto& st, auto gfd) {
		if (std::find(knownBadEvents.begin(), knownBadEvents.end(), st.config) != knownBadEvents.end() || performanceCounterCompat == -1) {
			return;
		}
		fd = syscall(__NR_perf_event_open, &(st), s->pid, -1, gfd, 0);
		//std::cout << "fd = " << to_string(fd) << std::endl;
		if (fd != -1) {
			performanceCounterCompat = 1;
			ioctl(fd, PERF_EVENT_IOC_ID, &(id));
		} else if (fd == -1) {
			switch(errno) {
				case E2BIG:
					hjlog.out<Error, Threadless>("Event perfstruct is too small");
					return;
				case EACCES:
					hjlog.out<Error, Threadless>("Performance counters not permitted; try using a newer Linux kernel or assigning Hajime the CAP_PERFMON capability");
					performanceCounterCompat = -1;
					return;
				case EBADF:
					if (gfd > -1) {
						hjlog.out<Error, Threadless>("Event group_fd not valid");
					}
					return;
				case EBUSY:
					hjlog.out<Error, Threadless>("Another process has exclusive access to performance counters");
					performanceCounterCompat = -1;
					return;
				case EFAULT:
					hjlog.out<Error, Threadless>("Invalid memory address");
					return;
				case EINVAL:
					hjlog.out<Error, Threadless>("Invalid event");
					knownBadEvents.push_back(st.config);
					return;
				case EMFILE:
					hjlog.out<Error, Threadless>("Not enough file descriptors available");
					performanceCounterCompat = -1;
					return;
				case ENODEV:
					hjlog.out<Error, Threadless>("Event not supported on this CPU");
					knownBadEvents.push_back(st.config);
					return;
				case ENOENT:
					hjlog.out<Error, Threadless>("Invalid event type");
					knownBadEvents.push_back(st.config);
					return;
				case ENOSPC:
					hjlog.out<Error, Threadless>("Too many hardware breakpoint events");
					return;
				case EOPNOTSUPP:
					hjlog.out<Error, Threadless>("Hardware support not available");
					knownBadEvents.push_back(st.config);
					return;
				case EPERM:
					hjlog.out<Error, Threadless>("Unsupported event exclusion setting");
					return;
				case ESRCH:
					hjlog.out<Error, Threadless>("Invalid PID for event; PID = " + to_string(s->pid));
					return;
				default:
					hjlog.out<Error, Threadless>("Other performance counter error; errno = " + std::to_string(errno));
					return;
			}
		}
	};
	//std::cout << "setting up counters for pid " << s->pid << std::endl;
	//std::cout << "cpu cycles" << std::endl;
	//group 1: hardware
	//std::cout << "cpu cycles" << std::endl;
	configureStruct(s->perfstruct[0][0], PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES);
	setupEvent(s->gfd[0][0], s->gid[0][0], s->perfstruct[0][0], -1);

	configureStruct(s->perfstruct[0][1], PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
	setupEvent(s->gfd[0][1], s->gid[0][1], s->perfstruct[0][1], s->gfd[0][0]);
	//std::cout << "cache misses" << std::endl;
	configureStruct(s->perfstruct[1][0], PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES);
	setupEvent(s->gfd[1][0], s->gid[1][0], s->perfstruct[1][0], -1);

	configureStruct(s->perfstruct[2][0], PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS);
	setupEvent(s->gfd[2][0], s->gid[2][0], s->perfstruct[2][0], -1);
	//std::cout << "branch misses" << std::endl;
	configureStruct(s->perfstruct[2][1], PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES);
	setupEvent(s->gfd[2][1], s->gid[2][1], s->perfstruct[2][1], s->gfd[2][0]);

	configureStruct(s->perfstruct[1][1], PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES);
	setupEvent(s->gfd[1][1], s->gid[1][1], s->perfstruct[1][1], s->gfd[1][0]);
	//std::cout << "stalled cycles" << std::endl;
	configureStruct(s->perfstruct[0][2], PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND);
	setupEvent(s->gfd[0][2], s->gid[0][2], s->perfstruct[0][2], s->gfd[0][0]);

	configureStruct(s->perfstruct[0][3], PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND);
	setupEvent(s->gfd[0][3], s->gid[0][3], s->perfstruct[0][3], s->gfd[0][0]);
	//std::cout << "bus cycles" << std::endl;
	configureStruct(s->perfstruct[2][2], PERF_TYPE_HARDWARE, PERF_COUNT_HW_BUS_CYCLES);
	setupEvent(s->gfd[2][2], s->gid[2][2], s->perfstruct[2][2], s->gfd[2][0]);
	//group 2: software
	configureStruct(s->perfstruct[3][0], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS);
	setupEvent(s->gfd[3][0], s->gid[3][0], s->perfstruct[3][0], -1);

	configureStruct(s->perfstruct[3][1], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MIN);
	setupEvent(s->gfd[3][1], s->gid[3][1], s->perfstruct[3][1], s->gfd[3][0]);

	configureStruct(s->perfstruct[3][2], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MAJ);
	setupEvent(s->gfd[3][2], s->gid[3][2], s->perfstruct[3][2], s->gfd[3][0]);

	configureStruct(s->perfstruct[4][0], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES);
	setupEvent(s->gfd[4][0], s->gid[4][0], s->perfstruct[4][0], -1);

	configureStruct(s->perfstruct[4][1], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_MIGRATIONS);
	setupEvent(s->gfd[4][1], s->gid[4][1], s->perfstruct[4][1], s->gfd[4][0]);

	configureStruct(s->perfstruct[4][2], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_ALIGNMENT_FAULTS);
	setupEvent(s->gfd[4][2], s->gid[4][2], s->perfstruct[4][2], s->gfd[4][0]);
	//std::cout << "emu faults" << std::endl;
	configureStruct(s->perfstruct[4][3], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_EMULATION_FAULTS);
	setupEvent(s->gfd[4][3], s->gid[4][3], s->perfstruct[4][3], s->gfd[4][0]);
	//group 3: cache
	//std::cout << "l1d cache" << std::endl;
	configureStruct(s->perfstruct[5][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	setupEvent(s->gfd[5][0], s->gid[5][0], s->perfstruct[5][0], -1);
 	//we need to bitshift the second and third enums by 8 and 16 bits respectively, and we do that with <<
	configureStruct(s->perfstruct[5][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	setupEvent(s->gfd[5][1], s->gid[5][1], s->perfstruct[5][1], s->gfd[5][0]);

	configureStruct(s->perfstruct[6][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	setupEvent(s->gfd[6][0], s->gid[6][0], s->perfstruct[6][0], -1);

	configureStruct(s->perfstruct[6][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	setupEvent(s->gfd[6][1], s->gid[6][1], s->perfstruct[6][1], s->gfd[6][0]);

	configureStruct(s->perfstruct[7][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	setupEvent(s->gfd[7][0], s->gid[7][0], s->perfstruct[7][0], -1);

	configureStruct(s->perfstruct[7][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	setupEvent(s->gfd[7][1], s->gid[7][1], s->perfstruct[7][1], s->gfd[7][0]);
	//std::cout << "dtlb write" << std::endl;
	configureStruct(s->perfstruct[8][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	setupEvent(s->gfd[8][0], s->gid[8][0], s->perfstruct[8][0], -1);

	configureStruct(s->perfstruct[8][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	setupEvent(s->gfd[8][1], s->gid[8][1], s->perfstruct[8][1], s->gfd[8][0]);
	//std::cout << "itlb read" << std::endl;
	configureStruct(s->perfstruct[9][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_ITLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	setupEvent(s->gfd[9][0], s->gid[9][0], s->perfstruct[9][0], -1);

	configureStruct(s->perfstruct[9][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_ITLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	setupEvent(s->gfd[9][1], s->gid[9][1], s->perfstruct[9][1], s->gfd[9][0]);

	configureStruct(s->perfstruct[10][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_BPU | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	setupEvent(s->gfd[10][0], s->gid[10][0], s->perfstruct[10][0], -1);

	configureStruct(s->perfstruct[10][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_BPU | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	setupEvent(s->gfd[10][1], s->gid[10][1], s->perfstruct[10][1], s->gfd[10][0]);

	configureStruct(s->perfstruct[11][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	setupEvent(s->gfd[11][0], s->gid[11][0], s->perfstruct[11][0], -1);

	configureStruct(s->perfstruct[11][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	setupEvent(s->gfd[11][1], s->gid[11][1], s->perfstruct[11][1], s->gfd[11][0]);

	configureStruct(s->perfstruct[11][2], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	setupEvent(s->gfd[11][2], s->gid[11][2], s->perfstruct[11][2], s->gfd[11][0]);

	configureStruct(s->perfstruct[12][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	setupEvent(s->gfd[12][0], s->gid[12][0], s->perfstruct[12][0], -1);

	configureStruct(s->perfstruct[12][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	setupEvent(s->gfd[12][1], s->gid[12][1], s->perfstruct[12][1], s->gfd[12][0]);
	//std::cout << "end" << std::endl;
	configureStruct(s->perfstruct[13][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	setupEvent(s->gfd[13][0], s->gid[13][0], s->perfstruct[13][0], -1);

	configureStruct(s->perfstruct[13][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	setupEvent(s->gfd[13][1], s->gid[13][1], s->perfstruct[13][1], s->gfd[13][0]);

	configureStruct(s->perfstruct[14][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	setupEvent(s->gfd[14][0], s->gid[14][0], s->perfstruct[14][0], -1);
 	//we need to bitshift the second and third enums by 8 and 16 bits respectively, and we do that with <<
	configureStruct(s->perfstruct[14][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	setupEvent(s->gfd[14][1], s->gid[14][1], s->perfstruct[14][1], s->gfd[14][0]);

	configureStruct(s->perfstruct[15][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1I | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	setupEvent(s->gfd[15][0], s->gid[15][0], s->perfstruct[15][0], -1);
 	//we need to bitshift the second and third enums by 8 and 16 bits respectively, and we do that with <<
	configureStruct(s->perfstruct[15][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1I | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	setupEvent(s->gfd[15][1], s->gid[15][1], s->perfstruct[15][1], s->gfd[15][0]);

	configureStruct(s->perfstruct[16][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1I | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	setupEvent(s->gfd[16][0], s->gid[16][0], s->perfstruct[16][0], -1);
 	//we need to bitshift the second and third enums by 8 and 16 bits respectively, and we do that with <<
	configureStruct(s->perfstruct[16][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1I | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	setupEvent(s->gfd[16][1], s->gid[16][1], s->perfstruct[16][1], s->gfd[16][0]);
}

void Server::createCounters(vector<struct pcounter*>& counters, const vector<long>& pids) {
	for (const auto& it : pids) {
		counters.emplace_back(new pcounter);
		counters.back()->pid = it;
		//std::cout << "creating counter for pid " << counters.back()->pid << std::endl;
		setupCounter(counters.back());
	}
}

void Server::cullCounters(vector<struct pcounter*>& counters, const vector<long>& pids) {
	for (const auto culledpid : pids) {
		for (auto& s : counters) {
			if (s->pid == culledpid) {
				for (const auto group : s->gfd) {
					for (const auto filedescriptor : group) {
						if (filedescriptor > 2) { //check that we are not closing a built-in file descriptor for stdout, stdin, or stderr
							std::cout << "closing fd " << filedescriptor << std::endl;
							close(filedescriptor);
						}
					}
				}
				//std::cout << "culling counter for pid " << s->pid << std::endl;
				delete s;
				counters.erase(std::find(begin(counters), end(counters), s));
			}
		}
	}
}

void Server::resetAndEnableCounters(auto& counters) {
	for (auto& s : counters) {
		for (auto& group : s->gfd) {
			ioctl(group[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP); //reset the counters for ALL the events that are members of group 1 (g1fd1)
			ioctl(group[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
		}
	}
}

void Server::disableCounters(auto& counters) {
	for (auto& s : counters) {
		for (auto& group : s->gfd) {
			ioctl(group[0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP); //disable all counters in the groups
		}
	}
}

void Server::readCounters(auto& counters) {
	for (auto& s : counters) {
		//memset(&(s->buf), 0, sizeof(s->buf));
		long size = read(s->gfd[0][0], s->buf, sizeof(s->buf)); //get information from the counters
		//std::cout << "size for g1 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) { //read data from all the events in the struct pointed to by data
				if (s->data->values[i].id == s->gid[0][0]) { //data->values[i].id points to an event id, and we want to match this id to the one belonging to event 1
					s->gv[0][0] = s->data->values[i].value; //store the counter value in g1v1
				} else if (s->data->values[i].id == s->gid[0][1]) {
					s->gv[0][1] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[0][2]) {
					s->gv[0][2] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[0][3]) {
					s->gv[0][3] = s->data->values[i].value;
				}
			}
		}

		//memset(&(s->buf), 0, sizeof(s->buf));
		size = read(s->gfd[1][0], s->buf, sizeof(s->buf));
		//std::cout << "size for g2 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[1][0]) {
					s->gv[1][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[1][1]) {
					s->gv[1][1] = s->data->values[i].value;
				}
			}
		}

		//memset(&(s->buf), 0, sizeof(s->buf));
		size = read(s->gfd[2][0], s->buf, sizeof(s->buf));
		//std::cout << "size for g3 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[2][0]) {
					s->gv[2][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[2][1]) {
					s->gv[2][1] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[2][2]) {
					s->gv[2][2] = s->data->values[i].value;
				}
			}
		}

		size = read(s->gfd[3][0], s->buf, sizeof(s->buf)); //get information from the counters
		//std::cout << "size for g1 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[3][0]) {
					s->gv[3][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[3][1]) {
					s->gv[3][1] = s->data->values[i].value;
				}  else if (s->data->values[i].id == s->gid[3][2]) {
					s->gv[3][2] = s->data->values[i].value;
				}
			}
		}

		size = read(s->gfd[4][0], s->buf, sizeof(s->buf)); //get information from the counters
		//std::cout << "size for g1 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[4][0]) {
					s->gv[4][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[4][1]) {
					s->gv[4][1] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[4][2]) {
					s->gv[4][2] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[4][3]) {
					s->gv[4][3] = s->data->values[i].value;
				}
			}
		}

		size = read(s->gfd[5][0], s->buf, sizeof(s->buf)); //get information from the counters
		//std::cout << "size for g1 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[5][0]) {
					s->gv[5][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[5][1]) {
					s->gv[5][1] = s->data->values[i].value;
				}
			}
		}

		size = read(s->gfd[6][0], s->buf, sizeof(s->buf)); //get information from the counters
		//std::cout << "size for g1 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[6][0]) {
					s->gv[6][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[6][1]) {
					s->gv[6][1] = s->data->values[i].value;
				}
			}
		}

		size = read(s->gfd[7][0], s->buf, sizeof(s->buf)); //get information from the counters
		//std::cout << "size for g1 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[7][0]) {
					s->gv[7][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[7][1]) {
					s->gv[7][1] = s->data->values[i].value;
				}
			}
		}

		size = read(s->gfd[8][0], s->buf, sizeof(s->buf)); //get information from the counters
		//std::cout << "size for g1 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[8][0]) {
					s->gv[8][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[8][1]) {
					s->gv[8][1] = s->data->values[i].value;
				}
			}
		}

		size = read(s->gfd[9][0], s->buf, sizeof(s->buf)); //get information from the counters
		//std::cout << "size for g1 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[9][0]) {
					s->gv[9][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[9][1]) {
					s->gv[9][1] = s->data->values[i].value;
				}
			}
		}

		size = read(s->gfd[10][0], s->buf, sizeof(s->buf)); //get information from the counters
		//std::cout << "size for g1 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[10][0]) {
					s->gv[10][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[10][1]) {
					s->gv[10][1] = s->data->values[i].value;
				}
			}
		}

		size = read(s->gfd[11][0], s->buf, sizeof(s->buf)); //get information from the counters
		//std::cout << "size for g1 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[11][0]) {
					s->gv[11][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[11][1]) {
					s->gv[11][1] = s->data->values[i].value;
				}
			}
		}


		size = read(s->gfd[12][0], s->buf, sizeof(s->buf)); //get information from the counters
		//std::cout << "size for g1 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[12][0]) {
					s->gv[12][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[12][1]) {
					s->gv[12][1] = s->data->values[i].value;
				}
			}
		}

		size = read(s->gfd[13][0], s->buf, sizeof(s->buf)); //get information from the counters
		//std::cout << "size for g1 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[13][0]) {
					s->gv[13][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[13][1]) {
					s->gv[13][1] = s->data->values[i].value;
				}
			}
		}

		size = read(s->gfd[14][0], s->buf, sizeof(s->buf)); //get information from the counters
		//std::cout << "size for g1 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[14][0]) {
					s->gv[14][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[14][1]) {
					s->gv[14][1] = s->data->values[i].value;
				}
			}
		}

		size = read(s->gfd[15][0], s->buf, sizeof(s->buf)); //get information from the counters
		//std::cout << "size for g1 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[15][0]) {
					s->gv[15][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[15][1]) {
					s->gv[15][1] = s->data->values[i].value;
				}
			}
		}

		size = read(s->gfd[16][0], s->buf, sizeof(s->buf)); //get information from the counters
		//std::cout << "size for g1 = " << size << std::endl;
		if (size != -1) {
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->gid[16][0]) {
					s->gv[16][0] = s->data->values[i].value;
				} else if (s->data->values[i].id == s->gid[16][1]) {
					s->gv[16][1] = s->data->values[i].value;
				}
			}
		}
	}
}
#endif

void Server::processPerfStats() {
	std::this_thread::sleep_for(std::chrono::seconds(15));
	#if defined(__linux__)
	std::vector<struct pcounter*> MyCounters = {};
	hjlog.out<Debug>("Making performance counters");
	vector<long> newPids = {};
	vector<long> diffPids = {};
	vector<long> currentPids = getProcessChildPids(pid);
	createCounters(MyCounters, currentPids);
	hjlog.out<Debug>("Done making performance counters");
	#endif
	while (true) {
		#if defined(__linux__)
		resetAndEnableCounters(MyCounters);
		#endif
		updateCPUusage(cpuusagereadings);
		updateRAMusage();
		std::this_thread::sleep_for(std::chrono::seconds(15));
		if (performanceCounterCompat != -1) {
			#if defined(__linux__)
			disableCounters(MyCounters);
			readCounters(MyCounters);
			cpucyclereadings.emplace_back(0);
			cpuinstructionreadings.emplace_back(0);
			cpuusagereadings.emplace_back(0);
			cpumigrationreadings.emplace_back(0);
			rambytereadings.emplace_back(0);
			contextswitchreadings.emplace_back(0);
			pagefaultreadings.emplace_back(0);
			branchinstructionreadings.emplace_back(0);
			branchmissreadings.emplace_back(0);
			cachemissreadings.emplace_back(0);
			cachereferencereadings.emplace_back(0);
			stalledcyclesfrontendreadings.emplace_back(0);
			stalledcyclesbackendreadings.emplace_back(0);
			buscyclereadings.emplace_back(0);
			alignmentfaultreadings.emplace_back(0);
			emulationfaultreadings.emplace_back(0);
			minorpagefaultreadings.emplace_back(0);
			majorpagefaultreadings.emplace_back(0);
			l1dreadaccessreadings.emplace_back(0);
			l1dreadmissreadings.emplace_back(0);
			l1dprefetchaccessreadings.emplace_back(0);
			l1dprefetchmissreadings.emplace_back(0);
			llreadaccessreadings.emplace_back(0);
			llreadmissreadings.emplace_back(0);
			llwriteaccessreadings.emplace_back(0);
			llwritemissreadings.emplace_back(0);
			llprefetchmissreadings.emplace_back(0);
			dtlbreadaccessreadings.emplace_back(0);
			dtlbreadmissreadings.emplace_back(0);
			dtlbwriteaccessreadings.emplace_back(0);
			dtlbwritemissreadings.emplace_back(0);
			itlbreadaccessreadings.emplace_back(0);
			itlbreadmissreadings.emplace_back(0);
			bpureadaccessreadings.emplace_back(0);
			bpureadmissreadings.emplace_back(0);
			dtlbprefetchaccessreadings.emplace_back(0);
			dtlbprefetchmissreadings.emplace_back(0);
			l1dwriteaccessreadings.emplace_back(0);
			l1dwritemissreadings.emplace_back(0);
			l1ireadaccessreadings.emplace_back(0);
			l1ireadmissreadings.emplace_back(0);
			l1iprefetchaccessreadings.emplace_back(0);
			l1iprefetchmissreadings.emplace_back(0);
			for (const auto& s : MyCounters) {
				cpucyclereadings.back() += s->gv[0][0];
				cpuinstructionreadings.back() += s->gv[0][1];
				cachemissreadings.back() += s->gv[1][0];
				branchinstructionreadings.back() += s->gv[2][0];
				branchmissreadings.back() += s->gv[2][1];
				cachereferencereadings.back() += s->gv[1][1];
				stalledcyclesfrontendreadings.back() += s->gv[0][2];
				stalledcyclesbackendreadings.back() += s->gv[0][3];
				buscyclereadings.back() += s->gv[2][2];
				pagefaultreadings.back() += s->gv[3][0];
				contextswitchreadings.back() += s->gv[4][0];
				cpumigrationreadings.back() += s->gv[4][1];
				alignmentfaultreadings.back() += s->gv[4][2];
				emulationfaultreadings.back() += s->gv[4][3];
				minorpagefaultreadings.back() += s->gv[3][1];
				majorpagefaultreadings.back() += s->gv[3][2];
				l1dreadaccessreadings.back() += s->gv[5][0];
				l1dreadmissreadings.back() += s->gv[5][1];
				llreadaccessreadings.back() += s->gv[6][0];
				llreadmissreadings.back() += s->gv[6][1];
				dtlbreadaccessreadings.back() += s->gv[7][0];
				dtlbreadmissreadings.back() += s->gv[7][1];
				dtlbwriteaccessreadings.back() += s->gv[8][0];
				dtlbwritemissreadings.back() += s->gv[8][1];
				itlbreadaccessreadings.back() += s->gv[9][0];
				itlbreadmissreadings.back() += s->gv[9][1];
				bpureadaccessreadings.back() += s->gv[10][0];
				bpureadmissreadings.back() += s->gv[10][1];
				llwriteaccessreadings.back() += s->gv[11][0];
				llwritemissreadings.back() += s->gv[11][1];
				llprefetchmissreadings.back() += s->gv[11][2];
				dtlbprefetchaccessreadings.back() += s->gv[12][0];
				dtlbprefetchmissreadings.back() += s->gv[12][1];
				l1dprefetchaccessreadings.back() += s->gv[13][0];
				l1dprefetchmissreadings.back() += s->gv[13][1];
				l1dwriteaccessreadings.back() += s->gv[14][0];
				l1dwritemissreadings.back() += s->gv[14][1];
				l1ireadaccessreadings.back() += s->gv[15][0];
				l1ireadmissreadings.back() += s->gv[15][1];
				l1iprefetchaccessreadings.back() += s->gv[16][0];
				l1iprefetchmissreadings.back() += s->gv[16][1];
			}
			//std::cout << "cache readings: " << L1dReadAccesses << " and " << DTLBReadMisses << std::endl;
			auto cullList = [](auto& list) {
				while (list.size() > 240) {
					list.pop_front();
				}
			};
			cullList(cpucyclereadings);
			cullList(cpuinstructionreadings);
			cullList(cachemissreadings);
			cullList(branchinstructionreadings);
			cullList(branchmissreadings);
			cullList(cachereferencereadings);
			cullList(stalledcyclesfrontendreadings);
			cullList(stalledcyclesbackendreadings);
			cullList(pagefaultreadings);
			cullList(contextswitchreadings);
			cullList(cpumigrationreadings);
			cullList(alignmentfaultreadings);
			cullList(emulationfaultreadings);
			cullList(minorpagefaultreadings);
			cullList(majorpagefaultreadings);
			cullList(l1dreadaccessreadings);
			cullList(l1dreadmissreadings);
			cullList(l1dprefetchaccessreadings);
			cullList(l1dprefetchmissreadings);
			cullList(llreadaccessreadings);
			cullList(llreadmissreadings);
			cullList(llwriteaccessreadings);
			cullList(llwritemissreadings);
			cullList(llprefetchmissreadings);
			cullList(dtlbreadaccessreadings);
			cullList(dtlbreadmissreadings);
			cullList(dtlbwriteaccessreadings);
			cullList(dtlbprefetchaccessreadings);
			cullList(dtlbprefetchmissreadings);
			cullList(itlbreadaccessreadings);
			cullList(itlbreadmissreadings);
			cullList(bpureadaccessreadings);
			cullList(bpureadmissreadings);
			cullList(l1dwriteaccessreadings);
			cullList(l1dwritemissreadings);
			cullList(l1ireadaccessreadings);
			cullList(l1ireadmissreadings);
			cullList(l1iprefetchaccessreadings);
			cullList(l1iprefetchmissreadings);
			newPids = getProcessChildPids(pid);
			diffPids.clear();
			std::set_difference(newPids.begin(), newPids.end(), currentPids.begin(), currentPids.end(), std::inserter(diffPids, diffPids.begin())); //calculate what's in newPids that isn't in oldPids
			createCounters(MyCounters, diffPids);
			diffPids.clear();
			std::set_difference(currentPids.begin(), currentPids.end(), newPids.begin(), newPids.end(), std::inserter(diffPids, diffPids.begin())); //calculate what's in newPids that isn't in oldPids;
			cullCounters(MyCounters, diffPids);
			currentPids = newPids;
			#endif
		}
	}
}

void Server::updateCPUusage(std::list<long long>& CPUreadings) {
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
	std::fstream pidprocstat("/proc/" + to_string(pid) + "/stat", std::fstream::in);
	std::getline(pidprocstat, line);
	std::regex repid("\\S+", std::regex_constants::optimize);
	std::vector<std::string> pidcpuinfo;
	for (auto it = std::sregex_iterator(line.begin(), line.end(), repid); it != std::sregex_iterator(); ++it) {
		std::smatch m = *it;
		pidcpuinfo.push_back(m.str());
	}
	if (pidcpuinfo.size() < 15) {
		hjlog.out<Error, Threadless>("Could not get CPU usage info");
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
		hjlog.out<Error, Threadless>("Failed to add PID jiffies");
	}
	for (new_cpujiffies = 0; const auto& it : procstatinfo) { //add together all the number parameters in procstatinfo
		try {
			new_cpujiffies += std::stol(it); //even though we are adding the PID of the process, it doesn't matter because we will only care about the deltas
		} catch(...) {
			hjlog.out<Error, Threadless>("Failed to add CPU jiffies");
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
		hjlog.out<Error, Threadless>("Error updating CPU usage");
	}
	procstat.close();
	#else
	//macOS and BSD here
	#endif
}

void Server::updateRAMusage() {
	auto addReading = [](auto& list, const auto& entry) {
		list.push_back(entry);
		while (list.size() > 60) {
			list.pop_front();
		}
	};
	#if defined(_WIN32) || defined(_WIN64)
	//do stuff here
	//update the values in server.hpp
	#elif defined(__linux__)
	string line;
	std::fstream pidprocstatm("/proc/" + to_string(pid) + "/statm", std::fstream::in);
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
		hjlog.out<Error, Threadless>("Error adding RAM bytes");
	}
	struct sysinfo info;
	sysinfo(&info);
	try {
		addReading(rampercentreadings, (100.0 * (double)stol(pidmeminfo.at(1)) * (double)sysconf(_SC_PAGESIZE)) / info.totalram);
	} catch(...) {
		hjlog.out<Error, Threadless>("Error adding RAM percent");
	}
	pidprocstatm.close();
	#else
	#endif
}
