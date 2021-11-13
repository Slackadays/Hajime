#include <fstream>
#include <filesystem>
#include <cstring>
#include <string>
#include <iostream>

#if defined(_win64) || defined (_WIN32)
#include <Windows.h>
#include <shlobj_core.h>
#else
#include <unistd.h>
#endif

using std::cout;
using std::endl;

#include "output.h"
#include "languages.h"
#include "installer.h"

namespace fs = std::filesystem;

Installer::Installer(std::shared_ptr<Output> log) {
	logObj = log;
}

void Installer::installDefaultServerConfFile(string conf) {
	logObj->out("Installing default server config file at " + conf + "...", Info);
	if (fs::is_regular_file(conf)){
		logObj->out("The file is already here! To make a new one, delete the existing file.", Warning);
		logObj->out(text.questionMakeServerConfig, Question, 0, 0); // don't keep endlines, don't add endline
		if (logObj->getYN()) {
			logObj->out("Installing a new server config file with name " + conf + "...", Info);
			Installer::installNewServerConfigFile(conf);
		}
	} else {
		Installer::installNewServerConfigFile(conf);
	}
}

void Installer::installDefaultHajConfFile(string fileLocation = "(none)") {
	logObj->out("Installing default Hajime config file " + fileLocation + "...", Info);
	logObj->out("Checking for existing file...", Info);
	if (fs::is_regular_file(fileLocation)) {
		logObj->out("Hajime config file already present!", Warning);
	} else {
		ofstream outConf(fileLocation);
		outConf << "serversfile=servers.conf" << endl;
		outConf << "defserverconf=server0.conf" << endl;
		outConf << "logfile=" << endl;
		outConf << "systemdlocation=/etc/systemd/system/hajime.service" << endl;
		outConf << "optflags=-XX:+UseG1GC -XX:+ParallelRefProcEnabled -XX:MaxGCPauseMillis=200 -XX:+UnlockExperimentalVMOptions -XX:+DisableExplicitGC -XX:+AlwaysPreTouch -XX:G1NewSizePercent=30 -XX:G1MaxNewSizePercent=40 -XX:G1HeapRegionSize=8M -XX:G1ReservePercent=20 -XX:G1HeapWastePercent=5 -XX:G1MixedGCCountTarget=4 -XX:InitiatingHeapOccupancyPercent=15 -XX:G1MixedGCLiveThresholdPercent=90 -XX:G1RSetUpdatingPauseTimePercent=5 -XX:SurvivorRatio=32 -XX:+PerfDisableSharedMem  XX:MaxTenuringThreshold=1 -Daikars.new.flags=true -Dusing.aikars.flags=https://mcflags.emc.gs" << endl;
		outConf.close();
		logObj->out("Hajime config file (" + fileLocation + ") made!", Info);
	}
}

void Installer::installNewServerConfigFile(string fileLocation) {
	ofstream outConf(fileLocation);
	outConf << "file=server.jar" << endl << "path=PATH" << endl << "command=COMMAND" << endl << "flags=FLAGS" << endl << "debug=1" << endl << "device=" << endl;
	outConf << "# Anything after a # is a comment." << endl;
	cout << "The config file (" << fileLocation << ") has been created" << endl;
	outConf.close();
}

void Installer::installStartupService(string sysService) {
	#if defined(_WIN64) || defined (_WIN32)
	logObj->out("Installing Windows startup service", Info);
	string command = "schtasks.exe /create /sc ONLOGON /tn Hajime /tr " + fs::current_path().string() + "\\Hajime.exe";
	cout << command << endl;
	int result = system(command.c_str());
	if (!IsUserAnAdmin()) {
		logObj->out(text.errorStartupServiceWindowsAdmin, Error);
		logObj->out("Tip: Right click the terminal icon and then click \"Run as administrator\"", Info);
	}
	#else
	if (fs::is_directory("/etc/init.d") && !fs::is_regular_file("/lib/systemd/systemd")) {
		logObj->out("Installing sysVinit service", Info);
		ofstream service("/etc/init.d/hajime.sh");
		service << "#!/bin/sh\n"
		"### BEGIN INIT INFO\n"
		"# Provides: 		hajime\n"
		"# Required-Start:\n"
		"# Required-Stop:\n"
		"# Default-Start:	3\n"
		"# Default-Stop:\n"
		"# Short-Description:	Starts Hajime\n"
		"# Description:		SysVinit service to start the Hajime startup script.\n"
		"### END INIT INFO\n"
		"NAME=\"hajime\"\n"
		"PATH=\"/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin\"\n"
		"APPDIR=\"/root/Downloads/hajime/source\"\n"
		"APPBIN=\"/root/Downloads/hajime/source/hajime\"\n"
		"APPARGS=\"-h\"\n"
		"USER=\"vm\"\n"
		"GROUP=\"vm\"\n"
		"set -e\n"
		". /lib/lsb/init-functions\n"
		"start() {\n"
		"	printf \"Starting Hajime... \"\n"
		"	start-stop-daemon --start --chuid \"$USER:$GROUP\" --background --make-pidfile --pidfile /var/run/$NAME.pid --chdir \"$APPDIR\" --exec \"$APPBIN\" -- $APPARGS || true\n"
		"	printf \"done!\n\"\n"
		"}\n"
		"stop() {\n"
		"	printf \"Stopping Hajime!\n\"\n"
		"	[ -z `cat /var/run/$NAME.pid 2>/dev/null` ] || \\\n"
		"	kill -9 $(cat /var/run/$NAME.pid)\n"
		"	[ -z `cat /var/run/$NAME.pid 2>/dev/null` ] || rm /var/run/$NAME.pid\n"
		"	printf \"Stopped Hajime!\n\"\n"
		"}\n"
		"status() {\n"
		"	status_of_proc -p /var/run/$NAME.pid \"\" $NAME && exit 0 || exit $?\n"
		"}\n"
		"case \"$1\" in\n"
		"	start)\n"
		"		start\n"
		"		;;\n"
		"	stop)\n"
		"		stop\n"
		"		;;\n"
		"		stop\n"
		"	restart)\n"
		"		start\n"
		"		;;\n"
		"	status)\n"
		"		status\n"
		"		;;\n"
		"	*)\n"
		"		echo \"Usage: $NAME (start|stop|restart|status)\" >&2\n"
		"		exit 1\n"
		"esac\n"
		"exit 0\n" << endl;
	service.close();
	} else {
		if (getuid()) {
			logObj->out(text.errorSystemdRoot, Error);
		}
		if (fs::is_directory("/etc/systemd") && fs::is_regular_file(sysService)) {
			logObj->out("Found an existing systemd service", Warning);
		}
		if (fs::is_directory("/etc/systemd") && !fs::is_regular_file(sysService)) {
			logObj->out("Making systemd service at " + sysService + "...", Info);
			ofstream service(sysService);
			service << "[Unit]" << endl << "Description=Starts Hajime" << endl;
			service << endl << "[Service]\nType=simple\nWorkingDirectory=" << fs::current_path().string() << "\nExecStart=" << fs::current_path().string()  << "/hajime\n\n[Install]\nWantedBy=multi-user.target";
			service.close();
		}
		if (!fs::is_directory("/etc/systemd")) {
			logObj->out(text.errorNoSystemd, Error);
		}
	}
		#endif
}


void Installer::installDefaultServersFile(string serversFile) {
	logObj->out("Installing default servers file at " + serversFile + "...", Info);
	logObj->out("Checking for existing file...", Info);
	if (fs::is_regular_file(serversFile)) {
		logObj->out(text.errorServersFilePresent, Warning);
	} else {
		ofstream outConf(serversFile);
		outConf << "server0.conf" << endl;
		outConf.close();
		logObj->out("Servers file made!", Info);
	}
}
