#pragma once
#include "core/core.h"

#include <string>

namespace eop {

	void RunLuaConfig(std::string luaPresetFilePath, std::string importSpreadsheetFilePath, std::string exportSpreadsheetFilePath, int depth, bool fullRandom, bool entitiesRandom, std::string identifiers);

}
