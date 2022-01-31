#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#if defined(_WIN32) || defined(_WIN64)
#include <atlstr.h>
#include <WinNls.h>
#include <Windows.h>
#include <memory>
#endif

namespace fs = std::filesystem;
using std::string;

#include "getvarsfromfile.hpp"
#include "languages.hpp"
#include "output.hpp"
#include "constants.hpp"

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
	#if defined(_WIN32) || defined(_WIN64)
	std::unique_ptr<wchar_t[]> locale(new wchar_t[LOCALE_NAME_MAX_LENGTH]);
	int ret = GetUserDefaultLocaleName(locale.get(), LOCALE_NAME_MAX_LENGTH);
	if (!ret && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		hjlog->out("Too small buffer for locale", outType::Error);
	} else {
		hjlog->out("ret = " + std::to_string(ret), outType::Debug);
	}
	int len = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, locale.get(), ret - 1, nullptr, 0, NULL, NULL);
	if (!len) {
		hjlog->out("Error in WideCharToMultiByte", outType::Error);
	}
	string result(len, '\0');
	WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, locale.get(), ret - 1, result.data(), len, NULL, NULL);
	#else
	string result;
	if (getenv("LANGUAGE") != nullptr) {
		result = (string)getenv("LANGUAGE");
	} else if (getenv("LC_ALL") != nullptr) {
		result = (string)getenv("LC_ALL");
	} else if (getenv("LANG") != nullptr) {
		result = (string)getenv("LANG");
	}
	#endif
	hjlog->out("result = " + result, outType::Debug);
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
