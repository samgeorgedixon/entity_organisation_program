#include "evaluate.h"

namespace eop {

	void SetEntityCounts(const District& district, std::vector<Entity>& entities, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, const std::vector<int>& collapsedCells) {
		int entityCount = entities.size();

		for (int i = 0; i < entityCount; i++) {
			entities[i].count = originalEntities[i].count;
		}

		for (int i = 0; i < collapsedCells.size(); i++) {
			int entityId = -1;

			for (int j = 0; j < entityCount; j++) {
				if (cells[(collapsedCells[i] * entityCount) + j]) {
					entityId = j;
				}
			}
			if (entityId != -1) {
				DropEntityCount(district, entities, cells, collapsedCells, entityId);
			}
		}
	}

	int RunCollapses(const District& district, std::vector<Entity>& entities, const std::vector<Identifier>& identifiers, const std::vector<Entity>& originalEntities,
		std::vector<bool>& cells, std::vector<int>& collapsedCells, const std::vector<int>& defaultCollapsedCells, const std::vector<std::vector<int>>& entityIdentifierCounts,
		int iteration, int totalEntities, int cellsToCollapse, Route& route, int depth, bool fullRandom, bool entitiesRandom) {

		collapsedCells = defaultCollapsedCells;
		SetEntityCounts(district, entities, originalEntities, cells, collapsedCells);

		route.layer = 0;

		bool done = false;
		while (!done) {
			done = Collapse(district, entities, identifiers, originalEntities, cells, collapsedCells, entityIdentifierCounts, iteration, totalEntities, cellsToCollapse, route, fullRandom, entitiesRandom);

			route.layer++;

			if (route.layer >= depth) {
				fullRandom = true;
			}
		}

		return collapsedCells.size();
	}

	void SetDistrictCells(District& district, const std::vector<Entity>& entities, std::vector<bool>& cells, int iteration) {
		int cellCount = district.rows * district.cols;
		int entityCount = entities.size();

		for (int i = 0; i < cellCount; i++) {
			int entity = -1;

			for (int j = 0; j < entityCount; j++) {
				int cellIndex = (i * entityCount) + j;

				if (cells[cellIndex]) {
					if (entity != -1) {
						entity = -1;
						break;
					}
					entity = j;
				}
			}
			district.iterations[iteration].cells.push_back(entity);
		}
	}

	void ApplyGlobalEntityConditions(const District& district, const std::vector<Entity>& entities, std::vector<bool>& cells) {
		int cols = district.cols;

		int entityCount = entities.size();

		for (int i = 0; i < entityCount; i++) {
			for (int j = 0; j < entities[i].entityCellConditions.size(); j++) {
				int x = entities[i].entityCellConditions[j].x;
				int y = entities[i].entityCellConditions[j].y;

				int index = (y * cols) + x;

				cells[index * entityCount + i] = false;
			}
			for (int j = 0; j < entities[i].entityZoneConditions.size(); j++) {
				int zone = entities[i].entityZoneConditions[j];
				for (int k = 0; k < district.zones[zone].cells.size(); k++) {
					int x = district.zones[zone].cells[k].x;
					int y = district.zones[zone].cells[k].y;

					int index = (y * cols) + x;

					cells[index * entityCount + i] = false;
				}
			}
		}
	}

	void RemoveUnoccupiableCells(const District& district, const std::vector<Entity>& entities, std::vector<bool>& cells, int& cellsToCollapse) {
		int cellCount = district.rows * district.cols;
		int entityCount = entities.size();

		cellsToCollapse = cellCount;

		for (int i = 0; i < cellCount; i++) {
			if (!district.occupiableCells[i]) {
				for (int j = 0; j < entityCount; j++) {
					cells[(i * entityCount) + j] = false;
				}
			}
		}
	}

	void DisableCellCoordinate(const District& district, const std::vector<Entity>& entities, std::vector<bool>& cells, vec2 cellCoord) {
		int cols = district.cols;

		int entityCount = entities.size();

		int cellIndex = ((cellCoord.y * cols) + cellCoord.x) * entityCount;

		for (int j = 0; j < entityCount; j++) {
			cells[cellIndex + j] = false;
		}
	}

	void DisableCellsZonesIdentifiers(const District& district, const std::vector<Entity>& entities, const std::vector<Identifier>& identifiers, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, int iteration) {
		int cellCount = district.rows * district.cols;
		int entityCount = entities.size();
		int identifierCount = identifiers.size();

		// Disable Cells
		for (int i = 0; i < district.iterations[iteration].disabledCells.size(); i++) {
			DisableCellCoordinate(district, entities, cells, district.iterations[iteration].disabledCells[i]);
		}

		// Disable Zones
		for (int i = 0; i < district.iterations[iteration].disabledZones.size(); i++) {
			int zoneIndex = district.iterations[iteration].disabledZones[i];

			for (int j = 0; j < district.zones[zoneIndex].cells.size(); j++) {
				DisableCellCoordinate(district, entities, cells, district.zones[district.iterations[iteration].disabledZones[i]].cells[j]);
			}
		}

		// Disable Identifiers
		std::vector<int> disabledEntities;

		for (int i = 0; i < district.iterations[iteration].disabledIdentifiers.size(); i++) {
			std::string identifier = district.iterations[iteration].disabledIdentifiers[i].name;
			int			identifierIndex = -1;

			for (int j = 0; j < identifierCount; j++) {
				if (Low(identifier) == Low(identifiers[j].name)) {
					identifierIndex = j;
					break;
				}
			}
			if (identifierIndex == -1) {
				continue;
			}

			std::string value = district.iterations[iteration].disabledIdentifiers[i].value;

			for (int j = 0; j < entityCount; j++) {
				std::string entityValue = originalEntities[j].identifiersValues[identifierIndex].value;
				if (Low(value) == Low(entityValue)) {
					disabledEntities.push_back(j);
				}
			}
		}
		for (int i = 0; i < cellCount; i++) {
			for (int j = 0; j < disabledEntities.size(); j++) {
				cells[(i * entityCount) + disabledEntities[j]] = false;
			}
		}
	}

	void CarryCellCoordinate(const District& district, std::vector<Entity>& entities, const std::vector<Identifier>& identifiers, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, std::vector<int>& collapsedCells, const std::vector<std::vector<int>>& entityIdentifierCounts, vec2 cellCoord, int iteration, int carryIterationIndex) {

		int cols = district.cols;
		int entityCount = entities.size();

		int cellIndex = (cellCoord.y * cols) + cellCoord.x;
		int entity = -1;

		for (int j = 0; j < entityCount; j++) {
			if (j != district.iterations[carryIterationIndex].cells[cellIndex]) {
				cells[(cellIndex * entityCount) + j] = false;
			}
			else {
				cells[(cellIndex * entityCount) + j] = true;
			}
		}
		entity = district.iterations[carryIterationIndex].cells[cellIndex];

		if (entity == -1) {
			return;
		}

		CollapseCell(district, entities, identifiers, originalEntities, cells, collapsedCells, entityIdentifierCounts, iteration, cellIndex, entity);
	}

	void CarryCellsZonesIdentifiers(const District& district, std::vector<Entity>& entities, const std::vector<Identifier>& identifiers, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, std::vector<int>& collapsedCells, const std::vector<std::vector<int>>& entityIdentifierCounts, int iteration) {

		int rows = district.rows;
		int cols = district.cols;

		int entityCount = entities.size();
		int identifierCount = identifiers.size();

		if (iteration == 0)
			return;

		// Carry Cells
		for (int l = 0; l < district.iterations[iteration].carriedCells.size(); l++) {
			int carryIterationIndex = -1;

			for (int i = 0; i < district.iterations.size(); i++) {
				if (Low(district.iterations[i].name) == Low(district.iterations[iteration].carriedCells[l].first)) {
					carryIterationIndex = i;
				}
			}
			if (carryIterationIndex == -1) {
				continue;
			}

			for (int i = 0; i < district.iterations[iteration].carriedCells[l].second.size(); i++) {
				CarryCellCoordinate(district, entities, identifiers, originalEntities, cells, collapsedCells, entityIdentifierCounts
					, district.iterations[iteration].carriedCells[l].second[i], iteration, carryIterationIndex);
			}
		}

		// Carry Zones
		for (int l = 0; l < district.iterations[iteration].carriedZones.size(); l++) {
			int carryIterationIndex = -1;

			for (int i = 0; i < district.iterations.size(); i++) {
				if (Low(district.iterations[i].name) == Low(district.iterations[iteration].carriedZones[l].first)) {
					carryIterationIndex = i;
				}
			}
			if (carryIterationIndex == -1) {
				continue;
			}

			for (int i = 0; i < district.iterations[iteration].carriedZones[l].second.size(); i++) {
				int zoneIndex = district.iterations[iteration].carriedZones[l].second[i];

				for (int j = 0; j < district.zones[zoneIndex].cells.size(); j++) {
					CarryCellCoordinate(district, entities, identifiers, originalEntities, cells, collapsedCells, entityIdentifierCounts, district.zones[district.iterations[iteration].carriedZones[l].second[i]].cells[j], iteration, carryIterationIndex);
				}
			}
		}

		// Carry Identifiers
		for (int l = 0; l < district.iterations[iteration].carriedIdentifiers.size(); l++) {
			int carryIterationIndex = -1;

			for (int i = 0; i < district.iterations.size(); i++) {
				if (Low(district.iterations[i].name) == Low(district.iterations[iteration].carriedIdentifiers[l].first)) {
					carryIterationIndex = i;
				}
			}
			if (carryIterationIndex == -1) {
				continue;
			}

			std::vector<int> carriedEntities;

			for (int i = 0; i < district.iterations[iteration].carriedIdentifiers[l].second.size(); i++) {
				std::string identifier = district.iterations[iteration].carriedIdentifiers[l].second[i].name;
				int			identifierIndex = -1;

				for (int j = 0; j < identifierCount; j++) {
					if (Low(identifier) == Low(identifiers[j].name)) {
						identifierIndex = j;
						break;
					}
				}
				if (identifierIndex == -1) {
					continue;
				}

				std::string value = district.iterations[iteration].carriedIdentifiers[l].second[i].value;

				for (int j = 0; j < entityCount; j++) {
					std::string entityValue = originalEntities[j].identifiersValues[identifierIndex].value;
					if (Low(value) == Low(entityValue)) {
						carriedEntities.push_back(j);
					}
				}
			}

			for (int i = 0; i < cols; i++) {
				for (int j = 0; j < rows; j++) {
					for (int k = 0; k < carriedEntities.size(); k++) {
						int cellEntityIndex = district.iterations[carryIterationIndex].cells[(j * cols) + i];

						if (cellEntityIndex == carriedEntities[k]) {
							CarryCellCoordinate(district, entities, identifiers, originalEntities, cells, collapsedCells, entityIdentifierCounts, { i, j }, iteration, carryIterationIndex);
						}
					}
				}
			}
		}
	}

	void SetZoneIdentifierConditions(const District& district, const std::vector<Entity>& entities, const std::vector<Entity>& originalEntities, std::vector<bool>& cells) {
		int cols = district.cols;

		int entityCount = entities.size();

		for (int i = 0; i < district.zones.size(); i++) {
			std::vector<int> restrictedEntities;
			std::vector<int> allowedEntities;

			for (int l = 0; l < entityCount; l++) {
				for (int j = 0; j < district.zones[i].negativeZoneIdentifierConditions.size(); j++) {
					for (int k = 0; k < district.zones[i].negativeZoneIdentifierConditions[j].size(); k++) {
						std::string identifierCondition = district.zones[i].negativeZoneIdentifierConditions[j][k];

						if (Low(identifierCondition) == Low(originalEntities[l].identifiersValues[j].value)) {
							restrictedEntities.push_back(l);
							break;
						}
					}
				}
				for (int j = 0; j < district.zones[i].positiveZoneIdentifierConditions.size(); j++) {
					for (int k = 0; k < district.zones[i].positiveZoneIdentifierConditions[j].size(); k++) {
						std::string identifierCondition = district.zones[i].positiveZoneIdentifierConditions[j][k];

						if (Low(identifierCondition) != Low(entities[l].identifiersValues[j].value)) {
							restrictedEntities.push_back(l);
							break;
						}
						else if (Low(identifierCondition) == Low(entities[l].identifiersValues[j].value)) {
							allowedEntities.push_back(l);
							break;
						}
					}
				}
			}

			for (int l = 0; l < allowedEntities.size(); l++) {
				for (int j = 0; j < restrictedEntities.size(); j++) {
					if (allowedEntities[l] == restrictedEntities[j]) {
						restrictedEntities.erase(restrictedEntities.begin() + j);
						j--;
					}
				}
			}

			for (int l = 0; l < district.zones[i].cells.size(); l++) {
				int cellIndex = ((district.zones[i].cells[l].y * cols) + district.zones[i].cells[l].x) * entityCount;

				for (int j = 0; j < restrictedEntities.size(); j++) {
					cells[cellIndex + restrictedEntities[j]] = false;
				}
			}
		}
	}

	void CollapseZoneIdentifiers(District& district, std::vector<Entity>& entities, const std::vector<Identifier>& identifiers, std::vector<std::vector<int>>& entityIdentifierCounts, int iteration) {
		int cols = district.cols;

		int identifierCount = identifiers.size();

		for (int i = 0; i < district.zones.size(); i++) {
			std::vector<int> collpasedIdentifierCellCounts(district.zones[i].collapsedIdentifiers.size(), 0);
			int occupiedCellsCount = district.zones[i].cells.size();

			std::vector<int> entityIndexes;

			for (int j = 0; j < district.zones[i].cells.size(); j++) {
				int cellIndex = (district.zones[i].cells[j].y * cols) + district.zones[i].cells[j].x;
				int entityIndex = district.iterations[iteration].cells[cellIndex];

				if (entityIndex == -1) {
					occupiedCellsCount--;
					continue;
				}

				bool already = false;
				for (int k = 0; k < entityIndexes.size(); k++) {
					if (entityIndex == entityIndexes[k]) {
						already = true;
					}
				}
				if (!already) {
					entityIndexes.push_back(entityIndex);
				}

				for (int k = 0; k < district.zones[i].collapsedIdentifiers.size(); k++) {
					std::string identifier = district.zones[i].collapsedIdentifiers[k].name;
					int	identifierIndex = -1;

					for (int j = 0; j < identifierCount; j++) {
						if (Low(identifier) == Low(identifiers[j].name)) {
							identifierIndex = j;
							break;
						}
					}
					if (identifierIndex == -1) {
						continue;
					}

					std::string entityValue = entities[entityIndex].identifiersValues[identifierIndex].value;
					std::string collapsedIdentifierValue = district.zones[i].collapsedIdentifiers[k].value;

					if (Low(entityValue) == Low(collapsedIdentifierValue)) {
						collpasedIdentifierCellCounts[k]++;
					}
				}
			}

			std::string zoneCollapsedIdentifier = "";

			if (occupiedCellsCount == 0) {
				district.iterations[iteration].zoneCollapsedIdentifiers[i] = zoneCollapsedIdentifier;
				continue;
			}

			for (int j = 0; j < collpasedIdentifierCellCounts.size(); j++) {
				if (collpasedIdentifierCellCounts[j] == occupiedCellsCount) {
					bool disabled = false;

					for (int k = 0; k < district.iterations[iteration].disabledZoneCollapseIdentifiers.size(); k++) {
						if (Low(district.zones[i].collapsedIdentifiers[j].name) == Low(district.iterations[iteration].disabledZoneCollapseIdentifiers[k].name)
							&& Low(district.zones[i].collapsedIdentifiers[j].value) == Low(district.iterations[iteration].disabledZoneCollapseIdentifiers[k].value)) {
							disabled = true;
						}
					}
					if (disabled) {
						continue;
					}

					int identifierIndex = -1;

					for (int k = 0; k < identifierCount; k++) {
						if (Low(identifiers[k].name) == Low(district.zones[i].collapsedIdentifiers[j].name)) {
							identifierIndex = k;
							break;
						}
					}

					if (!district.iterations[iteration].disableDropIterationCount) {
						for (int k = 0; k < entityIndexes.size(); k++) {
							if (entityIdentifierCounts[entityIndexes[k]][identifierIndex] == 0) {
								continue;
							}
							entityIdentifierCounts[entityIndexes[k]][identifierIndex]--;

							if (entityIdentifierCounts[entityIndexes[k]][identifierIndex] == 0) {
								entities[entityIndexes[k]].identifiersValues[identifierIndex].value = ".";
							}
						}
					}
					zoneCollapsedIdentifier = district.zones[i].collapsedIdentifiers[j].name + ", " + district.zones[i].collapsedIdentifiers[j].value;
					break;
				}
			}
			district.iterations[iteration].zoneCollapsedIdentifiers[i] = zoneCollapsedIdentifier;
		}
	}

	void FindCellsToCollapseAndTotalEntities(const District& district, const std::vector<Entity>& entities, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, int& cellsToCollapse, int& totalEntities) {
		int cellCount = district.rows * district.cols;
		int entityCount = entities.size();

		std::vector<bool> entityFound(entityCount, false);

		for (int i = 0; i < cellCount; i++) {
			bool occupied = false;
			for (int j = 0; j < entityCount; j++) {
				if (cells[(i * entityCount) + j]) {
					entityFound[j] = true;

					occupied = true;
				}
			}
			if (!occupied) {
				cellsToCollapse--;
			}
		}

		if (totalEntities == INT_MAX) {
			return;
		}

		for (int i = 0; i < entityFound.size(); i++) {
			if (!entityFound[i]) {
				totalEntities -= originalEntities[i].count;
			}
		}
	}

	std::vector<bool> RepeatRunningCollapses(const District& district, std::vector<Entity>& entities, const std::vector<Identifier>& identifiers,
		const std::vector<Entity>& originalEntities, const std::vector<bool>& cellsBase, std::vector<int>& collapsedCells, const std::vector<int>& defaultCollapsedCells,
		const std::vector<std::vector<int>>& entityIdentifierCounts, int iteration, int totalEntities, int cellsToCollapse, int depth, bool fullRandom, bool entitiesRandom) {

		int cellCount = district.rows * district.cols;
		int entityCount = entities.size();

		std::vector<bool> cellsBest((cellCount)*entityCount, 1);
		std::vector<bool> cellsWorking((cellCount)*entityCount, 1);

		Route route = { { 0 }, { 0 }, 0 };

		int currentCollapsedCellsCount = 0;
		int repeat = 0;

		while (true) {
			cellsWorking = cellsBase;

			int collapsedCellsCount = RunCollapses(district, entities, identifiers, originalEntities, cellsWorking, collapsedCells, defaultCollapsedCells, entityIdentifierCounts, iteration, totalEntities, cellsToCollapse, route, depth, fullRandom, entitiesRandom);

			if (collapsedCellsCount > currentCollapsedCellsCount) {
				cellsBest = cellsWorking;

				currentCollapsedCellsCount = collapsedCellsCount;
				if (collapsedCellsCount >= cellsToCollapse || currentCollapsedCellsCount >= totalEntities) {
					break;
				}
			}

			if (route.cellRoute.size() > depth) {
				if (route.cellRoute[depth] >= 1) {
					break;
				}
			}

			repeat++;
			if (!fullRandom) {
				route.cellRoute[0]++;
			}
			else if (repeat >= depth) {
				break;
			}
		}
		return cellsBest;
	}

	void RunIteration(District& district, std::vector<Entity>& entities, const std::vector<Identifier>& identifiers, const std::vector<Entity>& originalEntities,
		std::vector<std::vector<int>>& entityIdentifierCounts, int iteration, int totalEntities, int depth, bool fullRandom, bool entitiesRandom) {

		int cellCount = district.rows * district.cols;
		int entityCount = entities.size();

		std::vector<bool> cellsBase((cellCount)*entityCount, 1);

		std::vector<int> collapsedCells;
		std::vector<int> defaultCollapsedCells;

		collapsedCells.reserve(cellCount);
		defaultCollapsedCells.reserve(cellCount);

		int cellsToCollapse;

		district.iterations[iteration].zoneCollapsedIdentifiers = std::vector<std::string>(district.zones.size(), "");

		ApplyGlobalEntityConditions(district, entities, cellsBase);
		RemoveUnoccupiableCells(district, entities, cellsBase, cellsToCollapse);

		SetZoneIdentifierConditions(district, entities, originalEntities, cellsBase);

		DisableCellsZonesIdentifiers(district, entities, identifiers, originalEntities, cellsBase, iteration);
		CarryCellsZonesIdentifiers(district, entities, identifiers, originalEntities, cellsBase, collapsedCells, entityIdentifierCounts, iteration);

		FindCellsToCollapseAndTotalEntities(district, entities, originalEntities, cellsBase, cellsToCollapse, totalEntities);

		defaultCollapsedCells = collapsedCells;

		std::vector<bool> cellsBest = RepeatRunningCollapses(district, entities, identifiers, originalEntities, cellsBase, collapsedCells, defaultCollapsedCells, entityIdentifierCounts, iteration, totalEntities, cellsToCollapse, depth, fullRandom, entitiesRandom);

		SetDistrictCells(district, entities, cellsBest, iteration);

		CollapseZoneIdentifiers(district, entities, identifiers, entityIdentifierCounts, iteration);
	}

	void SetTotalEntities(const std::vector<Entity>& entities, int& totalEntities) {
		int entityCount = entities.size();

		totalEntities = 0;
		for (int i = 0; i < entityCount; i++) {
			if (entities[i].count == 0) {
				totalEntities = INT_MAX;
				return;
			}
			totalEntities += entities[i].count;
		}
	}

	void SetEntityIdentifierCounts(const std::vector<Entity>& entities, const std::vector<Identifier>& identifiers, std::vector<std::vector<int>>& entityIdentifierCounts) {
		int identifierCount = identifiers.size();

		std::vector<int> identifierCounts(identifierCount);

		for (int j = 0; j < identifierCount; j++) {
			identifierCounts[j] = identifiers[j].iterationCount;
		}

		for (int i = 0; i < entities.size(); i++) {
			entityIdentifierCounts.push_back(identifierCounts);
		}
	}

	void Setup(const std::vector<Entity>& entities, const std::vector<Identifier>& identifiers, std::vector<std::vector<int>>& entityIdentifierCounts, int& totalEntities) {
		SetupRandom();

		SetTotalEntities(entities, totalEntities);

		SetEntityIdentifierCounts(entities, identifiers, entityIdentifierCounts);
	}

	void EvaluateEOP_Config(EOP_Config& eop_config, int depth, bool fullRandom, bool entitiesRandom) {
		District district = eop_config.district;
		std::vector<Entity> entities = eop_config.entities.entities;

		std::vector<std::vector<int>> entityIdentifierCounts;

		int totalEntities;

		Setup(entities, eop_config.entities.identifiers, entityIdentifierCounts, totalEntities);

		for (int i = 0; i < district.iterations.size(); i++) {
			RunIteration(district, entities, eop_config.entities.identifiers, eop_config.entities.entities, entityIdentifierCounts, i, totalEntities, depth, fullRandom, entitiesRandom);
		}

		eop_config.district.iterations = district.iterations;
	}

}
