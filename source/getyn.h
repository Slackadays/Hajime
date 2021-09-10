#include <string>

using std::cout;
using std::cin;
using std::string;

bool getYN(){
        string response = "";
        cout << "\e[1;1m";
        cin >> response;
        cout << "\e[1;0m";
        if (response == "y" || response == "Y" || response == "yes" || response == "Yes" || response == "YES"){
                return true;
        } else {
                return false;
        }
}
