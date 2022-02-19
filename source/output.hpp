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
#include <concepts>

#include "languages.hpp"

using std::string;
using std::ofstream;

enum outFlag {None, Info, Error, Warning, Debug, Question, Force, NoEndline, KeepEndlines, Threadless};
enum lines {Def = 2, True = 1, False = 0};

class Output {
	inline static std::mutex outMutex;
	std::thread::id main_thread = std::this_thread::get_id();
	ofstream fileObj;

	void processOutput(string data, std::same_as<outFlag> auto ...flags) {
		outFlag type;
		bool force = false;
		bool keepEndlines = false;
		bool endLineAtEnd = true;
		bool oldThreadless = threadless;
		auto processFlags = [&](outFlag& flags) constexpr {
			switch (flags) {
				case None:
					type = None;
					break;
				case Info:
					type = Info;
					break;
				case Error:
					type = Error;
					break;
				case Warning:
					type = Warning;
					break;
				case Debug:
					type = Debug;
					break;
				case Question:
					type = Question;
					break;
				case Force:
					force = true;
					break;
				case NoEndline:
					endLineAtEnd = false;
					break;
				case KeepEndlines:
					keepEndlines = true;
					break;
				case Threadless:
					threadless = true;
					break;
			}
		};
		(processFlags(flags), ...);
		if (isExcluded(type)) {
			return;
		}
		if (type == Question) {
			endLineAtEnd = false;
		}
		string outputString;
		outputString = Output::addPrefixByType(Output::removeEndlines(data, keepEndlines), type);
		outputString = processMonochrome(outputString);
		if (hajimeTerminal && (type != None) && endLineAtEnd) {
			outputString = '\r' + outputString;
		}
		terminalDispatch(outputString, type, endLineAtEnd);
		if (logToFile) {
			fileDispatch(outputString, type, endLineAtEnd);
		}
		threadless = oldThreadless;
	}

	string removeEndlines(string input, bool keepEndlines = false);
	string addPrefixByType(string data = "", outFlag type = None);
	string getColorByID();
	string processMonochrome(string input);
	bool isExcluded(outFlag type);
	void terminalDispatch(string input, outFlag type, bool endLineAtEnd);
	void fileDispatch(string input, outFlag type, bool endLineAtEnd);
	inline static int threadCounter;
	inline static std::unordered_map<std::thread::id, int> threadToNumMap;
	inline static std::unordered_map<std::thread::id, string> threadToNameMap;
	inline static bool logToFile;
	inline static string logFilename;

	public:
		void out(string data, std::same_as<outFlag> auto ...flags) {
			processOutput(data, flags...);
			outFlag type;
			bool endLineAtEnd = true;
			auto processFlags = [&](outFlag& flags) constexpr {
				switch (flags) {
					case None:
						type = None;
						break;
					case Info:
						type = Info;
						break;
					case Error:
						type = Error;
						break;
					case Warning:
						type = Warning;
						break;
					case Debug:
						type = Debug;
						break;
					case Question:
						type = Question;
						break;
					case NoEndline:
						endLineAtEnd = false;
						break;
				}
			};
			(processFlags(flags), ...);
			if (hajimeTerminal && (type != None) && endLineAtEnd) {
				terminalDispatch('\r' + text.info.EnterCommand, None, 0);
			}
		}

		template<typename ...T>
		int getYN(T ...options) {
			string response;
			bool isComplex = false;
			int i = 0;
			if constexpr ((std::is_same_v<string, T> || ...)) { //check if we get a string in it or not
				isComplex = true;
				this->out("\n\033[1m" + string(" ─> " + text.option.ChooseOptionBelow), None, KeepEndlines);
				(this->out(("\033[1m " + std::to_string(++i) + ")\033[0m " + options), None), ...);
				this->out("\033[1m" + string(" ─> " + text.option.YourChoice), None, NoEndline);
			} else {
				this->out("\033[1m " + text.question.Prompt + ' ', None, NoEndline);
			}
			std::getline(std::cin, response);
			this->out("\033[0m", None, NoEndline);
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
						this->out("\033[1mYour choice: ", None, NoEndline);
						std::getline(std::cin, response);
						this->out("\033[0m", None, NoEndline);
					}
				}
			}
		}

		inline static bool isWindows;
		void init(const string& file, bool debugOrNot = false);
		void addServerName(const string& name);
		void end();
		inline static bool threadless;
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
