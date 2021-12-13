module;

#include <iostream>
#include <filesystem>
#include <cstring>
#include <string>
#include <memory>

#if defined(_win64) || defined (_WIN32)
#include <Windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#endif

export module Hajime:Installer;

import :Output;

using std::cout;
using std::endl;
using std::string;

//#include "output.hpp"
//#include "languages.hpp"

export class Installer {
        static void installNewServerConfigFile(string fileLocation, string flags);
        public:
                void installStartupService(string sysService);
                static void installDefaultHajConfFile(string fileLocation, bool skipFileCheck = false);
                static void installDefaultServerConfFile(string conf, bool skipFileCheck = false);
                static void installDefaultServersFile(string serversFile, bool skipFileCheck = false);
                Installer(std::shared_ptr<Output> log);
};

export extern Installer installer;
