#pragma once //this guards against g++ error "redefinition of class Output"

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>

using std::string;
using std::ofstream;

enum Type {None, Info, Error, Warning, Debug};

class Output {
	std::mutex outMutex;
	std::mutex endMutex;
	std::thread::id main_thread = std::this_thread::get_id();
	bool logToFile = false;
	bool debug = true;
	string logFilename;
	ofstream fileObj;
	string removeEndlines(string input, bool keepEndlines);
	string addColorsByType(string data, Type type);
	public:
		void out(string data, Type type, bool keepEndlines, bool endLineAtEnd);
		void init(string file, bool debugOrNot);
		void end();
};

void Output::init(string file, bool debugOrNot = true) {
	logToFile = true;
	logFilename = file;
	debug = debugOrNot;
	fileObj.open(logFilename, std::ios::app); //appends to a current file and creates it if needed
}

void Output::out(string data, Type type = None, bool keepEndlines = false, bool endLineAtEnd = true) {
	if (!debug && type == Debug) {
		return;
	}
	string outputString = Output::addColorsByType(Output::removeEndlines(data, keepEndlines), type);
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

void Output::end(){
	std::lock_guard<std::mutex> lock(endMutex);
	fileObj.close();
	logToFile = false;
}

string Output::removeEndlines(string input, bool keepEndlines = false){
	if (!keepEndlines){
		input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
	}
	return input;
}

string Output::addColorsByType(string input = "", Type type = None){
	string prefix = "";
	if (type == None){return input;} //None is if you want to preserve input
	if (type == Info){prefix = text.prefixInfo;} //cyan background
	if (type == Error){prefix = text.prefixError;} //red background, yellow text
	if (type == Warning){prefix = text.prefixWarning;} //yellow text
	if ((type == Debug) && debug){prefix = text.prefixDebug;} //magenta background
	if (main_thread == std::this_thread::get_id()) {prefix += "| Main ]\033[0m ";} else {prefix += "| Worker ]\033[0m ";}
	return (prefix + input);
}
