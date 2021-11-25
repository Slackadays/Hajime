#pragma once //this guards against g++ error "redefinition of class Output"

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <regex>
#include <memory>

using std::string;
using std::ofstream;

enum outType {None, Info, Error, Warning, Debug, Question};

class Output {
        std::mutex outMutex;
        std::thread::id main_thread = std::this_thread::get_id();
        bool logToFile = false;
        string logFilename;
        ofstream fileObj;
        string removeEndlines(string input, bool keepEndlines = false);
        string addPrefixByType(string data = "", outType type = None);
        public:
                void out(string data, outType type = None, bool keepEndlines = false, bool endLineAtEnd = true);
		            bool getYN(string prompt = "[y/n]");
                void init(string file, bool debugOrNot = true);
                void end();
                bool noColors = false;
                bool verbose = false;
                bool debug = false;
};

extern std::shared_ptr<Output> logObj; // make this pointer global
