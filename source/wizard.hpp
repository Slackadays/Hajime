#include <utility>

#include "output.hpp"
#include "languages.hpp"

class Wizard {

	string confFile, serverFile, defaultLang;

	std::vector<string> servers;

	bool installedS = false;

	void doLanguageStep();
	void dividerLine();
	void pause(float mean, float stdev);
	void doHajimeStep();
	void doServerStep();
	void doAdvancedServerStep();
	void doAdvancedHajimeStep();
	void doStartupStep();
	void doNextStepStep();
	void applySteps();

	public:
		bool doArtificialPauses = true;

		void initialHajimeSetupAttended(string tempConfFile, string tempServerFile);
		void initialHajimeSetupUnattended(string tempConfFile, string tempServerFile);

		template<typename Fn, typename ...Fx>
		bool wizardStep(string filename, Fn func, string foundFile, string fileNotMade, Fx... extras) { //allow for 0 or more extra flags to be passed to the inner function that we call
			for (bool skipFileCheck = false; true;) {
				try {
					func(filename, skipFileCheck, extras...);
					return 1;
				}
				catch (int i) {
					if (i == 0) {
						hjlog.out<Warning>(foundFile);
						hjlog.out<Question>(text.question.InstallNewOne);
						if (hjlog.getYN()) {
							skipFileCheck = true;
						} else {
							return 0;
						}
					} else if (i == 1) {
						hjlog.out<Error>(fileNotMade);
						hjlog.out<Question>(text.question.InstallNewOneAgain);
						if (!hjlog.getYN()) {
							return 0;
						}
					}
				}
			}
		}
};

extern const string aikarFlags;
extern Wizard wizard;
