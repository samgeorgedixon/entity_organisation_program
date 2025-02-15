#pragma once
#include "core/core.h"

#include "evaluate/eop_config.h"
#include "evaluate/evaluate.h"

#include "port/export_eop_config.h"

namespace eop {

	std::pair<bool, bool> RunLuaConfig(std::string luaPresetFilePath, std::string importSpreadsheetFilePath, std::string exportSpreadsheetFilePath, int depth, bool fullRandom, bool entitiesRandom, std::string identifiers);

}
