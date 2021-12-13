module;

#include <string>

export module Hajime:Wizard;

using std::string;

export void initialHajimeSetup(string confFile, string serversFile, string serverFile, string sysdService);

export template<typename Fn>
void wizardStep(string filename, Fn func, string foundFile, string fileNotMade);

export extern const string optFlags;
