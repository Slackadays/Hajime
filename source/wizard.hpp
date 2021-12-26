#include <utility>

#include "output.hpp"
#include "languages.hpp"

void initialHajimeSetup(string confFile, string serversFile, string serverFile, string sysdService);

template<typename Fn, typename ...Fx>
bool wizardStep(string filename, Fn func, string foundFile, string fileNotMade, Fx... extras) {
	for (bool skipFileCheck = false; true;) {
		try {
			func(filename, skipFileCheck, extras...);
			return 1;
		}
		catch (int i) {
			if (i == 0) {
				logObj->out(foundFile, Warning);
				logObj->out(text.questionInstallNewOne, Question);
				if (logObj->getYN()) {
					skipFileCheck = true;
				} else {
					return 0;
				}
			} else if (i == 1) {
				logObj->out(fileNotMade, Error);
				logObj->out(text.questionInstallNewOneAgain, Question);
				if (!logObj->getYN()) {
					return 0;
				}
			}
		}
	}
}

extern const string optFlags;
