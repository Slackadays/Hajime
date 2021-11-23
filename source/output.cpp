#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <regex>

using std::string;
using std::ofstream;

#include "output.h"
#include "languages.h"

void Output::init(string file, bool debugOrNot) {
	logToFile = true;
	logFilename = file;
	debug = debugOrNot;
	fileObj.open(logFilename, std::ios::app); //appends to a current file and creates it if needed
}

void Output::out(string data, outType type, bool keepEndlines, bool endLineAtEnd) {
	if (!debug && type == Debug) {
		return;
	}
	if (type == Question) {
		keepEndlines = false;
		endLineAtEnd = false;
	}
	string outputString = Output::addPrefixByType(Output::removeEndlines(data, keepEndlines), type);
	if (noColors) {
		outputString = std::regex_replace(outputString, std::regex("\\\033\\[(\\d+;)*\\d+m", std::regex_constants::optimize), ""); //I hate this
	}
	if (!logToFile) {
		std::lock_guard<std::mutex> lock(outMutex);
		std::cout << outputString;
		if (endLineAtEnd) {
			std::cout << std::endl;
		}
	} else {
		std::lock_guard<std::mutex> lock(outMutex);
		fileObj << outputString << std::endl;
	}
}

bool Output::getYN(string prompt) {
	string response = "";
	if (prompt != "") {
		prompt = " " + prompt + " ";
	}
	this->out("\033[1m" + prompt, None, 0, 0);
	std::cin >> response;
	this->out("\033[0m", None, 0, 0);
	if (response == "y" || response == "Y" || response == "yes" || response == "Yes" || response == "YES" || response == "YEs") {
		return true;
	}
	else {
		return false;
	}
}

void Output::end(){
	std::lock_guard<std::mutex> lock(outMutex);
	fileObj.close();
	logToFile = false;
}

string Output::removeEndlines(string input, bool keepEndlines){
	if (!keepEndlines){
		input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
	}
	return input;
}

string Output::addPrefixByType(string input, outType type){
	string prefix = "";
	bool blank = false;
	if (verbose) {
		switch (type) {
			case None:
				blank = true; //None is if you want to preserve input
				break;
			case Info:
				prefix = text.prefixVInfo; //cyan background
				break;
			case Error:
				prefix = text.prefixVError; //red background, yellow text
				break;
			case Warning:
				prefix = text.prefixVWarning; //yellow text
				break;
			case Question:
				prefix = text.prefixVQuestion; //green background
				break;
			case Debug:
				if (debug) {
					prefix = text.prefixVDebug;
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
				prefix = text.prefixInfo; //cyan background
				break;
			case Error:
				prefix = text.prefixError; //red background, yellow text
				break;
			case Warning:
				prefix = text.prefixWarning; //yellow text
				break;
			case Question:
				prefix = text.prefixQuestion; //green background
				break;
			case Debug:
				if (debug) {
					prefix = text.prefixDebug;
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
			prefix += "| Main]\033[0m ";
		} else {
			prefix += "|M]\033[0m ";
		}
	} else {
		if (verbose) {
			prefix += "| Worker]\033[0m ";
		} else {
			prefix += "|W]\033[0m ";
		}
	}
	return (prefix + input);
}
std::shared_ptr<Output> logObj = std::make_shared<Output>();
