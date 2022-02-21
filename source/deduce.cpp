#include <iostream>
#include <string>
#include <filesystem>
#include <regex>
#include <stdio.h>

#include "constants.hpp"
#include "deduce.hpp"
#include "output.hpp"

using std::string;
namespace fs = std::filesystem;

string Deduce::hajimeFile() {
	if (!fs::is_regular_file(hajDefaultConfFile)) {
		return hajDefaultConfFile;
	} else {
		hjlog.out<Error>("Could not install Hajime config file");
		return "";
	}
}

string Deduce::serverFile() {
	if (!fs::is_regular_file(defaultServerConfFile)) {
		return defaultServerConfFile;
	} else {
		if (!fs::is_regular_file(defaultServerConfFile + ".old")) {
			rename(defaultServerConfFile.c_str(), (defaultServerConfFile + ".old").c_str());
			return defaultServerConfFile;
		}
		for (int i = 1; i < 10; i++) {
			string temp = std::regex_replace(defaultServerConfFile, std::regex(".+(?=\\.\\w+)", std::regex_constants::optimize), "$&" + std::to_string(i));
			if (!fs::is_regular_file(temp)) {
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
