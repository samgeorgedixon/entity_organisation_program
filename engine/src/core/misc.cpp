#include "misc.h"

namespace eop {

	void SetupRandom() {
		srand((unsigned)time(NULL));
	}

	int RandomIntRange(int low, int high) {
		if (high == 0)
			return 0;

		return low + (rand() % high);
	}

	std::string Low(std::string value) {
		std::string output = "";

		for (int i = 0; i < value.size(); i++) {
			output.push_back(tolower(value[i]));
		}
		return output;
	}
	std::string Trim(std::string str, std::string whitespace) {
		int strBegin = str.find_first_not_of(whitespace);

		if (strBegin == std::string::npos) {
			return "";
		}

		int strEnd = str.find_last_not_of(whitespace);
		int strRange = strEnd - strBegin + 1;

		return str.substr(strBegin, strRange);
	}
}
