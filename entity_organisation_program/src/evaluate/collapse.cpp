#include "collapse.h"

namespace eop {

	void DropEntityCount(EOP_Config& eop_config, std::vector<bool>& cells, const std::vector<int>& collapsedCells, int entity) {
		int cellCount = eop_config.district.rows * eop_config.district.cols;
		int entityCount = eop_config.entities.entities.size();

		if (eop_config.entities.entities[entity].count > 1) {
			eop_config.entities.entities[entity].count--;
		}
		else if (eop_config.entities.entities[entity].count == 1) {
			eop_config.entities.entities[entity].count--;

			for (int i = 0; i < cellCount; i++) {
				bool isCollapsed = false;

				for (int j = 0; j < collapsedCells.size(); j++) {
					if (collapsedCells[j] == i)
						isCollapsed = true;
				}
				if (isCollapsed) {
					continue;
				}
				cells[(i * entityCount) + entity] = false;
			}
		}
	}

	void CollapseCell(EOP_Config& eop_config, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, std::vector<int>& collapsedCells, const std::vector<std::vector<int>>& entityIdentifierCounts, int iteration, int cellIndex, int entity) {
		int cols = eop_config.district.cols;

		int cellCount = eop_config.district.rows * eop_config.district.cols;
		int entityCount = eop_config.entities.entities.size();
		int identifierCount = eop_config.entities.identifiers.size();

		collapsedCells.push_back(cellIndex);

		// Collapse Cell into Entity.
		for (int i = 0; i < entityCount; i++) {
			if (i == entity) {
				continue;
			}
			cells[(cellIndex * entityCount) + i] = false;
		}

		DropEntityCount(eop_config, cells, collapsedCells, entity);

		// Get Current Zone
		int currentZone = -1;

		for (int i = 0; i < eop_config.district.zones.size(); i++) {
			for (int j = 0; j < eop_config.district.zones[i].cells.size(); j++) {
				if (cellIndex == (eop_config.district.zones[i].cells[j].y * cols) + eop_config.district.zones[i].cells[j].x) {
					currentZone = i;
					break;
				}
			}
			if (currentZone != -1) {
				break;
			}
		}

		if (currentZone != -1) {
			bool zoneCollapsed = false;

			for (int i = 0; i < eop_config.district.zones[currentZone].cells.size(); i++) {
				int zoneCellIndex = (eop_config.district.zones[currentZone].cells[i].y * cols) + eop_config.district.zones[currentZone].cells[i].x;

				if (cellIndex == zoneCellIndex) {
					continue;
				}

				int count = 0;
				for (int j = 0; j < entityCount; j++) {
					if (cells[(zoneCellIndex * entityCount) + j] == true) {
						count++;
					}
				}
				if (count == 1) {
					zoneCollapsed = true;
					break;
				}
			}

			if (!zoneCollapsed) {
				// Collapse Zone
				for (int i = 0; i < eop_config.district.zones[currentZone].collapsedIdentifiers.size(); i++) {
					std::string identifier = eop_config.district.zones[currentZone].collapsedIdentifiers[i].name;
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

					std::string entityValue = eop_config.entities.entities[entity].identifiersValues[identifierIndex].value;
					std::string collapsedIdentifierValue = eop_config.district.zones[currentZone].collapsedIdentifiers[i].value;

					if (Low(entityValue) == Low(collapsedIdentifierValue)) {
						bool disabled = false;

						for (int k = 0; k < eop_config.district.iterations[iteration].disabledZoneCollapseIdentifiers.size(); k++) {
							if (Low(eop_config.district.zones[currentZone].collapsedIdentifiers[i].name) == Low(eop_config.district.iterations[iteration].disabledZoneCollapseIdentifiers[k].name)
								&& Low(eop_config.district.zones[currentZone].collapsedIdentifiers[i].value) == Low(eop_config.district.iterations[iteration].disabledZoneCollapseIdentifiers[k].value)) {
								disabled = true;
							}
						}
						if (disabled) {
							continue;
						}

						int identifierIndex = -1;

						for (int k = 0; k < identifierCount; k++) {
							if (Low(eop_config.entities.identifiers[k].name) == Low(eop_config.district.zones[currentZone].collapsedIdentifiers[i].name)) {
								identifierIndex = k;
								break;
							}
						}

						if (entityIdentifierCounts[entity][identifierIndex] == 0) {
							continue;
						}

						std::vector<int> restrictedEntities;

						for (int j = 0; j < entityCount; j++) {
							std::string identifierValue = eop_config.district.zones[currentZone].collapsedIdentifiers[i].value;
							std::string entityIdentifierValue = eop_config.entities.entities[j].identifiersValues[identifierIndex].value;

							if (Low(entityIdentifierValue) != Low(identifierValue)) {
								restrictedEntities.push_back(j);
							}
						}

						for (int j = 0; j < eop_config.district.zones[currentZone].cells.size(); j++) {
							int zoneCellIndex = (eop_config.district.zones[currentZone].cells[j].y * cols) + eop_config.district.zones[currentZone].cells[j].x;

							if (cellIndex == zoneCellIndex) {
								continue;
							}

							for (int k = 0; k < restrictedEntities.size(); k++) {
								cells[(zoneCellIndex * entityCount) + restrictedEntities[k]] = false;
							}
						}
						break;
					}
				}
			}
		}

		// Check Cells.
		for (int i = 0; i < eop_config.entities.entities[entity].identifiersValues.size(); i++) {
			std::vector<int> entityConditions;

			for (int j = 0; j < entityCount; j++) {
				for (int k = 0; k < eop_config.entities.entities[entity].identifiersValues[i].conditions.size(); k++) {
					if (Low(originalEntities[j].identifiersValues[i].value) == Low(eop_config.entities.entities[entity].identifiersValues[i].conditions[k])) {
						entityConditions.push_back(j);
					}
				}
			}

			// Cell Conditions.
			for (int j = 0; j < eop_config.entities.identifiers[i].relitiveCellConditions.size(); j++) {
				int relMovX = eop_config.entities.identifiers[i].relitiveCellConditions[j].x;
				int relMovY = eop_config.entities.identifiers[i].relitiveCellConditions[j].y * cols;

				if (cellIndex + relMovX + relMovY < 0 || cellIndex + relMovX + relMovY > cellCount - 1) {
					continue;
				}
				if (ceil((cellIndex + relMovX) / cols) != ceil(cellIndex / cols)) {
					continue;
				}

				bool already = false;
				for (int k = 0; k < collapsedCells.size(); k++) {
					if ((cellIndex + relMovX + relMovY) == collapsedCells[k]) {
						already = true;
						break;
					}
				}
				if (already)
					continue;

				int cellPeopleIndex = ((cellIndex + relMovX + relMovY) * entityCount);

				for (int k = 0; k < entityConditions.size(); k++) {
					cells[cellPeopleIndex + entityConditions[k]] = false;
				}

				int count = 0;
				int collapseEntityIndex = -1;

				for (int k = 0; k < entityCount; k++) {
					if (cells[cellPeopleIndex + k] == true) {
						count++;
						collapseEntityIndex = k;
					}
				}
				if (count == 1) {
					CollapseCell(eop_config, originalEntities, cells, collapsedCells, entityIdentifierCounts, iteration, cellIndex + relMovX + relMovY, collapseEntityIndex);
				}
			}

			if (currentZone == -1) {
				continue;
			}

			// Zone Conditions.
			for (int l = 0; l < eop_config.entities.identifiers[i].relitiveZoneConditions.size(); l++) {
				int zone = currentZone + eop_config.entities.identifiers[i].relitiveZoneConditions[l];

				if (zone < 0 || zone >= eop_config.district.zones.size()) {
					continue;
				}

				for (int j = 0; j < eop_config.district.zones[zone].cells.size(); j++) {
					int index = (eop_config.district.zones[zone].cells[j].y * cols) + eop_config.district.zones[zone].cells[j].x;

					bool already = false;
					for (int k = 0; k < collapsedCells.size(); k++) {
						if (index == collapsedCells[k]) {
							already = true;
							break;
						}
					}
					if (already) {
						continue;
					}

					int cellPeopleIndex = (index * entityCount);

					for (int k = 0; k < entityConditions.size(); k++) {
						cells[cellPeopleIndex + entityConditions[k]] = false;
					}

					int count = 0;
					int collapseEntityIndex = -1;

					for (int k = 0; k < entityCount; k++) {
						if (cells[cellPeopleIndex + k] == true) {
							count++;
							collapseEntityIndex = k;
						}
					}
					if (count == 1) {
						CollapseCell(eop_config, originalEntities, cells, collapsedCells, entityIdentifierCounts, iteration, index, collapseEntityIndex);
					}
				}
			}
		}
	}

	std::vector<int> GetPossibleCellCollapses(const EOP_Config& eop_config, const std::vector<bool>& cells,const std::vector<int>& collapsedCells) {
		int cellCount = eop_config.district.rows * eop_config.district.cols;
		int entityCount = eop_config.entities.entities.size();

		int minSuperposCount = INT_MAX;

		std::vector<int> possibleCellIndexes;
		possibleCellIndexes.reserve(cellCount);

		for (int i = 0; i < cellCount; i++) {
			int superposCount = 0;
			for (int j = 0; j < entityCount; j++) {
				superposCount += cells[(i * entityCount) + j];
			}

			bool collapsed = false;
			if (superposCount == 1) {
				for (int j = 0; j < collapsedCells.size(); j++) {
					if (collapsedCells[j] == i) {
						collapsed = true;
					}
				}
				if (collapsed) {
					continue;
				}
			}

			if (superposCount <= minSuperposCount && superposCount >= 1) {
				if (superposCount < minSuperposCount) {
					minSuperposCount = superposCount;

					possibleCellIndexes.clear();
					possibleCellIndexes.reserve(cellCount);
				}
				possibleCellIndexes.push_back(i);
			}
		}

		return possibleCellIndexes;
	}

	bool Collapse(EOP_Config& eop_config, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, std::vector<int>& collapsedCells, const std::vector<std::vector<int>>& entityIdentifierCounts, int iteration, int totalEntities, int cellsToCollapse, Route& route, bool fullRandom, bool entitiesRandom) {

		int cellCount = eop_config.district.rows * eop_config.district.cols;
		int entityCount = eop_config.entities.entities.size();

		std::vector<int> possibleCellIndexes = GetPossibleCellCollapses(eop_config, cells, collapsedCells);

		if (possibleCellIndexes.size() == 0 || collapsedCells.size() >= totalEntities || collapsedCells.size() >= cellsToCollapse) {
			EOP_LOG("Collapsed Iteration " << iteration << ": " << collapsedCells.size() << " / " << totalEntities << " / " << cellsToCollapse << "\n");
			return true;
		}

		if (route.layer >= route.cellRoute.size()) {
			route.cellRoute.push_back(0);
		}
		if (route.layer >= route.entityRoute.size()) {
			route.entityRoute.push_back(0);
		}

		if (fullRandom) {
			int cellIndex = possibleCellIndexes[RandomIntRange(0, possibleCellIndexes.size())];

			std::vector<int> possibleEntityIndexes;
			possibleEntityIndexes.reserve(entityCount);

			for (int i = 0; i < entityCount; i++) {
				if (cells[(cellIndex * entityCount) + i] == true) {
					possibleEntityIndexes.push_back(i);
				}
			}
			int entityIndex = possibleEntityIndexes[RandomIntRange(0, possibleEntityIndexes.size())];

			CollapseCell(eop_config, originalEntities, cells, collapsedCells, entityIdentifierCounts, iteration, cellIndex, entityIndex);
			return false;
		}

		if (route.cellRoute[route.layer] >= possibleCellIndexes.size()) {
			route.cellRoute[route.layer] = 0;

			if (entitiesRandom) {
				route.cellRoute[route.layer + 1]++;
			}
			else {
				route.entityRoute[route.layer]++;
			}
		}
		int cellIndex = possibleCellIndexes[route.cellRoute[route.layer]];

		std::vector<int> possibleEntityIndexes;
		possibleEntityIndexes.reserve(entityCount);

		for (int i = 0; i < entityCount; i++) {
			if (cells[(cellIndex * entityCount) + i] == true) {
				possibleEntityIndexes.push_back(i);
			}
		}

		int entityIndex = -1;
		if (entitiesRandom) {
			entityIndex = possibleEntityIndexes[RandomIntRange(0, possibleEntityIndexes.size())];
		}
		else {
			if (route.entityRoute[route.layer] >= possibleEntityIndexes.size()) {
				route.entityRoute[route.layer] = 0;

				route.cellRoute[route.layer + 1]++;
			}
			entityIndex = possibleEntityIndexes[route.entityRoute[route.layer]];
		}

		CollapseCell(eop_config, originalEntities, cells, collapsedCells, entityIdentifierCounts, iteration, cellIndex, entityIndex);

		return false;
	}

}
