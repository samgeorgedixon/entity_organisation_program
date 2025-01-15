#pragma once
#include "core.h"

namespace eop {

	struct vec2 {
		int x, y;
	};

	struct Iteration {
		std::string name;

		bool hide;

		std::vector<vec2>			disabledCells;
		std::vector<int>			disabledZones;
		std::vector<std::pair<std::string, std::string>> disabledIdentifiers;

		std::vector<vec2>			carriedCells;
		std::vector<int>			carriedZones;
		std::vector<std::pair<std::string, std::string>> carriedIdentifiers;

		std::vector<std::pair<std::string, std::string>> disabledZoneCollapseIdentifiers;

		std::vector<std::string>	zoneCollapsedIdentifiers;
		std::vector<int>			cells;
	};

	struct Zone {
		std::string name;

		std::vector<vec2> cells;

		std::vector<std::pair<std::string, std::string>> collapsedIdentifiers;

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

