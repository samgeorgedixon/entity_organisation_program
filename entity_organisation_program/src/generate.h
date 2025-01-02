#pragma once
#include "core.h"

/*

- Entity Oranisation Program

- Recursion Uses a lot of Stack so Set "Stack Reserve Size" to 50,000,000.

- i = (y * cols) + x

Identifier Iteration Count "The Amount of Iterations that Identifier can be used in."

Iterations: "Repeats of the Program with Identifier Counts and Cell Carries."
	Iteration:
		Name

		Cell Disable
		Zone Disable

		Cell Carry
		Zone Carry
	...

*/

namespace eop {

	struct vec2 {
		int x, y;
	};


	struct Iteration {
		std::string name;

		std::vector<vec2>	disabledCells;
		std::vector<int>	disabledZones;

		std::vector<vec2>	carriedCells;
		std::vector<int>	carriedZones;

		std::vector<int>	cells;
	};

	struct District {
		int rows, cols;
		
		std::vector<bool> occupiableCells;
		std::vector<std::vector<vec2>> zones;

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

	void PrintDistrictIteration(EOP_Config& eop_config, int iteration, int identifierId);
	
	void GenerateDistrict(EOP_Config& eop_config, int repeats);

}
