module;

#include <memory>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

export module Hajime:Getvarsfromfile;

using std::string;
using std::vector;

export vector<string> getVarsFromFile(string filename, vector<string> inputVars);
export vector<string> getVarsFromFile(string filename);
