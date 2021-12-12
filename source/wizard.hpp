void initialHajimeSetup(string confFile, string serversFile, string serverFile, string sysdService);

template<typename Fn>
void wizardStep(string filename, Fn func, string foundFile, string fileNotMade);
