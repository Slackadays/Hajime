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

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <regex>
#if defined(_WIN32) || defined(_WIN64)
#include <atlstr.h>
#include <WinNls.h>
#include <Windows.h>
#include <memory>
#endif

namespace fs = std::filesystem;

#include "constants.hpp"
#include "getvarsfromfile.hpp"
#include "languages.hpp"
#include "output.hpp"

void Text::applyLang(const string& lang) {
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
	applyLang(filterLanguage(getUserLanguage()));
}

string Text::filterLanguage(const string& input) {
	if (std::regex_search(input, std::regex("en[_-].*", std::regex_constants::optimize | std::regex_constants::icase))) {
		return "en";
	} else if (std::regex_search(input, std::regex("es[_-].*", std::regex_constants::optimize | std::regex_constants::icase))) {
		return "es";
	} else if (std::regex_search(input, std::regex("pt[_-].*", std::regex_constants::optimize | std::regex_constants::icase))) {
		return "pt";
	} else {
		return "en";
	}
}

string Text::getUserLanguage() {
	#if defined(_WIN32) || defined(_WIN64)
	std::unique_ptr<wchar_t[]> locale(new wchar_t[LOCALE_NAME_MAX_LENGTH]);
	int ret = GetUserDefaultLocaleName(locale.get(), LOCALE_NAME_MAX_LENGTH);
	if (!ret && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		term.out<outFlag::Error>("Too small buffer for locale");
	} else {
		term.out<outFlag::Debug>("ret = " + std::to_string(ret));
	}
	int len = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, locale.get(), ret - 1, nullptr, 0, NULL, NULL);
	if (!len) {
		term.out<outFlag::Error>("Error in WideCharToMultiByte");
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
	term.out<outFlag::Debug>("result = " + result);
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

Text text(hajDefaultConfFile);
