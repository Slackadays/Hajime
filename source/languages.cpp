#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#if defined(_WIN32) || defined(_WIN64)
#include <atlstr.h>
#include <WinNls.h>
#include <Windows.h>
#endif

namespace fs = std::filesystem;
using std::string;

#include "getvarsfromfile.hpp"
#include "languages.hpp"
#include "output.hpp"

void Text::applyLang(string lang) {
	//the fallack is English
	#include "en.hpp"
	language = lang;
	if (lang == "en") {
		help.clear();
		#include "en.hpp"
	} else if (lang == "es") { //spanish
		help.clear(); //reset the help vector
		#include "es.hpp"
	} else if (lang == "pt") {
		help.clear();
		#include "pt.hpp"
	}
}

void Text::autoSetLanguage() {
	string lang = getUserLanguage();
	if (std::regex_search(lang, std::regex("en[_-].*", std::regex_constants::optimize | std::regex_constants::icase))) {
		applyLang("en");
	} else if (std::regex_search(lang, std::regex("es[_-].*", std::regex_constants::optimize | std::regex_constants::icase))) {
		applyLang("es");
	} else if (std::regex_search(lang, std::regex("pt[_-].*", std::regex_constants::optimize | std::regex_constants::icase))) {
		applyLang("pt");
	} else {
		applyLang("en");
	}
}

string Text::getUserLanguage() {
	string result;
	#if defined(_WIN32) || defined(_WIN64)
	LPWSTR* locale = (LPWSTR*)malloc(100); //we could use LOCALE_NAME_MAX_LENGTH here, but it turns out it doesn't work anymore because it's too small.
	int ret = GetUserDefaultLocaleName(*locale, 100);
	if (!ret && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		hjlog->out("Too small buffer for locale", outType::Error);
	} else {
		//std::cout << ret << std::endl;
	}
	result = CW2A(*locale); //convert the utf-16 locale to a utf-8 result
	free(locale); //prevent a memory leak
	#else
	if (getenv("LANGUAGE") != nullptr) {
		result = (string)getenv("LANGUAGE");
	} else if (getenv("LC_ALL") != nullptr) {
		result = (string)getenv("LC_ALL");
	} else if (getenv("LANG") != nullptr) {
		result = (string)getenv("LANG");
	}
	#endif
	//std::cout << result << std::endl;
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
