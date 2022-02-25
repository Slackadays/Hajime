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

#include <iostream>
#include <string>
#include <filesystem>
#include <regex>
#include <stdio.h>
#include <vector>

#include "constants.hpp"
#include "deduce.hpp"
#include "output.hpp"

using std::string;
using std::vector;

namespace fs = std::filesystem;

string Deduce::hajimeFile() {
	if (!fs::is_regular_file(hajDefaultConfFile)) {
		return hajDefaultConfFile;
	} else {
		term.out<Error>("Could not install Hajime config file");
		return "";
	}
}

vector<string> Deduce::serverFiles(const fs::path& p) {
	vector<string> results;
	auto path = fs::directory_iterator{p};
	for (auto file = begin(path); file != end(path); ++file) {
		if (std::regex_match(file->path().filename().string(), std::regex(".+\\.jar(?!.+)", std::regex_constants::optimize | std::regex_constants::icase))) {
			if (file->is_regular_file() || file->is_symlink()) {
				results.emplace_back(file->path().filename().string());
			}
		}
	}
	return results;
}

string Deduce::serverConfig() {
	if (!fs::is_regular_file(defaultServerConfFile)) {
		return defaultServerConfFile;
	} else {
		for (int i = 1; i < 10; i++) {
			string temp = std::regex_replace(defaultServerConfFile, std::regex(".+(?=\\.\\w+)", std::regex_constants::optimize), "$&" + std::to_string(i));
			if (!fs::is_regular_file(temp)) {
				return temp;
			}
		}
		if (!fs::is_regular_file(defaultServerConfFile + ".old")) {
			rename(defaultServerConfFile.c_str(), (defaultServerConfFile + ".old").c_str());
			return defaultServerConfFile;
		}
		for (int i = 1; i < 10; i++) {
			string temp = std::regex_replace(defaultServerConfFile, std::regex(".+(?=\\.\\w+)", std::regex_constants::optimize), "$&" + std::to_string(i));
			if (!fs::is_regular_file(temp + ".old")) {
				rename(defaultServerConfFile.c_str(), (temp + ".old").c_str());
				return temp;
			}
		}
		return "bleh";
	}
}

string Deduce::usagePattern() {
	return std::string("bar");
}

Deduce deduce;
