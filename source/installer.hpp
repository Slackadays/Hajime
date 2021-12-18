#include <fstream>
#include <filesystem>
#include <cstring>
#include <string>
#include <vector>

#if defined(_win64) || defined (_WIN32)
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
        static void installNewServerConfigFile(string fileLocation, string flags);
        public:
                void installStartupService(string sysService);
                static void installDefaultHajConfFile(string fileLocation, bool skipFileCheck = false);
                static void installDefaultServerConfFile(string conf, bool skipFileCheck = false);
                static void installDefaultServersFile(string serversFile, bool skipFileCheck = false, std::vector<string> servers = {"MyServer"});
                Installer(std::shared_ptr<Output> log);
};

extern Installer installer;
