#include <memory>
#include <iostream>
#include <string>
#include <fstream>

#pragma once

using std::vector;

vector<string> getVarsFromFile(string filename, vector<string> inputVars);

vector<string> getVarsFromFile(string filename, vector<string> inputVars) {
        std::fstream file;
        file.open(filename, std::fstream::in); //configuration file open for reading
        vector <string> outputVars;
        string line= "", temp1 = "", temp2 = "";
        bool skipLine = false;
 	for (unsigned int i = 0, lineNum = 0; file.good() && !file.eof(); lineNum++, i = 0, temp1 = "", temp2 = "") {
                getline(file, line); //get a line from file and save it to line
 		if (line[i] == '\n' || line[i] == '\0' || line[i] == '#') {
                        skipLine = true;
                }
		if (!skipLine) {
                	for (;line[i] != '='; i++) { //skips past anything that isn't in a quote
                	        temp1 += line[i];
				if (line[i] == '#') {
					break;
				}
                	}
                	i++; //the current position is that of a quote, so increment it 1
                	for (;i < line.length(); i++) {
                	        temp2 += line[i]; //append the finished product
				if (line[i] == '#') {
                                        break;
                                }
                	}
			for (vector<string>::iterator paramIterator = inputVars.begin(); paramIterator != inputVars.end(); ++paramIterator) {
				if (temp1 == *paramIterator) {
					outputVars.push_back(temp2);
				}
			}
        
		}
	}
        file.close();  //get rid of the file in memory
	return outputVars;
}
