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

const string optFlags = "-XX:+UseG1GC -XX:+ParallelRefProcEnabled -XX:MaxGCPauseMillis=200 -XX:+UnlockExperimentalVMOptions -XX:+DisableExplicitGC -XX:+AlwaysPreTouch -XX:G1NewSizePercent=30 -XX:G1MaxNewSizePercent=40 -XX:G1HeapRegionSize=8M -XX:G1ReservePercent=20 -XX:G1HeapWastePercent=5 -XX:G1MixedGCCountTarget=4 -XX:InitiatingHeapOccupancyPercent=15 -XX:G1MixedGCLiveThresholdPercent=90 -XX:G1RSetUpdatingPauseTimePercent=5 -XX:SurvivorRatio=32 -XX:+PerfDisableSharedMem  XX:MaxTenuringThreshold=1 -Daikars.new.flags=true -Dusing.aikars.flags=https://mcflags.emc.gs";

Installer::Installer(std::shared_ptr<Output> log) {
	logObj = log;
}

void Installer::installDefaultServerConfFile(string conf, bool skipFileCheck) {
	logObj->out(text.questionUseFlags, Question);
	string flags;
	if (logObj->getYN()) {
		flags = optFlags;
	} else {
		flags = "";
	}
	logObj->out(text.infoInstallingDefServConf + conf + "...", Info);
	if (fs::is_regular_file(conf) && !skipFileCheck) {
		throw 0;
	} else {
		Installer::installNewServerConfigFile(conf, flags);
		logObj->out(text.infoInstallingNewServConf + conf + "...", Info);
		if (!fs::is_regular_file(conf)) {
			throw 1;
		}
	}
}

void Installer::installDefaultHajConfFile(string fileLocation = "(none)", bool skipFileCheck) {
	logObj->out(text.infoInstallingDefHajConf + fileLocation + "...", Info);
	logObj->out(text.infoCheckingExistingFile, Info);
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
		logObj->out(text.infoHajConfigMade1 + fileLocation + text.infoHajConfigMade2, Info);
		if (!fs::is_regular_file(fileLocation)) {
			throw 1;
		}
	}
}

void Installer::installNewServerConfigFile(string fileLocation, string flags) {
	ofstream outConf(fileLocation);
	outConf << "name=" << std::regex_replace(fileLocation, std::regex("\\..*", std::regex_constants::optimize), "") << endl << "path=" << fs::current_path().string() << endl << "exec=java" << endl << "flags=-jar " + flags << endl << "file=server.jar" << endl << "command=" << endl << "method=new" << endl << "device=" << endl;
	outConf << text.fileServerConfComment << endl;
	logObj->out(text.infoCreatedServerConfig1 + fileLocation + text.infoCreatedServerConfig2, Info);
	outConf.close();
}

void Installer::installStartupService(string sysService) {
	#if defined(_WIN64) || defined (_WIN32)
	logObj->out(text.infoInstallingWStartServ, Info);
	string command = "schtasks.exe /create /sc ONLOGON /tn Hajime /tr " + fs::current_path().string() + "\\Hajime.exe";
	cout << command << endl;
	int result = system(command.c_str());
	if (!IsUserAnAdmin()) {
		logObj->out(text.errorStartupServiceWindowsAdmin, Error);
		logObj->out(text.infoTipAdministrator, Info);
	}
	#else
	if (fs::is_directory("/etc/init.d") && !fs::is_regular_file("/lib/systemd/systemd")) {
		logObj->out(text.infoInstallingSysvinit, Info);
		bool continueInstall = true;
		if (fs::is_regular_file("/etc/init.d/hajime.sh")) {
			logObj->out(text.warningFoundSysvinitService, Warning);
			logObj->out(text.questionMakeNewSysvinitService, Question, 0, 0);
			if (logObj->getYN()) {
				continueInstall = true;
				logObj->out(text.infoInstallingNewSysvinit, Info);
			} else {
				continueInstall = false;
			}
		}
		if (continueInstall) {
			ofstream service("/etc/init.d/hajime.sh");
			string user;
			logObj->out(text.questionSysvinitUser, Question, 0, 0);
			std::cin >> user;
			string group;
			logObj->out(text.questionSysvinitGroup, Question, 0, 0);
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
			logObj->out(text.infoInstalledSysvinit, Info);
		} else {
			logObj->out(text.infoAbortedSysvinit, Info);
		}
	} else if (fs::is_directory("/Library/LaunchAgents")) { //macOS agent directory
		logObj->out(text.infoInstallingLaunchdServ, Info);
		bool continueInstall = true;
		if (fs::is_regular_file("/Library/LaunchAgents/Hajime.plist")) {
			logObj->out(text.warningLaunchdServPresent, Warning);
			logObj->out(text.questionMakeLaunchdServ, Question, 0, 0);
			if (logObj->getYN()) {
					continueInstall = true;
					logObj->out(text.infoInstallingNewLaunchdServ, Info);
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
			logObj->out(text.infoInstalledLaunchServ, Info);
		} else {
			logObj->out(text.infoAbortedLaunchServ, Info);
		}
	} else {
		if (fs::is_directory("/etc/systemd") && fs::is_regular_file(sysService)) {
			logObj->out(text.warningFoundSystemdService, Warning);
		}
		if (fs::is_directory("/etc/systemd") && !fs::is_regular_file(sysService)) {
			logObj->out(text.infoMakingSystemdServ + sysService + "...", Info);
			if (getuid()) {
				logObj->out(text.errorSystemdRoot, Error);
			}
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

void Installer::installDefaultServersFile(string serversFile, bool skipFileCheck, std::vector<string> servers) {
	logObj->out(text.infoInstallingServersFile + serversFile + "...", Info);
	logObj->out(text.infoCheckingExistingServersFile, Info);
	if (fs::is_regular_file(serversFile) && !skipFileCheck) {
		throw 0;
	} else {
		ofstream outConf(serversFile);
		for (const auto& it : servers) {
			outConf << it << endl;
		}
		outConf.close();
		logObj->out(text.infoMadeServersFile, Info);
		if (!fs::is_regular_file(serversFile)) {
			throw 1;
		}
	}
}

Installer installer(logObj);
