#include "generate.h"
  
namespace eop {

	District m_district;
	Entities m_entities;

	std::vector<Entity> m_originalEntities;

	std::vector<int> m_collapsedCells;
	std::vector<int> m_defaultCollapsedCells;
	std::vector<int> m_entityCount;
	std::vector<int> m_defaultEntityCount;

	std::vector<std::vector<int>> entityIdentifierCounts;

	int m_totalEntities;
	int m_cellsToCollapse;
	int m_repeats;

	void ApplyGlobalEntityConditions(std::vector<bool>& cells) {
		for (int i = 0; i < m_entities.entities.size(); i++) {
			for (int j = 0; j < m_entities.entities[i].entityCellConditions.size(); j++) {
				int x = m_entities.entities[i].entityCellConditions[j].x;
				int y = m_entities.entities[i].entityCellConditions[j].y;
				
				int index = (y * m_district.cols) + x;

				cells[index * m_entities.entities.size() + i] = false;
			}
			for (int j = 0; j < m_entities.entities[i].entityZoneConditions.size(); j++) {
				int zone = m_entities.entities[i].entityZoneConditions[j];
				for (int k = 0; k < m_district.zones[zone].cells.size(); k++) {
					int x = m_district.zones[zone].cells[k].x;
					int y = m_district.zones[zone].cells[k].y;
					
					int index = (y * m_district.cols) + x;

					cells[index * m_entities.entities.size() + i] = false;
				}
			}
		}
	}

	void RemoveUnoccupiableCells(std::vector<bool>& cells) {
		m_cellsToCollapse = m_district.rows * m_district.cols;

		for (int i = 0; i < m_district.rows * m_district.cols; i++) {
			if (!m_district.occupiableCells[i]) {
				for (int j = 0; j < m_entities.entities.size(); j++)
					cells[(i * m_entities.entities.size()) + j] = false;
			}
		}
	}

	void DropEntityCount(std::vector<bool>& cells, int entity) {
		if (m_entityCount[entity] > 1) {
			m_entityCount[entity]--;
		}
		else if (m_entityCount[entity] == 1) {
			m_entityCount[entity]--;
			for (int i = 0; i < (m_district.rows * m_district.cols); i++) {
				bool isCollapsed = false;
				for (int j = 0; j < m_collapsedCells.size(); j++) {
					if (m_collapsedCells[j] == i)
						isCollapsed = true;
				}
				if (isCollapsed)
					continue;
				cells[(i * m_entities.entities.size()) + entity] = false;
			}
		}
	}

	void SetEntityCounts(std::vector<bool>& cells) {
		m_entityCount.clear();
		for (int i = 0; i < m_entities.entities.size(); i++) {
			m_entityCount.push_back(m_entities.entities[i].count);
		}
		for (int i = 0; i < m_collapsedCells.size(); i++) {
			int entityId = -1;
			for (int j = 0; j < m_entities.entities.size(); j++) {
				if (cells[(m_collapsedCells[i] * m_entities.entities.size()) + j]) {
					entityId = j;
				}
			}
			if (entityId != -1) {
				DropEntityCount(cells, entityId);
			}
		}
	}

	int RandomIntRange(int low, int high) {
		if (high == 0)
			return 0;

		return low + (rand() % high);
	}
	
	void CollapseCell(std::vector<bool>& cells, int cellIndex) {
		m_collapsedCells.push_back(cellIndex);
		
		std::vector<int> possibleCollapses;
		for (int i = 0; i < m_entities.entities.size(); i++) {
			if (cells[(cellIndex * m_entities.entities.size()) + i] == true)
				possibleCollapses.push_back(i);
		}

		if (possibleCollapses.size() == 0)
			return;
		
		// Collapse Cell into Random Entity.
		int entity = possibleCollapses[RandomIntRange(0, possibleCollapses.size())];
		for (int i = 0; i < m_entities.entities.size(); i++) {
			if (i == entity)
				continue;
			cells[(cellIndex * m_entities.entities.size()) + i] = false;
		}
		
		DropEntityCount(cells, entity);
		
		int currentZone = -1;

		for (int i = 0; i < m_district.zones.size(); i++) {
			for (int j = 0; j < m_district.zones[i].cells.size(); j++) {
				if (cellIndex == (m_district.zones[i].cells[j].y * m_district.cols) + m_district.zones[i].cells[j].x) {
					currentZone = i;
					break;
				}
			}
			if (currentZone != -1)
				break;
		}
		
		// Check Cells.
		for (int i = 0; i < m_entities.entities[entity].identifiersValues.size(); i++) {
			std::vector<int> entityConditions;

			for (int j = 0; j < m_entities.entities.size(); j++) {
				for (int k = 0; k < m_entities.entities[entity].identifiersValues[i].conditions.size(); k++) {
					if (m_originalEntities[j].identifiersValues[i].value == m_entities.entities[entity].identifiersValues[i].conditions[k]) {
						entityConditions.push_back(j);
					}
				}
			}

			// Cell Conditions.
			for (int j = 0; j < m_entities.identifiers[i].relitiveCellConditions.size(); j++) {
				int relMovX = m_entities.identifiers[i].relitiveCellConditions[j].x;
				int relMovY = m_entities.identifiers[i].relitiveCellConditions[j].y * m_district.cols;

				if (cellIndex + relMovX + relMovY < 0 || cellIndex + relMovX + relMovY > m_district.rows * m_district.cols - 1)
					continue;

				if (ceil((cellIndex + relMovX) / m_district.cols) != ceil(cellIndex / m_district.cols))
					continue;

				bool already = false;
				for (int k = 0; k < m_collapsedCells.size(); k++) {
					if ((cellIndex + relMovX + relMovY) == m_collapsedCells[k]) {
						already = true;
						break;
					}
				}
				if (already)
					continue;

				int cellPeopleIndex = ((cellIndex + relMovX + relMovY) * m_entities.entities.size());

				for (int k = 0; k < entityConditions.size(); k++) {
					cells[cellPeopleIndex + entityConditions[k]] = false;
				}

				int count = 0;
				for (int k = 0; k < m_entities.entities.size(); k++) {
					if (cells[cellPeopleIndex + k] == true)
						count++;
				}
				if (count == 1)
					CollapseCell(cells, cellIndex + relMovX + relMovY);
			}

			if (currentZone == -1)
				continue;

			// Zone Conditions.
			for (int l = 0; l < m_entities.identifiers[i].relitiveZoneConditions.size(); l++) {
				int zone = currentZone + m_entities.identifiers[i].relitiveZoneConditions[l];

				if (zone < 0 || zone >= m_district.zones.size())
					continue;

				for (int j = 0; j < m_district.zones[zone].cells.size(); j++) {
					int index = (m_district.zones[zone].cells[j].y * m_district.cols) + m_district.zones[zone].cells[j].x;

					bool already = false;
					for (int k = 0; k < m_collapsedCells.size(); k++) {
						if (index == m_collapsedCells[k]) {
							already = true;
							break;
						}
					}
					if (already)
						continue;

					int cellPeopleIndex = (index * m_entities.entities.size());

					for (int k = 0; k < entityConditions.size(); k++) {
						cells[cellPeopleIndex + entityConditions[k]] = false;
					}

					int count = 0;
					for (int k = 0; k < m_entities.entities.size(); k++) {
						if (cells[cellPeopleIndex + k] == true)
							count++;
					}
					if (count == 1)
						CollapseCell(cells, index);
				}
			}
		}
	}

	bool Collapse(std::vector<bool>& cells) {
		// Find Lowest Entropy Seat.
		int minSuperposCount = INT_MAX;
		int cellIndex;
		//std::vector<int> cellIndexes;

		for (int i = 0; i < m_district.rows * m_district.cols * m_entities.entities.size(); i += m_entities.entities.size()) {
			int superposCount = 0;
			for (int j = 0; j < m_entities.entities.size(); j++)
				superposCount += cells[i + j];

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
		
		CollapseCell(cells, cellIndex / m_entities.entities.size());
		return false;
	}
	
	int RunCollapses(std::vector<bool>& cells) {
		m_collapsedCells = m_defaultCollapsedCells;
		//m_entityCount = m_defaultEntityCount;
		SetEntityCounts(cells);

		bool done = false;
		while (true) {
			done = Collapse(cells);

			if (done)
				break;

			//PrintDistrict(m_district, m_entities, m_cells);
			//LOG("---\n");
		}

		return m_collapsedCells.size();
	}

	void PrintDistrictIteration(EOP_Config& eop_config, int iteration, int identifierId) {
		LOG("Row: - ");
		int j = 0;
		for (int i = 0; i < eop_config.district.rows * eop_config.district.cols; i++) {
			int id = eop_config.district.iterations[iteration].cells[i];
			if (id == -1)
				LOG("NA : ");
			else
				LOG(eop_config.entities.entities[id].identifiersValues[identifierId].value << " : ");
			
			j++;
			if (j >= eop_config.district.cols && i + 1 < eop_config.district.rows * eop_config.district.cols) {
				LOG("\nRow: - ");
				j = 0;
			}
		}
		LOG("\n");
	}

	void PrintDistrictDebug(District& district, Entities& entities, std::vector<bool>& cellsIds) {
		LOG("Row: - ");
		int j = 0;
		for (int i = 0; i < (district.rows * district.cols) * entities.entities.size(); i += entities.entities.size()) {
			int initialIndex = 0;
			for (int j = 0; j < entities.entities.size(); j++) {
				LOG(cellsIds[i + j] << ", ");

				if (cellsIds[i + j] == true)
					initialIndex = j;
			}
			LOG(": ");
			j++;
			if (j >= district.cols && i + entities.entities.size() < (district.rows * district.cols) * entities.entities.size()) {
				LOG("\nRow: - ");
				j = 0;
			}
		}
		LOG("\n");
	}

	void SetDistrictCells(std::vector<bool>& cells, int iteration) {
		for (int i = 0; i < m_district.rows * m_district.cols; i++) {
			m_district.iterations[iteration].cells.push_back(-1);

			for (int j = 0; j < m_entities.entities.size(); j++) {
				if (cells[(i * m_entities.entities.size()) + j])
					m_district.iterations[iteration].cells[i] = j;
			}
		}
	}

	void DisableCellCoordinate(std::vector<bool>& cells, vec2 cellCoord) {
		int cellIndex = ((cellCoord.y * m_district.cols) + cellCoord.x) * m_entities.entities.size();

		for (int j = 0; j < m_entities.entities.size(); j++) {
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

			for (int j = 0; j < m_entities.identifiers.size(); j++) {
				if (identifier == m_entities.identifiers[j].name) {
					identifierIndex = j;
					break;
				}
			}
			if (identifierIndex == -1) {
				continue;
			}

			std::string value = m_district.iterations[iteration].disabledIdentifiers[i].second;

			for (int j = 0; j < m_entities.entities.size(); j++) {
				std::string entityValue = m_originalEntities[j].identifiersValues[identifierIndex].value;
				if (value == entityValue) {
					disabledEntities.push_back(j);
				}
			}
		}
		for (int i = 0; i < m_district.rows * m_district.cols; i++) {
			for (int j = 0; j < disabledEntities.size(); j++) {
				cells[(i * m_entities.entities.size()) + disabledEntities[j]] = false;
			}
		}
	}

	void CarryCellCoordinate(std::vector<bool>& cells, vec2 cellCoord, int iteration) {
		int cellIndex = (cellCoord.y * m_district.cols) + cellCoord.x;

		for (int j = 0; j < m_entities.entities.size(); j++) {
			if (j != m_district.iterations[iteration - 1].cells[cellIndex]) {
				cells[(cellIndex * m_entities.entities.size()) + j] = false;
			} else {
				cells[(cellIndex * m_entities.entities.size()) + j] = true;
			}
		}

		CollapseCell(cells, cellIndex);
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

			for (int j = 0; j < m_entities.identifiers.size(); j++) {
				if (identifier == m_entities.identifiers[j].name) {
					identifierIndex = j;
					break;
				}
			}
			if (identifierIndex == -1) {
				continue;
			}

			std::string value = m_district.iterations[iteration].carriedIdentifiers[i].second;

			for (int j = 0; j < m_entities.entities.size(); j++) {
				std::string entityValue = m_originalEntities[j].identifiersValues[identifierIndex].value;
				if (value == entityValue) {
					carriedEntities.push_back(j);
				}
			}
		}
		for (int i = 0; i < m_district.cols; i++) {
			for (int j = 0; j < m_district.rows; j++) {
				for (int k = 0; k < carriedEntities.size(); k++) {
					int cellEntityIndex = m_district.iterations[iteration - 1].cells[(j * m_district.cols) + i];

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
			
			for (int l = 0; l < m_entities.entities.size(); l++) {
				for (int j = 0; j < m_district.zones[i].positiveZoneIdentifierConditions.size(); j++) {
					for (int k = 0; k < m_district.zones[i].negativeZoneIdentifierConditions[j].size(); k++) {
						std::string identifierCondition = m_district.zones[i].negativeZoneIdentifierConditions[j][k];

						if (identifierCondition == m_originalEntities[l].identifiersValues[j].value) {
							restrictedEntities.push_back(l);
							break;
						}
					}
					for (int k = 0; k < m_district.zones[i].positiveZoneIdentifierConditions[j].size(); k++) {
						std::string identifierCondition = m_district.zones[i].positiveZoneIdentifierConditions[j][k];

						if (identifierCondition != m_entities.entities[l].identifiersValues[j].value) {
							restrictedEntities.push_back(l);
							break;
						} else if (identifierCondition == m_entities.entities[l].identifiersValues[j].value) {
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
				int cellIndex = ((m_district.zones[i].cells[l].y * m_district.cols) + m_district.zones[i].cells[l].x) * m_entities.entities.size();

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
				int cellIndex = (m_district.zones[i].cells[j].y * m_district.cols) + m_district.zones[i].cells[j].x;
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

					for (int j = 0; j < m_entities.identifiers.size(); j++) {
						if (identifier == m_entities.identifiers[j].name) {
							identifierIndex = j;
							break;
						}
					}
					if (identifierIndex == -1) {
						continue;
					}

					std::string entityValue = m_entities.entities[entityIndex].identifiersValues[identifierIndex].value;
					std::string collapsedIdentifierValue = m_district.zones[i].collapsedIdentifiers[k].second;

					if (entityValue == collapsedIdentifierValue) {
						collpasedIdentifierCellCounts[k]++;
					}
				}
			}

			std::string zoneCollapsedIdentifier = "";

			for (int j = 0; j < collpasedIdentifierCellCounts.size(); j++) {
				if (collpasedIdentifierCellCounts[j] == occupiedCellsCount) {
					bool disabled = false;

					for (int k = 0; k < m_district.iterations[iteration].disabledZoneCollapseIdentifiers.size(); k++) {
						if (m_district.zones[i].collapsedIdentifiers[j].first == m_district.iterations[iteration].disabledZoneCollapseIdentifiers[k].first
							&& m_district.zones[i].collapsedIdentifiers[j].second == m_district.iterations[iteration].disabledZoneCollapseIdentifiers[k].second) {
							disabled = true;
						}
					}
					if (disabled) {
						continue;
					}

					int identifierIndex = -1;

					for (int k = 0; k < m_entities.identifiers.size(); k++) {
						if (m_entities.identifiers[k].name == m_district.zones[i].collapsedIdentifiers[j].first) {
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
							m_entities.entities[entityIndexes[k]].identifiersValues[identifierIndex].value = ".";
						}
					}
					zoneCollapsedIdentifier = m_district.zones[i].collapsedIdentifiers[j].first + ", " + m_district.zones[i].collapsedIdentifiers[j].second;
					break;
				}
			}
			m_district.iterations[iteration].zoneCollapsedIdentifiers.push_back(zoneCollapsedIdentifier);
		}
	}

	void FindCellsToCollapse(std::vector<bool>& cells) {
		for (int i = 0; i < m_district.rows * m_district.cols; i++) {
			bool occupied = false;
			for (int j = 0; j < m_entities.entities.size(); j++) {
				if (cells[(i * m_entities.entities.size()) + j]) {
					occupied = true; break;
				}
			}
			if (!occupied) {
				m_cellsToCollapse--;
			}
		}
	}
	
	void RunIteration(int iteration) {
		std::vector<bool> cellsBase((m_district.rows * m_district.cols) * m_entities.entities.size(), 1);
		std::vector<bool> cellsBest((m_district.rows * m_district.cols) * m_entities.entities.size(), 1);
		std::vector<bool> cellsWorking((m_district.rows * m_district.cols) * m_entities.entities.size(), 1);

		m_collapsedCells.clear();
		//SetEntityCounts();

		ApplyGlobalEntityConditions(cellsBase);
		RemoveUnoccupiableCells(cellsBase);

		DisableCellsZonesIdentifiers(cellsBase, iteration);
		CarryCellsZonesIdentifiers(cellsBase, iteration);

		SetZoneIdentifierConditions(cellsBase);
		
		FindCellsToCollapse(cellsBase);

		//m_defaultEntityCount = m_entityCount;
		m_defaultCollapsedCells = m_collapsedCells;

		//PrintDistrictDebug(m_district, m_entities, cellsBase);
		//LOG("---\n");

		// Repeat Running Collapses
		int currentCollapsedCellsCount = 0;
		for (int i = 0; i < m_repeats; i++) {
			cellsWorking = cellsBase;

			int collapsedCellsCount = RunCollapses(cellsWorking);

			if (collapsedCellsCount > currentCollapsedCellsCount) {
				cellsBest = cellsWorking;

				currentCollapsedCellsCount = collapsedCellsCount;
				if (collapsedCellsCount == m_cellsToCollapse || currentCollapsedCellsCount >= m_totalEntities)
					break;
			}
		}

		SetDistrictCells(cellsBest, iteration);

		CollapseZoneIdentifiers(iteration);

		//PrintDistrictDebug(m_district, m_entities, cellsBest);
		LOG("---\n");
	}

	void SetTotalEntities() {
		m_totalEntities = 0;
		for (int i = 0; i < m_entities.entities.size(); i++) {
			if (m_entities.entities[i].count == 0) {
				m_totalEntities = INT_MAX;
				return;
			}
			m_totalEntities += m_entities.entities[i].count;
		}
	}

	void SetEntityIdentifierCounts() {
		int identifierCount = m_entities.identifiers.size();

		std::vector<int> identifierCounts(identifierCount);

		for (int j = 0; j < identifierCount; j++) {
			identifierCounts[j] = m_entities.identifiers[j].iterationCount;
		}

		for (int i = 0; i < m_entities.entities.size(); i++) {
			entityIdentifierCounts.push_back(identifierCounts);
		}
	}
	
	void GenerateDistrict(EOP_Config& eop_config, int repeats) {
		m_district = eop_config.district;
		m_entities = eop_config.entities;

		m_originalEntities = eop_config.entities.entities;
		m_repeats = repeats;

		srand((unsigned)time(NULL)); // Random Seed

		SetTotalEntities();
		SetEntityIdentifierCounts();

		for (int i = 0; i < m_district.iterations.size(); i++) {
			RunIteration(i);
		}

		eop_config.district.iterations = m_district.iterations;
	}

}
