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
#include <type_traits>

#include "languages.hpp"

using std::string;
using std::ofstream;

enum outType {None, Info, Error, Warning, Debug, Question, Force};
enum lines {Def = 2, True = 1, False = 0};

class Output {
	inline static std::mutex outMutex;
	std::thread::id main_thread = std::this_thread::get_id();
	ofstream fileObj;
	string removeEndlines(string input, bool keepEndlines = false);
	string addPrefixByType(string data = "", outType type = None);
	string getColorByID();
	string processMonochrome(string input);
	bool isExcluded(outType type);
	void processOutput(string data, outType type, bool keepEndlines, bool endLineAtEnd);
	void terminalDispatch(string input, outType type, bool endLineAtEnd);
	void fileDispatch(string input, outType type, bool endLineAtEnd);
	inline static int threadCounter;
	inline static std::unordered_map<std::thread::id, int> threadToNumMap;
	inline static std::unordered_map<std::thread::id, string> threadToNameMap;
	inline static bool logToFile;
	inline static string logFilename;

	public:
		void out(string data, outType type = None, bool keepEndlines = false, bool endLineAtEnd = true);

		template<typename ...T>
		int getYN(T ...options) {
			string response;
			bool isComplex = false;
			int i = 0;
			if constexpr ((std::is_same_v<string, T> || ...)) { //check if we get a string in it or not
				isComplex = true;
				this->out("\n\033[1m" + string(" ─> " + text.option.ChooseOptionBelow), None, 1, 1);
				(this->out(("\033[1m " + std::to_string(++i) + ")\033[0m " + options), None), ...);
				this->out("\033[1m" + string(" ─> " + text.option.YourChoice), None, 0, 0);
			} else {
				this->out("\033[1m " + text.question.Prompt + ' ', None, 0, 0);
			}
			std::getline(std::cin, response);
			this->out("\033[0m", None, 0, 0);
			if (!isComplex) {
				if (std::regex_match(text.question.Prompt, std::regex("\\[" + response.substr(0, 1) + "\\/.*", std::regex_constants::optimize | std::regex_constants::icase))) { //match the first character of the response plus the rest of the prompt against the prompt provided by the language
					return true;
				}
				else {
					return false;
				}
			} else {
				while (true) {
					try {
						if ((stoi(response) > i) || (stoi(response) < 1)) {
							throw 1;
						}
						return stoi(response);
					} catch(...) {
						this->out("Answer not valid", Error);
						this->out("\033[1mYour choice: ", None, 0, 0);
						std::getline(std::cin, response);
						this->out("\033[0m", None, 0, 0);
					}
				}
			}
		}

		inline static bool isWindows;
		void init(const string& file, bool debugOrNot = false);
		void addServerName(const string& name);
		void end();
		inline static int showThreadsAsColors;
		inline static bool showExplicitInfoType;
		inline static bool normalDisabled;
		inline static bool hajimeTerminal;
		inline static bool noColors;
		inline static bool reduceColors;
		inline static bool verbose;
		inline static int debug;
		Output();
};

static Output hjlog;
