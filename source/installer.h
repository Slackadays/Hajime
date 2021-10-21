#include <fstream>
#include <filesystem>
#include <cstring>
#include <string>

#if defined(_win64) || defined (_WIN32)
#else
#include <unistd.h>
#endif

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
	outConf << "file=server.jar" << endl << "path=PATH" << endl << "command=COMMAND" << endl << "flags=FLAGS" << endl << "debug=1" << endl << "device=" << endl;
	outConf << "# Anything after a # is a comment." << endl;
	cout << "The config file (" << fileLocation << ") has been created and is now ready for your settings." << endl;
	outConf.close();
}

#if defined(_win64) || defined (_WIN32)
void Installer::installSystemdService(string sysdService) {
	logObj.out("This feature only works on Linux", "error");
}
#else
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
#endif

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
