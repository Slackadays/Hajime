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
	string removeEndlines(string input);
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
	if (!logToFile){
		cout << Output::removeEndlines(data) << endl;
	} else {
		fileObj << Output::removeEndlines(data) << endl;
	}
}

void Output::end(){
	fileObj.close();
	logToFile = false;
}

string Output::removeEndlines(string input){
	for (int i = 0; i <= input.length(); i++){ //removes all \n's from the string
		while (input[i] == '\n'){ //loops a position because when the character is removed, the next one shifts into place
			input[i] = '\0'; // \0 is the null character
		}
	}
	return input;
}
