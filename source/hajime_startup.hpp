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

#pragma once
#include "server/server.hpp"

#if defined(_WIN64) || defined (_WIN32)
extern void setupTerminal();
#else
extern void setupRLimits();
#endif

#if defined(__linux__)
extern void setupSensors();
#endif

extern void shutdownServers();
extern void setupSignals();
extern void setupHajimeDirectory();
extern bool readSettings();
extern void hajimeUserExit(int sig);
extern std::vector<std::string> getServerFiles();
extern std::vector<std::string> splitToVec(std::string input);
extern void setupServers();
extern void setupFirstTime();
extern void processHajimeCommand(std::vector<std::string> input);
extern bool isUserPrivileged();
extern void doHajimeTerminal();

extern std::vector<std::shared_ptr<Server>> serverVec; //create an array of individual server objects
extern std::vector<std::thread> threadVec; //create an array of thread objects
extern std::string hajConfFile;
extern std::string version;
extern int stopOnExit;