#include <string>
#include <vector>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;
using std::string;

#include "getvarsfromfile.hpp"
#include "languages.hpp"
#include "output.hpp"

void Text::applyLang(string lang) {
	//the fallack is English
	#include "en.hpp"
	language = lang;
	if (lang == "es") { //spanish
		help.clear(); //reset the help vector
		#include "es.hpp"
	} else if (lang == "pt") {
		help.clear();
		#include "pt.hpp"
	}
}

void Text::autoSetLanguage() {
	string lang = getUserLanguage();
	if (std::regex_search(lang, std::regex("en_.*", std::regex_constants::optimize | std::regex_constants::icase))) {
		applyLang("en");
	}
	if (std::regex_search(lang, std::regex("es_.*", std::regex_constants::optimize | std::regex_constants::icase))) {
		applyLang("es");
	}
	if (std::regex_search(lang, std::regex("pt_.*", std::regex_constants::optimize | std::regex_constants::icase))) {
		applyLang("pt");
	}
}

string Text::getUserLanguage() {
	string result;
	if (getenv("LANGUAGE") != nullptr) {
		result = (string)getenv("LANGUAGE");
	} else if (getenv("LC_ALL") != nullptr) {
		result = (string)getenv("LC_ALL");
	} else if (getenv("LANG") != nullptr) {
		result = (string)getenv("LANG");
	}
	return result;
}

Text::Text(string file) {
	if (!fs::is_regular_file(file)) {
		autoSetLanguage();
	}
	else {
		std::vector<string> settings = {"lang"};
		std::vector<string> results = getVarsFromFile(file, settings);
		if (results[0] != "") {
			applyLang(results[0]);
		} else {
			autoSetLanguage();
		}
	}
}

string hajDefaultConfFile = "hajime.conf";
Text text(hajDefaultConfFile);
