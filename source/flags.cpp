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
    
#include <vector>
#include <string>

#include "flags.hpp"
#include "server/server.hpp"
#include "wizard.hpp"
#include "installer.hpp"

void doPreemptiveFlags(std::vector<std::string> flags) {
	if (getenv("NO_COLOR") != NULL) {
		term.noColors = true;
	}
	for (int i = 1; i < flags.size(); i++) { //search for the help flag first
		auto flag = [&flags, &i](auto ...fs){
			return ((fs == flags.at(i)) || ...);
		}; //compare flags with a parameter pack pattern
		auto assignNextToVar = [&flags, &i](auto &var){
			if (i == (flags.size() - 1)) {
				return false;
			} else {
				var = flags.at(i + 1);
				i++;
				return true;
			}
		};
		if (flag("-h", "--help")) { //-h = --help = help
			for (auto it : text.help) {
				it = std::regex_replace(it, std::regex("^-(?=\\w+)", std::regex_constants::optimize), "\033[1m$&");
				it = std::regex_replace(it, std::regex(" (?=\\w+ \\w+ --)", std::regex_constants::optimize), "$&\033[3m");
				it = std::regex_replace(it, std::regex(" (?=\\w+ --)", std::regex_constants::optimize), "$&\033[0m");
				it = std::regex_replace(it, std::regex("--(?=\\w+)", std::regex_constants::optimize), "$&\033[1m");
				it = std::regex_replace(it, std::regex(" (?=\\w+ \\|)", std::regex_constants::optimize), "$&\033[3m");
				it = std::regex_replace(it, std::regex("\\|", std::regex_constants::optimize), "\033[0m\033[1m$&\033[0m");
				term.out<Border>(it);
			}
			exit(0); //if someone is asking for help, ignore any other flags and just display the help screen
		}
		if (flag("-l", "--language")) {
			if ((i < (flags.size() - 1)) && flags.at(i + 1).front() != '-') {
				text.applyLang(flags.at(i + 1));
			} else {
				term.out<Error>(text.error.NotEnoughArgs);
				exit(0);
			}
		}
	}
}

void doRegularFlags(std::vector<std::string> flags) {
	for (int i = 1; i < flags.size(); i++) {//start at i = 1 to improve performance because we will never find a flag at 0
		auto flag = [&flags, &i](auto ...fs){
			return ((fs == flags.at(i)) || ...);
		};
		auto assignNextToVar = [&flags, &i](auto &var){
			if (i == (flags.size() - 1)) {
				return false;
			} else {
				var = flags.at(i + 1);
				i++;
				return true;
			}
		}; //tries to assign the next argv argument to some variable; if it is not valid, then return an error
		if (flag("-f", "--server-file")) {
			if (!assignNextToVar(defaultServerConfFile)) {
				term.out<Error>(text.error.NotEnoughArgs);
				exit(0);
			}
		}
		if (flag("-hf", "--hajime-file")) {
			if (!assignNextToVar(hajDefaultConfFile)) {
				term.out<Error>(text.error.NotEnoughArgs);
				exit(0);
			}
		}
		if (flag("-ih", "--install-hajime-config")) { //can accept either no added file or an added file
			std::string tempHajConfFile;
			if (std::string var = "-"; assignNextToVar(var) && var.at(0) != '-') { //compare the next flag if present and check if it is a filename
				tempHajConfFile = var;
			} else {
				tempHajConfFile = hajDefaultConfFile;
			}
			wizard.wizardStep(tempHajConfFile, installer.installDefaultHajConfFile, text.warning.FoundHajConf, text.error.HajFileNotMade, text.language);
			exit(0);
		}
		if (flag("-p", "--privileged")) {
			bypassPriviligeCheck = true;
		}
		if (flag("-s", "--install-default-server")) {
			ServerConfigFile tempConfig;
			tempConfig.fileLocation = defaultServerConfFile;
			tempConfig.skipFileCheck = false;
			tempConfig.flags = "";
			tempConfig.serverFile = "server.jar";
			installer.installNewServerConfigFile(tempConfig);
			exit(0);
		}
		if (flag("-S", "--install-service")) {
			installer.installStartupService("/etc/systemd/system/hajime.service");
			exit(0);
		}
		if (flag("-t", "--tui")) {
			useTUI = true;
		}
		if (flag("-v", "--verbose")) {
			term.verbose = true;
		}
		if (flag("-m", "--monochrome", "--no-colors")) {
			term.noColors = true;
		}
		if (flag("-d", "--debug")) {
			term.debug = true;
		}
		if (flag("-ee")) {
			ee = true;
		}
		if (flag("-i", "--install-hajime")) {
			wizard.initialHajimeSetupAttended(hajDefaultConfFile, defaultServerConfFile);
			exit(0);
		}
		if (flag("-np", "--no-pauses")) {
			wizard.doArtificialPauses = false;
		}
		if (flag("-nt", "--no-tui")) {
			useTUI = false;
		}
		if (flag("-tc", "--thread-colors")) {
			term.showThreadsAsColors = true;
		}
		if (flag("-ntc", "--no-thread-colors")) {
			term.showThreadsAsColors = false;
		}
		if (flag("-it", "--show-info-type")) {
			term.showExplicitInfoType = true;
		}
	}
}