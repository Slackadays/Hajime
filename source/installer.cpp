#include <fstream>
#include <filesystem>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>

#if defined(_win64) || defined (_WIN32)
#include <Windows.h>
#include <shlobj.h>
#pragma comment (lib, "Shell32")
#else
#include <unistd.h>
#endif

using std::cout;
using std::endl;

#include "output.hpp"
#include "languages.hpp"
#include "installer.hpp"
#include "constants.hpp"

namespace fs = std::filesystem;

void Installer::installNewServerConfigFile(const string& fileLocation, const bool& skipFileCheck, const string& flags, const string& serverFile) {
	if (fs::is_regular_file(fileLocation) && !skipFileCheck) {
		throw 0;
	} else {
		ofstream outConf(fileLocation);
		outConf << "name=" << std::regex_replace(fileLocation, std::regex("\\..*", std::regex_constants::optimize), "") << endl << "path=" << fs::current_path().string() << endl;
		outConf << "exec=java # The file that gets called in the \"new\" method." << endl << "flags=-jar -Xmx4G -Xms4G " + flags + " # This is where your Java flags go." << endl << "file=" + serverFile  + " # The server file you want to start."<< endl;
		outConf << "command= # Only use this if you using the \"old\" method."<< endl << "method=new" << endl << "device=" << endl << "restartmins= # The interval (in minutes) that you want your server to auto-restart with." << endl;
		outConf << "commands=1" << endl << "silentcommands=0" << endl;
		outConf << text.fileServerConfComment << endl;
		hjlog.out(text.info.CreatedServerConfig1 + fileLocation + text.info.CreatedServerConfig2, Info);
		outConf.close();
		if (!fs::is_regular_file(fileLocation)) {
			throw 1;
		}
	}
}

void Installer::installDefaultHajConfFile(string fileLocation = "(none)", bool skipFileCheck, const string& lang) {
	hjlog.out(text.info.InstallingDefHajConf + fileLocation + "...", Info);
	hjlog.out(text.info.CheckingExistingFile, Info);
	if (fs::is_regular_file(fileLocation) && !skipFileCheck) {
		throw 0;
	} else {
		ofstream outConf(fileLocation);
		outConf << "version=" << hajime_version << endl;
		outConf << "serversfile=servers.conf" << endl;
		outConf << "defserverconf=MyServer.server" << endl;
		outConf << "logfile=hajime.log" << endl;
		outConf << "lang=" << lang << endl;
		outConf << "debug=0" << endl;
		outConf << "threadcolors=1" << endl;
		outConf << "systemdlocation=/etc/systemd/system/hajime.service" << endl;
		outConf.close();
		hjlog.out(text.info.HajConfigMade1 + fileLocation + text.info.HajConfigMade2, Info);
		if (!fs::is_regular_file(fileLocation)) {
			throw 1;
		}
	}
}

void Installer::installStartupService(const string& sysService) {
	#if defined(_WIN64) || defined (_WIN32)
	hjlog.out(text.info.InstallingWStartServ, Info);
	string command = "schtasks.exe /create /sc ONLOGON /tn Hajime /tr " + fs::current_path().string() + "\\hajime.exe";
	cout << command << endl;
	int result = system(command.c_str());
	if (result == 0 || result == -1) {
		hjlog.out("system() error", Error);
	}
	if (!IsUserAnAdmin()) {
		hjlog.out(text.error.StartupServiceWindowsAdmin, Error);
		hjlog.out(text.info.TipAdministrator, Info);
	}
	#else
	if (fs::is_directory("/etc/init.d") && !fs::is_regular_file("/lib/systemd/systemd")) {
		hjlog.out(text.info.InstallingSysvinit, Info);
		bool continueInstall = true;
		if (fs::is_regular_file("/etc/init.d/hajime.sh")) {
			hjlog.out(text.warning.FoundSysvinitService, Warning);
			hjlog.out(text.question.MakeNewSysvinitService, Question, 0, 0);
			if (hjlog.getYN()) {
				continueInstall = true;
				hjlog.out(text.info.InstallingNewSysvinit, Info);
			} else {
				continueInstall = false;
			}
		}
		if (continueInstall) {
			ofstream service("/etc/init.d/hajime.sh");
			string user;
			hjlog.out(text.question.SysvinitUser, Question, 0, 0);
			std::cin >> user;
			string group;
			hjlog.out(text.question.SysvinitGroup, Question, 0, 0);
			std::cin >> group;
			service << "#!/bin/sh\n"
			"### BEGIN INIT INFO\n"
			"# Provides: 		hajime\n"
			"# Required-Start:\n"
			"# Required-Stop:\n"
			"# Default-Start:	3\n"
			"# Default-Stop:\n"
			"# Short-Description:	Starts Hajime\n"
			"# Description:		sysVinit startup service to start the Hajime startup script.\n"
			"### END INIT INFO\n"
			"NAME=\"hajime\"\n"
			"PATH=\"/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin\"\n"
			"APPDIR=" << fs::current_path() << "\n"
			"APPBIN=\"" << fs::current_path().string() << "/hajime\"\n"
			"APPARGS=\"\"\n"
			"USER=\"" << user << "\"\n"
			"GROUP=\"" << group << "\"\n"
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
			"exit 0" << endl;
			service.close();
			hjlog.out(text.info.InstalledSysvinit, Info);
		} else {
			hjlog.out(text.info.AbortedSysvinit, Info);
		}
	} else if (fs::is_directory("/Library/LaunchAgents")) { //macOS agent directory
		hjlog.out(text.info.InstallingLaunchdServ, Info);
		bool continueInstall = true;
		if (fs::is_regular_file("/Library/LaunchAgents/Hajime.plist")) {
			hjlog.out(text.warning.LaunchdServPresent, Warning);
			hjlog.out(text.question.MakeLaunchdServ, Question, 0, 0);
			if (hjlog.getYN()) {
					continueInstall = true;
					hjlog.out(text.info.InstallingNewLaunchdServ, Info);
			} else {
					continueInstall = false;
			}
		}
		if (continueInstall) {
			ofstream service("/Library/LaunchAgents/Hajime.plist");
			service << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<!DOCTYPE plist PUBLIC \"-\/\/Apple\/\/DTD PLIST 1.0\/\/EN\" \"http:\/\/www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
			"<plist version=\"1.0\">\n"
			"	<dict>\n"
			"		<key>Label</key>\n"
			"		<string>Hajime</string>\n"
			"		<key>Program</key>\n"
			"		<string>" << fs::current_path().string() << "/hajime</string>\n"
			"		<key>WorkingDirectory</key>\n"
			"		<string>" << fs::current_path().string() << "</string>\n"
			"		<key>RunAtLoad</key>\n"
			"		<true/>\n"
			"		<key>KeepAlive</key>\n"
			"		<true/>\n"
			"	</dict>\n"
			"</plist>" << endl;
			service.close();
			hjlog.out(text.info.InstalledLaunchServ, Info);
		} else {
			hjlog.out(text.info.AbortedLaunchServ, Info);
		}
	} else {
		if (fs::is_directory("/etc/systemd") && fs::is_regular_file(sysService)) {
			hjlog.out(text.warning.FoundSystemdService, Warning);
		}
		if (fs::is_directory("/etc/systemd") && !fs::is_regular_file(sysService)) {
			hjlog.out(text.info.MakingSystemdServ + sysService + "...", Info);
			if (getuid()) {
				hjlog.out(text.error.SystemdRoot, Error);
			}
			ofstream service(sysService);
			service << "[Unit]" << endl << "Description=Starts Hajime" << endl;
			service << endl << "[Service]\nType=simple\nWorkingDirectory=" << fs::current_path().string() << "\nExecStart=" << fs::current_path().string()  << "/hajime\n\n[Install]\nWantedBy=multi-user.target";
			service.close();
		}
		if (!fs::is_directory("/etc/systemd")) {
			hjlog.out(text.error.NoSystemd, Error);
		}
	}
	#endif
}

void Installer::installDefaultServersFile(string serversFile, bool skipFileCheck, std::vector<string> servers) {
	hjlog.out(text.info.InstallingServersFile + serversFile + "...", Info);
	hjlog.out(text.info.CheckingExistingServersFile, Info);
	if (fs::is_regular_file(serversFile) && !skipFileCheck) {
		throw 0;
	} else {
		ofstream outConf(serversFile);
		for (const auto& it : servers) {
			outConf << it << endl;
		}
		outConf.close();
		hjlog.out(text.info.MadeServersFile, Info);
		if (!fs::is_regular_file(serversFile)) {
			throw 1;
		}
	}
}

Installer installer;
