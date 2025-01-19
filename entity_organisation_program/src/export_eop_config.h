#pragma once
#include "core.h"

#include "eop_config.h"

namespace eop {

	void PrintEOP_ConfigIteration(const EOP_Config& eopConfig, std::string identifiers, int iteration);
	void PrintEOP_Config(const EOP_Config& eopConfig, std::string identifiers);

	int ExportEOP_ConfigXLSX(std::string filePath, const EOP_Config& eopConfig, std::string identifiers);

}
