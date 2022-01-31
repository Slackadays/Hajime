#include <vector>

using std::string;

#pragma once

class Text {
	public:
		void autoSetLanguage();
		string getUserLanguage();
		void applyLang(string lang);
		Text(string lang);
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
			string ServersFilePresent;
			string ServerFileNotPresent1;
			string ServerFileNotPresent2;
			string ServersFileNotCreated;
			string CouldntSetPath;
			string Generic;
			string MethodNotValid;
			string CreatingDirectory;
			string FilesInPath;
			string NoServersFile;
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
			string WizardServersFile;
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
			string InstallingServersFile;
			string CheckingExistingServersFile;
			string MadeServersFile;
			string EnterNewNameForServer1;
			string EnterNewNameForServer2;
			string EnterCommand;
			struct Wizard {
				string HajimeFile;
				string ServersFile;
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
						string time;
						string hajime;
						string discord;
						string help;
						string name;
						string uptime;
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
				};
				System system;
			};
			Command command;
		};
		Server server;
		string fileServerConfComment;
};

extern string hajDefaultConfFile;
extern Text text;
