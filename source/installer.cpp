/*  Hajime, the ultimate startup script.
    Copyright (C) 2022 Slackadays and other contributors to Hajime on GitHub.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.*/

#include <fstream>
#include <filesystem>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#if defined(_win64) || defined (_WIN32)
#include <Windows.h>
#include <shlobj.h>
#pragma comment (lib, "Shell32")
#else
#include <unistd.h>
#endif

#include "nlohmann/json.hpp"

#include "constants.hpp"
#include "output.hpp"
#include "languages.hpp"
#include "installer.hpp"
#include "flexi_format.hpp"

#include "files/hajime.h"
#include "files/default.h"
#include "files/sysvservice.h"
#include "files/hajimeplist.h"

namespace fs = std::filesystem;

void Installer::installNewServerConfigFile(ServerConfigFile& conf) {
	if (fs::is_regular_file(conf.fileLocation) && !conf.skipFileCheck) {
		throw 0;
	} else {
		std::string json;
		for (int i = 0; i < default_json_len; i++) {
			json += default_json[i];
		}
		try {
			nlohmann::json content = nlohmann::json::parse(json);
			content["version"] = hajime_version;
			content["name"] = conf.serverName;
			content["path"] = fs::current_path().string();
			content["flags"] = "-jar -Xmx4G -Xms4G " + conf.flags;
			content["file"] = conf.serverFile;
			content["counterintervalmax"] = std::to_string(defaultCounterInterval);
			//write json to file
			std::ofstream outConf(conf.fileLocation);
			outConf << content.dump(4);
			outConf.close();
		} catch (std::exception& e) {
			std::cout << e.what() << std::endl;
		}
		term.out<Info>(flexi_format(text.info.CreatedServerConfig, conf.fileLocation));
		if (!fs::is_regular_file(conf.fileLocation)) {
			throw "Could not create server config file";
		}
	}
}

void Installer::installDefaultHajConfFile(std::string fileLocation = "(none)", bool skipFileCheck, const std::string& lang) {
	term.out<Info>(text.info.InstallingDefHajConf + fileLocation + "...");
	term.out<Info>(text.info.CheckingExistingFile);
	if (fs::is_regular_file(fileLocation) && !skipFileCheck) {
		throw 0;
	} else {
		std::string json;
		for (int i = 0; i < hajime_json_len; i++) {
			json += hajime_json[i];
		}
		try {
			nlohmann::json content = nlohmann::json::parse(json);
			content["version"] = hajime_version;
			content["logfile"] = logFile;
			content["lang"] = lang;
			//write json to file
			std::ofstream outConf(fileLocation);
			outConf << content.dump(4);
			outConf.close();
		} catch (std::exception& e) {
			std::cout << e.what() << std::endl;
		}
		term.out<Info>(flexi_format(text.info.HajConfigMade, fileLocation));
		if (!fs::is_regular_file(fileLocation)) {
			throw "Could not create Hajime config file";
		}
	}
}

void Installer::installStartupService(const std::string& sysService) {
	#if defined(_WIN64) || defined (_WIN32)
	term.out<Info>(text.info.InstallingWStartServ);
	installWindows();
	#else
	if (fs::is_directory("/etc/init.d") && !fs::is_regular_file("/lib/systemd/systemd")) {
		term.out<Info>(text.info.InstallingSysvinit);
		bool continueInstall = true;
		if (fs::is_regular_file("/etc/init.d/hajime.sh")) {
			term.out<Warning>(text.warning.FoundSysvinitService);
			term.out<Question, NoEndline>(text.question.MakeNewSysvinitService);
			if (term.getYN()) {
				continueInstall = true;
				term.out<Info>(text.info.InstallingNewSysvinit);
			} else {
				continueInstall = false;
			}
		}
		if (continueInstall) {
			installSysVInit();
			term.out<Info>(text.info.InstalledSysvinit);
		} else {
			term.out<Info>(text.info.AbortedSysvinit);
		}
	} else if (fs::is_directory("/Library/LaunchAgents")) { //macOS agent directory
		term.out<Info>(text.info.InstallingLaunchdServ);
		bool continueInstall = true;
		if (fs::is_regular_file("/Library/LaunchAgents/Hajime.plist")) {
			term.out<Warning>(text.warning.LaunchdServPresent);
			term.out<Question, NoEndline>(text.question.MakeLaunchdServ);
			if (term.getYN()) {
					continueInstall = true;
					term.out<Info>(text.info.InstallingNewLaunchdServ);
			} else {
					continueInstall = false;
			}
		}
		if (continueInstall) {
			installLaunchd();
			term.out<Info>(text.info.InstalledLaunchServ);
		} else {
			term.out<Info>(text.info.AbortedLaunchServ);
		}
	} else {
		if (fs::is_directory("/etc/systemd") && fs::is_regular_file(sysService)) {
			term.out<Warning>(text.warning.FoundSystemdService);
		}
		if (fs::is_directory("/etc/systemd") && !fs::is_regular_file(sysService)) {
			term.out<Info>(text.info.MakingSystemdServ + sysService + "...");
			if (getuid()) {
				term.out<Error>(text.error.SystemdRoot);
			}
			installSystemd(sysService);
		}
		if (!fs::is_directory("/etc/systemd")) {
			term.out<Error>(text.error.NoSystemd);
		}
	}
	#endif
}

void Installer::installWindows() {
	#if defined(_WIN64) || defined (_WIN32)
	PWSTR* startupfolder = new PWSTR;
	WCHAR* executablename = new WCHAR[512];
	WCHAR* directoryname = new WCHAR[512];
	HRESULT h;
	h = SHGetKnownFolderPath(FOLDERID_Startup, NULL, NULL, startupfolder);
	if (!SUCCEEDED(h))
	{
		term.out<Error>("SHGetKnownFolderPath failed");
		CoTaskMemFree(startupfolder);
		delete[] executablename;
		delete[] directoryname;
		return;
	}
	GetModuleFileNameW(NULL, executablename, 512);
	GetCurrentDirectoryW(512, directoryname);
	IShellLinkW* shortcut;
	IPersistFile* persistfile;
	CoInitialize(NULL);
	h = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)(&shortcut));
	if (SUCCEEDED(h))
	{
		shortcut->SetPath(executablename);
		shortcut->SetWorkingDirectory(directoryname);
		h = shortcut->QueryInterface(IID_IPersistFile, (LPVOID*)(&persistfile));
		if (SUCCEEDED(h))
		{
			persistfile->Save((std::wstring(*startupfolder) + L"\\Hajime.lnk").c_str(), TRUE);
			persistfile->Release();
		}
		else
		{
			term.out<Error>("QueryInterface failed");
		}
		shortcut->Release();
	}
	else
	{
		term.out<Error>("CoCreateInstance failed");
	}
	CoTaskMemFree(startupfolder);
	delete[] executablename;
	delete[] directoryname;
	#endif
}

void Installer::installSysVInit() {
	std::ofstream service("/etc/init.d/hajime.sh");
	string user;
	term.out<Question, NoEndline>(text.question.SysvinitUser);
	std::cin >> user;
	string group;
	term.out<Question, NoEndline>(text.question.SysvinitGroup);
	std::cin >> group;
	std::string content;
	for (int i = 0; i < sysvservice_sh_len; i++) {
		content += sysvservice_sh[i];
	}
	service << flexi_format(content, fs::current_path().string(), fs::current_path().string(), user, group);
	service.close();
}

void Installer::installLaunchd() {
	std::ofstream service("/Library/LaunchAgents/Hajime.plist");
	std::string content;
	for (int i = 0; i < hajime_plist_len; i++) {
		content += hajime_plist[i];
	}
	service << flexi_format(content, fs::current_path().string(), fs::current_path().string());
	service.close();
}

void Installer::installSystemd(const std::string& location) {
	std::ofstream service(location);
	service << "[Unit]" << std::endl << "Description=Starts the Hajime startup system" << std::endl;
	service << std::endl << "[Service]\nType=forking\nWorkingDirectory=" << fs::current_path().string() << "\nExecStart=screen -S hajime -d -m " << fs::current_path().string()  << "/hajime\n\n[Install]\nWantedBy=multi-user.target";
	service.close();
}

Installer installer;
