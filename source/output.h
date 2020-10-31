//output.h
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#pragma once //this guards against "error: redefinition of class Output"
namespace fs = std::filesystem;
using std::string;
using std::cout;
using std::endl;

class Output {
	bool log = false;
	string filename;
	std::ofstream fileObj;
	public:
		void out(string data);
		void init(string file);
		void end();
};

void Output::init(string file) {
	log = true;
	filename = file;
	fileObj.open(filename, std::ios::app); //appends to a current file and creates it if needed
}

void Output::out(string data){
	if(log == false){
		cout << data << endl;
	} else {
		fileObj << data << endl;
	}
}

void Output::end(){
	fileObj.close();
	log = false;
}
