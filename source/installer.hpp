/*  Hajime, the ultimate startup script.
    Copyright (C) 2022 Slackadays and other contributors to Hajime on GitHub.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.*/

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
	void installWindows();
	void installSysVInit();
	void installLaunchd();
	void installSystemd(const string& location);
	public:
		void installStartupService(const string& sysService);
		static void installNewServerConfigFile(const string& fileLocation, const bool& skipFileCheck, const string& flags, const string& serverFile);
		static void installDefaultHajConfFile(string fileLocation, bool skipFileCheck = false, const string& lang = "en");
		//explicit Installer(std::shared_ptr<Output> log);
};

extern Installer installer;
