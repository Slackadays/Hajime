#include <fstream>
#include <filesystem>
#include <cstring>
#include <string>

#if defined(_win64) || defined (_WIN32)
#include <Windows.h>
#include <shlobj_core.h>
#else
#include <unistd.h>
#endif

using std::cout;
using std::endl;

#include "output.h"
#include "languages.h"

class Installer {
        void installNewServerConfigFile(string fileLocation);
        public:
                void installStartupService(string sysService);
                void installDefaultHajConfFile(string fileLocation);
                void installDefaultServerConfFile(string conf);
                void installDefaultServersFile(string serversFile);
                Installer(std::shared_ptr<Output> log);
};
