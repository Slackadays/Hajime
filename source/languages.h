#include <vector>

using std::string;

#pragma once

class Text {    
        void applyLang(string lang); 
        public: 
                Text(string lang);
                std::vector<string> help;
                string errnoNotPermitted;
                string errnoNoFileOrDir;
                string errnoPermissionDenied;
                string errnoInOut;
                string errnoMemory;
                string errnoUnavailable;
                string errnoAddress; 
                string errnoBlockDev; 
                string errnoBusy;
                string errnoDirectory;
                string errnoBadArgs;
                string errnoUnknownDev;
                string errnoUnknownGeneric;
                string prefixInfo;
                string prefixError;
                string prefixWarning;
                string prefixDebug;
                string prefixQuestion;
                string errorNotEnoughArgs;
                string errorConfDoesNotExist1;
                string errorConfDoesNotExist2;
                string errorNoHajimeConfig;
                string errorStartupServiceWindowsAdmin;
                string errorSystemdRoot;
                string errorNoSystemd;
                string errorServersFilePresent;
                string questionMakeHajimeConfig;
                string questionMakeServerConfig;
};

extern string hajDefaultConfFile;
extern Text text;
