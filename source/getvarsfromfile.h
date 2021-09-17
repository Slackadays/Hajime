#include <memory>
#include <iostream>
#include <string>
#include <fstream>

#pragma once

using std::vector;

vector<string> getVarsFromFile(string filename, vector<string> inputVars);

vector<string> getVarsFromFile(string filename, vector<string> inputVars) {
        std::fstream file(filename, std::fstream::in); //configuration file open for reading
	vector <string> outputVars;
        string line= "", temp1 = "", temp2 = "";
 	for (unsigned int i = 0, lineNum = 0; file.good() && !file.eof(); lineNum++, i = 0, temp1 = "", temp2 = "") {
                getline(file, line); //get a line from file and save it to line
		if ((line[i] != '\0') && (line[i] != '#')) {
                	for (;(line[i] != '=') && (line[i] != '#'); i++) { //skips past anything that isn't in a quote
                	        temp1 += line[i];
                	}
                	for (i++;(i < line.length()) && (line[i] != '#'); i++) { //increment i by 1 to skip the =
                	        temp2 += line[i];
                	}
			for (const auto &iteratorVal : inputVars) {
				if (temp1 == iteratorVal) {
					outputVars.push_back(temp2);
				}
			}
		}
	}
        file.close();  //get rid of the file in memory
	return outputVars;
}
