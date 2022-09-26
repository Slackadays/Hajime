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

#include <vector>

#include "constants.hpp"

using std::string;

#pragma once

class Text {
	public:
		void autoSetLanguage();
		string filterLanguage(const string& input);
		string getUserLanguage();
		void applyLang(const string& lang);
		explicit Text(string lang);
		std::vector<string> help;
		string language;
		struct Errno {
			string NotPermitted;
			string NoFileOrDir;
			string PermissionDenied;
			string InOut;
			string Memory;
			string Unavailable;
			string Address;
			string BlockDev;
			string Busy;
			string Directory;
			string BadArgs;
			string UnknownDev;
			string UnknownGeneric;
		};
		Errno eno;
		struct Prefix {
			string Info;
			string Error;
			string Warning;
			string Debug;
			string Question;
			string VInfo;
			string VError;
			string VWarning;
			string VDebug;
			string VQuestion;
		};
		Prefix prefix;
		struct Error {
			string NotEnoughArgs;
			string ConfDoesNotExist1;
			string ConfDoesNotExist2;
			string NoHajimeConfig;
			string StartupServiceWindowsAdmin;
			string SystemdRoot;
			string NoSystemd;
			string ServerFileNotPresent1;
			string ServerFileNotPresent2;
			string CouldntSetPath;
			string Generic;
			string MethodNotValid;
			string CreatingDirectory;
			string FilesInPath;
			string Mount;
			string Code;
			string HajFileNotMade;
			string ServerConfNotCreated;
			string OptionNotAvailable;
			string InvalidServerNumber;
			string ServerSelectionInvalid;
			string DoesntSupportWindows;
			string InvalidCommand;
			string InvalidHajCommand1;
			string InvalidServerCommand1;
			string CreatingPipe;
		};
		Error error;
		struct Warning {
			string FoundSysvinitService;
			string FoundSystemdService;
			string FoundServerConf;
			string FoundHajConf;
			string IsRunningFalse;
			string TestingWindowsSupport;
			string HajConfPresent;
			string LaunchdServPresent;
			string FoundServerConfPlusFile;
			string FileDoesntExist;
		};
		Warning warning;
		struct Question {
			string MakeLaunchdServ;
			string Prompt;
			string MakeHajimeConfig;
			string MakeServerConfig;
			string MakeNewSysvinitService;
			string WizardServerFile;
			string WizardStartupService;
			string SysvinitUser;
			string SysvinitGroup;
			string DoSetupInstaller;
			string StartHajime;
			string UseFlags;
			string InstallNewOne;
			string InstallNewOneAgain;
			string CreateAnotherServerFile;
			string ApplyConfigToServerFile;
			string UseDefaultServerFile1;
			string UseDefaultServerFile2;
			string EnterNewServerFile;
			string EnterCustomFlags;
			string HajimeLanguage;
		};
		Question question;
		struct Option {
			string MakeServerFileManually;
			string DoManually;
			string EnterManually;
			string LetHajimeDeduce;
			string SkipStep;
			string UseDefault;
			string ChooseOptionBelow;
			string YourChoice;
			string AttendedInstallation;
			string UnattendedInstallation;
			string SkipSetup;
			string AikarFlags;
			string HillttyFlags;
			string FroggeMCFlags;
			string BasicZGCFlags;
			string CustomFlags;
			string CurrentLanguage1;
			string CurrentLanguage2;
			string NoLanguage;
		};
		Option option;
		struct Info {
			string InstallingSysvinit;
			string InstallingNewSysvinit;
			string InstalledSysvinit;
			string AbortedSysvinit;
			string NoLogFile;
			string ReadingServerSettings;
			string ServerFile;
			string ServerPath;
			string ServerCommand;
			string ServerMethod;
			string ServerDevice;
			string ServerDebug;
			string ServerIsRunning;
			string CreatedServerConfig1;
			string CreatedServerConfig2;
			string TryingToStartProgram;
			string StartingServer;
			string ServerStartCompleted;
			string POSIXdriveMount;
			string TryingFilesystem1;
			string TryingFilesystem2;
			string TryingMount;
			string CreatingDirectory;
			string DeviceMounted;
			string NoMount;
			string InstallingDefServConf;
			string InstallingNewServConf;
			string InstallingDefHajConf;
			string CheckingExistingFile;
			string HajConfigMade1;
			string HajConfigMade2;
			string InstallingWStartServ;
			string TipAdministrator;
			string InstallingLaunchdServ;
			string InstallingNewLaunchdServ;
			string InstalledLaunchServ;
			string AbortedLaunchServ;
			string MakingSystemdServ;
			string EnterNewNameForServer1;
			string EnterNewNameForServer2;
			string EnterCommand;
			struct Wizard {
				string HajimeFile;
				string ServerFile;
				string StartupService;
				string Complete;
				string NextStepServerFile1;
				string NextStepServerFile2;
			};
			Wizard wizard;
		};
		Info info;
		struct Debug {
			string HajDefConfNoExist1;
			string HajDefConfNoExist2;
			string ReadingReadsettings;
			string ReadReadsettings;
			string UsingOldMethod;
			string UsingNewMethod;
			string Flags;
			string ValidatingSettings;
			struct Flag {
				string Array0;
				string Array1;
				string VecInFor;
				string VecOutFor;
			};
			Flag flag;
			struct Counters {
				string perfstructTooSmall;
				string groupfdNotValid;
				string exclusiveAccess;
				string invalidMemoryAddress;
				string invalidEvent;
				string eventNotSupported;
				string invalidEventType;
				string tooManyHardwareBreakpoints;
				string hardwareSupport;
				string unsupportedEventExclusion;
				string invalidPID;
				string otherError;
			};
			Counters counters;
		};
		Debug debug;
		struct Server {
			struct Restart {
				string minutes5;
				string minutes15;
				string alert1;
				string alert2;
			};
			Restart restart;
			struct Command {
				struct Hajime {
					string regex;
					string output;
				};
				Hajime hajime;
				struct Time {
					string regex;
					string output;
				};
				Time time;
				struct Help {
					string regex;
					string output;
					struct Message {
						string coinflip;
						string die;
						string d20;
						string time;
						string hajime;
						string discord;
						string help;
						string name;
						string info;
						string uptime;
						string system;
						string perf;
						string hwperf;
						string swperf;
						string caperf;
						string restart;
					};
					Message message;
				};
				Help help;
				struct Die {
					string regex;
					string output;
				};
				Die die;
				struct D20 {
					string regex;
					string output;
				};
				D20 d20;
				struct Coinflip {
					string regex;
					struct Output {
						string heads;
						string tails;
					};
					Output output;
				};
				Coinflip coinflip;
				struct Discord {
					string regex;
					string output;
				};
				Discord discord;
				struct Name {
					string regex;
					string output;
				};
				Name name;
				struct Info {
					string regex;
				};
				Info info;
				struct Uptime {
					string regex;
					string output1;
					string output2;
					string output3;
				};
				Uptime uptime;
				struct Restart {
					string regex;
					string output1;
					string output2;
					string output3;
					string outputDisabled;
				};
				Restart restart;
				struct System {
					string regex;
					string output;
					struct Key {
						string os;
						string cpu;
						string ram;
						string swap;
						string uptime;
						string processes;
						string loadavg;
					};
					Key key;
					struct Value {
						string os;
						string cpu;
						string ram;
						string swap;
						string uptime;
						string processes;
						string loadavg;
					};
					Value value;
				};
				System system;
				struct Perf {
					string regex;
					string output;
					struct Key {
						string cpuusage;
						string ramusage;
						string cpumigrations;
						string ipc;
						string cps;
						string ips;
						string minorpagefaults;
						string majorpagefaults;
						string contextswitches;
						string stalledfrontend;
						string stalledbackend;
						string buscycles;
						string branchmisses;
						string cachemisses;
						string emufaults;
						string alignfaults;
						string l1dreadmisses;
						string l1dprefetchmisses;
						string l1dwritemisses;
						string l1ireadmisses;
						string l1iprefetchmisses;
						string llcreadmisses;
						string llcwritemisses;
						string llcprefetchmisses;
						string dtlbreadmisses;
						string dtlbwritemisses;
						string dtlbprefetchmisses;
						string itlbreadmisses;
						string bpureadmisses;
					};
					Key key;
					struct Value {
						string cpuusage;
						string ramusage;
						string cpumigrations;
						string ipc;
						string cps;
						string ips;
						string minorpagefaults;
						string majorpagefaults;
						string contextswitches;
						string stalledfrontend;
						string stalledbackend;
						string buscycles;
						string branchmisses;
						string cachemisses;
						string emufaults;
						string alignfaults;
						string l1dreadmisses;
						string l1dprefetchmisses;
						string l1dwritemisses;
						string l1ireadmisses;
						string l1iprefetchmisses;
						string llcreadmisses;
						string llcwritemisses;
						string llcprefetchmisses;
						string dtlbreadmisses;
						string dtlbwritemisses;
						string dtlbprefetchmisses;
						string itlbreadmisses;
						string bpureadmisses;
					};
					Value value;
				};
				Perf perf;
				struct HWPerf {
					string regex;
					string output;
				};
				HWPerf hwperf;
				struct SWPerf {
					string regex;
					string output;
				};
				SWPerf swperf;
				struct CAPerf {
					string regex;
					string output;
				};
				CAPerf caperf;
			};
			Command command;
		};
		Server server;
		string fileServerConfComment;
};

extern Text text;
