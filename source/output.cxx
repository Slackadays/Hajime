module;

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <regex>
#include <memory>

export module Hajime:Output;

import :Languages;
//#include "languages.hpp"

using std::string;
using std::ofstream;

export enum outType {None, Info, Error, Warning, Debug, Question};

export class Output {
        std::mutex outMutex;
        std::thread::id main_thread = std::this_thread::get_id();
        bool logToFile = false;
        string logFilename;
        ofstream fileObj;
        string removeEndlines(string input, bool keepEndlines = false);
        string addPrefixByType(string data = "", outType type = None);
        public:
                void out(string data, outType type = None, bool keepEndlines = false, bool endLineAtEnd = true);
		            bool getYN(string prompt = text.questionPrompt);
                void init(string file, bool debugOrNot = true);
                void end();
                bool noColors = false;
                bool verbose = false;
                int debug = 0;
};

export extern std::shared_ptr<Output> logObj; // make this pointer global
