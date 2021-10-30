#pragma once //this guards against g++ error "redefinition of class Output"

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <regex>

using std::string;
using std::ofstream;

enum outType {None, Info, Error, Warning, Debug, Question};

class Output {
	std::mutex outMutex;
	std::thread::id main_thread = std::this_thread::get_id();
	bool logToFile = false;
	bool debug = true;
	string logFilename;
	ofstream fileObj;
	string removeEndlines(string input, bool keepEndlines);
	string addPrefixByType(string data, outType type);
	public:
		void out(string data, outType type, bool keepEndlines, bool endLineAtEnd);
		bool getYN(string prompt);
		void init(string file, bool debugOrNot);
		void end();
		bool noColors = false;
};

void Output::init(string file, bool debugOrNot = true) {
	logToFile = true;
	logFilename = file;
	debug = debugOrNot;
	fileObj.open(logFilename, std::ios::app); //appends to a current file and creates it if needed
}

void Output::out(string data, outType type = None, bool keepEndlines = false, bool endLineAtEnd = true) {
	if (!debug && type == Debug) {
		return;
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

bool Output::getYN(string prompt = "[y/n]") {
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

string Output::removeEndlines(string input, bool keepEndlines = false){
	if (!keepEndlines){
		input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
	}
	return input;
}

string Output::addPrefixByType(string input = "", outType type = None){
	string prefix = "";
	switch (type) {
		case None:
			return input; //None is if you want to preserve input
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
	if (main_thread == std::this_thread::get_id()) {
		prefix += "| Main]\033[0m ";} else {prefix += "| Worker]\033[0m ";
	}
	return (prefix + input);
}
