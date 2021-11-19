#include <memory>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

using std::vector;
using std::string;

auto continueInFile = [](std::fstream &inFile){return (inFile.good() && !inFile.eof());};
auto checkBadChars = [](auto ch){return (ch != '\0') && (ch != '#') && (ch != '=');};

vector<string> getVarsFromFile(string filename, vector<string> inputVars) {
        std::fstream file(filename, std::fstream::in); //configuration file open for reading
        vector <string> outputVars;
        string line = "", key = "", value = "";
        for (unsigned int i = 0, lineNum = 0; continueInFile(file); lineNum++, i = 0, key = "", value = "") {
                getline(file, line); //get a line from file and save it to line
                if (checkBadChars(line[i])) {
                        for (;checkBadChars(line[i]); i++) { //skips past anything that isn't in a quote
                                key += line[i]; //append the thing to look for with a known-good character
                        }
                        for (i++;(i < line.length()) && (line[i] != '#'); i++) { //increment i by 1 to skip the =
                                value += line[i];
                        }
                        for (const auto &iteratorVal : inputVars) {
                                if (key == iteratorVal) {
                                        outputVars.push_back(value);
                                }
                        }
                } else {
                        outputVars.push_back(value);
                }
        }
        file.close();  //get rid of the file in memory
        return outputVars;
}

vector<string> getVarsFromFile(string filename) {
        std::fstream file(filename, std::fstream::in);
        vector<string> outputVars;
        string line = "", value = "";
        for (unsigned int i = 0, lineNum = 0; continueInFile(file); lineNum++, i = 0, value = "") {
                getline(file, line);
                if (checkBadChars(line[i])) {
                        for (;checkBadChars(line[i]); i++) { //skips past anything that isn't in a quote
                                value += line[i];
                        }
                        outputVars.push_back(value);
                }
        }
        file.close();
        return outputVars;
}
