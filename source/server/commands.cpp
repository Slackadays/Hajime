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
	
#include "../httplib/httplib.h"

#if defined(_WIN64) || defined(_WIN32)
#include <Windows.h>
#include <shellapi.h>
#include <VersionHelpers.h>
#include <intrin.h>
#include <powerbase.h>
#pragma comment (lib, "Shell32")
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <signal.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <termios.h>
#else
#include <unistd.h>
#include <termios.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <fmt/format.h>

#include <random>
#include <memory>
#include <iterator>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <fcntl.h>
#include <fstream>
#include <thread>
#include <list>
#include <deque>
#include <atomic>
#include <cstring>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>
#include <errno.h>
#include <regex>
#include <ctime>
//#include <format>
#include <array>

#include "server.hpp"

namespace fs = std::filesystem;


void Server::commandHajime() {
	writeToServerTerminal(formatWrapper(text.server.command.hajime.output));
}

void Server::commandTime() {
	std::time_t timeNow = std::time(nullptr);
	string stringTimeNow = std::asctime(std::localtime(&timeNow));
	stringTimeNow.erase(std::remove(stringTimeNow.begin(), stringTimeNow.end(), '\n'), stringTimeNow.end());
	string hajInfo = text.server.command.time.output + stringTimeNow + '\"';
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandHelp() {
	writeToServerTerminal(formatWrapper(text.server.command.help.output));
	writeToServerTerminal(formatWrapper("[{\"text\":\"" + text.server.command.coinflip.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.coinflip + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.coinflip.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.die.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.die + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.die.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.d20.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.d20 + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.d20.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.discord.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.discord + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.discord.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.hajime.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.hajime + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.hajime.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.help.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.help + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.help.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.name.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.name + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.name.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.info.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.info + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.info.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.time.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.time + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.time.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.uptime.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.uptime + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.uptime.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.system.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.system + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.system.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.perf.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.perf + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.perf.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.hwperf.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.hwperf + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.hwperf.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.swperf.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.swperf + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.swperf.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.caperf.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.caperf + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.caperf.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.restart.regex + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.restart + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.restart.regex, std::regex("(\\(|\\))"), "") + "\"}}]"));
}

void Server::commandDie() {
	std::random_device rand;
	std::mt19937 mt(rand());
	std::uniform_int_distribution<int> die(1, 6);
	string hajInfo = text.server.command.die.output + std::to_string(die(mt));
	writeToServerTerminal(formatWrapper(addNumberColors(hajInfo)));
	//switch this to C++20 format when it becomes supported
}

void Server::commandD20() {
	std::random_device rand;
	std::mt19937 mt(rand());
	std::uniform_int_distribution<int> die(1, 20);
	string hajInfo = text.server.command.d20.output + std::to_string(die(mt));
	writeToServerTerminal(formatWrapper(addNumberColors(hajInfo)));
	//switch this to C++20 format when it becomes supported
}

void Server::commandCoinflip() {
	std::random_device rand;
	std::mt19937 mt(rand());
	std::uniform_int_distribution<int> flip(1, 2);
	if (flip(mt) == 1) {
		writeToServerTerminal(formatWrapper(text.server.command.coinflip.output.heads));
	} else {
		writeToServerTerminal(formatWrapper(text.server.command.coinflip.output.tails));
	}
}

void Server::commandDiscord() {
	writeToServerTerminal(formatWrapper(text.server.command.discord.output));
}

void Server::commandName() {
	string hajInfo = text.server.command.name.output + serverSettings.name;
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandInfo() {
	if (serverSettings.customMessage != "") {
		string hajInfo = "[Hajime] " + serverSettings.customMessage;
		writeToServerTerminal(formatWrapper(hajInfo));
	} else {
		writeToServerTerminal(formatWrapper("[Hajime] This server does not have a custom message set."));
	}
}

void Server::commandUptime() {
	string hajInfo = fmt::vformat(fmt::to_string_view(text.server.command.uptime.output), fmt::make_format_args(std::to_string(serverAttributes.uptime), std::to_string(serverAttributes.uptime / 60.0)));
	writeToServerTerminal(formatWrapper(addNumberColors(hajInfo)));
}

void Server::commandRestart() {
	string hajInfo;
	if (serverSettings.restartMins > 0) {
		hajInfo = fmt::vformat(text.server.command.restart.output, fmt::make_format_args(std::to_string(serverSettings.restartMins - serverAttributes.uptime), std::to_string((serverSettings.restartMins - serverAttributes.uptime) / 60.0)));
	} else {
		hajInfo = text.server.command.restart.outputDisabled;
	}
	writeToServerTerminal(formatWrapper(addNumberColors(hajInfo)));
}

void Server::commandSystem() {
	string hajInfo;
	hajInfo = "[{\"text\":\"[Hajime] \"},{\"text\":\"" + text.server.command.system.key.os + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getOS() + "\"}},"
	"{\"text\":\"" + text.server.command.system.key.cpu + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCPU() + "\"}},"
	"{\"text\":\"" + text.server.command.system.key.ram + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getRAM() + "\"}},"
	"{\"text\":\"" + text.server.command.system.key.swap + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getSwap() + "\"}},"
	"{\"text\":\"" + text.server.command.system.key.uptime + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getUptime() + "\"}},"
	"{\"text\":\"" + text.server.command.system.key.processes + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getProcesses() + "\"}},"
	"{\"text\":\"" + text.server.command.system.key.loadavg + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getLoadavg() + "\"}},"
	"{\"text\":\"" + text.server.command.system.key.temps + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getTemps() + "\"}},"
	"{\"text\":\"" + text.server.command.system.key.storage + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getStorage() + "\"}}]";
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandPerf() {
	writeToServerTerminal(formatWrapper("[Hajime] Roll over an item to show its explanation."));
	string hajInfo;
	hajInfo = "[{\"text\":\"" + text.server.command.perf.key.cpuusage + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.cpuusage + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.ramusage + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.ramusage + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.cpumigrations + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.cpumigrations + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.ipc + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.ipc + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.cps + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.cps + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.ips + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.ips + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.minorpagefaults + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.minorpagefaults + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.majorpagefaults + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.majorpagefaults + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.contextswitches + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.contextswitches + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.stalledfrontend + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.stalledfrontend + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.stalledbackend + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.stalledbackend + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.buscycles + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.buscycles + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.branchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.branchmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.cachemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.cachemisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.emufaults + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.emufaults + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.alignfaults + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.alignfaults + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1dreadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.l1dreadmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1dwritemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.l1dwritemisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1dprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.l1dprefetchmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1ireadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.l1ireadmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1iprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.l1iprefetchmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.llcreadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.llcreadmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.llcwritemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.llcwritemisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.llcprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.llcprefetchmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.dtlbreadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.dtlbreadmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.dtlbwritemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.dtlbwritemisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.dtlbprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.dtlbprefetchmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.itlbreadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.itlbreadmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.bpureadmisses + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.bpureadmisses + "\"}}]";
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandHWPerf() {
	string hajInfo;
	hajInfo = "[{\"text\":\"[Hajime] \"},{\"text\":\"" + text.server.command.perf.key.cpuusage + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCPUusage() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.ramusage + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getRAMusage() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.ipc + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getIPC() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.cps + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCPS() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.ips + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getIPS() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.branchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getBranchMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.cachemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCacheMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.stalledfrontend + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getStalledCyclesFrontend() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.stalledbackend + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getStalledCyclesBackend() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.buscycles + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getBusCycles() + "\"}}]";
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandSWPerf() {
	string hajInfo;
	hajInfo = "[{\"text\":\"[Hajime] \"},{\"text\":\"" + text.server.command.perf.key.contextswitches + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getContextSwitches() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.cpumigrations + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCPUmigs() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.emufaults + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getEmulationFaults() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.alignfaults + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getAlignmentFaults() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.minorpagefaults + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getMinorPagefaults() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.majorpagefaults + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getMajorPagefaults() + "\"}}]";
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandCAPerf() {
	string hajInfo;
	hajInfo = "[{\"text\":\"[Hajime] \"},{\"text\":\"" + text.server.command.perf.key.l1dreadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getL1dReadMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1dprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getL1dPrefetchMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1dwritemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getL1dWriteMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1ireadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getL1iReadMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1iprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getL1iPrefetchMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.llcwritemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getLLWriteMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.llcprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getLLPrefetchMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.dtlbreadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getdTLBReadMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.dtlbwritemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getdTLBWriteMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.dtlbprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getdTLBPrefetchMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.itlbreadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getiTLBReadMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.bpureadmisses + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getBPUReadMisses() + "\"}}]";
	writeToServerTerminal(formatWrapper(hajInfo));
}

string Server::getCPUusage() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return std::to_string(averageVal(counterData.cpuusagereadings, 1)) + "% last 1 minute, " + std::to_string(averageVal(counterData.cpuusagereadings, 5)) + "% last 5, " + std::to_string(averageVal(counterData.cpuusagereadings, 15)) + "% last 15 (Lower is better)";
}

string Server::getCPUmigs() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return std::to_string(averageVal(counterData.cpumigrationreadings, 1)) + " last 1 minute, " + std::to_string(averageVal(counterData.cpumigrationreadings, 5)) + " last 5, " + std::to_string(averageVal(counterData.cpumigrationreadings, 15)) + " last 15";
}

string Server::getRAMusage() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return std::to_string(averageVal(counterData.rampercentreadings, 1)) + "% last 1 minute, " + std::to_string(averageVal(counterData.rampercentreadings, 5)) + "% last 5, " + std::to_string(averageVal(counterData.rampercentreadings, 15)) + "% last 15 (" + std::to_string((averageVal(counterData.rambytereadings, 1) / 1024) / 1024) + "MB/" + std::to_string((averageVal(counterData.rambytereadings, 5) / 1024) / 1024) + "MB/" + std::to_string((averageVal(counterData.rambytereadings, 15) / 1024) / 1024) + "MB) (Lower is better)";
}

string Server::getIPC() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return std::to_string((double)averageVal(counterData.cpuinstructionreadings, 1) / (double)averageVal(counterData.cpucyclereadings, 1)) + " last 1 minute, " + std::to_string((double)averageVal(counterData.cpuinstructionreadings, 5) / (double)averageVal(counterData.cpucyclereadings, 5)) + " last 5, " + std::to_string((double)averageVal(counterData.cpuinstructionreadings, 15) / (double)averageVal(counterData.cpucyclereadings, 15)) + " last 15 (Higher is better)";
}

string Server::getIPS() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return std::to_string((averageVal(counterData.cpuinstructionreadings, 1) / 60) / 1000000) + "M/s last 1 minute, " + std::to_string((averageVal(counterData.cpuinstructionreadings, 5) / 60) / 1000000) + "M/s last 5, " + std::to_string((averageVal(counterData.cpuinstructionreadings, 15) / 60) / 1000000) + "M/s last 15";
}

string Server::getCPS() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return std::to_string((averageVal(counterData.cpucyclereadings, 1) / 60) / 1000000) + "M/s last 1 minute, " + std::to_string((averageVal(counterData.cpucyclereadings, 5) / 60) / 1000000) + "M/s last 5, " + std::to_string((averageVal(counterData.cpucyclereadings, 15) / 60) / 1000000) + "M/s last 15";
}

string Server::getContextSwitches() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.contextswitchreadings, counterData.cpuinstructionreadings);
}

string Server::getStalledCyclesFrontend() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.stalledcyclesfrontendreadings);
}

string Server::getStalledCyclesBackend() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.stalledcyclesbackendreadings);
}

string Server::getBusCycles() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsHIB(counterData.buscyclereadings);
}

string Server::getBranchMisses() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.branchmissreadings, counterData.branchinstructionreadings);
}

string Server::getCacheMisses() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.cachemissreadings, counterData.cachereferencereadings);
}

string Server::getAlignmentFaults() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.alignmentfaultreadings);
}

string Server::getEmulationFaults() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.emulationfaultreadings);
}

string Server::getMinorPagefaults() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.minorpagefaultreadings, counterData.pagefaultreadings);
}

string Server::getMajorPagefaults() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.majorpagefaultreadings, counterData.pagefaultreadings);
}

string Server::getL1dReadMisses() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.l1dreadmissreadings, counterData.l1dreadaccessreadings);
}

string Server::getL1dPrefetchMisses() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.l1dprefetchmissreadings, counterData.l1dprefetchaccessreadings);
}

string Server::getL1dWriteMisses() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.l1dwritemissreadings, counterData.l1dwriteaccessreadings);
}

string Server::getL1iReadMisses() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.l1ireadmissreadings, counterData.l1ireadaccessreadings);
}

string Server::getL1iPrefetchMisses() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.l1iprefetchmissreadings, counterData.l1iprefetchaccessreadings);
}

string Server::getLLReadMisses() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.llreadmissreadings, counterData.llreadaccessreadings);
}

string Server::getLLWriteMisses() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.llwritemissreadings, counterData.llwriteaccessreadings);
}

string Server::getLLPrefetchMisses() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.llprefetchmissreadings);
}

string Server::getdTLBReadMisses() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.dtlbreadmissreadings, counterData.dtlbreadaccessreadings);
}

string Server::getdTLBWriteMisses() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.dtlbwritemissreadings, counterData.dtlbwriteaccessreadings);
}

string Server::getdTLBPrefetchMisses() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.dtlbprefetchmissreadings, counterData.dtlbprefetchaccessreadings);
}

string Server::getiTLBReadMisses() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.itlbreadmissreadings, counterData.itlbreadaccessreadings);
}

string Server::getBPUReadMisses() {
	std::lock_guard<std::mutex> lock(counterData.mutex);
	return formatReadingsLIB(counterData.bpureadmissreadings, counterData.bpureadaccessreadings);
}

string Server::getOS() {
	#if defined(__linux__) && !defined(__FreeBSD__)
	std::fstream proc;
	proc.open("/proc/version", std::fstream::in);
	std::ostringstream temp;
	temp << proc.rdbuf();
	string out = temp.str();
	out.erase(std::remove(out.begin(), out.end(), '\n'), out.end());
	return out;
	#elif defined(_WIN32) || defined (_WIN64)
	std::string name;
	if (IsWindows10OrGreater()) {
		name = "Windows 10+";
	}
	else if (IsWindows8Point1OrGreater()) {
		name = "Windows 8.1+";
	}
	else if (IsWindows8OrGreater()) {
		name = "Windows 8+";
	}
	else if (IsWindows7OrGreater()) {
		name = "Windows 7+";
	}
	else if (IsWindowsVistaOrGreater()) {
		name = "Windows Vista+";
	}
	else if (IsWindowsXPOrGreater()) {
		name = "Windows XP+";
	} else {
		name = "Unknown Windows Version";
	}
	if (IsWindowsServer()) {
		name += " (Server)";
	}
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	switch (sys_info.wProcessorArchitecture) {
	case PROCESSOR_ARCHITECTURE_INTEL:
		name += " x86";
		break;
	case PROCESSOR_ARCHITECTURE_AMD64:
		name += " x64";
		break;
	case PROCESSOR_ARCHITECTURE_ARM:
		name += " arm";
		break;
	case PROCESSOR_ARCHITECTURE_ARM64:
		name += " arm64";
		break;
	}
	return name;
	#elif defined(__APPLE__)
	size_t len;
	sysctlbyname("kern.version", NULL, &len, NULL, 0);
	string result(len, '\0');
	sysctlbyname("kern.version", result.data(), &len, NULL, 0);
	sysctlbyname("kern.osproductversion", NULL, &len, NULL, 0);
       	string macosresult(len, '\0');
        sysctlbyname("kern.osproductversion", macosresult.data(), &len, NULL, 0);
	return result + " (macOS " + macosresult + ")";
	#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
	return "Blah";
	#endif
	return "Not available on your platform";
}

string Server::getCPU() {
	#if defined(__linux__) && !defined(__FreeBSD__)
	std::smatch m;
	std::fstream proc;
	proc.open("/proc/cpuinfo", std::fstream::in);
	std::ostringstream temp;
	temp << proc.rdbuf();
	string temp2 = temp.str();
	std::regex_search(temp2, m, std::regex("(?:model name\\s*:\\s*)(.*)", std::regex_constants::optimize));
	return m[1];
	#elif defined (_WIN32)
	std::string brandname;
	#if defined(_M_IX86) || defined(_M_X64)
	std::array<int, 4> cpui = {};
	std::array<char, 16> cpui_c = {};
	std::vector<char> brand;
	__cpuid(cpui.data(), 0x80000000);
	unsigned int maxid = cpui[0];
	if (maxid < 0x80000004U) {
		brandname = "Unknown CPU";
	}	else {
		for (unsigned int i = 0x80000002U; i <= 0x80000004U; i++) {
			// here it actually gives chars, but cpuidex only accepts int*
			__cpuidex(reinterpret_cast<int*>(cpui_c.data()), i, 0);
			brand.insert(brand.end(), cpui_c.begin(), cpui_c.end());
		}
		brandname = std::string(brand.begin(), brand.end() - 1);
		// erase trailing whitespaces
		while (brandname.back() == ' ') {
			brandname.pop_back();
		}
	}
	#else
	brandname = "Unknown or ARM CPU";
	#endif
	// number of threads
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	DWORD number_of_processors = sys_info.dwNumberOfProcessors;
	brandname.insert(0, std::to_string(number_of_processors) + "x ");
	return brandname;
	#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
	return "Blah";
	#elif defined(__APPLE__)
	size_t len;
  sysctlbyname("machdep.cpu.brand_string", NULL, &len, NULL, 0);
  string cpuname(len, '\0');
  sysctlbyname("machdep.cpu.brand_string", cpuname.data(), &len, NULL, 0);
	sysctlbyname("hw.ncpu", NULL, &len, NULL, 0);
  int cpucount;
  sysctlbyname("hw.ncpu", &cpucount, &len, NULL, 0);
  return std::to_string(cpucount) + "x " + cpuname;
	#endif
	return "Only available on Linux or Windows";
}

string Server::getRAM() {
	#if defined(__linux__)
	std::fstream proc;
	proc.open("/proc/meminfo", std::fstream::in);
	std::ostringstream temp;
	temp << proc.rdbuf();
	string temp2 = temp.str();
	std::regex re("\\d+", std::regex_constants::optimize);
	std::vector<std::string> meminfo;
	for (auto it = std::sregex_iterator(temp2.begin(), temp2.end(), re); it != std::sregex_iterator(); ++it) {
		std::smatch m = *it;
		meminfo.push_back(m.str());
	}
	if (meminfo.size() < 3) {
		return string("Could not get memory info");
	}
	string result = meminfo[0] + "kB total, " + meminfo[1] + "kB free, " + meminfo[2] + "kB available";
	return result;
	#elif defined(_WIN32)
	MEMORYSTATUSEX mem;
	mem.dwLength = sizeof(mem);
	GlobalMemoryStatusEx(&mem);
	constexpr double div = 1024 * 1024 * 1024;
	const auto roundto2 = [](double d) -> std::string {
		std::stringstream ss;
		ss << std::fixed << std::setprecision(2) << d;
		return ss.str();
	};
	DWORDLONG total_totalmem = mem.ullTotalPhys + mem.ullTotalPageFile;
	DWORDLONG total_availmem = mem.ullAvailPhys + mem.ullAvailPageFile;
	DWORDLONG total_usedmem = total_totalmem - total_availmem;
	std::string result = roundto2(total_usedmem / div) + '/' + roundto2(total_totalmem / div) + " GB Total, " + std::to_string(std::lround((total_usedmem * 100.0) / total_totalmem)) + "% Used (" +
		roundto2((mem.ullTotalPhys - mem.ullAvailPhys) / div) + '/' + roundto2(mem.ullTotalPhys / div) + " GB Physical, " + std::to_string(mem.dwMemoryLoad) + "% Used)";
	return result;
	#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
	return "Blah";
	#elif defined(__APPLE__)
	size_t len;
  sysctlbyname("hw.memsize", NULL, &len, NULL, 0);
  long int memtotal;
  sysctlbyname("hw.memsize", &memtotal, &len, NULL, 0);
  return std::to_string(memtotal) + "B total";
	#else
	return "Not available yet";
	#endif
}

string Server::getUptime() {
	#if defined(__linux__)
	std::smatch m;
	std::fstream proc;
	proc.open("/proc/uptime", std::fstream::in);
	std::ostringstream temp;
	temp << proc.rdbuf();
	string temp2 = temp.str();
	std::regex_search(temp2, m, std::regex("[0-9]+(\\.[0-9]+)?", std::regex_constants::optimize));
	try {
		return string(m[0]) + " seconds (" + std::to_string(stoi(m[0]) / 60) + string(" minutes, ") + std::to_string(stoi(m[0]) / 3600) + " hours)";
	} catch (...) {
		return "Error parsing memory";
	}
	#else
	return string("Not available yet");
	#endif
}

string Server::getLoadavg() {
	#if defined(__linux__)
	std::fstream proc;
	proc.open("/proc/loadavg", std::fstream::in);
	std::ostringstream temp;
	temp << proc.rdbuf();
	string temp2 = temp.str();
	std::regex re("[0-9.]+", std::regex_constants::optimize);
	std::vector<std::string> loadinfo;
	for (auto it = std::sregex_iterator(temp2.begin(), temp2.end(), re); it != std::sregex_iterator(); ++it) {
		std::smatch m = *it;
		loadinfo.push_back(m.str());
	}
	if (loadinfo.size() < 3) {
		return string("Could not get load average info");
	}
	string result = "last 1 minute: " + loadinfo[0] + ", last 5 minutes: " + loadinfo[1] + ", last 10 minutes: " + loadinfo[2];
	return result;
	#else
	return "Not available yet";
	#endif
}

string Server::getStorage() {
	fs::space_info si = fs::space(fs::current_path());
	string result = std::to_string(si.capacity / 1024 / 1024) + "MB total, " + std::to_string(si.free / 1024 / 1024) + "MB free, " + std::to_string(si.available / 1024 / 1024) + "MB available";
	return result;
}

string Server::getSwap() {
	#if defined(__linux__)
	struct sysinfo info;
	sysinfo(&info);
	string result = std::to_string(info.totalswap / 1024) + "kB total, " + std::to_string(info.freeswap / 1024) + "kB available";
	return result;
	#else
	return "Not available yet";
	#endif
}

string Server::getProcesses() {
	#if defined(__linux__)
	struct sysinfo info;
	sysinfo(&info);
	string result = std::to_string(info.procs) + " total";
	return result;
	#else
	return "Not available yet";
	#endif
}

string Server::getTemps() {
	string result = "Currently not available";
	return result;
}

bool Server::areCountersAvailable() {
	#if defined(_WIN32) || defined(_WIN64)
	return false;
	#elif defined(__linux__)
	if (performanceCounterCompat == -1) {
		return false;
	} else {
		return true;
	}
	#else
	return false;
	#endif
}

string Server::formatReadingsLIB(const std::deque<long long>& little, const std::deque<long long>& big) {
	if (areCountersAvailable()) {
		return std::to_string(little.back()) + " (" + std::to_string(100.0 * (double)little.back() / (double)big.back()) + "% of total) last 5 seconds, " + std::to_string(averageVal(little, 1)) + " (" + std::to_string(100.0 * (double)averageVal(little, 1) / (double)averageVal(big, 1)) + "%) last 1 minute, " + std::to_string(averageVal(little, 5)) + " (" + std::to_string(100.0 * (double)averageVal(little, 5) / (double)averageVal(big, 5)) + "%) last 5, " + std::to_string(averageVal(little, 15)) + " (" + std::to_string(100.0 * (double)averageVal(little, 15) / (double)averageVal(big, 15)) + "%) last 15, " + std::to_string(averageVal(little, 60)) + " (" + std::to_string(100.0 * (double)averageVal(little, 60) / (double)averageVal(big, 60)) + "%) last 1 hour (Lower is better)";
	} else {
		return "Currently not available on this server";
	}
}

string Server::formatReadingsLIB(const std::deque<long long>& readings) {
	if (areCountersAvailable()) {
		return std::to_string(readings.back()) + " last 5 seconds, " + std::to_string(averageVal(readings, 1)) + " last 1 minute, " + std::to_string(averageVal(readings, 5)) + " last 5, " + std::to_string(averageVal(readings, 15)) + " last 15, " + std::to_string(averageVal(readings, 60)) + " last 1 hour (Lower is better)";
	} else {
		return "Currently not available on this server";
	}
}

string Server::formatReadingsHIB(const std::deque<long long>& readings) {
	if (areCountersAvailable()) {
		return std::to_string(readings.back()) + " last 5 seconds, " + std::to_string(averageVal(readings, 1)) + " last 1 minute, " + std::to_string(averageVal(readings, 5)) + " last 5, " + std::to_string(averageVal(readings, 15)) + " last 15, " + std::to_string(averageVal(readings, 60)) + " last 1 hour (Higher is better)";
	} else {
		return "Currently not available on this server";
	}
}