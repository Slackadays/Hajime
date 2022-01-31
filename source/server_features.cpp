#if defined(_WIN64) || defined(_WIN32)
#include <Windows.h>
#include <shellapi.h>
#pragma comment (lib, "Shell32")
#else
#include <unistd.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <fcntl.h>
#include <fstream>
#include <thread>
#include <list>
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

#include "getvarsfromfile.hpp"
#include "server.hpp"

using std::shared_ptr;
using std::string;
using std::fstream;
using std::to_string;
using std::ofstream;
using std::ios;
using std::vector;
using std::cout;
using namespace std::chrono;

namespace fs = std::filesystem;
namespace ch = std::chrono;

void Server::processTerminalBuffer(string input) {
	while (lines.size() >= 100000) {
		//std::cout << "Popping, ws.row = " << w.ws_row << std::endl;
		lines.pop_front();
		//std::cout << "lines size = " << (unsigned short)lines.size() << w.ws_row << std::endl;
	}
	input = std::regex_replace(input, std::regex(">\\.\\.\\.\\.", std::regex_constants::optimize), ">"); //replace ">...." with ">" because this shows up in the temrinal output
	//std::cout << "Pushing back" << std::endl;
	lines.push_back(input);
	if (wantsLiveOutput) {
		std::cout << input << std::flush;
	}
}

void Server::processServerCommand(string input) {
	if (std::regex_search(input, std::regex("\\" + text.server.command.hajime.regex + "(?![\\w])", std::regex_constants::optimize))) {
		commandHajime();
	} else if (std::regex_search(input, std::regex("\\" + text.server.command.time.regex + "(?![\\w])", std::regex_constants::optimize))) {
		commandTime();
	} else if (std::regex_search(input, std::regex("\\" + text.server.command.help.regex + "{0,1}(?![\\w])", std::regex_constants::optimize))) {
		commandHelp();
	} else if (std::regex_search(input, std::regex("\\" + text.server.command.die.regex + "(?![\\w])", std::regex_constants::optimize))) {
		commandDie();
	} else if (std::regex_search(input, std::regex("\\" + text.server.command.coinflip.regex + "(?![\\w])", std::regex_constants::optimize))) {
		commandCoinflip();
	} else if (std::regex_search(input, std::regex("\\" + text.server.command.discord.regex + "(?![\\w])", std::regex_constants::optimize))) {
		commandDiscord();
	} else if (std::regex_search(input, std::regex("\\" + text.server.command.name.regex + "(?![\\w])", std::regex_constants::optimize))) {
		commandName();
	} else if (std::regex_search(input, std::regex("\\" + text.server.command.uptime.regex + "(?![\\w])", std::regex_constants::optimize))) {
		commandUptime();
	} else if (std::regex_search(input, std::regex("\\" + text.server.command.restart.regex + "(?![\\w])", std::regex_constants::optimize))) {
		commandRestart();
	} else if (std::regex_search(input, std::regex("\\" + text.server.command.system.regex + "(?![\\w])", std::regex_constants::optimize))) {
		commandSystem();
	}
}

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
	writeToServerTerminal(formatWrapper("[{\"text\":\"" + text.server.command.coinflip.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.coinflip + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + text.server.command.coinflip.regex + "\"}},"
	"{\"text\":\"" + text.server.command.die.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.die + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + text.server.command.die.regex + "\"}},"
	"{\"text\":\"" + text.server.command.discord.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.discord + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + text.server.command.discord.regex + "\"}},"
	"{\"text\":\"" + text.server.command.hajime.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.hajime + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + text.server.command.hajime.regex + "\"}},"
	"{\"text\":\"" + text.server.command.help.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.help + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.help.regex, std::regex("(\\(|\\))", std::regex_constants::optimize), "") + "\"}},"
	"{\"text\":\"" + text.server.command.name.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.name + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + text.server.command.name.regex + "\"}},"
	"{\"text\":\"" + text.server.command.time.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.time + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + text.server.command.time.regex + "\"}},"
	"{\"text\":\"" + text.server.command.uptime.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.uptime + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + text.server.command.uptime.regex + "\"}},"
	"{\"text\":\"" + text.server.command.system.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.system + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + text.server.command.system.regex + "\"}},"
	"{\"text\":\"" + text.server.command.restart.regex + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.restart + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + text.server.command.restart.regex + "\"}}]"));
}

void Server::commandDie() {
	std::random_device rand;
	std::uniform_int_distribution<int> die(1, 6);
	string hajInfo = text.server.command.die.output + std::to_string(die(rand));
	writeToServerTerminal(formatWrapper(addNumberColors(hajInfo)));
	//switch this to C++20 format when it becomes supported
}

void Server::commandCoinflip() {
	std::random_device rand;
	std::uniform_int_distribution<int> flip(1, 2);
	if (flip(rand) == 1) {
		writeToServerTerminal(formatWrapper(text.server.command.coinflip.output.heads));
	} else {
		writeToServerTerminal(formatWrapper(text.server.command.coinflip.output.tails));
	}
}

void Server::commandDiscord() {
	writeToServerTerminal(formatWrapper(text.server.command.discord.output));
}

void Server::commandName() {
	string hajInfo = text.server.command.name.output + name;
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandUptime() {
	string hajInfo = text.server.command.uptime.output1 + std::to_string(uptime) + text.server.command.uptime.output2 + std::to_string(uptime / 60.0) + text.server.command.uptime.output3;
	writeToServerTerminal(formatWrapper(addNumberColors(hajInfo)));
}

void Server::commandRestart() {
	string hajInfo;
	if (restartMins > 0) {
		hajInfo = text.server.command.restart.output1 + std::to_string(restartMins - uptime) + text.server.command.restart.output2 + std::to_string((restartMins - uptime) / 60.0) + text.server.command.restart.output3;
	} else {
		hajInfo = text.server.command.restart.outputDisabled;
	}
	writeToServerTerminal(formatWrapper(addNumberColors(hajInfo)));
}

void Server::commandSystem() {
	string hajInfo;
	hajInfo = "[{\"text\":\"[Hajime] \"},{\"text\":\"OS, \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getOS() + "\"}},"
	"{\"text\":\"" + string("CPU") + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCPU() + "\"}},"
	"{\"text\":\"" + string("RAM") + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getRAM() + "\"}}]";
	writeToServerTerminal(formatWrapper(hajInfo));
}

string Server::getOS() {
	#if defined(__linux__)
	std::fstream proc;
	proc.open("/proc/version", std::fstream::in);
	std::ostringstream temp;
	temp << proc.rdbuf();
	string out = temp.str();
	out.erase(std::remove(out.begin(), out.end(), '\n'), out.end());
	return out;
	#endif
	return "Only works on Linux right now";
}

string Server::getCPU() {
	#if defined(__linux__)
	std::smatch m;
	std::fstream proc;
	proc.open("/proc/cpuinfo", std::fstream::in);
	std::ostringstream temp;
	temp << proc.rdbuf();
	string temp2 = temp.str();
	std::regex_search(temp2, m, std::regex("(?:model name\\s*:\\s*)(.*)", std::regex_constants::optimize));
	return m[1];
	#endif
	return "Only works on Linux right now";
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
	#endif
	return "Only works on Linux right now";
}

void Server::processRestartAlert(string input) {
	std::smatch m;
	if (restartMins > 0 && uptime >= (restartMins - 5) && std::regex_search(input, m, std::regex("\\[.+\\]: ([\\w\\d]+)\\[.+\\] .+", std::regex_constants::optimize))) {
		string hajInfo = "tellraw " + string(m[1]) + text.server.restart.alert1 + std::to_string(restartMins - uptime) + text.server.restart.alert2;
		writeToServerTerminal(addNumberColors(hajInfo));
	}
}

string Server::addNumberColors(string input) {
	return std::regex_replace(input, std::regex("( |\\()\\d+\\.?\\d*", std::regex_constants::optimize), "§b$&§f");
}

string Server::formatWrapper(string input) {
	string output;
	if (input.front() == '[' && input.back() == ']') {
		output = "tellraw @a " + input;
	} else {
		output = "tellraw @a \"" + input + "\"";
	}
	output = std::regex_replace(output, std::regex("\\[Hajime\\]", std::regex_constants::optimize), "§6$&§f");
	return output;
}

void Server::writeToServerTerminal(string input) {
	input += "\n"; //this is the delimiter of the server command
	#if defined(_WIN64) || defined(_WIN32)
	DWORD byteswritten;
	if (!WriteFile(inputwrite, input.c_str(), input.size(), &byteswritten, NULL)) {// write to input pipe
		std::cout << "Unable to write to pipe" << std::endl;
	} else if (byteswritten != input.size()) {
		std::cout << "Wrote " + std::to_string(byteswritten) + "bytes, expected " + std::to_string(input.size()) << std::endl;
	}
	#else
	write(fd, input.c_str(), input.length());
	#endif
}

void Server::processServerTerminal() {
	while (true) {
		string terminalOutput = readFromServer();
		processServerCommand(terminalOutput);
		processRestartAlert(terminalOutput);
		processTerminalBuffer(terminalOutput);
	}
}

string Server::readFromServer() {
	char input[1000];
	#if defined(_WIN32) || defined (_WIN64)
	DWORD length = 0;
	if (!ReadFile(outputread, input, 1000, &length, NULL))
	{
		std::cout << "ReadFile failed (unable to read from pipe)" << std::endl;
		return std::string();
	}
	#else
	int length;
	length = read(fd, input, sizeof(input));
	if (length == -1) {
		std::cout << "Error reading file descriptor (errno = " + to_string(errno) + ")" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	#endif
	std::string output;
	for (int i = 0; i < length; i++) {
		output.push_back(input[i]);
	}
	return output;
}

void Server::updateUptime() {
	timeCurrent = std::chrono::steady_clock::now();
	auto tempUptime = std::chrono::duration_cast<std::chrono::minutes>(timeCurrent - timeStart);
	uptime = tempUptime.count();
	//std::cout << "uptime = " + to_string(uptime) << std::endl;
}

void Server::processAutoRestart() {
	if (restartMins > 0 && uptime >= restartMins) {
		writeToServerTerminal("stop");
	}	else if (restartMins > 0 && uptime >= (restartMins - 5) && !said5MinRestart) {
		writeToServerTerminal(formatWrapper(addNumberColors(text.server.restart.minutes5)));
		said5MinRestart = true;
	} else if (restartMins > 0 && uptime >= (restartMins - 15) && !said15MinRestart) {
		writeToServerTerminal(formatWrapper(addNumberColors(text.server.restart.minutes15)));
		said15MinRestart = true;
	}
}

void Server::terminalAccessWrapper() {
	hjlog->normalDisabled = true;
	std::cout << "----->" << name << std::endl;
	wantsLiveOutput = true;
	for (const auto& it : lines) {
		std::cout << it << std::flush;
	}
	while (true) {
		std::string user_input = "";
		std::getline(std::cin, user_input); //getline allows for spaces
		if (user_input == ".d") {
			wantsLiveOutput = false;
			break;
		} else if (user_input[0] == '.') {
			std::cout << text.error.InvalidCommand << std::endl;
			std::cout << text.error.InvalidServerCommand1 << std::endl;
		} else {
			writeToServerTerminal(user_input); //write to the master side of the pterminal with user_input converted into a c-style string
		}
	}
	std::cout << "Hajime<-----" << std::endl;
	hjlog->normalDisabled = false;
}
