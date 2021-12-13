module Hajime:Languages;

#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;
using std::string;

import Getvarsfromfile;
import Languages;

void Text::applyLang(string lang) {
              //the fallack is English
              #include "en.hpp"
              if (lang == "es") { //spanish
                help.clear(); //reset the help vector
                #include "es.hpp"
              }
}

Text::Text(string file) {
        if (!fs::is_regular_file(file)) {
                applyLang("en");
        }
        else {
                std::vector<string> settings = { "lang" };
                std::vector<string> results = getVarsFromFile(file, settings);
                if (results[0] != "") {
                        applyLang(results[0]);
                }
                else {
                        applyLang("en");
                }
        }
}
string hajDefaultConfFile = "hajime.conf";
Text text(hajDefaultConfFile);
