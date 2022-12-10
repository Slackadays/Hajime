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
#include <WinNls.h>
#include <Windows.h>
#include <memory>
#endif

#include "../nlohmann/json.hpp"

namespace fs = std::filesystem;

#include "../hajime_main/constants.hpp"
#include "languages.hpp"
#include "flexi_format.hpp"
#include "output.hpp"

void Text::applyLang(const std::string& lang) {
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
	applyLang(filterLanguage(getUserLanguage()));
}

std::string Text::filterLanguage(const std::string& input) {
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

std::string Text::getUserLanguage() {
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
	std::string result(len, '\0');
	WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, locale.get(), ret - 1, result.data(), len, NULL, NULL);
	#else
	std::string result;
	if (getenv("LANGUAGE") != nullptr) {
		result = (std::string)getenv("LANGUAGE");
	} else if (getenv("LC_ALL") != nullptr) {
		result = (std::string)getenv("LC_ALL");
	} else if (getenv("LANG") != nullptr) {
		result = (std::string)getenv("LANG");
	}
	#endif
	term.out<outFlag::Debug>("result = " + result);
	return result;
}

Text::Text(std::string inputFile) {
	if (!fs::is_regular_file(inputFile)) {
		autoSetLanguage();
	}
	else {
		std::string lang;
		//read contents of file into a variable
		std::ifstream file(inputFile);
		std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		try {
			nlohmann::json content = nlohmann::json::parse(contents);
			lang = content["lang"];
		} catch (std::exception& e) {
			term.out<outFlag::Error, outFlag::Threadless>(flexi_format("Error parsing JSON for language: {}", e.what()));
		}
		if (lang != "") {
			applyLang(lang);
		} else {
			autoSetLanguage();
		}
	}
}

Text text(hajDefaultConfFile);
