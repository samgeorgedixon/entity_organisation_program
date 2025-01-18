#pragma once
#include "core.h"

#include "eop_config.h"

namespace eop {

	int ExportEOP_ConfigXLSX(std::string filePath, const EOP_Config& eopConfig, std::string identifiers);

}
