#include <string>
#include <vector>

using std::string;

#pragma once

#include "getvarsfromfile.h"

class Text {
	void applyLang(string lang);
	public:
		Text(string lang);
		std::vector<string> help;
		string errnoNotPermitted;
		string errnoNoFileOrDir;
		string errnoPermissionDenied;
		string errnoInOut;
		string errnoMemory;
		string errnoUnavailable;
		string errnoAddress;
		string errnoBlockDev;
		string errnoBusy;
		string errnoDirectory;
		string errnoBadArgs;
		string errnoUnknownDev;
		string errnoUnknownGeneric;
		string prefixInfo;
		string prefixError;
		string prefixWarning;
		string prefixDebug;
		string prefixQuestion;
};

void Text::applyLang(string lang) {
	if (lang[0] == 'e' && lang[1] == 'n') {
		help.push_back("Hajime is a high-performance startup script that can start a Minecraft server from an external device.");
		help.push_back("\033[1m\033[32mUsage:\033[0m ");
		help.push_back(" [the following flags]"); //1 and 2 sandwich a variable
		help.push_back("\033[1m-f \033[3mfile\033[0m or \033[1m--server-file \033[3mfile \033[0m\033[1m|\033[0m  Specify a server configuration file to use manually.");
		help.push_back("\033[1m-h \033[0mor\033[1m --help |\033[0m  Show this help message.");
		help.push_back("\033[1m-hf \033[0mor\033[1m --hajime-file\033[0m \033[1m\033[3mfile \033[0m\033[1m|\033[0m Manually specify the configuration file that Hajime uses.");
		help.push_back("\033[1m-s  \033[0mor\033[1m --install-server \033[1m|\033[0m  Create a default server configuration file.");
		help.push_back("\033[1m-i  \033[0mor\033[1m --install-hajime \033[1m\033[3mfile \033[0m\033[1m|\033[0m Install the default Hajime configuration file.");
		help.push_back("\033[1m-S  \033[0mor\033[1m --install-service \033[1m|\033[0m  Install a startup service file for your platform to start Hajime automatically.");
		help.push_back("\033[1m-ss \033[0mor\033[1m --install-servers-file \033[1m|\033[0m Install a server listing file.");
		help.push_back("\033[1m\033[32mNotes:\033[0m\nUse -f in conjunction with a custom config file. A plain filename is treated as being in the same directory Hajime is located in, so use a \033[1m/\033[0m to specify otherwise.");
		help.push_back("\033[1m\033[32mNeed more help?\033[0m Join our Discord group at https://discord.gg/J6asnc3pEG");
		errnoNotPermitted = "Not permitted. Is the device correct?";
		errnoNoFileOrDir = "No such file or directory.";
		errnoPermissionDenied = "Permission denied. Is Hajime being run under root?";
		errnoInOut = "Input/output error. Is the drive OK?";
		errnoMemory = "Not enough memory. Is there a shortage of it?";
		errnoUnavailable = "Resource unavailable.";
		errnoAddress = "Bad address.";
		errnoBlockDev = "Not a block device. Did you make sure you're mounting a mass storage device?";
		errnoBusy = "Busy. Is the device being accessed right now?";
		errnoDirectory = "It's a directory. Did you make sure you're mounting a mass stoage device?";
		errnoBadArgs = "Bad arguments. Is the configuration set correctly?";
		errnoUnknownDev = "Unknown device. Is the filesystem supported?";
		errnoUnknownGeneric = "Unknown error.";
		prefixInfo = "\033[46m[Info ";
		prefixError = "\033[41m\033[33m[Error ";
		prefixWarning = "\033[33m[Warning ";
		prefixDebug = "\033[105m[Debug ";
		#if defined(_WIN64) || defined (_WIN32)
		prefixQuestion = "\033[102m[Question "; //Windows doesn't support 24 bit terminal colors
		#else
		prefixQuestion = "\033[48;2;0;255;0m\033[38;2;108;104;161m[Question ";
		#endif
	}
}

Text::Text(string file) {
	if (!fs::is_regular_file(file)) {
		applyLang("en");
	}
	else {
		std::vector<string> settings = { "lang" };
		std::vector<string> results = getVarsFromFile(file, settings);
		if (results[0] != "") {
			applyLang(results[0]);
		}
		else {
			applyLang("en");
		}
	}
}
