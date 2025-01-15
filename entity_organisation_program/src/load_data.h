#pragma once
#include "core.h"

#include "eop_config.h"

#include "std_incl.h"

namespace eop {

	struct XMLVariable {
		std::string name;
		std::string value;
	};

	struct XMLLine {
		std::string line;
		std::string tag;

		std::vector<XMLVariable> variables;
	};

	struct Worksheet {
		std::string name;

		std::vector<std::vector<std::string>> table; // Rows, Cols, String
	};

	EOP_Config LoadXMLFile(std::string filePath);

	int WriteXML_EOPConfig(std::string filePath, EOP_Config& eopConfig, std::string identifier);

}
