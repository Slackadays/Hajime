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

#include <filesystem>
#if defined(_WIN64) || defined(_WIN32) //Windows compatibility
#include <Windows.h>
#include <shlobj.h>
#else
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <signal.h>
#include <fstream>
#include <memory>
#include <thread>
#include <iostream>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <regex>
#include <chrono>
#include <sstream>

#ifdef _MSC_VER
#if (_MSC_VER < 1928 || _MSVC_LANG <= 201703L) // msvc usually doesn't define __cplusplus to the correct value
#define jthread thread
#endif
#elif (__cplusplus <= 201703L || defined(__APPLE__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__clang__)) //jthreads are only in C++20 and up and not supported by Apple Clang yet
#define jthread thread
#endif

#include "hajime_startup.hpp"
#include "constants.hpp"
#include "output.hpp"
#include "languages.hpp"
#include "installer.hpp"
#include "getvarsfromfile.hpp"
#include "wizard.hpp"
#include "deduce.hpp"
#include "flags.hpp"

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
	//auto then = std::chrono::high_resolution_clock::now();
	setupSignals();
	#if defined(_WIN64) || defined (_WIN32)
	setupTerminal();
	#endif
	term.dividerLine();
	std::vector<std::string> flags;
	for (int i = 0; i < argc; i++) {
		flags.push_back(argv[i]);
	}
	doPreemptiveFlags(flags);
	doRegularFlags(flags);
	term.out<Info>("Starting Hajime");
	// check for a subdirectory called "Hajime" and if it does not exist, create it
	if (!fs::is_directory(hajimePath)) {
		term.out<Info>("Creating Hajime directory");
		try {
			fs::create_directory(hajimePath);
		} catch (fs::filesystem_error& e) {
			term.out<Error>("Could not create Hajime directory");
			term.out<Error>(e.what());
			exit(1);
		}
	}
	if (fs::is_regular_file(hajDefaultConfFile)) {
		readSettings();
		empty(hajimePath + logFile) ? term.out<Info>(text.info.NoLogFile) : term.init(hajimePath + logFile);
	} else {
		setupFirstTime();
	}
	if (!bypassPriviligeCheck && isUserPrivileged()) {
		term.out<Error>("You cannot run Hajime as a priviliged user");
		return 1;
	}
	#if !defined(_WIN64) && !defined(_WIN32)
	setupRLimits();
	#endif
	//std::cout << "This took " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - then).count() << " microseconds" << std::endl;
	//exit(0);
	setupServers();
	doHajimeTerminal();
	return 0;
}

