#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <thread>

#pragma once //this guards against g++ error "redefinition of class Output"

using std::string;
using std::cout;
using std::endl;
using std::ofstream;
using std::thread;

class Output {
	thread::id main_thread = std::this_thread::get_id();
	bool logToFile = false;
	bool debug = true;
	string logFilename;
	ofstream fileObj;
	string removeEndlines(string input, bool keepEndlines);
	string addColorsByType(string data, string type);
	public:
		void out(string data, string type, bool keepEndlines, bool endLineAtEnd);
		void init(string file, bool debugOrNot);
		void end();
};

void Output::init(string file, bool debugOrNot = true) {
	logToFile = true;
	logFilename = file;
	debug = debugOrNot;
	fileObj.open(logFilename, std::ios::app); //appends to a current file and creates it if needed
}

void Output::out(string data, string type = "none", bool keepEndlines = false, bool endLineAtEnd = true) {
	if (!debug && type == "debug") {
		return;
	}
	if (!logToFile) {
		cout << Output::addColorsByType(Output::removeEndlines(data, keepEndlines), type);
		if (endLineAtEnd) {
			cout << endl;
		}
	} else {
		fileObj << Output::addColorsByType(Output::removeEndlines(data, keepEndlines), type) << endl;
	}
}

void Output::end(){
	fileObj.close();
	logToFile = false;
}

string Output::removeEndlines(string input = "", bool keepEndlines = false){
	if (!keepEndlines){
		for (unsigned int i = 0; i <= input.length(); i++){ //removes all \n's from the string
			while (input[i] == '\n'){ //loops a position because when the character is removed, the next one shifts into place
				input[i] = '\0'; // \0 is the null character
			}
		}
	}
	return input;
}

string Output::addColorsByType(string input = "", string type = "none"){
	string prefix = "";
	if (type == "none"){return input;} //"none" is if you want to preserve input
	if (type == "info"){prefix = text.prefixInfo;} //cyan background
	if (type == "error"){prefix = text.prefixError;} //red background, yellow text
	if (type == "warning"){prefix = text.prefixWarning;} //yellow text
	if ((type == "debug") && debug){prefix = text.prefixDebug;} //magenta background
	if (main_thread == std::this_thread::get_id()) {prefix += "| Main ]\033[0m ";} else {prefix += "| Worker ]\033[0m ";}
	return (prefix + input);
}
