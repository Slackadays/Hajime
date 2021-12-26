#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <regex>

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

void Output::out(string data, outType type, int keepEndlines, int endLineAtEnd) {
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
				prefix = "\033[36m" + text.prefixVInfo; //cyan background
				break;
			case Error:
				prefix = "\033[41m\033[33m" + text.prefixVError; //red background, yellow text
				break;
			case Warning:
				prefix = "\033[33m" + text.prefixVWarning; //yellow text
				break;
			case Question:
				#if defined(_WIN64) || defined (_WIN32)
				prefix = "\033[92m" + text.prefixVQuestion; //green background
				#else
				prefix = "\033[38;2;0;255;0m" + text.prefixVQuestion; //green background
				#endif
				break;
			case Debug:
				if (debug) {
					prefix = "\033[95m" + text.prefixVDebug;
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
				prefix = "\033[36m" + text.prefixInfo; //cyan background
				break;
			case Error:
				prefix = "\033[41m\033[33m" + text.prefixError; //red background, yellow text
				break;
			case Warning:
				prefix = "\033[33m" + text.prefixWarning; //yellow text
				break;
			case Question:
				#if defined(_WIN64) || defined (_WIN32)
				prefix = "\033[92m" + text.prefixQuestion; //green background
				#else
				prefix = "\033[38;2;0;255;0m" + text.prefixQuestion; //green background
				#endif
				break;
			case Debug:
				if (debug) {
					prefix = "\033[95m" + text.prefixDebug;
				} //magenta background
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
			prefix += "| Main]";
		} else {
			prefix += "|M]";
		}
	} else {
		if (verbose) {
			prefix += "| Worker]";
		} else {
			prefix += "|W]";
		}
	}
	if (reduceColors) {
		prefix += "\033[0m ";
	} else {
		input += "\033[0m";
	}
	return (prefix + input);
}
std::shared_ptr<Output> logObj = std::make_shared<Output>();
