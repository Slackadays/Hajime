#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <regex>
#include <functional>
#include <unordered_map>
#include <cmath>

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
				prefix = (showThreadsAsColors ? getColorByID() : "\033[36m") + "[" + (showExplicitInfoType ? text.prefixVInfo + "|" : ""); //cyan background
				break;
			case Error:
				prefix = (showThreadsAsColors ? getColorByID() : "\033[41m\033[33m") + "[" + (showExplicitInfoType ? text.prefixVError + "|" : ""); //red background, yellow text
				break;
			case Warning:
				prefix = (showThreadsAsColors ? getColorByID() : "\033[33m") + "[" + (showExplicitInfoType ? text.prefixVWarning + "|" : ""); //yellow text
				break;
			case Question:
				#if defined(_WIN64) || defined (_WIN32)
				prefix = (showThreadsAsColors ? getColorByID() : "\033[92m") + "[" + (showExplicitInfoType ? text.prefixVQuestion + "|" : ""); //green background
				#else
				prefix = (showThreadsAsColors ? getColorByID() : "\033[38;2;0;255;0m") + "[" + (showExplicitInfoType ? text.prefixVQuestion + "|" : ""); //green background
				#endif
				break;
			case Debug:
				if (debug) {
					prefix = (showThreadsAsColors ? getColorByID() : "\033[95m") + "[" + (showExplicitInfoType ? text.prefixVDebug + "|" : "");
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
	int selection;
	if (threadToNameMap.count(std::this_thread::get_id())) { //do not access threadtonamemap directly because doing so would add the thread id as a key
		selection = hasher(threadToNameMap[std::this_thread::get_id()]) % 211;
	} else if (std::this_thread::get_id() == main_thread) {
		return "\033[36m";
	} else {
		return "";
	}
	float dummy;
 	float bgID = std::modf(selection / 14.0, &dummy);
	string returnedColor;
	switch ((int)ceil(selection / 15.0)) {
		case 1:
			returnedColor += "\033[31m"; //dark red text
			break;
		case 2:
			returnedColor += "\033[32m"; //dark green text
			break;
		case 3:
			returnedColor += "\033[33m"; //dark yellow text
			break;
		case 4:
			returnedColor += "\033[34m"; //dark blue text
			break;
		case 5:
			returnedColor += "\033[35m"; //dark magenta text
			break;
		case 6:
			returnedColor += "\033[36m"; //dark cyan text
			break;
		case 7:
			returnedColor += "\033[37m"; //dark white text
			break;
		case 8:
			returnedColor += "\033[90m"; //bright black text
			break;
		case 9:
			returnedColor += "\033[91m"; //bright red text
			break;
		case 10:
			returnedColor += "\033[92m"; //bright green text
			break;
		case 11:
			returnedColor += "\033[93m"; //bright yellow text
			break;
		case 12:
			returnedColor += "\033[94m"; //bright blue text
			break;
		case 13:
			returnedColor += "\033[95m"; //bright magenta text
			break;
		case 14:
			returnedColor += "\033[96m"; //bright cyan text
			break;
		default:
			break;
		}
	switch ((int)round(bgID * 15.0)) {
		case 0:
			returnedColor += "\033[41m"; //dark red background
			break;
		case 1:
			returnedColor += "\033[42m"; //gark green bg
			break;
		case 2:
			returnedColor += "\033[43m"; //dy bg
			break;
		case 3:
			returnedColor += "\033[44m"; //db bg
			break;
		case 4:
			returnedColor += "\033[45m"; //dm bg
			break;
		case 5:
			returnedColor += "\033[46m"; //dc bg
			break;
		case 6:
			returnedColor += "\033[47m"; //dw bg
			break;
		case 7:
			returnedColor += "\033[100m"; //bb bg
			break;
		case 8:
			returnedColor += "\033[101m"; //br bg
			break;
		case 9:
			returnedColor += "\033[102m"; //bg bg
			break;
		case 10:
			returnedColor += "\033[103m"; //by bg
			break;
		case 11:
			returnedColor += "\033[104m"; //bb bg
			break;
		case 12:
			returnedColor += "\033[105m"; //bm bg
			break;
		case 13:
			returnedColor += "\033[106m"; //bc bg
			break;
		case 14:
			break;
		default:
			break;
	}
	return returnedColor;
}

std::shared_ptr<Output> hjlog = std::make_shared<Output>();
