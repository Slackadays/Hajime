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

	vector<vector<struct perf_event_attr>> perfstruct{vector<struct perf_event_attr>(9), vector<struct perf_event_attr>(7), vector<struct perf_event_attr>(14)};
	vector<vector<unsigned long long>> gid{vector<unsigned long long>(9, 0), vector<unsigned long long>(7, 0), vector<unsigned long long>(14, 0)};
	vector<vector<unsigned long long>> gv{vector<unsigned long long>(9, 0), vector<unsigned long long>(7, 0), vector<unsigned long long>(14, 0)};
	vector<vector<int>> gfd{vector<int>(9, 0), vector<int>(7, 0), vector<int>(14, 0)};

	char buf[512];
	struct read_format* data = (struct read_format*)buf;
};

vector<long> Server::getProcessChildPids(long pid) {
	std::vector<long> pids;
	std::regex re("/proc/\\d+/task/", std::regex_constants::optimize);
	for (const auto& dir : fs::directory_iterator{"/proc/" + std::to_string(pid) + "/task"}) {
		try {
			pids.emplace_back(stol(std::regex_replace(dir.path().string(), re, "")));
		} catch(...) {
			hjlog.out("could not add pid to list", Error);
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

	auto setupFD = [&](auto& fd, auto& id) {
		if (performanceCounterCompat == -1) {
			return;
		} else if (fd == -1 && errno == EACCES && performanceCounterCompat == 0) {
			hjlog.out("Error: performance counters not permitted; try using a newer Linux kernel or assigning Hajime the CAP_PERFMON capability", Error);
			performanceCounterCompat = -1;
		} else if (fd == -1 && errno != EACCES) {
			hjlog.out("Performance counter error; errno = " + std::to_string(errno), Error);
		} else if (performanceCounterCompat == 0) {
			performanceCounterCompat = 1;
			ioctl(fd, PERF_EVENT_IOC_ID, &(id));
		} else {
			ioctl(fd, PERF_EVENT_IOC_ID, &(id));
		}
	};
	//std::cout << "setting up counters for pid " << s->pid << std::endl;
	//group 1: hardware
	configureStruct(s->perfstruct[0][0], PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES);
	s->gfd[0][0] = syscall(__NR_perf_event_open, &(s->perfstruct[0][0]), s->pid, -1, -1, 0); //create the group file descriptor to share
	setupFD(s->gfd[0][0], s->gid[0][0]);

	configureStruct(s->perfstruct[0][1], PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
	s->gfd[0][1] = syscall(__NR_perf_event_open, &(s->perfstruct[0][1]), s->pid, -1, s->gfd[0][0], 0); //use our group file descriptor
	setupFD(s->gfd[0][1], s->gid[0][1]);

	configureStruct(s->perfstruct[0][2], PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES);
	s->gfd[0][2] = syscall(__NR_perf_event_open, &(s->perfstruct[0][2]), s->pid, -1, s->gfd[0][0], 0);
	setupFD(s->gfd[0][2], s->gid[0][2]);

	configureStruct(s->perfstruct[0][3], PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS);
	s->gfd[0][3] = syscall(__NR_perf_event_open, &(s->perfstruct[0][3]), s->pid, -1, s->gfd[0][0], 0);
	setupFD(s->gfd[0][3], s->gid[0][3]);

	configureStruct(s->perfstruct[0][4], PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES);
	s->gfd[0][4] = syscall(__NR_perf_event_open, &(s->perfstruct[0][4]), s->pid, -1, s->gfd[0][0], 0);
	setupFD(s->gfd[0][4], s->gid[0][4]);

	configureStruct(s->perfstruct[0][5], PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES);
	s->gfd[0][5] = syscall(__NR_perf_event_open, &(s->perfstruct[0][5]), s->pid, -1, s->gfd[0][0], 0);
	setupFD(s->gfd[0][5], s->gid[0][5]);
	/*
	configureStruct(s->perfstruct[0][6], PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND);
	s->gfd[0][6] = syscall(__NR_perf_event_open, &(s->perfstruct[0][6]), s->pid, -1, s->gfd[0][0], 0);
	setupFD(s->gfd[0][6], s->gid[0][6]);

	configureStruct(s->perfstruct[0][7], PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND);
	s->gfd[0][7] = syscall(__NR_perf_event_open, &(s->perfstruct[0][7]), s->pid, -1, s->gfd[0][0], 0);
	setupFD(s->gfd[0][7], s->gid[0][7]);

	configureStruct(s->perfstruct[0][8], PERF_TYPE_HARDWARE, PERF_COUNT_HW_BUS_CYCLES);
	s->gfd[0][8] = syscall(__NR_perf_event_open, &(s->perfstruct[0][8]), s->pid, -1, s->gfd[0][0], 0);
	setupFD(s->gfd[0][8], s->gid[0][8]);*/
	//group 2: software
	configureStruct(s->perfstruct[1][0], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS);
	s->gfd[1][0] = syscall(__NR_perf_event_open, &(s->perfstruct[1][0]), s->pid, -1, -1, 0);
	setupFD(s->gfd[1][0], s->gid[1][0]);

	configureStruct(s->perfstruct[1][1], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES);
	s->gfd[1][1] = syscall(__NR_perf_event_open, &(s->perfstruct[1][1]), s->pid, -1, s->gfd[1][0], 0); //use our group file descriptor
	setupFD(s->gfd[1][1], s->gid[1][1]);

	configureStruct(s->perfstruct[1][2], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_MIGRATIONS);
	s->gfd[1][2] = syscall(__NR_perf_event_open, &(s->perfstruct[1][2]), s->pid, -1, s->gfd[1][0], 0);
	setupFD(s->gfd[1][2], s->gid[1][2]);

	configureStruct(s->perfstruct[1][3], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_ALIGNMENT_FAULTS);
	s->gfd[1][3] = syscall(__NR_perf_event_open, &(s->perfstruct[1][3]), s->pid, -1, s->gfd[1][0], 0);
	setupFD(s->gfd[1][3], s->gid[1][3]);

	configureStruct(s->perfstruct[1][4], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_EMULATION_FAULTS);
	s->gfd[1][4] = syscall(__NR_perf_event_open, &(s->perfstruct[1][4]), s->pid, -1, s->gfd[1][0], 0);
	setupFD(s->gfd[1][4], s->gid[1][4]);

	configureStruct(s->perfstruct[1][5], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MIN);
	s->gfd[1][5] = syscall(__NR_perf_event_open, &(s->perfstruct[1][5]), s->pid, -1, s->gfd[1][0], 0);
	setupFD(s->gfd[1][5], s->gid[1][5]);

	configureStruct(s->perfstruct[1][6], PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MAJ);
	s->gfd[1][6] = syscall(__NR_perf_event_open, &(s->perfstruct[1][6]), s->pid, -1, s->gfd[1][0], 0);
	setupFD(s->gfd[1][6], s->gid[1][6]);
	//group 3: cache
	configureStruct(s->perfstruct[2][0], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	s->gfd[2][0] = syscall(__NR_perf_event_open, &(s->perfstruct[2][0]), s->pid, -1, -1, 0); //create the group file descriptor to share
	setupFD(s->gfd[2][0], s->gid[2][0]);
 	//we need to bitshift the second and third enums by 8 and 16 bits respectively, and we do that with <<
	configureStruct(s->perfstruct[2][1], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	s->gfd[2][1] = syscall(__NR_perf_event_open, &(s->perfstruct[2][1]), s->pid, -1, s->gfd[2][0], 0);
	setupFD(s->gfd[2][1], s->gid[2][1]);

	configureStruct(s->perfstruct[2][2], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	s->gfd[2][2] = syscall(__NR_perf_event_open, &(s->perfstruct[2][2]), s->pid, -1, s->gfd[2][0], 0);
	setupFD(s->gfd[2][2], s->gid[2][2]);

	configureStruct(s->perfstruct[2][3], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	s->gfd[2][3] = syscall(__NR_perf_event_open, &(s->perfstruct[2][3]), s->pid, -1, s->gfd[2][0], 0);
	setupFD(s->gfd[2][3], s->gid[2][3]);

	configureStruct(s->perfstruct[2][4], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	s->gfd[2][4] = syscall(__NR_perf_event_open, &(s->perfstruct[2][4]), s->pid, -1, s->gfd[2][0], 0);
	setupFD(s->gfd[2][4], s->gid[2][4]);

	configureStruct(s->perfstruct[2][5], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	s->gfd[2][5] = syscall(__NR_perf_event_open, &(s->perfstruct[2][5]), s->pid, -1, s->gfd[2][0], 0);
	setupFD(s->gfd[2][5], s->gid[2][5]);

	configureStruct(s->perfstruct[2][6], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	s->gfd[2][6] = syscall(__NR_perf_event_open, &(s->perfstruct[2][6]), s->pid, -1, s->gfd[2][0], 0);
	setupFD(s->gfd[2][6], s->gid[2][6]);

	configureStruct(s->perfstruct[2][7], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	s->gfd[2][7] = syscall(__NR_perf_event_open, &(s->perfstruct[2][7]), s->pid, -1, s->gfd[2][0], 0);
	setupFD(s->gfd[2][7], s->gid[2][7]);
	/*
	configureStruct(s->perfstruct[2][8], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_ITLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	s->gfd[2][8] = syscall(__NR_perf_event_open, &(s->perfstruct[2][8]), s->pid, -1, s->gfd[2][0], 0);
	setupFD(s->gfd[2][8], s->gid[2][8]);

	configureStruct(s->perfstruct[2][9], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_ITLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	s->gfd[2][9] = syscall(__NR_perf_event_open, &(s->perfstruct[2][9]), s->pid, -1, s->gfd[2][0], 0);
	setupFD(s->gfd[2][9], s->gid[2][9]);

	configureStruct(s->perfstruct[2][10], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_BPU | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	s->gfd[2][10] = syscall(__NR_perf_event_open, &(s->perfstruct[2][10]), s->pid, -1, s->gfd[2][0], 0);
	setupFD(s->gfd[2][10], s->gid[2][10]);

	configureStruct(s->perfstruct[2][11], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_BPU | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	s->gfd[2][11] = syscall(__NR_perf_event_open, &(s->perfstruct[2][11]), s->pid, -1, s->gfd[2][0], 0);
	setupFD(s->gfd[2][11], s->gid[2][11]);
	*/
	configureStruct(s->perfstruct[2][12], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	s->gfd[2][12] = syscall(__NR_perf_event_open, &(s->perfstruct[2][12]), s->pid, -1, s->gfd[2][0], 0);
	setupFD(s->gfd[2][12], s->gid[2][12]);

	configureStruct(s->perfstruct[2][13], PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	s->gfd[2][13] = syscall(__NR_perf_event_open, &(s->perfstruct[2][13]), s->pid, -1, s->gfd[2][0], 0);
	setupFD(s->gfd[2][13], s->gid[2][13]);
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
		for (int i = 0; i < s->data->nr; i++) { //read data from all the events in the struct pointed to by data
			if (s->data->values[i].id == s->gid[0][0]) { //data->values[i].id points to an event id, and we want to match this id to the one belonging to event 1
				s->gv[0][0] = s->data->values[i].value; //store the counter value in g1v1
			} else if (s->data->values[i].id == s->gid[0][1]) {
				s->gv[0][1] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[0][2]) {
				s->gv[0][2] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[0][3]) {
				s->gv[0][3] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[0][4]) {
				s->gv[0][4] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[0][5]) {
				s->gv[0][5] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[0][6]) {
				s->gv[0][6] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[0][7]) {
				s->gv[0][7] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[0][8]) {
				s->gv[0][8] = s->data->values[i].value;
			}
		}
		//memset(&(s->buf), 0, sizeof(s->buf));
		size = read(s->gfd[1][0], s->buf, sizeof(s->buf));
		//std::cout << "size for g2 = " << size << std::endl;
		for (int i = 0; i < s->data->nr; i++) {
			if (s->data->values[i].id == s->gid[1][0]) {
				s->gv[1][0] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[1][1]) {
				s->gv[1][1] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[1][2]) {
				s->gv[1][2] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[1][3]) {
				s->gv[1][3] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[1][4]) {
				s->gv[1][4] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[1][5]) {
				s->gv[1][5] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[1][6]) {
				s->gv[1][6] = s->data->values[i].value;
			}
		}
		//memset(&(s->buf), 0, sizeof(s->buf));
		size = read(s->gfd[2][0], s->buf, sizeof(s->buf));
		//std::cout << "size for g3 = " << size << std::endl;
		for (int i = 0; i < s->data->nr; i++) {
			if (s->data->values[i].id == s->gid[2][0]) {
				s->gv[2][0] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[2][1]) {
				s->gv[2][1] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[2][2]) {
				s->gv[2][2] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[2][3]) {
				s->gv[2][3] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[2][4]) {
				s->gv[2][4] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[2][5]) {
				s->gv[2][5] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[2][6]) {
				s->gv[2][6] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[2][7]) {
				s->gv[2][7] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[2][8]) {
				s->gv[2][8] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[2][9]) {
				s->gv[2][9] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[2][10]) {
				s->gv[2][10] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[2][11]) {
				s->gv[2][11] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[2][12]) {
				s->gv[2][12] = s->data->values[i].value;
			} else if (s->data->values[i].id == s->gid[2][13]) {
				s->gv[2][13] = s->data->values[i].value;
			}
		}
	}
}
#endif

void Server::processPerfStats() {
	std::this_thread::sleep_for(std::chrono::seconds(5));
	#if defined(__linux__)
	struct rlimit rlimits;
	rlimits.rlim_cur = 8192; //soft
	rlimits.rlim_max = 8192; //hard
	if (setrlimit(RLIMIT_NOFILE, &rlimits) == -1) {
		hjlog.out("error changing limits, errno = " + std::to_string(errno), Error);
	}
	std::vector<struct pcounter*> MyCounters = {};
	//std::cout << "creating counters" << std::endl;
	vector<long> newPids = {};
	vector<long> diffPids = {};
	vector<long> currentPids = getProcessChildPids(pid);
	createCounters(MyCounters, currentPids);
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
			llreadaccessreadings.emplace_back(0);
			llreadmissreadings.emplace_back(0);
			llwriteaccessreadings.emplace_back(0);
			llwritemissreadings.emplace_back(0);
			dtlbreadaccessreadings.emplace_back(0);
			dtlbreadmissreadings.emplace_back(0);
			dtlbwriteaccessreadings.emplace_back(0);
			dtlbwritemissreadings.emplace_back(0);
			itlbreadaccessreadings.emplace_back(0);
			itlbreadmissreadings.emplace_back(0);
			bpureadaccessreadings.emplace_back(0);
			bpureadmissreadings.emplace_back(0);
			for (const auto& s : MyCounters) {
				cpucyclereadings.back() += s->gv[0][0];
				cpuinstructionreadings.back() += s->gv[0][1];
				cachemissreadings.back() += s->gv[0][2];
				branchinstructionreadings.back() += s->gv[0][3];
				branchmissreadings.back() += s->gv[0][4];
				cachereferencereadings.back() += s->gv[0][5];
				stalledcyclesfrontendreadings.back() += s->gv[0][6];
				stalledcyclesbackendreadings.back() += s->gv[0][7];
				buscyclereadings.back() += s->gv[0][8];
				pagefaultreadings.back() += s->gv[1][0];
				contextswitchreadings.back() += s->gv[1][1];
				cpumigrationreadings.back() += s->gv[1][2];
				alignmentfaultreadings.back() += s->gv[1][3];
				emulationfaultreadings.back() += s->gv[1][4];
				minorpagefaultreadings.back() += s->gv[1][5];
				majorpagefaultreadings.back() += s->gv[1][6];
				l1dreadaccessreadings.back() += s->gv[2][0];
				l1dreadmissreadings.back() += s->gv[2][1];
				llreadaccessreadings.back() += s->gv[2][2];
				llreadmissreadings.back() += s->gv[2][3];
				dtlbreadaccessreadings.back() += s->gv[2][4];
				dtlbreadmissreadings.back() += s->gv[2][5];
				dtlbwriteaccessreadings.back() += s->gv[2][6];
				dtlbwritemissreadings.back() += s->gv[2][7];
				itlbreadaccessreadings.back() += s->gv[2][8];
				itlbreadmissreadings.back() += s->gv[2][9];
				bpureadaccessreadings.back() += s->gv[2][10];
				bpureadmissreadings.back() += s->gv[2][11];
				llwriteaccessreadings.back() += s->gv[2][12];
				llwritemissreadings.back() += s->gv[2][13];
			}
			//std::cout << "cache readings: " << L1dReadAccesses << " and " << DTLBReadMisses << std::endl;
			auto cullList = [](auto& list) {
				while (list.size() > 60) {
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
			cullList(llreadaccessreadings);
			cullList(llreadmissreadings);
			cullList(llwriteaccessreadings);
			cullList(llwritemissreadings);
			cullList(dtlbreadaccessreadings);
			cullList(dtlbreadmissreadings);
			cullList(dtlbwriteaccessreadings);
			cullList(itlbreadaccessreadings);
			cullList(bpureadaccessreadings);
			cullList(bpureadmissreadings);
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
		hjlog.out("Could not get CPU usage info", Error);
	}
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
		new_pidjiffies = (std::stol(pidcpuinfo[13]) + std::stol(pidcpuinfo[14]));
	} catch(...) {
		hjlog.out("Failed to add PID jiffies", Error);
	}
	for (new_cpujiffies = 0; const auto& it : procstatinfo) { //add together all the number parameters in procstatinfo
		try {
			new_cpujiffies += std::stol(it); //even though we are adding the PID of the process, it doesn't matter because we will only care about the deltas
		} catch(...) {
			hjlog.out("Failed to add CPU jiffies", Error);
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
		hjlog.out("Error updating CPU usage", Error);
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
		hjlog.out("Error adding RAM bytes", Error);
	}
	struct sysinfo info;
	sysinfo(&info);
	try {
		addReading(rampercentreadings, (100.0 * (double)stol(pidmeminfo.at(1)) * (double)sysconf(_SC_PAGESIZE)) / info.totalram);
	} catch(...) {
		hjlog.out("Error adding RAM percent", Error);
	}
	pidprocstatm.close();
	#else
	#endif
}
