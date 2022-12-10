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

#include "../hajime_main/constants.hpp"

#pragma once

class Text {
	public:
		void autoSetLanguage();
		std::string filterLanguage(const std::string& input);
		std::string getUserLanguage();
		void applyLang(const std::string& lang);
		explicit Text(std::string lang);
		std::vector<std::string> help;
		std::string language;
		struct Errno {
			std::string NotPermitted;
			std::string NoFileOrDir;
			std::string PermissionDenied;
			std::string InOut;
			std::string Memory;
			std::string Unavailable;
			std::string Address;
			std::string BlockDev;
			std::string Busy;
			std::string Directory;
			std::string BadArgs;
			std::string UnknownDev;
			std::string UnknownGeneric;
		};
		Errno eno;
		struct Prefix {
			std::string Info;
			std::string Error;
			std::string Warning;
			std::string Debug;
			std::string Question;
			std::string VInfo;
			std::string VError;
			std::string VWarning;
			std::string VDebug;
			std::string VQuestion;
		};
		Prefix prefix;
		struct Error {
			std::string NotEnoughArgs;
			std::string ConfDoesNotExist;
			std::string NoHajimeConfig;
			std::string StartupServiceWindowsAdmin;
			std::string SystemdRoot;
			std::string NoSystemd;
			std::string ServerFileNotPresent;
			std::string CouldntSetPath;
			std::string Generic;
			std::string MethodNotValid;
			std::string CreatingDirectory;
			std::string FilesInPath;
			std::string Mount;
			std::string Code;
			std::string HajFileNotMade;
			std::string ServerConfNotCreated;
			std::string OptionNotAvailable;
			std::string InvalidServerNumber;
			std::string ServerSelectionInvalid;
			std::string DoesntSupportWindows;
			std::string InvalidCommand;
			std::string InvalidHajCommand1;
			std::string InvalidServerCommand1;
			std::string CreatingPipe;
			std::string PrivilegedUser;
		};
		Error error;
		struct Warning {
			std::string FoundSysvinitService;
			std::string FoundSystemdService;
			std::string FoundServerConf;
			std::string FoundHajConf;
			std::string IsRunningFalse;
			std::string TestingWindowsSupport;
			std::string HajConfPresent;
			std::string LaunchdServPresent;
			std::string FoundServerConfPlusFile;
			std::string FileDoesntExist;
		};
		Warning warning;
		struct Question {
			std::string MakeLaunchdServ;
			std::string Prompt;
			std::string MakeHajimeConfig;
			std::string MakeServerConfig;
			std::string MakeNewSysvinitService;
			std::string WizardServerFile;
			std::string WizardStartupService;
			std::string SysvinitUser;
			std::string SysvinitGroup;
			std::string DoSetupInstaller;
			std::string StartHajime;
			std::string UseFlags;
			std::string InstallNewOne;
			std::string InstallNewOneAgain;
			std::string CreateAnotherServerFile;
			std::string ApplyConfigToServerFile;
			std::string UseDefaultServerFile;
			std::string EnterNewServerFile;
			std::string EnterCustomFlags;
			std::string HajimeLanguage;
		};
		Question question;
		struct Option {
			std::string MakeServerFileManually;
			std::string DoManually;
			std::string EnterManually;
			std::string LetHajimeDeduce;
			std::string SkipStep;
			std::string UseDefault;
			std::string ChooseOptionBelow;
			std::string YourChoice;
			std::string AttendedInstallation;
			std::string UnattendedInstallation;
			std::string SkipSetup;
			std::string AikarFlags;
			std::string HillttyFlags;
			std::string FroggeMCFlags;
			std::string BasicZGCFlags;
			std::string CustomFlags;
			std::string CurrentLanguage;
			std::string NoLanguage;
		};
		Option option;
		struct Info {
			std::string InstallingSysvinit;
			std::string InstallingNewSysvinit;
			std::string InstalledSysvinit;
			std::string AbortedSysvinit;
			std::string NoLogFile;
			std::string ReadingServerSettings;
			std::string ServerFile;
			std::string ServerPath;
			std::string ServerCommand;
			std::string ServerMethod;
			std::string ServerDevice;
			std::string ServerDebug;
			std::string ServerIsRunning;
			std::string CreatedServerConfig;
			std::string TryingToStartProgram;
			std::string StartingServer;
			std::string ServerStartCompleted;
			std::string POSIXdriveMount;
			std::string TryingFilesystem;
			std::string TryingMount;
			std::string CreatingDirectory;
			std::string DeviceMounted;
			std::string NoMount;
			std::string InstallingDefServConf;
			std::string InstallingNewServConf;
			std::string InstallingDefHajConf;
			std::string CheckingExistingFile;
			std::string HajConfigMade;
			std::string InstallingWStartServ;
			std::string TipAdministrator;
			std::string InstallingLaunchdServ;
			std::string InstallingNewLaunchdServ;
			std::string InstalledLaunchServ;
			std::string AbortedLaunchServ;
			std::string MakingSystemdServ;
			std::string EnterNewNameForServer;
			std::string MakingHajimeDirectory;
			struct Wizard {
				std::string HajimeFile;
				std::string ServerFile;
				std::string StartupService;
				std::string Complete;
				std::string NextStepServerFile;
			};
			Wizard wizard;
		};
		Info info;
		struct Debug {
			std::string HajDefConfNoExist;
			std::string ReadingReadsettings;
			std::string ReadReadsettings;
			std::string UsingOldMethod;
			std::string UsingNewMethod;
			std::string Flags;
			std::string ValidatingSettings;
			struct Flag {
				std::string Array0;
				std::string Array1;
				std::string VecInFor;
				std::string VecOutFor;
			};
			Flag flag;
			struct Counters {
				std::string perfstructTooSmall;
				std::string groupfdNotValid;
				std::string exclusiveAccess;
				std::string invalidMemoryAddress;
				std::string invalidEvent;
				std::string eventNotSupported;
				std::string invalidEventType;
				std::string tooManyHardwareBreakpoints;
				std::string hardwareSupport;
				std::string unsupportedEventExclusion;
				std::string invalidPID;
				std::string otherError;
			};
			Counters counters;
		};
		Debug debug;
		struct Server {
			struct Restart {
				std::string minutes5;
				std::string minutes15;
				std::string alert;
			};
			Restart restart;
			struct Command {
				struct Hajime {
					std::string regex;
					std::string output;
				};
				Hajime hajime;
				struct Time {
					std::string regex;
					std::string output;
				};
				Time time;
				struct Help {
					std::string regex;
					std::string output;
					struct Message {
						std::string coinflip;
						std::string die;
						std::string d20;
						std::string time;
						std::string hajime;
						std::string discord;
						std::string help;
						std::string name;
						std::string info;
						std::string uptime;
						std::string system;
						std::string perf;
						std::string hwperf;
						std::string swperf;
						std::string caperf;
						std::string restart;
					};
					Message message;
				};
				Help help;
				struct Die {
					std::string regex;
					std::string output;
				};
				Die die;
				struct D20 {
					std::string regex;
					std::string output;
				};
				D20 d20;
				struct Coinflip {
					std::string regex;
					struct Output {
						std::string heads;
						std::string tails;
					};
					Output output;
				};
				Coinflip coinflip;
				struct Discord {
					std::string regex;
					std::string output;
				};
				Discord discord;
				struct Name {
					std::string regex;
					std::string output;
				};
				Name name;
				struct Info {
					std::string regex;
				};
				Info info;
				struct Uptime {
					std::string regex;
					std::string output;
				};
				Uptime uptime;
				struct Restart {
					std::string regex;
					std::string output;
					std::string outputDisabled;
				};
				Restart restart;
				struct System {
					std::string regex;
					std::string output;
					struct Key {
						std::string os;
						std::string cpu;
						std::string ram;
						std::string swap;
						std::string uptime;
						std::string processes;
						std::string loadavg;
						std::string temps;
						std::string storage;
					};
					Key key;
					struct Value {
						std::string os;
						std::string cpu;
						std::string ram;
						std::string swap;
						std::string uptime;
						std::string processes;
						std::string loadavg;
						std::string temps;
						std::string storage;
					};
					Value value;
				};
				System system;
				struct Perf {
					std::string regex;
					std::string output;
					struct Key {
						std::string cpuusage;
						std::string ramusage;
						std::string cpumigrations;
						std::string ipc;
						std::string cps;
						std::string ips;
						std::string minorpagefaults;
						std::string majorpagefaults;
						std::string contextswitches;
						std::string stalledfrontend;
						std::string stalledbackend;
						std::string buscycles;
						std::string branchmisses;
						std::string cachemisses;
						std::string emufaults;
						std::string alignfaults;
						std::string l1dreadmisses;
						std::string l1dprefetchmisses;
						std::string l1dwritemisses;
						std::string l1ireadmisses;
						std::string l1iprefetchmisses;
						std::string llcreadmisses;
						std::string llcwritemisses;
						std::string llcprefetchmisses;
						std::string dtlbreadmisses;
						std::string dtlbwritemisses;
						std::string dtlbprefetchmisses;
						std::string itlbreadmisses;
						std::string bpureadmisses;
					};
					Key key;
					struct Value {
						std::string cpuusage;
						std::string ramusage;
						std::string cpumigrations;
						std::string ipc;
						std::string cps;
						std::string ips;
						std::string minorpagefaults;
						std::string majorpagefaults;
						std::string contextswitches;
						std::string stalledfrontend;
						std::string stalledbackend;
						std::string buscycles;
						std::string branchmisses;
						std::string cachemisses;
						std::string emufaults;
						std::string alignfaults;
						std::string l1dreadmisses;
						std::string l1dprefetchmisses;
						std::string l1dwritemisses;
						std::string l1ireadmisses;
						std::string l1iprefetchmisses;
						std::string llcreadmisses;
						std::string llcwritemisses;
						std::string llcprefetchmisses;
						std::string dtlbreadmisses;
						std::string dtlbwritemisses;
						std::string dtlbprefetchmisses;
						std::string itlbreadmisses;
						std::string bpureadmisses;
					};
					Value value;
				};
				Perf perf;
				struct HWPerf {
					std::string regex;
					std::string output;
				};
				HWPerf hwperf;
				struct SWPerf {
					std::string regex;
					std::string output;
				};
				SWPerf swperf;
				struct CAPerf {
					std::string regex;
					std::string output;
				};
				CAPerf caperf;
			};
			Command command;
		};
		Server server;
		std::string fileServerConfComment;
};

extern Text text;
