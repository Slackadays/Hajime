#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <regex>
#include <functional>
#include <unordered_map>

using std::string;
using std::ofstream;

#include "output.hpp"
#include "languages.hpp"

void Output::init(string file, bool debugOrNot) {
	logToFile = true;
	logFilename = file;
	debug = debugOrNot;
	fileObj.open(logFilename, std::ios::app); //appends to a current file and creates it if needed
}

void Output::out(string data, outType type, bool keepEndlines, bool endLineAtEnd) {
	if (normalDisabled && !debug) {
		return;
	}
	if (!debug && type == Debug) {
		return;
	}
	if (type == Question) {
		keepEndlines = false;
		endLineAtEnd = false;
	}
	string outputString = Output::addPrefixByType(Output::removeEndlines(data, keepEndlines), type);
	if (noColors || logToFile) {
		outputString = std::regex_replace(outputString, std::regex("\\\033\\[(\\d+;)*\\d+m", std::regex_constants::optimize), ""); //I hate this
	}
	if (!logToFile) {
		std::lock_guard<std::mutex> lock(outMutex);
		if (type == Error) {
			std::cerr << outputString;
		} else {
			std::cout << outputString;
		}
		if (endLineAtEnd) {
			if (type == Error) {
				std::cerr << std::endl;
			} else {
				std::cout << std::endl;
			}
		}
	} else {
		std::lock_guard<std::mutex> lock(outMutex);
		fileObj << outputString;
		if (endLineAtEnd) {
			fileObj << std::endl;
		}
	}
}

bool Output::getYN(string prompt) {
	string response;
	if (prompt != "") {
		prompt = " " + prompt + " ";
	}
	this->out("\033[1m" + prompt, None, 0, 0);
	std::cin >> response;
	this->out("\033[0m", None, 0, 0);
	if (std::regex_match(text.questionPrompt, std::regex("\\[" + response.substr(0, 1) + "\\/.*", std::regex_constants::optimize | std::regex_constants::icase))) { //match the first character of the response plus the rest of the prompt against the prompt provided by the language
		return true;
	}
	else {
		return false;
	}
}

void Output::end() {
	std::lock_guard<std::mutex> lock(outMutex);
	fileObj.close();
	logToFile = false;
}

string Output::removeEndlines(string input, bool keepEndlines) {
	if (!keepEndlines) {
		input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
	}
	return input;
}

string Output::addPrefixByType(string input, outType type) {
	string prefix = "";
	bool blank = false;
	if (verbose) {
		switch (type) {
			case None:
				blank = true; //None is if you want to preserve input
				break;
			case Info:
				prefix = (showThreadsAsColors ? getColorByID() : "\033[36m[") + "[" + (showExplicitInfoType ? text.prefixVInfo + "|" : ""); //cyan background
				break;
			case Error:
				prefix = (showThreadsAsColors ? getColorByID() : "\033[41m\033[33m[") + "[" + (showExplicitInfoType ? text.prefixVError + "|" : ""); //red background, yellow text
				break;
			case Warning:
				prefix = (showThreadsAsColors ? getColorByID() : "\033[33m[") + "[" + (showExplicitInfoType ? text.prefixVWarning + "|" : ""); //yellow text
				break;
			case Question:
				#if defined(_WIN64) || defined (_WIN32)
				prefix = (showThreadsAsColors ? getColorByID() : "\033[92m[") + "[" + (showExplicitInfoType ? text.prefixVQuestion + "|" : ""); //green background
				#else
				prefix = (showThreadsAsColors ? getColorByID() : "\033[38;2;0;255;0m[") + "[" + (showExplicitInfoType ? text.prefixVQuestion + "|" : ""); //green background
				#endif
				break;
			case Debug:
				if (debug) {
					prefix = (showThreadsAsColors ? getColorByID() : "\033[95m[") + "[" + (showExplicitInfoType ? text.prefixVDebug + "|" : "");
				} //magenta background
				break;
			default:
				break;
		}
	} else {
		switch (type) {
			case None:
				blank = true; //None is if you want to preserve input
				break;
			case Info:
				prefix = (showThreadsAsColors ? getColorByID() : "\033[36m") + "[" + (showExplicitInfoType ? text.prefixInfo + "|" : ""); //cyan background(showThreadsAsColors ? getColorByID() :  )
				break;
			case Error:
				prefix = (showThreadsAsColors ? getColorByID() : "\033[41m\033[33m") + "[" + (showExplicitInfoType ? text.prefixError + "|" : ""); //red background, yellow text
				break;
			case Warning:
				prefix = (showThreadsAsColors ? getColorByID() : "\033[33m") + "[" + (showExplicitInfoType ? text.prefixWarning + "|" : ""); //yellow text
				break;
			case Question:
				#if defined(_WIN64) || defined (_WIN32)
				prefix = (showThreadsAsColors ? getColorByID() : "\033[92m") + "[" + (showExplicitInfoType ? text.prefixQuestion + "|" : ""); //green background
				#else
				prefix = (showThreadsAsColors ? getColorByID() : "\033[38;2;0;255;0m") + "[" + (showExplicitInfoType ? text.prefixQuestion + "|" : ""); //green background
				#endif
				break;
			case Debug:
				if (debug) {
					prefix = (showThreadsAsColors ? getColorByID() : "\033[95m") + "[" + (showExplicitInfoType ? text.prefixDebug + "|" : ""); //magenta background
				}
				break;
			default:
				break;
		}
	}
	if (blank) {
		return input;
	}
	if (main_thread == std::this_thread::get_id()) {
		if (verbose) {
			prefix += "Hajime]";
		} else {
			prefix += "H]";
		}
	} else {
		if (verbose) {
			prefix += "Thread ";
		}
		if (threadToNumMap.count(std::this_thread::get_id()) == 0) { // thread id does not exist in threadtonummap
			threadToNumMap[std::this_thread::get_id()] = ++threadCounter;
		}
		prefix += std::to_string(threadToNumMap[std::this_thread::get_id()]);
		prefix += "]";
	}
	if (reduceColors) {
		prefix += "\033[0m ";
	} else {
		input += "\033[0m";
	}
	return (prefix + input);
}

void Output::addServerName(string serverName) {
	if (!threadToNameMap.count(std::this_thread::get_id())) {
		threadToNameMap[std::this_thread::get_id()] = serverName;
	} else {
		throw "Server name conflict";
	}
}

string Output::getColorByID() {
	std::hash<string> hasher;
	int colorID;
	if (threadToNameMap.count(std::this_thread::get_id())) { //do not access threadtonamemap directly because doing so would add the thread id as a key
		colorID = hasher(threadToNameMap[std::this_thread::get_id()]) % 15;
	} else if (std::this_thread::get_id() == main_thread) {
		colorID = 13;
	} else {
		colorID = 0;
	}
	switch (colorID) {
		case 1:
			return "\033[31m";
		case 2:
			return "\033[32m";
		case 3:
			return "\033[33m";
		case 4:
			return "\033[34m";
		case 5:
			return "\033[35m";
		case 6:
			return "\033[36m";
		case 7:
			return "\033[37m";
		case 8:
			return "\033[90m";
		case 9:
			return "\033[91m";
		case 10:
			return "\033[92m";
		case 11:
			return "\033[93m";
		case 12:
			return "\033[94m";
		case 13:
			return "\033[95m";
		case 14:
			return "\033[96m";
		default:
			return "";
	}
}

std::shared_ptr<Output> logObj = std::make_shared<Output>();
