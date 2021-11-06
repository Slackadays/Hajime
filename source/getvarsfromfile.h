#include <memory>
#include <iostream>
#include <string>
#include <fstream>

#pragma once

using std::string;
using std::vector;

vector<string> getVarsFromFile(string filename, vector<string> inputVars);
vector<string> getVarsFromFile(string filename);
