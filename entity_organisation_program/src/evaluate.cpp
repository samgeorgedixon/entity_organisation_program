#include "evaluate.h"

#include "random.h"

namespace eop {

	int m_rows, m_cols;
	int m_cellCount, m_entityCount, m_identifierCount;
	int m_repeats;

	int m_totalEntities;
	int m_cellsToCollapse;

	District				m_district;
	std::vector<Entity>		m_entities;
	std::vector<Identifier> m_identifiers;

	std::vector<Entity> m_originalEntities;

	std::vector<int> m_collapsedCells;
	std::vector<int> m_defaultCollapsedCells;

	std::vector<std::vector<int>> entityIdentifierCounts;

	std::string Low(std::string value) {
		std::string output = "";

		for (int i = 0; i < value.size(); i++) {
			output.push_back(tolower(value[i]));
		}
		return output;
	}

	void PrintEOP_ConfigIteration(EOP_Config& eop_config, int identifierId, int iteration) {
		LOG("Row: - ");

		int j = 0;
		for (int i = 0; i < eop_config.district.rows * eop_config.district.cols; i++) {
			int id = eop_config.district.iterations[iteration].cells[i];

			if (id == -1) {
				LOG("NA : ");
			}
			else {
				LOG(eop_config.entities.entities[id].identifiersValues[identifierId].value << " : ");
			}

			j++;
			if (j >= eop_config.district.cols && i + 1 < eop_config.district.rows * eop_config.district.cols) {
				LOG("\nRow: - ");
				j = 0;
			}
		}
		LOG("\n");
	}

	void PrintEOP_Config(EOP_Config& eop_config, int identifierId) {
		for (int i = 0; i < eop_config.district.iterations.size(); i++) {
			PrintEOP_ConfigIteration(eop_config, identifierId, i);

			if (i != eop_config.district.iterations.size() - 1) {
				LOG("---\n");
			}
		}
	}

	void DropEntityCount(std::vector<bool>& cells, int entity) {
		if (m_entities[entity].count > 1) {
			m_entities[entity].count--;
		}
		else if (m_entities[entity].count == 1) {
			m_entities[entity].count--;

			for (int i = 0; i < m_cellCount; i++) {
				bool isCollapsed = false;

				for (int j = 0; j < m_collapsedCells.size(); j++) {
					if (m_collapsedCells[j] == i)
						isCollapsed = true;
				}
				if (isCollapsed) {
					continue;
				}
				cells[(i * m_entityCount) + entity] = false;
			}
		}
	}

	void SetEntityCounts(std::vector<bool>& cells) {
		for (int i = 0; i < m_collapsedCells.size(); i++) {
			int entityId = -1;

			for (int j = 0; j < m_entityCount; j++) {
				if (cells[(m_collapsedCells[i] * m_entityCount) + j]) {
					entityId = j;
				}
			}
			if (entityId != -1) {
				DropEntityCount(cells, entityId);
			}
		}
	}

	void CollapseCell(std::vector<bool>& cells, int iteration, int cellIndex) {
		m_collapsedCells.push_back(cellIndex);

		std::vector<int> possibleCollapses;
		for (int i = 0; i < m_entityCount; i++) {
			if (cells[(cellIndex * m_entityCount) + i] == true) {
				possibleCollapses.push_back(i);
			}
		}

		if (possibleCollapses.size() == 0)
			return;

		// Collapse Cell into Random Entity.
		int entity = possibleCollapses[RandomIntRange(0, possibleCollapses.size())];
		for (int i = 0; i < m_entityCount; i++) {
			if (i == entity) {
				continue;
			}
			cells[(cellIndex * m_entityCount) + i] = false;
		}

		DropEntityCount(cells, entity);

		// Get Current Zone
		int currentZone = -1;

		for (int i = 0; i < m_district.zones.size(); i++) {
			for (int j = 0; j < m_district.zones[i].cells.size(); j++) {
				if (cellIndex == (m_district.zones[i].cells[j].y * m_cols) + m_district.zones[i].cells[j].x) {
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

			for (int i = 0; i < m_district.zones[currentZone].cells.size(); i++) {
				int zoneCellIndex = (m_district.zones[currentZone].cells[i].y * m_cols) + m_district.zones[currentZone].cells[i].x;

				if (cellIndex == zoneCellIndex) {
					continue;
				}

				int count = 0;
				for (int j = 0; j < m_entityCount; j++) {
					if (cells[(zoneCellIndex * m_entityCount) + j] == true) {
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
				for (int i = 0; i < m_district.zones[currentZone].collapsedIdentifiers.size(); i++) {
					std::string identifier = m_district.zones[currentZone].collapsedIdentifiers[i].first;
					int	identifierIndex = -1;

					for (int j = 0; j < m_identifierCount; j++) {
						if (Low(identifier) == Low(m_identifiers[j].name)) {
							identifierIndex = j;
							break;
						}
					}
					if (identifierIndex == -1) {
						continue;
					}

					std::string entityValue = m_entities[entity].identifiersValues[identifierIndex].value;
					std::string collapsedIdentifierValue = m_district.zones[currentZone].collapsedIdentifiers[i].second;

					if (Low(entityValue) == Low(collapsedIdentifierValue)) {
						bool disabled = false;

						for (int k = 0; k < m_district.iterations[iteration].disabledZoneCollapseIdentifiers.size(); k++) {
							if (Low(m_district.zones[currentZone].collapsedIdentifiers[i].first) == Low(m_district.iterations[iteration].disabledZoneCollapseIdentifiers[k].first)
								&& Low(m_district.zones[currentZone].collapsedIdentifiers[i].second) == Low(m_district.iterations[iteration].disabledZoneCollapseIdentifiers[k].second)) {
								disabled = true;
							}
						}
						if (disabled) {
							continue;
						}

						int identifierIndex = -1;

						for (int k = 0; k < m_identifierCount; k++) {
							if (Low(m_identifiers[k].name) == Low(m_district.zones[currentZone].collapsedIdentifiers[i].first)) {
								identifierIndex = k;
								break;
							}
						}

						if (entityIdentifierCounts[entity][identifierIndex] == 0) {
							continue;
						}

						std::vector<int> restrictedEntities;

						for (int j = 0; j < m_entityCount; j++) {
							std::string identifierValue = m_district.zones[currentZone].collapsedIdentifiers[i].second;
							std::string entityIdentifierValue = m_entities[j].identifiersValues[identifierIndex].value;

							if (Low(entityIdentifierValue) != Low(identifierValue)) {
								restrictedEntities.push_back(j);
							}
						}

						for (int j = 0; j < m_district.zones[currentZone].cells.size(); j++) {
							int zoneCellIndex = (m_district.zones[currentZone].cells[j].y * m_cols) + m_district.zones[currentZone].cells[j].x;

							if (cellIndex == zoneCellIndex) {
								continue;
							}

							for (int k = 0; k < restrictedEntities.size(); k++) {
								cells[(zoneCellIndex * m_entityCount) + restrictedEntities[k]] = false;
							}
						}
						break;
					}
				}
			}
		}

		// Check Cells.
		for (int i = 0; i < m_entities[entity].identifiersValues.size(); i++) {
			std::vector<int> entityConditions;

			for (int j = 0; j < m_entityCount; j++) {
				for (int k = 0; k < m_entities[entity].identifiersValues[i].conditions.size(); k++) {
					if (Low(m_originalEntities[j].identifiersValues[i].value) == Low(m_entities[entity].identifiersValues[i].conditions[k])) {
						entityConditions.push_back(j);
					}
				}
			}

			// Cell Conditions.
			for (int j = 0; j < m_identifiers[i].relitiveCellConditions.size(); j++) {
				int relMovX = m_identifiers[i].relitiveCellConditions[j].x;
				int relMovY = m_identifiers[i].relitiveCellConditions[j].y * m_cols;

				if (cellIndex + relMovX + relMovY < 0 || cellIndex + relMovX + relMovY > m_cellCount - 1) {
					continue;
				}
				if (ceil((cellIndex + relMovX) / m_cols) != ceil(cellIndex / m_cols)) {
					continue;
				}

				bool already = false;
				for (int k = 0; k < m_collapsedCells.size(); k++) {
					if ((cellIndex + relMovX + relMovY) == m_collapsedCells[k]) {
						already = true;
						break;
					}
				}
				if (already)
					continue;

				int cellPeopleIndex = ((cellIndex + relMovX + relMovY) * m_entityCount);

				for (int k = 0; k < entityConditions.size(); k++) {
					cells[cellPeopleIndex + entityConditions[k]] = false;
				}

				int count = 0;
				for (int k = 0; k < m_entityCount; k++) {
					if (cells[cellPeopleIndex + k] == true) {
						count++;
					}
				}
				if (count == 1) {
					CollapseCell(cells, iteration, cellIndex + relMovX + relMovY);
				}
			}

			if (currentZone == -1) {
				continue;
			}

			// Zone Conditions.
			for (int l = 0; l < m_identifiers[i].relitiveZoneConditions.size(); l++) {
				int zone = currentZone + m_identifiers[i].relitiveZoneConditions[l];

				if (zone < 0 || zone >= m_district.zones.size()) {
					continue;
				}

				for (int j = 0; j < m_district.zones[zone].cells.size(); j++) {
					int index = (m_district.zones[zone].cells[j].y * m_cols) + m_district.zones[zone].cells[j].x;

					bool already = false;
					for (int k = 0; k < m_collapsedCells.size(); k++) {
						if (index == m_collapsedCells[k]) {
							already = true;
							break;
						}
					}
					if (already) {
						continue;
					}

					int cellPeopleIndex = (index * m_entityCount);

					for (int k = 0; k < entityConditions.size(); k++) {
						cells[cellPeopleIndex + entityConditions[k]] = false;
					}

					int count = 0;
					for (int k = 0; k < m_entityCount; k++) {
						if (cells[cellPeopleIndex + k] == true) {
							count++;
						}
					}
					if (count == 1) {
						CollapseCell(cells, iteration, index);
					}
				}
			}
		}
	}

	bool Collapse(std::vector<bool>& cells, int iteration) {
		// Find Lowest Entropy Seat.
		int minSuperposCount = INT_MAX;
		int cellIndex;
		//std::vector<int> cellIndexes;

		for (int i = 0; i < m_cellCount * m_entityCount; i += m_entityCount) {
			int superposCount = 0;
			for (int j = 0; j < m_entityCount; j++) {
				superposCount += cells[i + j];
			}

			if (superposCount < minSuperposCount && superposCount > 1) {
				minSuperposCount = superposCount;
				cellIndex = i;
			}
			//if (superposCount <= minSuperposCount && superposCount > 1) {
			//	if (superposCount < minSuperposCount) {
			//		minSuperposCount = superposCount;
			//		cellIndexes.clear();
			//	}
			//	cellIndexes.push_back(i);
			//}
		}

		if (minSuperposCount == INT_MAX || m_collapsedCells.size() >= m_totalEntities) {
			// Finished this Collapse Repeat.
			LOG("Collapsed: " << m_collapsedCells.size() << " / " << m_cellsToCollapse << "\n");
			return true;
		}
		//int cellIndex = cellIndexes[RandomIntRange(0, cellIndexes.size())];

		CollapseCell(cells, iteration, cellIndex / m_entityCount);
		return false;
	}

	int RunCollapses(std::vector<bool>& cells, int iteration) {
		m_collapsedCells = m_defaultCollapsedCells;
		SetEntityCounts(cells);

		bool done = false;
		while (true) {
			done = Collapse(cells, iteration);

			if (done) {
				break;
			}
		}

		return m_collapsedCells.size();
	}

	void SetDistrictCells(std::vector<bool>& cells, int iteration) {
		for (int i = 0; i < m_cellCount; i++) {
			m_district.iterations[iteration].cells.push_back(-1);

			for (int j = 0; j < m_entityCount; j++) {
				if (cells[(i * m_entityCount) + j]) {
					m_district.iterations[iteration].cells[i] = j;
				}
			}
		}
	}

	void ApplyGlobalEntityConditions(std::vector<bool>& cells) {
		for (int i = 0; i < m_entityCount; i++) {
			for (int j = 0; j < m_entities[i].entityCellConditions.size(); j++) {
				int x = m_entities[i].entityCellConditions[j].x;
				int y = m_entities[i].entityCellConditions[j].y;

				int index = (y * m_cols) + x;

				cells[index * m_entityCount + i] = false;
			}
			for (int j = 0; j < m_entities[i].entityZoneConditions.size(); j++) {
				int zone = m_entities[i].entityZoneConditions[j];
				for (int k = 0; k < m_district.zones[zone].cells.size(); k++) {
					int x = m_district.zones[zone].cells[k].x;
					int y = m_district.zones[zone].cells[k].y;

					int index = (y * m_cols) + x;

					cells[index * m_entityCount + i] = false;
				}
			}
		}
	}

	void RemoveUnoccupiableCells(std::vector<bool>& cells) {
		m_cellsToCollapse = m_cellCount;

		for (int i = 0; i < m_cellCount; i++) {
			if (!m_district.occupiableCells[i]) {
				for (int j = 0; j < m_entityCount; j++)
					cells[(i * m_entityCount) + j] = false;
			}
		}
	}

	void DisableCellCoordinate(std::vector<bool>& cells, vec2 cellCoord) {
		int cellIndex = ((cellCoord.y * m_cols) + cellCoord.x) * m_entityCount;

		for (int j = 0; j < m_entityCount; j++) {
			cells[cellIndex + j] = false;
		}
	}

	void DisableCellsZonesIdentifiers(std::vector<bool>& cells, int iteration) {
		// Disable Cells
		for (int i = 0; i < m_district.iterations[iteration].disabledCells.size(); i++) {
			DisableCellCoordinate(cells, m_district.iterations[iteration].disabledCells[i]);
		}

		// Disable Zones
		for (int i = 0; i < m_district.iterations[iteration].disabledZones.size(); i++) {
			int zoneIndex = m_district.iterations[iteration].disabledZones[i];

			for (int j = 0; j < m_district.zones[zoneIndex].cells.size(); j++) {
				DisableCellCoordinate(cells, m_district.zones[m_district.iterations[iteration].disabledZones[i]].cells[j]);
			}
		}

		// Disable Identifiers
		std::vector<int> disabledEntities;

		for (int i = 0; i < m_district.iterations[iteration].disabledIdentifiers.size(); i++) {
			std::string identifier = m_district.iterations[iteration].disabledIdentifiers[i].first;
			int			identifierIndex = -1;

			for (int j = 0; j < m_identifierCount; j++) {
				if (Low(identifier) == Low(m_identifiers[j].name)) {
					identifierIndex = j;
					break;
				}
			}
			if (identifierIndex == -1) {
				continue;
			}

			std::string value = m_district.iterations[iteration].disabledIdentifiers[i].second;

			for (int j = 0; j < m_entityCount; j++) {
				std::string entityValue = m_originalEntities[j].identifiersValues[identifierIndex].value;
				if (Low(value) == Low(entityValue)) {
					disabledEntities.push_back(j);
				}
			}
		}
		for (int i = 0; i < m_cellCount; i++) {
			for (int j = 0; j < disabledEntities.size(); j++) {
				cells[(i * m_entityCount) + disabledEntities[j]] = false;
			}
		}
	}

	void CarryCellCoordinate(std::vector<bool>& cells, vec2 cellCoord, int iteration) {
		int cellIndex = (cellCoord.y * m_cols) + cellCoord.x;

		for (int j = 0; j < m_entityCount; j++) {
			if (j != m_district.iterations[iteration - 1].cells[cellIndex]) {
				cells[(cellIndex * m_entityCount) + j] = false;
			}
			else {
				cells[(cellIndex * m_entityCount) + j] = true;
			}
		}

		CollapseCell(cells, iteration, cellIndex);
	}

	void CarryCellsZonesIdentifiers(std::vector<bool>& cells, int iteration) {
		if (iteration == 0)
			return;

		// Carry Cells
		for (int i = 0; i < m_district.iterations[iteration].carriedCells.size(); i++) {
			CarryCellCoordinate(cells, m_district.iterations[iteration].carriedCells[i], iteration);
		}

		// Carry Zones
		for (int i = 0; i < m_district.iterations[iteration].carriedZones.size(); i++) {
			int zoneIndex = m_district.iterations[iteration].carriedZones[i];

			for (int j = 0; j < m_district.zones[zoneIndex].cells.size(); j++) {
				CarryCellCoordinate(cells, m_district.zones[m_district.iterations[iteration].carriedZones[i]].cells[j], iteration);
			}
		}

		// Carry Identifiers
		std::vector<int> carriedEntities;

		for (int i = 0; i < m_district.iterations[iteration].carriedIdentifiers.size(); i++) {
			std::string identifier = m_district.iterations[iteration].carriedIdentifiers[i].first;
			int			identifierIndex = -1;

			for (int j = 0; j < m_identifierCount; j++) {
				if (Low(identifier) == Low(m_identifiers[j].name)) {
					identifierIndex = j;
					break;
				}
			}
			if (identifierIndex == -1) {
				continue;
			}

			std::string value = m_district.iterations[iteration].carriedIdentifiers[i].second;

			for (int j = 0; j < m_entityCount; j++) {
				std::string entityValue = m_originalEntities[j].identifiersValues[identifierIndex].value;
				if (Low(value) == Low(entityValue)) {
					carriedEntities.push_back(j);
				}
			}
		}
		for (int i = 0; i < m_cols; i++) {
			for (int j = 0; j < m_rows; j++) {
				for (int k = 0; k < carriedEntities.size(); k++) {
					int cellEntityIndex = m_district.iterations[iteration - 1].cells[(j * m_cols) + i];

					if (cellEntityIndex == carriedEntities[k]) {
						CarryCellCoordinate(cells, { i, j }, iteration);
					}
				}
			}
		}
	}

	void SetZoneIdentifierConditions(std::vector<bool>& cells) {
		for (int i = 0; i < m_district.zones.size(); i++) {
			std::vector<int> restrictedEntities;
			std::vector<int> allowedEntities;

			for (int l = 0; l < m_entityCount; l++) {
				for (int j = 0; j < m_district.zones[i].positiveZoneIdentifierConditions.size(); j++) {
					for (int k = 0; k < m_district.zones[i].negativeZoneIdentifierConditions[j].size(); k++) {
						std::string identifierCondition = m_district.zones[i].negativeZoneIdentifierConditions[j][k];

						if (Low(identifierCondition) == Low(m_originalEntities[l].identifiersValues[j].value)) {
							restrictedEntities.push_back(l);
							break;
						}
					}
					for (int k = 0; k < m_district.zones[i].positiveZoneIdentifierConditions[j].size(); k++) {
						std::string identifierCondition = m_district.zones[i].positiveZoneIdentifierConditions[j][k];

						if (Low(identifierCondition) != Low(m_entities[l].identifiersValues[j].value)) {
							restrictedEntities.push_back(l);
							break;
						}
						else if (Low(identifierCondition) == Low(m_entities[l].identifiersValues[j].value)) {
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

			for (int l = 0; l < m_district.zones[i].cells.size(); l++) {
				int cellIndex = ((m_district.zones[i].cells[l].y * m_cols) + m_district.zones[i].cells[l].x) * m_entityCount;

				for (int j = 0; j < restrictedEntities.size(); j++) {
					cells[cellIndex + restrictedEntities[j]] = false;
				}
			}
		}
	}

	void CollapseZoneIdentifiers(int iteration) {
		for (int i = 0; i < m_district.zones.size(); i++) {
			std::vector<int> collpasedIdentifierCellCounts(m_district.zones[i].collapsedIdentifiers.size(), 0);
			int occupiedCellsCount = m_district.zones[i].cells.size();

			std::vector<int> entityIndexes;

			for (int j = 0; j < m_district.zones[i].cells.size(); j++) {
				int cellIndex = (m_district.zones[i].cells[j].y * m_cols) + m_district.zones[i].cells[j].x;
				int entityIndex = m_district.iterations[iteration].cells[cellIndex];

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

				for (int k = 0; k < m_district.zones[i].collapsedIdentifiers.size(); k++) {
					std::string identifier = m_district.zones[i].collapsedIdentifiers[k].first;
					int	identifierIndex = -1;

					for (int j = 0; j < m_identifierCount; j++) {
						if (Low(identifier) == Low(m_identifiers[j].name)) {
							identifierIndex = j;
							break;
						}
					}
					if (identifierIndex == -1) {
						continue;
					}

					std::string entityValue = m_entities[entityIndex].identifiersValues[identifierIndex].value;
					std::string collapsedIdentifierValue = m_district.zones[i].collapsedIdentifiers[k].second;

					if (Low(entityValue) == Low(collapsedIdentifierValue)) {
						collpasedIdentifierCellCounts[k]++;
					}
				}
			}

			std::string zoneCollapsedIdentifier = "";

			if (occupiedCellsCount == 0) {
				m_district.iterations[iteration].zoneCollapsedIdentifiers[i] = zoneCollapsedIdentifier;
				continue;
			}

			for (int j = 0; j < collpasedIdentifierCellCounts.size(); j++) {
				if (collpasedIdentifierCellCounts[j] == occupiedCellsCount) {
					bool disabled = false;

					for (int k = 0; k < m_district.iterations[iteration].disabledZoneCollapseIdentifiers.size(); k++) {
						if (Low(m_district.zones[i].collapsedIdentifiers[j].first) == Low(m_district.iterations[iteration].disabledZoneCollapseIdentifiers[k].first)
							&& Low(m_district.zones[i].collapsedIdentifiers[j].second) == Low(m_district.iterations[iteration].disabledZoneCollapseIdentifiers[k].second)) {
							disabled = true;
						}
					}
					if (disabled) {
						continue;
					}

					int identifierIndex = -1;

					for (int k = 0; k < m_identifierCount; k++) {
						if (Low(m_identifiers[k].name) == Low(m_district.zones[i].collapsedIdentifiers[j].first)) {
							identifierIndex = k;
							break;
						}
					}

					for (int k = 0; k < entityIndexes.size(); k++) {
						if (entityIdentifierCounts[entityIndexes[k]][identifierIndex] == 0) {
							continue;
						}
						entityIdentifierCounts[entityIndexes[k]][identifierIndex]--;

						if (entityIdentifierCounts[entityIndexes[k]][identifierIndex] == 0) {
							m_entities[entityIndexes[k]].identifiersValues[identifierIndex].value = ".";
						}
					}
					zoneCollapsedIdentifier = m_district.zones[i].collapsedIdentifiers[j].first + ", " + m_district.zones[i].collapsedIdentifiers[j].second;
					break;
				}
			}
			m_district.iterations[iteration].zoneCollapsedIdentifiers[i] = zoneCollapsedIdentifier;
		}
	}

	void FindCellsToCollapse(std::vector<bool>& cells) {
		for (int i = 0; i < m_cellCount; i++) {
			bool occupied = false;
			for (int j = 0; j < m_entityCount; j++) {
				if (cells[(i * m_entityCount) + j]) {
					occupied = true; break;
				}
			}
			if (!occupied) {
				m_cellsToCollapse--;
			}
		}
	}

	std::vector<bool> RepeatRunningCollapses(const std::vector<bool> cellsBase, int iteration) {
		std::vector<bool> cellsBest		((m_cellCount) * m_entityCount, 1);
		std::vector<bool> cellsWorking	((m_cellCount) * m_entityCount, 1);

		int currentCollapsedCellsCount = 0;
		for (int i = 0; i < m_repeats; i++) {
			cellsWorking = cellsBase;

			int collapsedCellsCount = RunCollapses(cellsWorking, iteration);

			if (collapsedCellsCount > currentCollapsedCellsCount) {
				cellsBest = cellsWorking;

				currentCollapsedCellsCount = collapsedCellsCount;
				if (collapsedCellsCount == m_cellsToCollapse || currentCollapsedCellsCount >= m_totalEntities) {
					break;
				}
			}
		}
		return cellsBest;
	}

	void RunIteration(int iteration) {
		std::vector<bool> cellsBase((m_cellCount) * m_entityCount, 1);

		m_collapsedCells.clear();
		m_district.iterations[iteration].zoneCollapsedIdentifiers = std::vector<std::string>(m_district.zones.size(), "");

		ApplyGlobalEntityConditions(cellsBase);
		RemoveUnoccupiableCells(cellsBase);

		DisableCellsZonesIdentifiers(cellsBase, iteration);
		CarryCellsZonesIdentifiers(cellsBase, iteration);

		SetZoneIdentifierConditions(cellsBase);

		FindCellsToCollapse(cellsBase);

		m_defaultCollapsedCells = m_collapsedCells;

		std::vector<bool> cellsBest = RepeatRunningCollapses(cellsBase, iteration);

		SetDistrictCells(cellsBest, iteration);

		CollapseZoneIdentifiers(iteration);
	}

	void SetTotalEntities() {
		m_totalEntities = 0;

		for (int i = 0; i < m_entityCount; i++) {
			if (m_entities[i].count == 0) {
				m_totalEntities = INT_MAX;
				return;
			}
			m_totalEntities += m_entities[i].count;
		}
	}

	void SetEntityIdentifierCounts() {
		int identifierCount = m_identifierCount;

		std::vector<int> identifierCounts(identifierCount);

		for (int j = 0; j < identifierCount; j++) {
			identifierCounts[j] = m_identifiers[j].iterationCount;
		}

		for (int i = 0; i < m_entities.size(); i++) {
			entityIdentifierCounts.push_back(identifierCounts);
		}
	}

	void Setup(EOP_Config& eop_config, int repeats) {
		m_district = eop_config.district;
		m_entities = eop_config.entities.entities;
		m_identifiers = eop_config.entities.identifiers;

		m_rows = m_district.rows;
		m_cols = m_district.cols;

		m_cellCount = m_rows * m_cols;
		m_entityCount = m_entities.size();
		m_identifierCount = m_identifiers.size();
		m_repeats = repeats;

		m_originalEntities = eop_config.entities.entities;

		SetupRandom();
		
		SetTotalEntities();
	}

	void EvaluateEOP_Config(EOP_Config& eop_config, int repeats) {
		Setup(eop_config, repeats);

		SetEntityIdentifierCounts();
		
		for (int i = 0; i < m_district.iterations.size(); i++) {
			RunIteration(i);
		}

		eop_config.district.iterations = m_district.iterations;
	}

}
