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
#include <termios.h>
#else
#include <unistd.h>
#include <termios.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <iterator>
#include <algorithm>
#include <sstream>
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
#include <ctime>
//#include <format>
#include <random>
#include <array>

#include "getvarsfromfile.hpp"
#include "server.hpp"

using std::shared_ptr;
using std::string;
using std::fstream;
using std::to_string;
using std::ofstream;
using std::ios;
using std::vector;
using std::cout;
using namespace std::chrono;

namespace fs = std::filesystem;
namespace ch = std::chrono;

#if defined(__linux__)

struct read_format {
	unsigned long long nr; //how many events there are
	struct {
		unsigned long long value; //the value of event nr
		unsigned long long id; //the id of event nr
	} values[];
};

struct pcounter {
	long pid;

	unsigned long long g1id1, g1id2, g1id3, g1id4, g1id5, g1id6, g1id7, g1id8; //group 1: hardware counters
	unsigned long long g1v1, g1v2, g1v3, g1v4, g1v5, g1v6, g1v7, g1v8;
	int g1fd1, g1fd2, g1fd3, g1fd4, g1fd5, g1fd6, g1fd7, g1fd8;

	unsigned long long g2id1, g2id2, g2id3, g2id4, g2id5, g2id6, g2id7; //group 2: software counters
	unsigned long long g2v1, g2v2, g2v3, g2v4, g2v5, g2v6, g2v7;
	int g2fd1, g2fd2, g2fd3, g2fd4, g2fd5, g2fd6, g2fd7;

	unsigned long long g3id1, g3id2, g3id3, g3id4, g3id5, g3id6, g3id7, g3id8; //group 2: software counters
	unsigned long long g3v1, g3v2, g3v3, g3v4, g3v5, g3v6, g3v7, g3v8;
	int g3fd1, g3fd2, g3fd3, g3fd4, g3fd5, g3fd6, g3fd7, g3fd8;

	char buf[8192];
	struct read_format* data = (struct read_format*)buf;

	struct perf_event_attr perfstruct1; //create a settings struct
	struct perf_event_attr perfstruct2; //repeat a data struct for each event we want to measure
	struct perf_event_attr perfstruct3;
	struct perf_event_attr perfstruct4;
	struct perf_event_attr perfstruct5;
	struct perf_event_attr perfstruct6;
	//struct perf_event_attr perfstruct7; //not supported on all cpus
	//struct perf_event_attr perfstruct8;
	struct perf_event_attr perfstruct9;
	struct perf_event_attr perfstruct10;
	struct perf_event_attr perfstruct11;
	struct perf_event_attr perfstruct12;
	struct perf_event_attr perfstruct13;
	struct perf_event_attr perfstruct14;
	struct perf_event_attr perfstruct15;
	struct perf_event_attr perfstruct16;
	struct perf_event_attr perfstruct17;
	struct perf_event_attr perfstruct18;
	struct perf_event_attr perfstruct19;
	struct perf_event_attr perfstruct20;
	struct perf_event_attr perfstruct21;
	struct perf_event_attr perfstruct22;
	struct perf_event_attr perfstruct23;
};

vector<long> Server::getProcessChildPids(long pid) {
	std::vector<long> pids;
	std::regex re("/proc/\\d+/task/", std::regex_constants::optimize);
	for (const auto& dir : fs::directory_iterator{"/proc/" + std::to_string(pid) + "/task"}) {
		try {
			pids.emplace_back(stol(std::regex_replace(dir.path().string(), re, "")));
		} catch(...) {
			std::cout << "could not convert group id" << std::endl;
		}
	}
	return pids;
}

void Server::setupCounter(auto& s) {
	auto configureStruct = [&](auto& st, const auto perftype, const auto config) {
		memset(&(st), 0, sizeof(struct perf_event_attr)); //fill the struct with 0s
		st.type = perftype; //the type of event
		st.size = sizeof(struct perf_event_attr);
		st.config = config; //the event we want to measure
		st.disabled = true;
		st.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID; //format the result in our all-in-one data struct
	};

	auto setupFD = [&](auto& fd, auto& id) {
		if (fd == -1) {
			std::cout << "error setting up performance counter event, errno = " << errno << std::endl;
		}
		ioctl(fd, PERF_EVENT_IOC_ID, &(id));
	};
	//std::cout << "setting up counters for pid " << s->pid << std::endl;

	//group 1: hardware
	configureStruct(s->perfstruct1, PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES);
	s->g1fd1 = syscall(__NR_perf_event_open, &(s->perfstruct1), s->pid, -1, -1, 0); //create the group file descriptor to share
	setupFD(s->g1fd1, s->g1id1);

	configureStruct(s->perfstruct2, PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
	s->g1fd2 = syscall(__NR_perf_event_open, &(s->perfstruct2), s->pid, -1, s->g1fd1, 0); //use our group file descriptor
	setupFD(s->g1fd2, s->g1id2);

	configureStruct(s->perfstruct3, PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES);
	s->g1fd3 = syscall(__NR_perf_event_open, &(s->perfstruct3), s->pid, -1, s->g1fd1, 0);
	setupFD(s->g1fd3, s->g1id3);

	configureStruct(s->perfstruct4, PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS);
	s->g1fd4 = syscall(__NR_perf_event_open, &(s->perfstruct4), s->pid, -1, s->g1fd1, 0);
	setupFD(s->g1fd4, s->g1id4);

	configureStruct(s->perfstruct5, PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES);
	s->g1fd5 = syscall(__NR_perf_event_open, &(s->perfstruct5), s->pid, -1, s->g1fd1, 0);
	setupFD(s->g1fd5, s->g1id5);

	configureStruct(s->perfstruct6, PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES);
	s->g1fd6 = syscall(__NR_perf_event_open, &(s->perfstruct6), s->pid, -1, s->g1fd1, 0);
	setupFD(s->g1fd6, s->g1id6);
	/*
	configureStruct(s->perfstruct7, PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND);
	s->g1fd7 = syscall(__NR_perf_event_open, &(s->perfstruct7), s->pid, -1, s->g1fd1, 0);
	setupFD(s->g1fd7, s->g1id7);

	configureStruct(s->perfstruct8, PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND);
	s->g1fd8 = syscall(__NR_perf_event_open, &(s->perfstruct8), s->pid, -1, s->g1fd1, 0);
	setupFD(s->g1fd8, s->g1id8);
	*/
	//group 2: software
	configureStruct(s->perfstruct9, PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS);
	s->g2fd1 = syscall(__NR_perf_event_open, &(s->perfstruct9), s->pid, -1, -1, 0);
	setupFD(s->g2fd1, s->g2id1);

	configureStruct(s->perfstruct10, PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES);
	s->g2fd2 = syscall(__NR_perf_event_open, &(s->perfstruct10), s->pid, -1, s->g2fd1, 0); //use our group file descriptor
	setupFD(s->g2fd2, s->g2id2);

	configureStruct(s->perfstruct11, PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_MIGRATIONS);
	s->g2fd3 = syscall(__NR_perf_event_open, &(s->perfstruct11), s->pid, -1, s->g2fd1, 0);
	setupFD(s->g2fd3, s->g2id3);

	configureStruct(s->perfstruct12, PERF_TYPE_SOFTWARE, PERF_COUNT_SW_ALIGNMENT_FAULTS);
	s->g2fd4 = syscall(__NR_perf_event_open, &(s->perfstruct12), s->pid, -1, s->g2fd1, 0);
	setupFD(s->g2fd4, s->g2id4);

	configureStruct(s->perfstruct13, PERF_TYPE_SOFTWARE, PERF_COUNT_SW_EMULATION_FAULTS);
	s->g2fd5 = syscall(__NR_perf_event_open, &(s->perfstruct13), s->pid, -1, s->g2fd1, 0);
	setupFD(s->g2fd5, s->g2id5);

	configureStruct(s->perfstruct14, PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MIN);
	s->g2fd6 = syscall(__NR_perf_event_open, &(s->perfstruct14), s->pid, -1, s->g2fd1, 0);
	setupFD(s->g2fd6, s->g2id6);

	configureStruct(s->perfstruct15, PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MAJ);
	s->g2fd7 = syscall(__NR_perf_event_open, &(s->perfstruct15), s->pid, -1, s->g2fd1, 0);
	setupFD(s->g2fd7, s->g2id7);
	//group 3: cache
	configureStruct(s->perfstruct16, PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	s->g3fd1 = syscall(__NR_perf_event_open, &(s->perfstruct16), s->pid, -1, -1, 0); //create the group file descriptor to share
	setupFD(s->g3fd1, s->g3id1);
 	//we need to bitshift the second and third enums by 8 and 16 bits respectively, and we do that with <<
	configureStruct(s->perfstruct17, PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	s->g3fd2 = syscall(__NR_perf_event_open, &(s->perfstruct17), s->pid, -1, s->g3fd1, 0);
	setupFD(s->g3fd2, s->g3id2);

	configureStruct(s->perfstruct18, PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	s->g3fd3 = syscall(__NR_perf_event_open, &(s->perfstruct18), s->pid, -1, s->g3fd1, 0);
	setupFD(s->g3fd3, s->g3id3);

	configureStruct(s->perfstruct19, PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	s->g3fd4 = syscall(__NR_perf_event_open, &(s->perfstruct19), s->pid, -1, s->g3fd1, 0);
	setupFD(s->g3fd4, s->g3id4);

	configureStruct(s->perfstruct20, PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	s->g3fd5 = syscall(__NR_perf_event_open, &(s->perfstruct20), s->pid, -1, s->g3fd1, 0);
	setupFD(s->g3fd5, s->g3id5);

	configureStruct(s->perfstruct21, PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	s->g3fd6 = syscall(__NR_perf_event_open, &(s->perfstruct21), s->pid, -1, s->g3fd1, 0);
	setupFD(s->g3fd6, s->g3id6);

	configureStruct(s->perfstruct22, PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
	s->g3fd7 = syscall(__NR_perf_event_open, &(s->perfstruct22), s->pid, -1, s->g3fd1, 0);
	setupFD(s->g3fd7, s->g3id7);

	configureStruct(s->perfstruct23, PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
	s->g3fd8 = syscall(__NR_perf_event_open, &(s->perfstruct23), s->pid, -1, s->g3fd1, 0);
	setupFD(s->g3fd8, s->g3id8);
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
	for (const auto& it : pids) {
		for (auto& s : counters) {
			if (s->pid == it) {
				//std::cout << "culling counter for pid " << s->pid << std::endl;
				close(s->g1fd1);
				close(s->g1fd2);
				close(s->g1fd3);
				close(s->g1fd4);
				close(s->g1fd5);
				close(s->g1fd6);
				//close(s->g1fd7);
				//close(s->g1fd8);
				close(s->g2fd1);
				close(s->g2fd2);
				close(s->g2fd3);
				close(s->g2fd4);
				close(s->g2fd5);
				close(s->g2fd6);
				close(s->g2fd7);
				counters.erase(std::find(begin(counters), end(counters), s));
			}
		}
	}
}
#endif

void Server::processPerfStats() {
	std::list<long long> cpuusagereadings;
	std::list<unsigned long long> cpucyclereadings, cpuinstructionreadings, cachemissreadings, branchinstructionreadings, branchmissreadings, cachereferencereadings, stalledcyclcesfrontendreadings, stalledcyclesbackendreadings;
	std::list<unsigned long long> pagefaultreadings, contextswitchreadings, cpumigrationreadings, alignmentfaultreadings, emulationfaultreadings, minorpagefaultreadings, majorpagefaultreadings;
	std::this_thread::sleep_for(std::chrono::seconds(10));
	#if defined(__linux__)
	struct rlimit rlimits;
	rlimits.rlim_cur = 4096; //soft
	rlimits.rlim_max = 4096; //hard
	if (setrlimit(RLIMIT_NOFILE, &rlimits) == -1) {
		std::cout << "error changing limits, errno = " << errno << std::endl;
	}
	std::vector<struct pcounter*> MyCounters = {};
	std::cout << "creating counters" << std::endl;
	vector<long> newPids = {};
	vector<long> diffPids = {};
	vector<long> currentPids = getProcessChildPids(pid);
	createCounters(MyCounters, currentPids);
	#endif
	while (true) {
		#if defined(__linux__)
		for (auto& s : MyCounters) {
			ioctl(s->g1fd1, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP); //reset the counters for ALL the events that are members of group 1 (g1fd1)
			ioctl(s->g1fd1, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP); //enable all the counters in group 1
			ioctl(s->g2fd1, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
			ioctl(s->g2fd1, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
			ioctl(s->g3fd1, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
			ioctl(s->g3fd1, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
		}
		#endif
		updateCPUusage(cpuusagereadings);
		std::this_thread::sleep_for(std::chrono::seconds(60));
		#if defined(__linux__)
		for (auto& s : MyCounters) {
			ioctl(s->g1fd1, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP); //disable all counters in group 1
			ioctl(s->g2fd1, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
			ioctl(s->g3fd1, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
		}
		for (auto& s : MyCounters) {
			long size = read(s->g1fd1, s->buf, sizeof(s->buf)); //get information from the counters
			for (int i = 0; i < s->data->nr; i++) { //read data from all the events in the struct pointed to by data
				if (s->data->values[i].id == s->g1id1) { //data->values[i].id points to an event id, and we want to match this id to the one belonging to event 1
	      	s->g1v1 = s->data->values[i].value; //store the counter value in g1v1
	    	} else if (s->data->values[i].id == s->g1id2) {
	      	s->g1v2 = s->data->values[i].value;
	    	} else if (s->data->values[i].id == s->g1id3) {
	      	s->g1v3 = s->data->values[i].value;
	    	} else if (s->data->values[i].id == s->g1id4) {
	      	s->g1v4 = s->data->values[i].value;
	    	} else if (s->data->values[i].id == s->g1id5) {
	      	s->g1v5 = s->data->values[i].value;
	    	} else if (s->data->values[i].id == s->g1id6) {
	      	s->g1v6 = s->data->values[i].value;
	    	/*} else if (s->data->values[i].id == s->g1id7) {
	      	s->g1v7 = s->data->values[i].value;
	    	} else if (s->data->values[i].id == s->g1id8) {
	      	s->g1v8 = s->data->values[i].value;*/
	    	}
			}
			size = read(s->g2fd1, s->buf, sizeof(s->buf));
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->g2id1) {
					s->g2v1 = s->data->values[i].value;
				} else if (s->data->values[i].id == s->g2id2) {
					s->g2v2 = s->data->values[i].value;
				} else if (s->data->values[i].id == s->g2id3) {
					s->g2v3 = s->data->values[i].value;
				} else if (s->data->values[i].id == s->g2id4) {
					s->g2v4 = s->data->values[i].value;
				} else if (s->data->values[i].id == s->g2id5) {
					s->g2v5 = s->data->values[i].value;
				} else if (s->data->values[i].id == s->g2id6) {
					s->g2v6 = s->data->values[i].value;
				} else if (s->data->values[i].id == s->g2id7) {
					s->g2v7 = s->data->values[i].value;
				}
			}
			size = read(s->g3fd1, s->buf, sizeof(s->buf));
			for (int i = 0; i < s->data->nr; i++) {
				if (s->data->values[i].id == s->g2id1) {
					s->g3v1 = s->data->values[i].value;
				} else if (s->data->values[i].id == s->g3id2) {
					s->g3v2 = s->data->values[i].value;
				} else if (s->data->values[i].id == s->g3id3) {
					s->g3v3 = s->data->values[i].value;
				} else if (s->data->values[i].id == s->g3id4) {
					s->g3v4 = s->data->values[i].value;
				} else if (s->data->values[i].id == s->g3id5) {
					s->g3v5 = s->data->values[i].value;
				} else if (s->data->values[i].id == s->g3id6) {
					s->g3v6 = s->data->values[i].value;
				} else if (s->data->values[i].id == s->g3id7) {
					s->g3v7 = s->data->values[i].value;
				} else if (s->data->values[i].id == s->g3id8) {
					s->g3v8 = s->data->values[i].value;
				}
			}
		}
		CPUcycles1m = 0;
		CPUinstructions1m = 0;
		cacheMisses1m = 0;
		branchInstructions1m = 0;
		branchMisses1m = 0;
		cacheReferences1m = 0;
		//stalledCyclesFrontend1m = 0;
		//stalledCyclesBackend1m = 0;
		pageFaults1m = 0;
		contextSwitches1m = 0;
		CPUmigrations1m = 0;
		alignmentFaults1m = 0;
		emulationFaults1m = 0;
		minorPagefaults1m = 0;
		majorPagefaults1m = 0;
		for (const auto& s : MyCounters) {
			CPUcycles1m += s->g1v1;
			CPUinstructions1m += s->g1v2;
			cacheMisses1m += s->g1v3;
			branchInstructions1m += s->g1v4;
			branchMisses1m += s->g1v5;
			cacheReferences1m += s->g1v6;
			//stalledCyclesFrontend1m += s->g1v7;
			//stalledCyclesBackend1m += s->g1v8;
			pageFaults1m += s->g2v1;
			contextSwitches1m += s->g2v2;
			CPUmigrations1m += s->g2v3;
			alignmentFaults1m += s->g2v4;
			emulationFaults1m += s->g2v5;
			minorPagefaults1m += s->g2v6;
			majorPagefaults1m += s->g2v7;
		}
		auto addAndAverageReadings = [](auto& list, auto& onemin, auto& fivemin, auto& fifteenmin) {
			list.push_back(onemin);
			while (list.size() > 15) {
				list.pop_front();
			}
			int readings = 0;
			fivemin = 0;
			for (const auto& it : list) {
				fivemin += it;
				readings++;
				if (readings == 5) {
					break;
				}
			}
			if (readings > 0) {
				fivemin /= readings;
			} else {
				std::cout << "/0 error" << std::endl;
			}
			fifteenmin = 0;
			for (readings = 0; const auto& it : list) {
				fifteenmin += it;
				readings++;
			}
			if (readings > 0) {
				fifteenmin /= readings;
			} else {
				std::cout << "/0 error" << std::endl;
			}
		};
		addAndAverageReadings(cpucyclereadings, CPUcycles1m, CPUcycles5m, CPUcycles15m);
		addAndAverageReadings(cpuinstructionreadings, CPUinstructions1m, CPUinstructions5m, CPUinstructions15m);
		addAndAverageReadings(cachemissreadings, cacheMisses1m, cacheMisses5m, cacheMisses15m);
		addAndAverageReadings(branchinstructionreadings, branchInstructions1m, branchInstructions5m, branchInstructions15m);
		addAndAverageReadings(branchmissreadings, branchMisses1m, branchMisses5m, branchMisses15m);
		addAndAverageReadings(cachereferencereadings, cacheReferences1m, cacheReferences5m, cacheReferences15m);
		//addAndAverageReadings(stalledcyclcesfrontendreadings, stalledCyclesFrontend1m, stalledCyclesFrontend5m, stalledCyclesFrontend15m);
		//addAndAverageReadings(stalledcyclesbackendreadings, stalledCyclesBackend1m, stalledCyclesBackend5m, stalledCyclesBackend15m);
		addAndAverageReadings(pagefaultreadings, pageFaults1m, pageFaults5m, pageFaults15m);
		addAndAverageReadings(contextswitchreadings, contextSwitches1m, contextSwitches5m, contextSwitches15m);
		addAndAverageReadings(cpumigrationreadings, CPUmigrations1m, CPUmigrations5m, CPUmigrations15m);
		addAndAverageReadings(alignmentfaultreadings, alignmentFaults1m, alignmentFaults5m, alignmentFaults15m);
		addAndAverageReadings(emulationfaultreadings, emulationFaults1m, emulationFaults5m, emulationFaults15m);
		addAndAverageReadings(minorpagefaultreadings, minorPagefaults1m, minorPagefaults5m, minorPagefaults15m);
		addAndAverageReadings(majorpagefaultreadings, majorPagefaults1m, majorPagefaults5m, majorPagefaults15m);
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
		std::cout << "Could not get CPU usage info" << std::endl;
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
		new_pidjiffies = (std::stoi(pidcpuinfo[13]) + std::stoi(pidcpuinfo[14]));
	} catch(...) {
		std::cout << "Failed to add PID jiffies" << std::endl;
	}
	for (new_cpujiffies = 0; const auto& it : procstatinfo) { //add together all the number parameters in procstatinfo
		try {
			new_cpujiffies += std::stoi(it); //even though we are adding the PID of the process, it doesn't matter because we will only care about the deltas
		} catch(...) {
			std::cout << "Failed to add CPU jiffies" << std::endl;
		}
	}
	try {
		CPUpercent1m = (long long)((double)cpuNum * 100.0 * ((new_pidjiffies - old_pidjiffies) / (new_cpujiffies - old_cpujiffies)));
		CPUreadings.push_back(CPUpercent1m);
		while (CPUreadings.size() > 15) {
			CPUreadings.pop_front();
		}
		int readings = 0;
		CPUpercent5m = 0;
		for (const auto& it : CPUreadings) {
			CPUpercent5m += it;
			readings++;
			if (readings == 5) {
				break;
			}
		}
		if (readings > 0) {
			CPUpercent5m /= readings;
		} else {
			std::cout << "/0 error" << std::endl;
		}
		CPUpercent15m = 0;
		for (readings = 0; const auto& it : CPUreadings) {
			CPUpercent15m += it;
			readings++;
		}
		if (readings > 0) {
			CPUpercent15m /= readings;
		} else {
			std::cout << "/0 error" << std::endl;
		}
		PIDjiffies = new_pidjiffies;
		CPUjiffies = new_cpujiffies;
	} catch(...) {
		std::cout << "Error updating CPU usage" << std::endl;
	}
	#else
	//macOS and BSD here
	#endif
}

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

void Server::processServerCommand(string input) {
	std::smatch m;
	std::regex_search(input, m, std::regex("\\[.+\\]: <(.+)> .+", std::regex_constants::optimize));
	lastCommandUser = m[1];
	if (std::regex_search(input, std::regex("\\" + text.server.command.hajime.regex + "?(?![\\w])", std::regex_constants::optimize))) {
		commandHajime();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.time.regex + "?(?!.\\w)", std::regex_constants::optimize))) {
		commandTime();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.help.regex + "?(?!.\\w)", std::regex_constants::optimize))) {
		commandHelp();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.die.regex + "?(?!.\\w)", std::regex_constants::optimize))) {
		commandDie();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.d20.regex + "(?!.\\w)", std::regex_constants::optimize))) {
		commandD20();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.coinflip.regex + "?(?!.\\w)", std::regex_constants::optimize))) {
		commandCoinflip();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.discord.regex + "?(?!.\\w)", std::regex_constants::optimize))) {
		commandDiscord();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.name.regex + "?(?!.\\w)", std::regex_constants::optimize))) {
		commandName();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.uptime.regex + "?(?!.\\w)", std::regex_constants::optimize))) {
		commandUptime();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.restart.regex + "?(?!.\\w)", std::regex_constants::optimize))) {
		commandRestart();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.system.regex + "?(?!.\\w)", std::regex_constants::optimize))) {
		commandSystem();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\" + text.server.command.perf.regex + "?(?!.\\w)", std::regex_constants::optimize))) {
		commandPerf();
	} else if (std::regex_search(input, std::regex("\\[.+\\]: <.+> \\.ee(?!.\\w)", std::regex_constants::optimize))) {
		writeToServerTerminal(formatWrapper("[Hajime] https://www.youtube.com/watch?v=kjPD_H81hDc"));
	}
}

void Server::commandHajime() {
	writeToServerTerminal(formatWrapper(text.server.command.hajime.output));
}

void Server::commandTime() {
	std::time_t timeNow = std::time(nullptr);
	string stringTimeNow = std::asctime(std::localtime(&timeNow));
	stringTimeNow.erase(std::remove(stringTimeNow.begin(), stringTimeNow.end(), '\n'), stringTimeNow.end());
	string hajInfo = text.server.command.time.output + stringTimeNow + '\"';
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandHelp() {
	writeToServerTerminal(formatWrapper(text.server.command.help.output));
	writeToServerTerminal(formatWrapper("[{\"text\":\"" + text.server.command.coinflip.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.coinflip + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.coinflip.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.die.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.die + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.die.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.d20.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.d20 + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.d20.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.discord.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.discord + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.discord.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.hajime.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.hajime + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.hajime.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.help.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.help + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.help.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.name.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.name + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.name.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.time.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.time + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.time.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.uptime.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.uptime + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.uptime.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.system.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.system + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.system.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.perf.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.perf + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.perf.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.restart.regex + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.restart + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.restart.regex, std::regex("(\\(|\\))"), "") + "\"}}]"));
}

void Server::commandDie() {
	std::random_device rand;
	std::uniform_int_distribution<int> die(1, 6);
	string hajInfo = text.server.command.die.output + std::to_string(die(rand));
	writeToServerTerminal(formatWrapper(addNumberColors(hajInfo)));
	//switch this to C++20 format when it becomes supported
}

void Server::commandD20() {
	std::random_device rand;
	std::uniform_int_distribution<int> die(1, 20);
	string hajInfo = text.server.command.d20.output + std::to_string(die(rand));
	writeToServerTerminal(formatWrapper(addNumberColors(hajInfo)));
	//switch this to C++20 format when it becomes supported
}

void Server::commandCoinflip() {
	std::random_device rand;
	std::uniform_int_distribution<int> flip(1, 2);
	if (flip(rand) == 1) {
		writeToServerTerminal(formatWrapper(text.server.command.coinflip.output.heads));
	} else {
		writeToServerTerminal(formatWrapper(text.server.command.coinflip.output.tails));
	}
}

void Server::commandDiscord() {
	writeToServerTerminal(formatWrapper(text.server.command.discord.output));
}

void Server::commandName() {
	string hajInfo = text.server.command.name.output + name;
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandUptime() {
	string hajInfo = text.server.command.uptime.output1 + std::to_string(uptime) + text.server.command.uptime.output2 + std::to_string(uptime / 60.0) + text.server.command.uptime.output3;
	writeToServerTerminal(formatWrapper(addNumberColors(hajInfo)));
}

void Server::commandRestart() {
	string hajInfo;
	if (restartMins > 0) {
		hajInfo = text.server.command.restart.output1 + std::to_string(restartMins - uptime) + text.server.command.restart.output2 + std::to_string((restartMins - uptime) / 60.0) + text.server.command.restart.output3;
	} else {
		hajInfo = text.server.command.restart.outputDisabled;
	}
	writeToServerTerminal(formatWrapper(addNumberColors(hajInfo)));
}

void Server::commandSystem() {
	string hajInfo;
	hajInfo = "[{\"text\":\"[Hajime] \"},{\"text\":\"OS, \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getOS() + "\"}},"
	"{\"text\":\"" + string("CPU") + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCPU() + "\"}},"
	"{\"text\":\"" + string("RAM") + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getRAM() + "\"}},"
	"{\"text\":\"" + string("uptime") + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getUptime() + "\"}},"
	"{\"text\":\"" + string("loadavg") + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getLoadavg() + "\"}}]";
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandPerf() {
	string hajInfo;
	hajInfo = "[{\"text\":\"[Hajime] \"},{\"text\":\"CPU usage, \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCPUusage() + "\"}},"
	"{\"text\":\"" + string("CPU migrations") + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCPUmigs() + "\"}},"
	"{\"text\":\"" + string("RAM usage") + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getRAMusage() + "\"}},"
	"{\"text\":\"" + string("IPC") + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getIPC() + "\"}},"
	"{\"text\":\"" + string("CPS") + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCPS() + "\"}},"
	"{\"text\":\"" + string("IPS") + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getIPS() + "\"}},"
	"{\"text\":\"" + string("context switches") + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getContextSwitches() + "\"}},"
	"{\"text\":\"" + string("stalled cycles frontend") + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getStalledCyclesFrontend() + "\"}},"
	"{\"text\":\"" + string("stalled cycles backend") + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getStalledCyclesBackend() + "\"}},"
	"{\"text\":\"" + string("branch misses") + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getBranchMisses() + "\"}},"
	"{\"text\":\"" + string("cache misses") + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCacheMisses() + "\"}},"
	"{\"text\":\"" + string("emulation faults") + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getEmulationFaults() + "\"}},"
	"{\"text\":\"" + string("alignment faults") + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getAlignmentFaults() + "\"}}]";
	writeToServerTerminal(formatWrapper(hajInfo));
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
        return to_string(cpucount) + "x " + cpuname;
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
  return to_string(memtotal) + "B total";
	#endif
	return "Only available on Linux or Windows";
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
		return string(m[0]) + " seconds (" + to_string(stoi(m[0]) / 60) + string(" minutes, ") + to_string(stoi(m[0]) / 3600) + " hours)";
	} catch (...) {
		return "Error parsing memory";
	}
	#else
	return string("Only works on Linux");
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
	return "Not available";
	#endif
}

string Server::getCPUusage() {
	return to_string(CPUpercent1m) + "% last 1 minute, " + to_string(CPUpercent5m) + "% last 5, " + to_string(CPUpercent15m) + "% last 15 (Lower is better)";
}

string Server::getCPUmigs() {
	return to_string(CPUmigrations1m) + " last 1 minute, " + to_string(CPUmigrations5m) + " last 5, " + to_string(CPUmigrations15m) + " last 15";
}

string Server::getRAMusage() {
	return to_string(RAMpercent1m) + "% last 1 minute, " + to_string(RAMpercent5m) + "% last 5, " + to_string(RAMpercent15m) + "% last 15 (" + to_string((RAMbytes1m / 1024) / 1024) + "MB/" + to_string((RAMbytes5m / 1024) / 1024) + "MB/" + to_string((RAMbytes15m / 1024) / 1024) + "MB) (Lower is better)";
}

string Server::getIPC() {
	return to_string((double)CPUinstructions1m / (double)CPUcycles1m) + " last 1 minute, " + to_string((double)CPUinstructions5m / (double)CPUcycles5m) + " last 5, " + to_string((double)CPUinstructions5m / (double)CPUcycles5m) + " last 15 (Higher is better)";
}

string Server::getIPS() {
	return to_string((CPUinstructions1m / 60) / 1000000) + "M/s last 1 minute, " + to_string((CPUinstructions5m / 60) / 1000000) + "M/s last 5, " + to_string((CPUinstructions15m / 60) / 1000000) + "M/s last 15";
}

string Server::getCPS() {
	return to_string((CPUcycles1m / 60) / 1000000) + "M/s last 1 minute, " + to_string((CPUcycles5m / 60) / 1000000) + "M/s last 5, " + to_string((CPUcycles15m / 60) / 1000000) + "M/s last 15";
}

string Server::getContextSwitches() {
	return to_string(contextSwitches1m) + " (" + to_string(100.0 * (double)contextSwitches1m / (double)CPUinstructions1m) + "% of CPU) last 1 minute, " + to_string(contextSwitches5m) + " (" + to_string(100.0 * (double)contextSwitches5m / (double)CPUinstructions5m) + "%) last 5, " + to_string(contextSwitches15m) + " (" + to_string(100.0 * (double)contextSwitches15m / (double)CPUinstructions15m) + "%) last 15 (Lower is better)";
}

string Server::getStalledCyclesFrontend() {
	return to_string(stalledCyclesFrontend1m) + " (" + to_string(100.0 * (double)stalledCyclesFrontend1m / (double)CPUcycles1m) + "% of all cycles) last 1 minute, " + to_string(stalledCyclesFrontend5m) + " (" + to_string(100.0 * (double)stalledCyclesFrontend5m / (double)CPUcycles5m) + "%) last 5, " + to_string(stalledCyclesFrontend15m) + " (" + to_string(100.0 * (double)stalledCyclesFrontend15m / (double)CPUcycles15m) + "%) last 15 (Lower is better)";
}

string Server::getStalledCyclesBackend() {
	return to_string(stalledCyclesBackend1m) + " (" + to_string(100.0 * (double)stalledCyclesBackend1m / (double)CPUcycles1m) + "% of all cycles) last 1 minute, " + to_string(stalledCyclesBackend5m) + " (" + to_string(100.0 * (double)stalledCyclesBackend5m / (double)CPUcycles5m) + "%) last 5, " + to_string(stalledCyclesBackend15m) + " (" + to_string(100.0 * (double)stalledCyclesBackend15m / (double)CPUcycles15m) + "%) last 15 (Lower is better)";
}

string Server::getBranchMisses() {
	return to_string(branchMisses1m) + " (" + to_string(100.0 * (double)branchMisses1m / (double)branchInstructions1m) + "% of all branch instructions) last 1 minute, " + to_string(branchMisses5m) + " (" + to_string(100.0 * (double)branchMisses5m / (double)branchInstructions5m) + "%) last 5, " + to_string(branchMisses15m) + " (" + to_string(100.0 * (double)branchMisses15m / (double)branchInstructions15m) + "%) last 15 (Lower is better)";
}

string Server::getCacheMisses() {
	return to_string(cacheMisses1m) + " (" + to_string(100.0 * (double)cacheMisses1m / (double)cacheReferences1m) + "% of total) last 1 minute, " + to_string(cacheMisses5m) + " (" + to_string(100.0 * (double)cacheMisses5m / (double)cacheReferences5m) + "%) last 5, " + to_string(cacheMisses15m) + " (" + to_string(100.0 * (double)cacheMisses15m / (double)cacheReferences15m) + "%) last 15 (Lower is better)";
}

string Server::getAlignmentFaults() {
	return to_string(alignmentFaults1m) + " last 1 minute, " + to_string(alignmentFaults5m) + " last 5, " + to_string(alignmentFaults15m) + " last 15 (Lower is better)";
}

string Server::getEmulationFaults() {
	return to_string(emulationFaults1m) + " last 1 minute, " + to_string(emulationFaults5m) + " last 5, " + to_string(emulationFaults15m) + " last 15 (Lower is better)";
}

string Server::getMinorPagefaults() {
	return to_string(minorPagefaults1m) + " (" + to_string(100.0 * (double)minorPagefaults1m / (double)pageFaults1m) + "% of total) last 1 minute, " + to_string(minorPagefaults5m) + " (" + to_string(100.0 * (double)minorPagefaults5m / (double)pageFaults5m) + "%) last 5, " + to_string(minorPagefaults15m) + " (" + to_string(100.0 * (double)minorPagefaults15m / (double)pageFaults15m) + "%) last 15";
}

string Server::getMajorPagefaults() {
	return to_string(majorPagefaults1m) + " (" + to_string(100.0 * (double)majorPagefaults1m / (double)pageFaults1m) + "% of total) last 1 minute, " + to_string(majorPagefaults5m) + " (" + to_string(100.0 * (double)majorPagefaults5m / (double)pageFaults5m) + "%) last 5, " + to_string(majorPagefaults15m) + " (" + to_string(100.0 * (double)majorPagefaults15m / (double)pageFaults15m) + "%) last 15";
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
		std::cout << "Unable to write to pipe" << std::endl;
	} else if (byteswritten != input.size()) {
		std::cout << "Wrote " + std::to_string(byteswritten) + "bytes, expected " + std::to_string(input.size()) << std::endl;
	}
	#else
	write(fd, input.c_str(), input.length());
	#endif
}

void Server::processServerTerminal() {
	while (true) {
		string terminalOutput = readFromServer();
		if (doCommands) {
			processServerCommand(terminalOutput);
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
		std::cout << "ReadFile failed (unable to read from pipe)" << std::endl;
		return std::string();
	}
	#else
	errno = 0;
	ssize_t length = read(fd, input, sizeof(input));
	if (length == -1 || errno == EAGAIN || errno == EINTR) {
		std::cout << "Error reading file descriptor (errno = " + to_string(errno) + ")" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
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
	//std::cout << "uptime = " + to_string(uptime) << std::endl;
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
	hjlog.normalDisabled = true;
	std::cout << "----->" << name << std::endl;
	wantsLiveOutput = true;
	for (const auto& it : lines) {
		std::cout << it << std::flush;
	}
	while (true) {
		std::string user_input = "";
		std::getline(std::cin, user_input); //getline allows for spaces
		if (user_input == ".d") {
			wantsLiveOutput = false;
			break;
		} else if (user_input[0] == '.') {
			std::cout << text.error.InvalidCommand << std::endl;
			std::cout << text.error.InvalidServerCommand1 << std::endl;
		} else {
			writeToServerTerminal(user_input); //write to the master side of the pterminal with user_input converted into a c-style string
		}
	}
	std::cout << "Hajime<-----" << std::endl;
	hjlog.normalDisabled = false;
}
