#pragma once
#include "core/core.h"

namespace eop {

	struct vec2 {
		int x, y;
	};

	struct value_pair {
		std::string name;
		std::string value;
	};

	struct Iteration {
		std::string name;

		bool hide;
		bool disableDropIterationCount;

		std::vector<vec2>			disabledCells;
		std::vector<int>			disabledZones;
		std::vector<value_pair> disabledIdentifiers;

		std::vector<std::pair<std::string, std::vector<vec2>>>			carriedCells;
		std::vector<std::pair<std::string, std::vector<int>>>			carriedZones;
		std::vector<std::pair<std::string, std::vector<value_pair>>>	carriedIdentifiers;

		std::vector<value_pair> disabledZoneCollapseIdentifiers;

		std::vector<std::string>	zoneCollapsedIdentifiers;
		std::vector<int>			cells;
	};

	struct Zone {
		std::string name;

		std::vector<vec2> cells;

		std::vector<value_pair> collapsedIdentifiers;

		std::vector<std::vector<std::string>> positiveZoneIdentifierConditions;
		std::vector<std::vector<std::string>> negativeZoneIdentifierConditions;
	};

	struct District {
		int rows, cols;

		std::vector<bool> occupiableCells;
		std::vector<Zone> zones;

		std::vector<Iteration> iterations;
	};

	struct Identifier {
		std::string name;

		int iterationCount;

		std::vector<vec2> relitiveCellConditions;
		std::vector<int> relitiveZoneConditions;
	};

	struct IdentifierEntry {
		std::string value;
		std::vector<std::string> conditions;
	};

	struct Entity {
		int count;

		std::vector<vec2> entityCellConditions;
		std::vector<int> entityZoneConditions;

		std::vector<IdentifierEntry> identifiersValues;
	};

	struct Entities {
		std::vector<Identifier> identifiers;

		std::vector<Entity> entities;
	};

	struct EOP_Config {
		District district;
		Entities entities;
	};

}
