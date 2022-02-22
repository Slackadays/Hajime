#include <string>
#include <vector>
#include <filesystem>

using std::vector;
using std::string;

namespace fs = std::filesystem;

class Deduce {

	public:
		vector<string> serverFiles(const fs::path& p);
		string hajimeFile();
		string serverConfig();
		string usagePattern();
};

extern Deduce deduce;
