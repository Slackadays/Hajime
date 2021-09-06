#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#pragma once //this guards against g++ error "redefinition of class Output"

using std::string;
using std::cout;
using std::endl;
using std::ofstream;

class Output {
	bool logToFile = false;
	string logFilename;
	ofstream fileObj;
	public:
		void out(string data);
		void init(string file);
		void end();
};

void Output::init(string file) {
	logToFile = true;
	logFilename = file;
	fileObj.open(logFilename, std::ios::app); //appends to a current file and creates it if needed
}

void Output::out(string data){
	if(!logToFile){
		cout << data << endl;
	} else {
		fileObj << data << endl;
	}
}

void Output::end(){
	fileObj.close();
	logToFile = false;
}
