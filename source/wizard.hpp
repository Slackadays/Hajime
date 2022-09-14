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
						term.out<Warning>(foundFile);
						term.out<Question>(text.question.InstallNewOne);
						if (term.getYN()) {
							skipFileCheck = true;
						} else {
							return 0;
						}
					} else if (i == 1) {
						term.out<Error>(fileNotMade);
						term.out<Question>(text.question.InstallNewOneAgain);
						if (!term.getYN()) {
							return 0;
						}
					}
				}
			}
		}
};

extern const string aikarFlags;
extern Wizard wizard;
