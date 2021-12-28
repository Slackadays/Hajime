#pragma once //this guards against g++ error "redefinition of class Output"

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <regex>
#include <memory>
#include <unordered_map>

#include "languages.hpp"

using std::string;
using std::ofstream;

enum outType {None, Info, Error, Warning, Debug, Question, Force};
enum lines {Def = 2, True = 1, False = 0};

class Output {
	std::mutex outMutex;
	std::thread::id main_thread = std::this_thread::get_id();
	bool logToFile = false;
	string logFilename;
	ofstream fileObj;
	string removeEndlines(string input, bool keepEndlines = false);
	string addPrefixByType(string data = "", outType type = None);
	string getColorByID();
	inline static int threadCounter = 0;
	inline static std::unordered_map<std::thread::id, int> threadToNumMap;
	inline static std::unordered_map<std::thread::id, string> threadToNameMap;
	public:
		void out(string data, outType type = None, bool keepEndlines = false, bool endLineAtEnd = true);
		bool getYN(string prompt = text.questionPrompt);
		void init(string file, bool debugOrNot = true);
		void addServerName(string name);
		void end();
		bool showThreadsAsColors = false;
		bool showExplicitInfoType = false;
		bool normalDisabled = false;
		bool noColors = false;
		bool reduceColors = true;
		bool verbose = false;
		int debug = 0;
};

extern std::shared_ptr<Output> logObj; // make this pointer global
