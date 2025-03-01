#pragma once
#include "core/core.h"

#include "eop_config.h"

namespace eop {

	struct Route {
		std::vector<int> cellRoute;
		std::vector<int> entityRoute;

		int layer;
	};

	void DropEntityCount(const District & district, std::vector<Entity>&entities, std::vector<bool>&cells, const std::vector<int>&collapsedCells, int entity);

	void CollapseCell(const District& district, std::vector<Entity>& entities, const std::vector<Identifier>& identifiers, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, std::vector<int>& collapsedCells, const std::vector<std::vector<int>>& entityIdentifierCounts, int iteration, int cellIndex, int entity);

	bool Collapse(const District& district, std::vector<Entity>& entities, const std::vector<Identifier>& identifiers, const std::vector<Entity>& originalEntities,
		std::vector<bool>& cells, std::vector<int>& collapsedCells, const std::vector<std::vector<int>>& entityIdentifierCounts,
		int iteration, int totalEntities, int cellsToCollapse, Route& route, bool fullRandom, bool entitiesRandom);

}
