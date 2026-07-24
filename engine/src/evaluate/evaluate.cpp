#include "evaluate.h"

namespace eop {

	void SetEntityCounts(EOP_Config& eop_config, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, const std::vector<int>& collapsedCells) {
		int entityCount = eop_config.entities.entities.size();

		for (int i = 0; i < entityCount; i++) {
			eop_config.entities.entities[i].count = originalEntities[i].count;
		}

		for (int i = 0; i < collapsedCells.size(); i++) {
			int entityId = -1;

			for (int j = 0; j < entityCount; j++) {
				if (cells[(collapsedCells[i] * entityCount) + j]) {
					entityId = j;
				}
			}
			if (entityId != -1) {
				DropEntityCount(eop_config, cells, collapsedCells, entityId);
			}
		}
	}

	int RunCollapses(EOP_Config& eop_config, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, std::vector<int>& collapsedCells, const std::vector<int>& defaultCollapsedCells, const std::vector<std::vector<int>>& entityIdentifierCounts, int iteration, int totalEntities, int cellsToCollapse, Route& route, int depth, bool fullRandom, bool entitiesRandom) {

		collapsedCells = defaultCollapsedCells;
		SetEntityCounts(eop_config, originalEntities, cells, collapsedCells);

		route.layer = 0;

		bool done = false;
		while (!done) {
			done = Collapse(eop_config, originalEntities, cells, collapsedCells, entityIdentifierCounts, iteration, totalEntities, cellsToCollapse, route, fullRandom, entitiesRandom);

			route.layer++;

			if (route.layer >= depth) {
				fullRandom = true;
			}
		}

		return collapsedCells.size();
	}

	void SetDistrictCells(EOP_Config& eop_config, std::vector<bool>& cells, int iteration) {
		int cellCount = eop_config.district.rows * eop_config.district.cols;
		int entityCount = eop_config.entities.entities.size();

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
			eop_config.district.iterations[iteration].cells.push_back(entity);
		}
	}

	void ApplyGlobalEntityConditions(const EOP_Config& eop_config, std::vector<bool>& cells) {
		int cols = eop_config.district.cols;

		int entityCount = eop_config.entities.entities.size();

		for (int i = 0; i < entityCount; i++) {
			for (int j = 0; j < eop_config.entities.entities[i].entityCellConditions.size(); j++) {
				int x = eop_config.entities.entities[i].entityCellConditions[j].x;
				int y = eop_config.entities.entities[i].entityCellConditions[j].y;

				int index = (y * cols) + x;

				cells[index * entityCount + i] = false;
			}
			for (int j = 0; j < eop_config.entities.entities[i].entityZoneConditions.size(); j++) {
				int zone = eop_config.entities.entities[i].entityZoneConditions[j];
				for (int k = 0; k < eop_config.district.zones[zone].cells.size(); k++) {
					int x = eop_config.district.zones[zone].cells[k].x;
					int y = eop_config.district.zones[zone].cells[k].y;

					int index = (y * cols) + x;

					cells[index * entityCount + i] = false;
				}
			}
		}
	}

	void RemoveUnoccupiableCells(const EOP_Config& eop_config, std::vector<bool>& cells, int& cellsToCollapse) {
		int cellCount = eop_config.district.rows * eop_config.district.cols;
		int entityCount = eop_config.entities.entities.size();
		
		cellsToCollapse = cellCount;

		for (int i = 0; i < cellCount; i++) {
			if (!eop_config.district.occupiableCells[i]) {
				for (int j = 0; j < entityCount; j++) {
					cells[(i * entityCount) + j] = false;
				}
			}
		}
	}

	void DisableCellCoordinate(const EOP_Config& eop_config, std::vector<bool>& cells, vec2 cellCoord) {
		int cols = eop_config.district.cols;

		int entityCount = eop_config.entities.entities.size();

		int cellIndex = ((cellCoord.y * cols) + cellCoord.x) * entityCount;

		for (int j = 0; j < entityCount; j++) {
			cells[cellIndex + j] = false;
		}
	}

	void DisableCellsZonesIdentifiers(const EOP_Config& eop_config, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, int iteration) {
		int cellCount = eop_config.district.rows * eop_config.district.cols;
		int entityCount = eop_config.entities.entities.size();
		int identifierCount = eop_config.entities.identifiers.size();

		// Disable Cells
		for (int i = 0; i < eop_config.district.iterations[iteration].disabledCells.size(); i++) {
			DisableCellCoordinate(eop_config, cells, eop_config.district.iterations[iteration].disabledCells[i]);
		}

		// Disable Zones
		for (int i = 0; i < eop_config.district.iterations[iteration].disabledZones.size(); i++) {
			int zoneIndex = eop_config.district.iterations[iteration].disabledZones[i] - 1;

			for (int j = 0; j < eop_config.district.zones[zoneIndex].cells.size(); j++) {
				DisableCellCoordinate(eop_config, cells, eop_config.district.zones[zoneIndex].cells[j]);
			}
		}

		// Disable Identifiers
		std::vector<int> disabledEntities;

		for (int i = 0; i < eop_config.district.iterations[iteration].disabledIdentifiers.size(); i++) {
			std::string identifier = eop_config.district.iterations[iteration].disabledIdentifiers[i].name;
			int			identifierIndex = -1;

			for (int j = 0; j < identifierCount; j++) {
				if (Low(identifier) == Low(eop_config.entities.identifiers[j].name)) {
					identifierIndex = j;
					break;
				}
			}
			if (identifierIndex == -1) {
				continue;
			}

			std::string value = eop_config.district.iterations[iteration].disabledIdentifiers[i].value;

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

	void CarryCellCoordinate(EOP_Config& eop_config, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, std::vector<int>& collapsedCells, const std::vector<std::vector<int>>& entityIdentifierCounts, vec2 cellCoord, int iteration, int carryIterationIndex) {

		int cols = eop_config.district.cols;
		int entityCount = eop_config.entities.entities.size();

		int cellIndex = (cellCoord.y * cols) + cellCoord.x;
		int entity = -1;

		for (int j = 0; j < entityCount; j++) {
			if (j != eop_config.district.iterations[carryIterationIndex].cells[cellIndex]) {
				cells[(cellIndex * entityCount) + j] = false;
			}
			else {
				cells[(cellIndex * entityCount) + j] = true;
			}
		}
		entity = eop_config.district.iterations[carryIterationIndex].cells[cellIndex];

		if (entity == -1) {
			return;
		}

		CollapseCell(eop_config, originalEntities, cells, collapsedCells, entityIdentifierCounts, iteration, cellIndex, entity);
	}

	void CarryCellsZonesIdentifiers(EOP_Config& eop_config, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, std::vector<int>& collapsedCells, const std::vector<std::vector<int>>& entityIdentifierCounts, int iteration) {

		int rows = eop_config.district.rows;
		int cols = eop_config.district.cols;

		int entityCount = eop_config.entities.entities.size();
		int identifierCount = eop_config.entities.identifiers.size();

		if (iteration == 0)
			return;

		// Carry Cells
		for (int l = 0; l < eop_config.district.iterations[iteration].carriedCells.size(); l++) {
			int carryIterationIndex = -1;

			for (int i = 0; i < eop_config.district.iterations.size(); i++) {
				if (Low(eop_config.district.iterations[i].name) == Low(eop_config.district.iterations[iteration].carriedCells[l].first)) {
					carryIterationIndex = i;
				}
			}
			if (carryIterationIndex == -1) {
				continue;
			}

			for (int i = 0; i < eop_config.district.iterations[iteration].carriedCells[l].second.size(); i++) {
				CarryCellCoordinate(eop_config, originalEntities, cells, collapsedCells, entityIdentifierCounts, eop_config.district.iterations[iteration].carriedCells[l].second[i], iteration, carryIterationIndex);
			}
		}

		// Carry Zones
		for (int l = 0; l < eop_config.district.iterations[iteration].carriedZones.size(); l++) {
			int carryIterationIndex = -1;

			for (int i = 0; i < eop_config.district.iterations.size(); i++) {
				if (Low(eop_config.district.iterations[i].name) == Low(eop_config.district.iterations[iteration].carriedZones[l].first)) {
					carryIterationIndex = i;
				}
			}
			if (carryIterationIndex == -1) {
				continue;
			}

			for (int i = 0; i < eop_config.district.iterations[iteration].carriedZones[l].second.size(); i++) {
				int zoneIndex = eop_config.district.iterations[iteration].carriedZones[l].second[i] - 1;

				for (int j = 0; j < eop_config.district.zones[zoneIndex].cells.size(); j++) {
					CarryCellCoordinate(eop_config, originalEntities, cells, collapsedCells, entityIdentifierCounts, eop_config.district.zones[zoneIndex].cells[j], iteration, carryIterationIndex);
				}
			}
		}

		// Carry Identifiers
		for (int l = 0; l < eop_config.district.iterations[iteration].carriedIdentifiers.size(); l++) {
			int carryIterationIndex = -1;

			for (int i = 0; i < eop_config.district.iterations.size(); i++) {
				if (Low(eop_config.district.iterations[i].name) == Low(eop_config.district.iterations[iteration].carriedIdentifiers[l].first)) {
					carryIterationIndex = i;
				}
			}
			if (carryIterationIndex == -1) {
				continue;
			}

			std::vector<int> carriedEntities;

			for (int i = 0; i < eop_config.district.iterations[iteration].carriedIdentifiers[l].second.size(); i++) {
				std::string identifier = eop_config.district.iterations[iteration].carriedIdentifiers[l].second[i].name;
				int identifierIndex = -1;

				for (int j = 0; j < identifierCount; j++) {
					if (Low(identifier) == Low(eop_config.entities.identifiers[j].name)) {
						identifierIndex = j;
						break;
					}
				}
				if (identifierIndex == -1) {
					continue;
				}

				std::string value = eop_config.district.iterations[iteration].carriedIdentifiers[l].second[i].value;

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
						int cellEntityIndex = eop_config.district.iterations[carryIterationIndex].cells[(j * cols) + i];

						if (cellEntityIndex == carriedEntities[k]) {
							CarryCellCoordinate(eop_config, originalEntities, cells, collapsedCells, entityIdentifierCounts, { i, j }, iteration, carryIterationIndex);
						}
					}
				}
			}
		}
	}

	void SetZoneIdentifierConditions(const EOP_Config& eop_config, const std::vector<Entity>& originalEntities, std::vector<bool>& cells) {
		int cols = eop_config.district.cols;

		int entityCount = eop_config.entities.entities.size();

		for (int i = 0; i < eop_config.district.zones.size(); i++) {
			std::vector<int> restrictedEntities;
			std::vector<int> allowedEntities;

			for (int l = 0; l < entityCount; l++) {
				for (int j = 0; j < eop_config.district.zones[i].negativeZoneIdentifierConditions.size(); j++) {
					for (int k = 0; k < eop_config.district.zones[i].negativeZoneIdentifierConditions[j].size(); k++) {
						std::string identifierCondition = eop_config.district.zones[i].negativeZoneIdentifierConditions[j][k];

						if (Low(identifierCondition) == Low(originalEntities[l].identifiersValues[j].value)) {
							restrictedEntities.push_back(l);
							break;
						}
					}
				}
				for (int j = 0; j < eop_config.district.zones[i].positiveZoneIdentifierConditions.size(); j++) {
					for (int k = 0; k < eop_config.district.zones[i].positiveZoneIdentifierConditions[j].size(); k++) {
						std::string identifierCondition = eop_config.district.zones[i].positiveZoneIdentifierConditions[j][k];

						if (Low(identifierCondition) != Low(eop_config.entities.entities[l].identifiersValues[j].value)) {
							restrictedEntities.push_back(l);
							break;
						}
						else if (Low(identifierCondition) == Low(eop_config.entities.entities[l].identifiersValues[j].value)) {
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

			for (int l = 0; l < eop_config.district.zones[i].cells.size(); l++) {
				int cellIndex = ((eop_config.district.zones[i].cells[l].y * cols) + eop_config.district.zones[i].cells[l].x) * entityCount;

				for (int j = 0; j < restrictedEntities.size(); j++) {
					cells[cellIndex + restrictedEntities[j]] = false;
				}
			}
		}
	}

	void CollapseZoneIdentifiers(EOP_Config& eop_config, std::vector<std::vector<int>>& entityIdentifierCounts, int iteration) {
		int cols = eop_config.district.cols;

		int identifierCount = eop_config.entities.identifiers.size();

		for (int i = 0; i < eop_config.district.zones.size(); i++) {
			std::vector<int> collpasedIdentifierCellCounts(eop_config.district.zones[i].collapsedIdentifiers.size(), 0);
			int occupiedCellsCount = eop_config.district.zones[i].cells.size();

			std::vector<int> entityIndexes;

			for (int j = 0; j < eop_config.district.zones[i].cells.size(); j++) {
				int cellIndex = (eop_config.district.zones[i].cells[j].y * cols) + eop_config.district.zones[i].cells[j].x;
				int entityIndex = eop_config.district.iterations[iteration].cells[cellIndex];

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

				for (int k = 0; k < eop_config.district.zones[i].collapsedIdentifiers.size(); k++) {
					std::string identifier = eop_config.district.zones[i].collapsedIdentifiers[k].name;
					int	identifierIndex = -1;

					for (int j = 0; j < identifierCount; j++) {
						if (Low(identifier) == Low(eop_config.entities.identifiers[j].name)) {
							identifierIndex = j;
							break;
						}
					}
					if (identifierIndex == -1) {
						continue;
					}

					std::string entityValue = eop_config.entities.entities[entityIndex].identifiersValues[identifierIndex].value;
					std::string collapsedIdentifierValue = eop_config.district.zones[i].collapsedIdentifiers[k].value;

					if (Low(entityValue) == Low(collapsedIdentifierValue)) {
						collpasedIdentifierCellCounts[k]++;
					}
				}
			}

			std::string zoneCollapsedIdentifier = "";

			if (occupiedCellsCount == 0) {
				eop_config.district.iterations[iteration].zoneCollapsedIdentifiers[i] = zoneCollapsedIdentifier;
				continue;
			}

			for (int j = 0; j < collpasedIdentifierCellCounts.size(); j++) {
				if (collpasedIdentifierCellCounts[j] == occupiedCellsCount) {
					bool disabled = false;

					for (int k = 0; k < eop_config.district.iterations[iteration].disabledZoneCollapseIdentifiers.size(); k++) {
						if (Low(eop_config.district.zones[i].collapsedIdentifiers[j].name) == Low(eop_config.district.iterations[iteration].disabledZoneCollapseIdentifiers[k].name)
							&& Low(eop_config.district.zones[i].collapsedIdentifiers[j].value) == Low(eop_config.district.iterations[iteration].disabledZoneCollapseIdentifiers[k].value)) {
							disabled = true;
						}
					}
					if (disabled) {
						continue;
					}

					int identifierIndex = -1;

					for (int k = 0; k < identifierCount; k++) {
						if (Low(eop_config.entities.identifiers[k].name) == Low(eop_config.district.zones[i].collapsedIdentifiers[j].name)) {
							identifierIndex = k;
							break;
						}
					}

					if (!eop_config.district.iterations[iteration].disableDropIterationCount) {
						for (int k = 0; k < entityIndexes.size(); k++) {
							if (entityIdentifierCounts[entityIndexes[k]][identifierIndex] == 0) {
								continue;
							}
							entityIdentifierCounts[entityIndexes[k]][identifierIndex]--;

							if (entityIdentifierCounts[entityIndexes[k]][identifierIndex] == 0) {
								eop_config.entities.entities[entityIndexes[k]].identifiersValues[identifierIndex].value = ".";
							}
						}
					}
					zoneCollapsedIdentifier = eop_config.district.zones[i].collapsedIdentifiers[j].name + ", " + eop_config.district.zones[i].collapsedIdentifiers[j].value;
					break;
				}
			}
			eop_config.district.iterations[iteration].zoneCollapsedIdentifiers[i] = zoneCollapsedIdentifier;
		}
	}

	void FindCellsToCollapseAndTotalEntities(const EOP_Config& eop_config, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, int& cellsToCollapse, int& totalEntities) {
		int cellCount = eop_config.district.rows * eop_config.district.cols;
		int entityCount = eop_config.entities.entities.size();

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

	std::vector<bool> RepeatRunningCollapses(EOP_Config& eop_config, const std::vector<Entity>& originalEntities, const std::vector<bool>& cellsBase, std::vector<int>& collapsedCells, const std::vector<int>& defaultCollapsedCells, const std::vector<std::vector<int>>& entityIdentifierCounts, int iteration, int totalEntities, int cellsToCollapse, int depth, bool fullRandom, bool entitiesRandom) {

		int cellCount = eop_config.district.rows * eop_config.district.cols;
		int entityCount = eop_config.entities.entities.size();

		std::vector<bool> cellsBest((cellCount)*entityCount, 1);
		std::vector<bool> cellsWorking((cellCount)*entityCount, 1);

		Route route = { { 0 }, { 0 }, 0 };

		int currentCollapsedCellsCount = 0;
		int repeat = 0;

		while (true) {
			cellsWorking = cellsBase;

			int collapsedCellsCount = RunCollapses(eop_config, originalEntities, cellsWorking, collapsedCells, defaultCollapsedCells, entityIdentifierCounts, iteration, totalEntities, cellsToCollapse, route, depth, fullRandom, entitiesRandom);

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

	void RunIteration(EOP_Config& eop_config, const std::vector<Entity>& originalEntities, std::vector<std::vector<int>>& entityIdentifierCounts, int iteration, int totalEntities, int depth, bool fullRandom, bool entitiesRandom) {

		int cellCount = eop_config.district.rows * eop_config.district.cols;
		int entityCount = eop_config.entities.entities.size();

		std::vector<bool> cellsBase(cellCount * entityCount, 1);

		std::vector<int> collapsedCells;
		collapsedCells.reserve(cellCount);

		std::vector<int> defaultCollapsedCells;
		defaultCollapsedCells.reserve(cellCount);

		int cellsToCollapse;

		eop_config.district.iterations[iteration].zoneCollapsedIdentifiers = std::vector<std::string>(eop_config.district.zones.size(), "");

		ApplyGlobalEntityConditions(eop_config, cellsBase);
		RemoveUnoccupiableCells(eop_config, cellsBase, cellsToCollapse);

		SetZoneIdentifierConditions(eop_config, originalEntities, cellsBase);

		DisableCellsZonesIdentifiers(eop_config, originalEntities, cellsBase, iteration);
		CarryCellsZonesIdentifiers(eop_config, originalEntities, cellsBase, collapsedCells, entityIdentifierCounts, iteration);

		FindCellsToCollapseAndTotalEntities(eop_config, originalEntities, cellsBase, cellsToCollapse, totalEntities);

		defaultCollapsedCells = collapsedCells;

		std::vector<bool> cellsBest = RepeatRunningCollapses(eop_config, originalEntities, cellsBase, collapsedCells, defaultCollapsedCells, entityIdentifierCounts, iteration, totalEntities, cellsToCollapse, depth, fullRandom, entitiesRandom);

		SetDistrictCells(eop_config, cellsBest, iteration);

		CollapseZoneIdentifiers(eop_config, entityIdentifierCounts, iteration);
	}

	void SetTotalEntities(const EOP_Config& eop_config, int& totalEntities) {
		int entityCount = eop_config.entities.entities.size();

		totalEntities = 0;
		for (int i = 0; i < entityCount; i++) {
			if (eop_config.entities.entities[i].count == 0) {
				totalEntities = INT_MAX;
				return;
			}
			totalEntities += eop_config.entities.entities[i].count;
		}
	}

	void SetEntityIdentifierCounts(const EOP_Config& eop_config, std::vector<std::vector<int>>& entityIdentifierCounts) {
		int identifierCount = eop_config.entities.identifiers.size();

		std::vector<int> identifierCounts(identifierCount);

		for (int j = 0; j < identifierCount; j++) {
			identifierCounts[j] = eop_config.entities.identifiers[j].iterationCount;
		}

		for (int i = 0; i < eop_config.entities.entities.size(); i++) {
			entityIdentifierCounts.push_back(identifierCounts);
		}
	}

	void EvaluateEOP_Config(EOP_Config& eop_config, int depth, bool fullRandom, bool entitiesRandom) {
		EOP_Config eop_config_copy = eop_config;

		std::vector<std::vector<int>> entityIdentifierCounts;

		int totalEntities;

		SetupRandom();

		SetTotalEntities(eop_config_copy, totalEntities);

		SetEntityIdentifierCounts(eop_config_copy, entityIdentifierCounts);

		for (int i = 0; i < eop_config_copy.district.iterations.size(); i++) {
			RunIteration(eop_config_copy, eop_config.entities.entities, entityIdentifierCounts, i, totalEntities, depth, fullRandom, entitiesRandom);
		}
		
		eop_config.district.iterations = eop_config_copy.district.iterations;
	}

}
