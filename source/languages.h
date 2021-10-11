#include <string>
#include <vector>

#include "getvarsfromfile.h"

#pragma once

using std::string;

class Text {
	string lang;
	public:
		Text(string lang);
		vector<string> help;
};

Text::Text(string lang) {
	if (lang[0] == 'e' && lang[1] == 'n') {
		help.push_back("Hajime is a high-performance startup script that can start a Minecraft server from an external device.");
		help.push_back("\033[1m\033[32mUsage:\033[1;0m ");
		help.push_back(" [the following flags]"); //1 and 2 sandwich a variable
		help.push_back("\033[1m-f \033[3mfile\033[0m or \033[1m--server-file \033[3mfile \033[0m\033[1;1m|\033[1;0m  Specify a server configuration file to use manually.");
		help.push_back("\033[1m-h \033[0mor\033[1m --help |\033[1;0m  Show this help message.");
		help.push_back("\033[1m--hajime-file\033[0m \033[1m\033[3mfile \033[0m \033[1m|\033[0m Manually specify the configuration file that Hajime uses.");
		help.push_back("\033[1m-s  \033[0mor\033[1m --install-server \033[1m|\033[0m  Create a default server configuration file.");
		help.push_back("\033[1m-S  \033[0mor\033[1m --systemd \033[1;1m|\033[1;0m  Install a systemd service file to start Hajime automatically.");
		help.push_back("\033[1m-ss \033[0mor\033[1m --install-servers-file \033[1;1m|\033[0m Install a server listing file.");
		help.push_back("\033[1;1m\033[1;32mNotes:\033[1;0m\nUse -f in conjunction with a custom config file. A plain filename is treated as being in the same directory Hajime is located in, so use a \033[1m/\033[0m to specify otherwise.");
		help.push_back("\033[1;1m\033[1;32mNeed more help?\033[1;0m Join our Discord group at https:/\/discord.gg/J6asnc3pEG");
	}
}
