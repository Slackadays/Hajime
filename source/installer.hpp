#include <fstream>
#include <filesystem>
#include <cstring>
#include <string>

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
        void installNewServerConfigFile(string fileLocation, string flags);
        public:
                void installStartupService(string sysService);
                void installDefaultHajConfFile(string fileLocation, bool skipFileCheck = false);
                void installDefaultServerConfFile(string conf, bool skipFileCheck = false, string flags = "");
                void installDefaultServersFile(string serversFile, bool skipFileCheck = false);
                Installer(std::shared_ptr<Output> log);
};

extern Installer installer;
