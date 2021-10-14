#include <fstream>
#include <filesystem>
#include <cstring>
#include <string>
#include <unistd.h>

#include "output.h"

class Installer {
	Output logObj;
	void installNewServerConfigFile(string fileLocation);
	public:
		void installSystemdService(string sysdService);
		void installDefaultHajConfFile(string fileLocation);
		void installDefaultServerConfFile(string conf);
		void installDefaultServersFile(string serversFile);
};

void Installer::installDefaultServerConfFile(string conf) {
	logObj.out("Installing default server config file...", "info");
	if (fs::is_regular_file(conf)){
		logObj.out("The file is already here! To make a new one, delete the existing file.", "warning");
		logObj.out("Would you like to create a new configuration file anyway?", "info", 0, 0); // don't keep endlines, don't add endline
		if (getYN()) {
			logObj.out("Installing a new server config file...", "info"); 
			Installer::installNewServerConfigFile(conf);
		}
	} else {
		Installer::installNewServerConfigFile(conf);
	}
}

void Installer::installDefaultHajConfFile(string fileLocation = "(none)") {
	logObj.out("Installing default Hajime config file " + fileLocation + "...", "info");
	logObj.out("Checking for existing file...", "info");
	if (fs::is_regular_file(fileLocation)) {
		logObj.out("Hajime config file already present!", "warning");
	} else {
		ofstream outConf(fileLocation);
		outConf << "serversfile=servers.conf" << endl;	
		outConf << "logfile=" << endl;
		outConf << "systemdlocation=/etc/systemd/system/hajime.service" << endl;
		outConf.close();
		logObj.out("Hajime config file made!", "info");
	}
}

void Installer::installNewServerConfigFile(string fileLocation) {
	ofstream outConf(fileLocation);
	outConf << "file=SERVER-FILE" << endl << "path=PATH-TO-DEVICE" << endl << "command=SERVER-EXECUTION-COMMAND" << endl << "debug=1" << endl << "device=DEVICE" << endl;
	outConf << "#" << endl << "This is the comment section. Anything after the # is a comment. \nThe first line is the file of the server that needs to be executed. The second line is the path to the device." << endl; 
	cout << "The config file (" << fileLocation << ") has been created and is now ready for your settings." << endl;
	outConf.close();
}

void Installer::installSystemdService(string sysdService) {
	if (getuid()) {logObj.out("You need to be the root user to install a systemd service", "error");}
	if (fs::is_directory("/etc/systemd") && fs::is_regular_file(sysdService)) {
		logObj.out("Found an existing systemd service", "warning");
	}
	if (fs::is_directory("/etc/systemd") && !fs::is_regular_file(sysdService)) {
		logObj.out("Making systemd service...", "info");
		ofstream service(sysdService);
		service << "[Unit]" << endl << "Description=Starts Hajime" << endl;
		service << endl << "[Service]\nType=simple\nWorkingDirectory=" << fs::current_path().string() << "\nExecStart=" << fs::current_path().string()  << "/hajime\n\n[Install]\nWantedBy=multi-user.target";
		service.close();
	}
	if (!fs::is_directory("/etc/systemd")) {
		logObj.out("Looks like there is no systemd; use another installation option instead.", "error");
	}
}

void Installer::installDefaultServersFile(string serversFile) {
	logObj.out("Installing default servers file...", "info");
	logObj.out("Checking for existing file...", "info");
	if (fs::is_regular_file(serversFile)) {
		logObj.out("Servers file already present!", "warning");
	} else {
		ofstream outConf(serversFile);
		outConf << "server0.conf" << endl;
		outConf.close();
		logObj.out("Servers file made!", "info");
	}
}
