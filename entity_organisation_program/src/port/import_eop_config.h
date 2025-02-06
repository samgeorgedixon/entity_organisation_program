#pragma once
#include "core/core.h"

#include "evaluate/eop_config.h"

namespace eop {

	EOP_Config ImportEOP_ConfigXLSX(std::string filePath);

	EOP_Config ImportLuaConfig(std::string luaFilePath, std::string excelFilePath);

}
