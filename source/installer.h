#include <fstream>
#include <filesystem>
#include <cstring>
#include <string>

#include "output.h"

class Installer {
	Output logObj;
	void installNewServerConfigFile(string fileLocation);
	public:
		void mainconfig(string conf);
		void systemd(string sysdService);
		void makeSconfig(string sconfFile);
};

void Installer::mainconfig(string conf) {
	logObj.out("Installing config file...", "info");
	if (fs::is_regular_file(conf)){
		logObj.out("The file is already here! To make a new one, delete the existing file.", "warning");
		logObj.out("Would you like to create a new configuration file anyway? [y/n] ", "info", 0, 0); // don't keep endlines, don't add endline
		if (getYN()) {logObj.out("Installing a new file...", "info"); Installer::installNewServerConfigFile(conf);}
	} else {
		Installer::installNewServerConfigFile(conf);
	}
}

void Installer::installNewServerConfigFile(string fileLocation) {
	ofstream outConf(fileLocation);
        outConf << "file=SERVER-FILE" << endl << "path=PATH-TO-DEVICE" << endl << "command=SERVER-EXECUTION-COMMAND" << endl << "debug=1" << endl << "device=DEVICE" << endl;
        outConf << "#" << endl << "This is the comment section. Anything after the # is a comment. \nThe first line is the file of the server that needs to be executed. The second line is the path to the device." << endl; 
	cout << "The config file (" << fileLocation << ") has been created and is now ready for your settings." << endl;
        outConf.close();
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
