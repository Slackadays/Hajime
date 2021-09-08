#include <fstream>
#include <filesystem>
#include <cstring>
#include <string>

#include "output.h"

class Installer {
	Output logObj;
	public:
		void mainconfig(string conf);
		void systemd(string sysdService);
		void makeSconfig(string sconfFile);
};

void Installer::mainconfig(string conf) {
	logObj.out("Installing config file...", "info");
	if (fs::is_regular_file(conf)){
		logObj.out("The file is already here! To make a new one, delete the existing file.", "warning");
	} else {
		ofstream outConf(conf);
		outConf << "file=SERVER-FILE"<< endl 
		<< "path=PATH-TO-DEVICE" << endl 
		<< "command=SERVER-EXECUTION-COMMAND" << endl 
		<< "debug=1" << endl 
		<< "device=DEVICE" << endl;
		outConf << "#" << endl 
		<< "This is the comment section. Anything after the # is a comment. \n The first line is the file of the server that needs to be executed. The second line is the path that leads to the home directory of the file nd can't end in a /. The third line is the command that needs to be executed in order to start the server. The fourth line is the debug setting.\n 0 means most output is disabled.\n 1 prevents most looped outputs.\n 2 enables all outputs.\n I recommend 1, but switch to 2 if there\'s a problem somewhere." << endl;
		cout << "The config file (" << conf << ") has been created and is now ready for your settings." << endl;
		outConf.close();
	}
}

void Installer::makeSconfig(string sconfFile) {
	if (fs::is_regular_file(sconfFile)) {
		logObj.out("There is a config file here!", "warning");
	} else {
		ofstream outsConf(sconfFile);
		outsConf << "defaultserverconf=server.conf" << endl 
		<< "systemdlocation=/etc/systemd/system/hajime.service"<< endl 
		<< "logfile=" << endl 
		<< "#" << endl;
		logObj.out("Config file made!", "info");
		outsConf.close();
	}
}

void Installer::systemd(string sysdService) {
	if (fs::is_directory("/etc/systemd") && fs::is_regular_file(sysdService)) {
		logObj.out("The systemd service is already here!", "warning");
	}
	if (fs::is_directory("/etc/systemd") && !fs::is_regular_file(sysdService)) {
		logObj.out("Making systemd service...", "info");
		ofstream service(sysdService);
		service << "[Unit]" << endl << "Description=Starts Hajime" << endl << endl << "[Service]\nType=simple\nWorkingDirectory=" << fs::current_path().string() << "\nExecStart=" << fs::current_path().string()  << "/hajime\n\n[Install]\nWantedBy=multi-user.target";
		service.close();
	}
	if (!fs::is_directory("/etc/systemd")) {
		logObj.out("Looks like there is no systemd; use another installation option instead.", "error");
	}
}
