#pragma once
#include "core.h"

namespace eop {

	void SetupRandom();
	int RandomIntRange(int low, int high);

	std::string Low(std::string value);
	std::string Trim(std::string str, std::string whitespace = " \t");

}
