#pragma once
#include "core/core.h"

#include "eop_config.h"

namespace eop {

	struct Route {
		std::vector<int> cellRoute;
		std::vector<int> entityRoute;

		int layer;
	};

	void DropEntityCount(EOP_Config& eop_config, std::vector<bool>&cells, const std::vector<int>&collapsedCells, int entity);

	void CollapseCell(EOP_Config& eop_config, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, std::vector<int>& collapsedCells, const std::vector<std::vector<int>>& entityIdentifierCounts, int iteration, int cellIndex, int entity);

	bool Collapse(EOP_Config& eop_config, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, std::vector<int>& collapsedCells, const std::vector<std::vector<int>>& entityIdentifierCounts, int iteration, int totalEntities, int cellsToCollapse, Route& route, bool fullRandom, bool entitiesRandom);

}
