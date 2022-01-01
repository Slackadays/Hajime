#include <fstream>
#include <filesystem>
#include <cstring>
#include <string>
#include <vector>

#if defined(_WIN64) || defined (_WIN32)
#include <Windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#endif

using std::cout;
using std::endl;

#include "output.hpp"
#include "languages.hpp"

class Installer {
	public:
		void installStartupService(string sysService);
		static void installNewServerConfigFile(string fileLocation, bool skipFileCheck, string flags, string serverFile);
		static void installDefaultHajConfFile(string fileLocation, bool skipFileCheck = false);
		static void installDefaultServersFile(string serversFile, bool skipFileCheck = false, std::vector<string> servers = {"MyServer"});
		Installer(std::shared_ptr<Output> log);
};

extern Installer installer;
