#include <memory>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

using std::vector;
using std::string;

auto continueInFile = [](std::fstream& inFile){
	return (inFile.good() && !inFile.eof());
};

auto checkBadChars = [](const auto& ch){
	return (ch != '\0') && (ch != '#') && (ch != '=');
};

vector<string> getVarsFromFile(const string& filename, const vector<string>& inputVars) {
	vector<string> outputVars;
	for (const auto& iteratorVal : inputVars) { //cycle the whole file for a particular key in inputVars so that they don't have to be in order
		std::fstream file(filename, std::fstream::in); //configuration file open for reading
		string line;
		string key;
		string value;
		bool success = false;
		for (unsigned int i = 0, lineNum = 0; continueInFile(file); lineNum++, i = 0, key = "", value = "") {
			getline(file, line); //get a line from file and save it to line
			if (checkBadChars(line[i])) {
				for (; checkBadChars(line[i]); i++) { //skips past anything that isn't in a quote
					key += line[i]; //append the thing to look for with a known-good character
				}
				for (i++;(i < line.length()) && (line[i] != '#'); i++) { //increment i by 1 to skip the =
					value += line[i];
				}
				if (key == iteratorVal) {
						outputVars.push_back(value);
						success = true;
				}
			}
		}
		if (success) {
			success = false;
		} else {
			outputVars.push_back("");
		}
		file.close();  //get rid of the file in memory
	}
	return outputVars;
}
