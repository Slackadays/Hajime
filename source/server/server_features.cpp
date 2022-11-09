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
#include <random>
#include <array>

namespace fs = std::filesystem;

#include "../getvarsfromfile.hpp"
#include "server.hpp"
#include "../flexi_format.hpp"

std::string Server::generateSecret() {
	std::string options = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789*_"; //64 options for 6 bits of entropy
	std::random_device rd;
	std::mt19937 mt(rd());
	std::shuffle(options.begin(), options.end(), mt);
	return options.substr(0, 16);
}

void Server::processTerminalBuffer(string input) {
	while (lines.size() >= 100000) {
		//std::cout << "Popping, ws.row = " << w.ws_row << std::endl;
		lines.pop_front();
		//std::cout << "lines size = " << (unsigned short)lines.size() << w.ws_row << std::endl;
	}
	input = std::regex_replace(input, std::regex(">\\.\\.\\.\\.", std::regex_constants::optimize), ">"); //replace ">...." with ">" because this shows up in the temrinal output
	//std::cout << "Pushing back" << std::endl;
	lines.push_back(input);
	if (serverAttributes.wantsLiveOutput) {
		std::cout << input << std::flush;
	}
}

void Server::processChatKicks(string input) {
	try {
		std::regex kickreg("" + serverSettings.chatKickRegex, std::regex_constants::optimize | std::regex_constants::icase);
		if (std::regex_search(input, kickreg)) {
			writeToServerTerminal("kick " + serverAttributes.lastCommandUser + " §4§LForbidden word in chat; please do not say that!");
			writeToServerTerminal(formatWrapper("[Hajime] Kicked " + serverAttributes.lastCommandUser + " for a chat infraction"));
		}
	} catch(...) {
		term.out<Error>("Invalid chat kick regex");
	}
}

void Server::processServerCommand(string input) {
	std::smatch m;
	std::string command;
	if (serverAttributes.usesHajimeHelper) {
		if (std::regex_search(input, m, std::regex("\\[.+\\]:\\s(\\.[\\w\\d]+)\\s([a-zA-Z0-9_\\*]{16}|\\s)\\s([\\w\\d]+)", std::regex_constants::optimize))) {
				serverAttributes.lastCommandUser = m[3];
			if (m[2] == serverAttributes.secret) {
				command = m[1];
			} else {
				writeToServerTerminal(formatWrapper("[Hajime] Invalid secret"));
				return;
			}
		} else {
			return;
		}
	} else {
		if (std::regex_search(input, m, std::regex("\\[.+\\]:\\s<(.+)>\\s(\\.[\\w\\d]+)?(?!\\w)", std::regex_constants::optimize))) {
			serverAttributes.lastCommandUser = m[1];
			command = m[2];
		} else {
			return;
		}
	}
	//std::cout << "command = " << command << std::endl;
	//std::cout << "lastCommandUser = " << serverAttributes.lastCommandUser << std::endl;
	if (std::regex_search(command, std::regex("\\" + text.server.command.hajime.regex, std::regex_constants::optimize))) {
		commandHajime();
	} else if (std::regex_search(command, std::regex("\\" + text.server.command.time.regex, std::regex_constants::optimize))) {
		commandTime();
	} else if (std::regex_search(command, std::regex("\\" + text.server.command.help.regex, std::regex_constants::optimize))) {
		commandHelp();
	} else if (std::regex_search(command, std::regex("\\" + text.server.command.die.regex, std::regex_constants::optimize))) {
		commandDie();
	} else if (std::regex_search(command, std::regex("\\" + text.server.command.d20.regex, std::regex_constants::optimize))) {
		commandD20();
	} else if (std::regex_search(command, std::regex("\\" + text.server.command.coinflip.regex, std::regex_constants::optimize))) {
		commandCoinflip();
	} else if (std::regex_search(command, std::regex("\\" + text.server.command.discord.regex, std::regex_constants::optimize))) {
		commandDiscord();
	} else if (std::regex_search(command, std::regex("\\" + text.server.command.name.regex, std::regex_constants::optimize))) {
		commandName();
	} else if (std::regex_search(command, std::regex("\\" + text.server.command.info.regex, std::regex_constants::optimize))) {
		commandInfo();
	} else if (std::regex_search(command, std::regex("\\" + text.server.command.uptime.regex, std::regex_constants::optimize))) {
		commandUptime();
	} else if (std::regex_search(command, std::regex("\\" + text.server.command.restart.regex, std::regex_constants::optimize))) {
		commandRestart();
	} else if (std::regex_search(command, std::regex("\\" + text.server.command.system.regex, std::regex_constants::optimize))) {
		commandSystem();
	} else if (std::regex_search(command, std::regex("\\" + text.server.command.perf.regex, std::regex_constants::optimize))) {
		commandPerf();
	} else if (std::regex_search(command, std::regex("\\" + text.server.command.hwperf.regex, std::regex_constants::optimize))) {
		commandHWPerf();
	} else if (std::regex_search(command, std::regex("\\" + text.server.command.swperf.regex, std::regex_constants::optimize))) {
		commandSWPerf();
	} else if (std::regex_search(command, std::regex("\\" + text.server.command.caperf.regex, std::regex_constants::optimize))) {
		commandCAPerf();
	} else if (std::regex_search(command, std::regex("\\.ee(?!.\\w)", std::regex_constants::optimize))) {
		writeToServerTerminal(formatWrapper("[Hajime] https://www.youtube.com/watch?v=kjPD_H81hDc"));
	}
}

void Server::processRestartAlert(string input) {
	std::smatch m;
	if (serverSettings.restartMins > 0 && serverAttributes.uptime >= (serverSettings.restartMins - 5) && std::regex_search(input, m, std::regex("\\[.+\\]: ([\\w\\d]+)\\[.+\\] .+", std::regex_constants::optimize))) {
		string hajInfo = "tellraw " + string(m[1]) + flexi_format(text.server.restart.alert, std::to_string(serverSettings.restartMins - serverAttributes.uptime));
		writeToServerTerminal(addNumberColors(hajInfo));
	}
}

string Server::addNumberColors(string input) {
	return std::regex_replace(input, std::regex("(?: |\\()\\d+\\.?\\d*", std::regex_constants::optimize), "§b$&§f");
}

string Server::formatWrapper(string input) {
	string output;
	if (serverAttributes.usesHajimeHelper) {
		if (input.front() == '[' && input.back() == ']') {
			output = "tellraw " + serverAttributes.lastCommandUser + " " + input;
		} else {
			output = "tellraw " + serverAttributes.lastCommandUser + " \"" + input + "\"";
		}
	} else {
		if (input.front() == '[' && input.back() == ']') {
			output = "tellraw @a " + input;
		} else {
			output = "tellraw @a \"" + input + "\"";
		}
	}
	output = std::regex_replace(output, std::regex("\\[Hajime\\]", std::regex_constants::optimize), "§6$&§f");
	return output;
}

void Server::writeToServerTerminal(string input) {
	input += "\n"; //this is the delimiter of the server command
	#if defined(_WIN64) || defined(_WIN32)
	DWORD byteswritten;
	if (!WriteFile(inputwrite, input.c_str(), input.size(), &byteswritten, NULL)) {// write to input pipe
		term.out<Error>("Unable to write to pipe");
	} else if (byteswritten != input.size()) {
		std::cout << "Wrote " + std::to_string(byteswritten) + "bytes, expected " + std::to_string(input.size()) << std::endl;
	}
	#else
	int len = write(fd, input.c_str(), input.length());
	if (len == -1) {
		term.out<Error>("Error writing to server terminal");
	}
	#endif
}

void Server::processServerTerminal() {
	string terminalOutput;
	while (true) {
		terminalOutput = readFromServer();
		if (serverSettings.doCommands) {
			if (!serverAttributes.usesHajimeHelper) {
				checkHajimeHelper(terminalOutput);
			}
			processServerCommand(terminalOutput);
		}
		if (serverSettings.chatKickRegex != "") {
			processChatKicks(terminalOutput);
		}
		processRestartAlert(terminalOutput);
		processTerminalBuffer(terminalOutput);
	}
}

void Server::checkHajimeHelper(std::string input) {
	if (std::regex_search(input, std::regex("\\[.+\\]: \\[HajimeHelper\\].*HajimeHelper", std::regex_constants::optimize))) {
		writeToServerTerminal("setsecret " + serverAttributes.secret);
		serverAttributes.usesHajimeHelper = true;
	}
}

string Server::readFromServer() {
	char input[2500];
	#if defined(_WIN32) || defined (_WIN64)
	DWORD length = 0;
	if (!ReadFile(outputread, input, 1000, &length, NULL)) {
		term.out<Error, Threadless>("ReadFile failed (unable to read from pipe)");
		return std::string();
	}
	#else
	ssize_t length = -1;
	while (length == -1) {
		length = read(fd, input, sizeof(input));
		if (length == -1) {
			switch(errno) {
				case EAGAIN:
					term.out<Error, Threadless>("Error reading file descriptor; not a socket and is nonblocking");
					break;
				case EINTR:
					term.out<Error, Threadless>("Error reading file descriptor; interrupted before read could complete");
					break;
				case EBADF:
					term.out<Error, Threadless>("Error reading file descriptor; not valid or not available for reading");
					break;
				case EIO:
					term.out<Error, Threadless>("Error reading file descriptor; I/O error (the server is likely down)");
					break;
				default:
					term.out<Error, Threadless>("Other error reading file descriptor; errno = " + std::to_string(errno));
					break;
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
	#endif
	std::string output;
	for (int i = 0; i < length; i++) {
		output.push_back(input[i]);
	}
	return output;
}

void Server::updateUptime() {
	serverAttributes.timeCurrent = std::chrono::steady_clock::now();
	auto tempUptime = std::chrono::duration_cast<std::chrono::minutes>(serverAttributes.timeCurrent - serverAttributes.timeStart);
	serverAttributes.uptime = tempUptime.count();
	//std::cout << "uptime = " + std::to_string(serverAttributes.uptime) << std::endl;
}

void Server::processAutoUpdate(bool force) {
	if (serverSettings.autoUpdateName == "") {
		return;
	}
	if ((serverSettings.restartMins > 0 && serverAttributes.uptime >= serverSettings.restartMins) || force) {
		std::string content;
		auto makeFile = [&content](std::string filename) {
			std::fstream file(filename, std::fstream::out);
			file << content;
			if (fs::file_size(filename) > 0) {
				term.out<Info>("Downloaded server file " + filename + " with file size " + std::to_string(fs::file_size(filename)));
			} else if (fs::file_size(filename) == 0) {
				term.out<Error>("There was an error downloading or saving the server file");
			} else if (fs::file_size(filename) == -1) {
				term.out<Error>("There was an error reading the server file");
			}
			file.close();
		};
		term.out<Info>("Updating server software with name " + serverSettings.autoUpdateName + " and version " + serverSettings.autoUpdateVersion);
		if (serverSettings.autoUpdateName == "purpur") {
			std::string target = "/v2/purpur/" + serverSettings.autoUpdateVersion + "/latest/download";
			#if defined(CPPHTTPLIB_OPENSSL_SUPPORT)
			httplib::SSLClient cli("api.purpurmc.org");
			cli.enable_server_certificate_verification(false);
			#else
			httplib::Client cli("http://api.purpurmc.org");
			#endif
			httplib::Headers headers = { {"Accept-Encoding", "gzip, deflate"} };
			auto res = cli.Get(target.c_str(), headers, [&](const char * data, size_t length) {
				content.append(data, length);
				return true;
			});
			makeFile("purpur.jar");
		} else if (serverSettings.autoUpdateName == "paper") {
			term.out<Error>("Paper is not yet supported");
		} else if (serverSettings.autoUpdateName == "fabric") {
			std::string target = "/v2/versions/loader/" + serverSettings.autoUpdateVersion;
			httplib::Headers headers = { {"Accept-Encoding", "gzip, deflate"} };

			#if defined(CPPHTTPLIB_OPENSSL_SUPPORT)
			httplib::SSLClient cli("meta.fabricmc.net");
			cli.enable_server_certificate_verification(false);
			#else
			httplib::Client cli("http://meta.fabricmc.net");
			#endif
			auto res = cli.Get(target.c_str(), headers, [&](const char * data, size_t length) {
				content.append(data, length);
				return true;
			});
			std::smatch matches;
			if(std::regex_search(content, matches, std::regex("\"loader\"(.+\n){4}.+(\"version\": \")([0-9.]+)"))) {
				target += "/" + string(matches[3]);
			}

			content = "";

			#if defined(CPPHTTPLIB_OPENSSL_SUPPORT)
			httplib::SSLClient cli2("meta.fabricmc.net");
			cli2.enable_server_certificate_verification(false);
			#else
			httplib::Client cli2("http://meta.fabricmc.net");
			#endif
			auto res2 = cli2.Get("/v2/versions/installer", headers, [&](const char * data, size_t length) {
				content.append(data, length);
				return true;
			});
			if(std::regex_search(content, matches, std::regex("(\"version\": \")([0-9.]+)"))) {
				target += "/" + string(matches[2]);
			}

			content = "";
			target += "/server/jar";

			#if defined(CPPHTTPLIB_OPENSSL_SUPPORT)
			httplib::SSLClient cli3("meta.fabricmc.net");
			cli3.enable_server_certificate_verification(false);
			#else
			httplib::Client cli3("http://meta.fabricmc.net");
			#endif
			auto res3 = cli3.Get(target.c_str(), headers, [&](const char * data, size_t length) {
				content.append(data, length);
				return true;
			});

			makeFile("fabric.jar");
		} else {
			term.out<Error>("Invalid auto update server name");
		}
	}
}

void Server::processAutoRestart() {
	if (serverSettings.restartMins > 0 && serverAttributes.uptime >= serverSettings.restartMins) {
		writeToServerTerminal("stop");
	}	else if (serverSettings.restartMins > 0 && serverAttributes.uptime >= (serverSettings.restartMins - 5) && !serverAttributes.said5MinRestart) {
		writeToServerTerminal(formatWrapper(addNumberColors(text.server.restart.minutes5)));
		serverAttributes.said5MinRestart = true;
	} else if (serverSettings.restartMins > 0 && serverAttributes.uptime >= (serverSettings.restartMins - 15) && !serverAttributes.said15MinRestart) {
		writeToServerTerminal(formatWrapper(addNumberColors(text.server.restart.minutes15)));
		serverAttributes.said15MinRestart = true;
	}
}

void Server::terminalAccessWrapper() {
	term.dividerLine(serverSettings.name + " terminal");
	term.hajimeTerminal = false;
	term.normalDisabled = true;
	serverAttributes.wantsLiveOutput = true;
	for (const auto& it : lines) {
		std::cout << it << std::flush;
	}
	std::string user_input = "";
	while (true) {
		std::getline(std::cin, user_input); //getline allows for spaces
		if (user_input == ".d") {
			serverAttributes.wantsLiveOutput = false;
			break;
		} else if (user_input[0] == '.') {
			std::cout << text.error.InvalidCommand << std::endl;
			std::cout << text.error.InvalidServerCommand1 << std::endl;
		} else {
			//std::cout << "sending to server" << std::endl;
			writeToServerTerminal(user_input); //write to the master side of the pterminal with user_input converted into a c-style string
		}
	}
	term.normalDisabled = false;
	term.hajimeTerminal = true;
	term.dividerLine("Back to Hajime");
}
