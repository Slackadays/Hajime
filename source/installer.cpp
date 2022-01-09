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
namespace fs = std::filesystem;

Installer::Installer(std::shared_ptr<Output> log) {
	hjlog = log;
}

void Installer::installNewServerConfigFile(string fileLocation, bool skipFileCheck, string flags, string serverFile) {
	if (fs::is_regular_file(fileLocation) && !skipFileCheck) {
		throw 0;
	} else {
		ofstream outConf(fileLocation);
		outConf << "name=" << std::regex_replace(fileLocation, std::regex("\\..*", std::regex_constants::optimize), "") << endl << "path=" << fs::current_path().string() << endl << "exec=java" << endl << "flags=-jar -Xmx4G -Xms4G " + flags << endl << "file=" + serverFile << endl << "command=" << endl << "method=new" << endl << "device=" << endl;
		outConf << text.fileServerConfComment << endl;
		hjlog->out(text.infoCreatedServerConfig1 + fileLocation + text.infoCreatedServerConfig2, Info);
		outConf.close();
		if (!fs::is_regular_file(fileLocation)) {
			throw 1;
		}
	}
}

void Installer::installDefaultHajConfFile(string fileLocation = "(none)", bool skipFileCheck) {
	hjlog->out(text.infoInstallingDefHajConf + fileLocation + "...", Info);
	hjlog->out(text.infoCheckingExistingFile, Info);
	if (fs::is_regular_file(fileLocation) && !skipFileCheck) {
		throw 0;
	} else {
		ofstream outConf(fileLocation);
		outConf << "serversfile=servers.conf" << endl;
		outConf << "defserverconf=MyServer.conf" << endl;
		outConf << "logfile=" << endl;
		outConf << "lang=en" << endl;
		outConf << "debug=0" << endl;
		outConf << "systemdlocation=/etc/systemd/system/hajime.service" << endl;
		outConf.close();
		hjlog->out(text.infoHajConfigMade1 + fileLocation + text.infoHajConfigMade2, Info);
		if (!fs::is_regular_file(fileLocation)) {
			throw 1;
		}
	}
}

void Installer::installStartupService(string sysService) {
	#if defined(_WIN64) || defined (_WIN32)
	hjlog->out(text.infoInstallingWStartServ, Info);
	string command = "schtasks.exe /create /sc ONLOGON /tn Hajime /tr " + fs::current_path().string() + "\\Hajime.exe";
	cout << command << endl;
	int result = system(command.c_str());
	if (!IsUserAnAdmin()) {
		hjlog->out(text.errorStartupServiceWindowsAdmin, Error);
		hjlog->out(text.infoTipAdministrator, Info);
	}
	#else
	if (fs::is_directory("/etc/init.d") && !fs::is_regular_file("/lib/systemd/systemd")) {
		hjlog->out(text.infoInstallingSysvinit, Info);
		bool continueInstall = true;
		if (fs::is_regular_file("/etc/init.d/hajime.sh")) {
			hjlog->out(text.warningFoundSysvinitService, Warning);
			hjlog->out(text.questionMakeNewSysvinitService, Question, 0, 0);
			if (hjlog->getYN()) {
				continueInstall = true;
				hjlog->out(text.infoInstallingNewSysvinit, Info);
			} else {
				continueInstall = false;
			}
		}
		if (continueInstall) {
			ofstream service("/etc/init.d/hajime.sh");
			string user;
			hjlog->out(text.questionSysvinitUser, Question, 0, 0);
			std::cin >> user;
			string group;
			hjlog->out(text.questionSysvinitGroup, Question, 0, 0);
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
			hjlog->out(text.infoInstalledSysvinit, Info);
		} else {
			hjlog->out(text.infoAbortedSysvinit, Info);
		}
	} else if (fs::is_directory("/Library/LaunchAgents")) { //macOS agent directory
		hjlog->out(text.infoInstallingLaunchdServ, Info);
		bool continueInstall = true;
		if (fs::is_regular_file("/Library/LaunchAgents/Hajime.plist")) {
			hjlog->out(text.warningLaunchdServPresent, Warning);
			hjlog->out(text.questionMakeLaunchdServ, Question, 0, 0);
			if (hjlog->getYN()) {
					continueInstall = true;
					hjlog->out(text.infoInstallingNewLaunchdServ, Info);
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
			hjlog->out(text.infoInstalledLaunchServ, Info);
		} else {
			hjlog->out(text.infoAbortedLaunchServ, Info);
		}
	} else {
		if (fs::is_directory("/etc/systemd") && fs::is_regular_file(sysService)) {
			hjlog->out(text.warningFoundSystemdService, Warning);
		}
		if (fs::is_directory("/etc/systemd") && !fs::is_regular_file(sysService)) {
			hjlog->out(text.infoMakingSystemdServ + sysService + "...", Info);
			if (getuid()) {
				hjlog->out(text.errorSystemdRoot, Error);
			}
			ofstream service(sysService);
			service << "[Unit]" << endl << "Description=Starts Hajime" << endl;
			service << endl << "[Service]\nType=simple\nWorkingDirectory=" << fs::current_path().string() << "\nExecStart=" << fs::current_path().string()  << "/hajime\n\n[Install]\nWantedBy=multi-user.target";
			service.close();
		}
		if (!fs::is_directory("/etc/systemd")) {
			hjlog->out(text.errorNoSystemd, Error);
		}
	}
	#endif
}

void Installer::installDefaultServersFile(string serversFile, bool skipFileCheck, std::vector<string> servers) {
	hjlog->out(text.infoInstallingServersFile + serversFile + "...", Info);
	hjlog->out(text.infoCheckingExistingServersFile, Info);
	if (fs::is_regular_file(serversFile) && !skipFileCheck) {
		throw 0;
	} else {
		ofstream outConf(serversFile);
		for (const auto& it : servers) {
			outConf << it << endl;
		}
		outConf.close();
		hjlog->out(text.infoMadeServersFile, Info);
		if (!fs::is_regular_file(serversFile)) {
			throw 1;
		}
	}
}

Installer installer(hjlog);
