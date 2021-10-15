#include <string>

using std::cout;
using std::cin;
using std::string;

bool getYN(string prompt);

bool getYN(string prompt = "[y/n]"){
	string response = "";
	if (prompt != "") {
		prompt = " " + prompt + " ";
	}
	cout << "\033[1;1m" << prompt;
	cin >> response;
	cout << "\033[1;0m";
	if (response == "y" || response == "Y" || response == "yes" || response == "Yes" || response == "YES" || response == "YEs"){
		return true;
	} else {
		return false;
	}
}
