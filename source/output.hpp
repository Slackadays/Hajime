/*  Hajime, the ultimate startup script.
    Copyright (C) 2022 Slackadays and other contributors to Hajime on GitHub.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.*/

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

	string removeEndlines(string input, bool keepEndlines = false);
	string addPrefixByType(string data = "", outFlag type = None);
	string getColorByID();
	string makeMonochrome(string input);
	bool isExcluded(outFlag type);
	void terminalDispatch(string input, outFlag type, bool endLineAtEnd);
	void fileDispatch(string input, outFlag type, bool endLineAtEnd);
	inline static int threadCounter;
	inline static std::unordered_map<std::thread::id, int> threadToNumMap;
	inline static std::unordered_map<std::thread::id, string> threadToNameMap;
	inline static bool logToFile;
	inline static string logFilename;

	public:
		int getTerminalWidth();
		void dividerLine(string tx = "");

		template <std::same_as<outFlag> auto... flags>
		void out(string data) {
			constexpr bool force = ((flags == Force) || ...);
			constexpr bool keepEndlines = ((flags == KeepEndlines) || ...);
			const bool oldThreadless = threadless;
			threadless = ((flags == Threadless) || ...);
			constexpr outFlag type = [] {
				for (auto flag : std::initializer_list<outFlag>{flags...}) {
					switch (flag) {
						case None:
							return None;
						case Info:
							return Info;
						case Error:
							return Error;
						case Warning:
							return Warning;
						case Debug:
							return Debug;
						case Question:
							return Question;
						case KeepEndlines: //prevent a warning from Clang
							break;
						case Force:
							break;
						case NoEndline:
							break;
						case Threadless:
							break;
					}
				}
				return None;
			}();
			constexpr bool endLineAtEnd = (type != Question) && !((flags == NoEndline) || ...);
			if (isExcluded(type)) {
				return;
			}
			string outputString;
			outputString = Output::addPrefixByType(Output::removeEndlines(data, keepEndlines), type);
			if (noColors) {
				outputString = makeMonochrome(outputString);
			}
			if (type != None) {
				outputString = "┃" + string(getTerminalWidth() - 2, ' ') + "┃\r┃" + outputString;
			}
			//std::cout << "first char of last output = " << makeMonochrome(lastOutput.substr(0, 3)) << std::endl;
			//std::cout << "new output = " << makeMonochrome(outputString.substr(0, 18)) << std::endl;
			/*if (!lastOutput.empty() && makeMonochrome(lastOutput.substr(0, 3)) == "━" && makeMonochrome(outputString.substr(0, 18)) == "┃") {
				lastOutput = "┏" + lastOutput;
				lastOutput.erase(lastOutput.length() - 1, 1);
				terminalDispatch('\r' + lastOutput, None, false);
			}*/
			if (hajimeTerminal && (type != None) && endLineAtEnd) {
				outputString = '\r' + outputString;
			}
			terminalDispatch(outputString, type, endLineAtEnd);
			if (logToFile) {
				fileDispatch(outputString, type, endLineAtEnd);
			}
			threadless = oldThreadless;
			if (hajimeTerminal && (type != None) && endLineAtEnd) {
				terminalDispatch("\r\033[92m\033[1m" + text.info.EnterCommand + "\033[0m", None, 0);
			}
			lastOutput = outputString;
		}

		template<typename ...T>
		int getYN(T ...options) {
			string response;
			bool isComplex = false;
			int i = 0;
			if constexpr ((std::is_same_v<string, T> || ...)) { //check if we get a string in it or not
				isComplex = true;
				this->out<None, KeepEndlines>("\n\033[1m" + string(" ─> " + text.option.ChooseOptionBelow));
				(this->out<None>(("\033[1m " + std::to_string(++i) + ")\033[0m " + options)), ...);
				this->out<None, NoEndline>("\033[1m" + string(" ─> " + text.option.YourChoice));
			} else {
				this->out<None, NoEndline>("\033[1m " + text.question.Prompt + ' ');
			}
			std::getline(std::cin, response);
			this->out<None, NoEndline>("\033[0m");
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
						this->out<Error>("Answer not valid");
						this->out<None, NoEndline>("\033[1mYour choice: ");
						std::getline(std::cin, response);
						this->out<None, NoEndline>("\033[0m");
					}
				}
			}
		}

		inline static bool isWindows;
		void init(const string& file);
		void registerServerName(const string& name);
		void end();
		inline static string lastOutput;
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

static Output term;
