#include "evaluate.h"

#include "random.h"

namespace eop {

	std::string Low(std::string value) {
		std::string output = "";

		for (int i = 0; i < value.size(); i++) {
			output.push_back(tolower(value[i]));
		}
		return output;
	}

	void DropEntityCount(const District& district, std::vector<Entity>& entities, std::vector<bool>& cells, const std::vector<int>& collapsedCells, int entity) {
		int cellCount = district.rows * district.cols;
		int entityCount = entities.size();

		if (entities[entity].count > 1) {
			entities[entity].count--;
		}
		else if (entities[entity].count == 1) {
			entities[entity].count--;

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

	void CollapseCell(const District& district, std::vector<Entity>& entities, const std::vector<Identifier>& identifiers, const std::vector<Entity>& originalEntities, std::vector<bool>& cells, std::vector<int>& collapsedCells, const std::vector<std::vector<int>>& entityIdentifierCounts, int iteration, int cellIndex, int entity) {
		int cols = district.cols;

		int cellCount = district.rows * district.cols;
		int entityCount = entities.size();
		int identifierCount = identifiers.size();

		collapsedCells.push_back(cellIndex);

		// Collapse Cell into Entity.
		for (int i = 0; i < entityCount; i++) {
			if (i == entity) {
				continue;
			}
			cells[(cellIndex * entityCount) + i] = false;
		}

		DropEntityCount(district, entities, cells, collapsedCells, entity);

		// Get Current Zone
		int currentZone = -1;

		for (int i = 0; i < district.zones.size(); i++) {
			for (int j = 0; j < district.zones[i].cells.size(); j++) {
				if (cellIndex == (district.zones[i].cells[j].y * cols) + district.zones[i].cells[j].x) {
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

			for (int i = 0; i < district.zones[currentZone].cells.size(); i++) {
				int zoneCellIndex = (district.zones[currentZone].cells[i].y * cols) + district.zones[currentZone].cells[i].x;

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
				for (int i = 0; i < district.zones[currentZone].collapsedIdentifiers.size(); i++) {
					std::string identifier = district.zones[currentZone].collapsedIdentifiers[i].name;
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

					std::string entityValue = entities[entity].identifiersValues[identifierIndex].value;
					std::string collapsedIdentifierValue = district.zones[currentZone].collapsedIdentifiers[i].value;

					if (Low(entityValue) == Low(collapsedIdentifierValue)) {
						bool disabled = false;

						for (int k = 0; k < district.iterations[iteration].disabledZoneCollapseIdentifiers.size(); k++) {
							if (Low(district.zones[currentZone].collapsedIdentifiers[i].name) == Low(district.iterations[iteration].disabledZoneCollapseIdentifiers[k].name)
								&& Low(district.zones[currentZone].collapsedIdentifiers[i].value) == Low(district.iterations[iteration].disabledZoneCollapseIdentifiers[k].value)) {
								disabled = true;
							}
						}
						if (disabled) {
							continue;
						}

						int identifierIndex = -1;

						for (int k = 0; k < identifierCount; k++) {
							if (Low(identifiers[k].name) == Low(district.zones[currentZone].collapsedIdentifiers[i].name)) {
								identifierIndex = k;
								break;
							}
						}

						if (entityIdentifierCounts[entity][identifierIndex] == 0) {
							continue;
						}

						std::vector<int> restrictedEntities;

						for (int j = 0; j < entityCount; j++) {
							std::string identifierValue = district.zones[currentZone].collapsedIdentifiers[i].value;
							std::string entityIdentifierValue = entities[j].identifiersValues[identifierIndex].value;

							if (Low(entityIdentifierValue) != Low(identifierValue)) {
								restrictedEntities.push_back(j);
							}
						}

						for (int j = 0; j < district.zones[currentZone].cells.size(); j++) {
							int zoneCellIndex = (district.zones[currentZone].cells[j].y * cols) + district.zones[currentZone].cells[j].x;

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
		for (int i = 0; i < entities[entity].identifiersValues.size(); i++) {
			std::vector<int> entityConditions;

			for (int j = 0; j < entityCount; j++) {
				for (int k = 0; k < entities[entity].identifiersValues[i].conditions.size(); k++) {
					if (Low(originalEntities[j].identifiersValues[i].value) == Low(entities[entity].identifiersValues[i].conditions[k])) {
						entityConditions.push_back(j);
					}
				}
			}

			// Cell Conditions.
			for (int j = 0; j < identifiers[i].relitiveCellConditions.size(); j++) {
				int relMovX = identifiers[i].relitiveCellConditions[j].x;
				int relMovY = identifiers[i].relitiveCellConditions[j].y * cols;

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
					CollapseCell(district, entities, identifiers, originalEntities, cells, collapsedCells, entityIdentifierCounts, iteration, cellIndex + relMovX + relMovY, collapseEntityIndex);
				}
			}

			if (currentZone == -1) {
				continue;
			}

			// Zone Conditions.
			for (int l = 0; l < identifiers[i].relitiveZoneConditions.size(); l++) {
				int zone = currentZone + identifiers[i].relitiveZoneConditions[l];

				if (zone < 0 || zone >= district.zones.size()) {
					continue;
				}

				for (int j = 0; j < district.zones[zone].cells.size(); j++) {
					int index = (district.zones[zone].cells[j].y * cols) + district.zones[zone].cells[j].x;

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
						CollapseCell(district, entities, identifiers, originalEntities, cells, collapsedCells, entityIdentifierCounts, iteration, index, collapseEntityIndex);
					}
				}
			}
		}
	}

	std::vector<int> GetPossibleCellCollapses(const District& district, const std::vector<Entity>& entities, const std::vector<bool>& cells) {
		int cellCount = district.rows * district.cols;
		int entityCount = entities.size();

		int minSuperposCount = INT_MAX;
		std::vector<int> possibleCellIndexes;

		for (int i = 0; i < cellCount; i++) {
			int superposCount = 0;
			for (int j = 0; j < entityCount; j++) {
				superposCount += cells[(i * entityCount) + j];
			}

			if (superposCount <= minSuperposCount && superposCount > 1) {
				if (superposCount < minSuperposCount) {
					minSuperposCount = superposCount;
					possibleCellIndexes.clear();
				}
				possibleCellIndexes.push_back(i);
			}
		}

		return possibleCellIndexes;
	}

	struct Route {
		std::vector<int> cellRoute;
		std::vector<int> entityRoute;

		int layer;
	};

	bool Collapse(const District& district, std::vector<Entity>& entities, const std::vector<Identifier>& identifiers, const std::vector<Entity>& originalEntities, 
		std::vector<bool>& cells, std::vector<int>& collapsedCells, const std::vector<std::vector<int>>& entityIdentifierCounts, 
		int iteration, int totalEntities, int cellsToCollapse, Route& route, bool fullRandom, bool entitiesRandom) {

		int cellCount = district.rows * district.cols;
		int entityCount = entities.size();

		std::vector<int> possibleCellIndexes = GetPossibleCellCollapses(district, entities, cells);

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
			for (int i = 0; i < entityCount; i++) {
				if (cells[(cellIndex * entityCount) + i] == true) {
					possibleEntityIndexes.push_back(i);
				}
			}
			int entityIndex = possibleEntityIndexes[RandomIntRange(0, possibleEntityIndexes.size())];

			CollapseCell(district, entities, identifiers, originalEntities, cells, collapsedCells, entityIdentifierCounts, iteration, cellIndex, entityIndex);
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
		
		CollapseCell(district, entities, identifiers, originalEntities, cells, collapsedCells, entityIdentifierCounts, iteration, cellIndex, entityIndex);
		return false;
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
				for (int j = 0; j < district.zones[i].positiveZoneIdentifierConditions.size(); j++) {
					for (int k = 0; k < district.zones[i].negativeZoneIdentifierConditions[j].size(); k++) {
						std::string identifierCondition = district.zones[i].negativeZoneIdentifierConditions[j][k];

						if (Low(identifierCondition) == Low(originalEntities[l].identifiersValues[j].value)) {
							restrictedEntities.push_back(l);
							break;
						}
					}
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
		
		std::vector<bool> cellsBest		((cellCount) * entityCount, 1);
		std::vector<bool> cellsWorking	((cellCount) * entityCount, 1);

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

		std::vector<bool> cellsBase((cellCount) * entityCount, 1);

		std::vector<int> collapsedCells;
		std::vector<int> defaultCollapsedCells;

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
