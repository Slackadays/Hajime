#if defined(_win64) || defined (_WIN32)
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
	bool hasOutput, hasOutputUSB, hasMounted = false;

	int systemi = 0;

	std::error_code ec;

	shared_ptr<Output> logObj;

	const string systems[8] = {"ext2", "ext3", "ext4", "vfat", "msdos", "f2fs", "ntfs", "fuseblk"};

	void mountDrive();
	void makeDir();
	void startProgram(string method);
	void readSettings(string confFile);
	void removeSlashesFromEnd(string& var);
	void readFd();
	int getPID(int pid = 0, string method = "new");
	vector<string> toArray(string input);
	auto toPointerArray(vector<string> &strings);

	string name, file, path, command, flags, confFile, device = "";
	string method;

	inline static int slave_fd, fd, pid;

	bool startedRfdThread = false;

	inline static bool wantsLiveOutput = false;

	inline static std::list<string> lines; //make this inline static so the program only has one copy of lines available
	//super duper important!!

	public:
		Server(shared_ptr<Output> tempObj);
		bool isRunning = false;
		void startServer(string confFile);
		#if !defined(_WIN64) && !defined (_WIN32)
		void terminalAccessWrapper();
		#endif
};
