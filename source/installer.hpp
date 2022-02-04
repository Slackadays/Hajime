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
		void installStartupService(const string& sysService);
		static void installNewServerConfigFile(const string& fileLocation, const bool& skipFileCheck, const string& flags, const string& serverFile);
		static void installDefaultHajConfFile(string fileLocation, bool skipFileCheck = false, const string& lang = "en");
		static void installDefaultServersFile(string serversFile, bool skipFileCheck = false, std::vector<string> servers = {"MyServer"});
		Installer(std::shared_ptr<Output> log);
};

extern Installer installer;
