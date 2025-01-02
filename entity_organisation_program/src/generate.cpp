#include "generate.h"
  
namespace eop {

	District m_district;
	Entities m_entities;

	std::vector<int> m_collapsedCells;
	std::vector<int> m_entityCount;

	int m_cellsToCollapse = 0;
	int m_repeats;
	int m_totalEntities = 0;

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
				for (int k = 0; k < m_district.zones[zone].size(); k++) {
					int x = m_district.zones[zone][k].x;
					int y = m_district.zones[zone][k].y;
					
					int index = (y * m_district.cols) + x;

					cells[index * m_entities.entities.size() + i] = false;
				}
			}
		}
	}

	void RemoveUnoccupiableCells(std::vector<bool>& cells) {
		for (int i = 0; i < m_district.rows * m_district.cols; i++) {
			int k = 0;
			for (int j = 0; j < m_entities.entities.size(); j++) {
				if (cells[(i * m_entities.entities.size()) + j] == false)
					k++;
			}

			if (k == m_entities.entities.size())
				m_cellsToCollapse--;
			else if (!m_district.occupiableCells[i]) {
				for (int j = 0; j < m_entities.entities.size(); j++)
					cells[(i * m_entities.entities.size()) + j] = false;
				m_cellsToCollapse--;
			}
		}
	}

	void SetEntityCounts() {
		m_entityCount.clear();
		for (int i = 0; i < m_entities.entities.size(); i++) {
			m_entityCount.push_back(m_entities.entities[i].count);
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
		
		if (m_entityCount[entity] > 1)
			m_entityCount[entity]--;
		
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
		
		int currentZone = -1;

		for (int i = 0; i < m_district.zones.size(); i++) {
			for (int j = 0; j < m_district.zones[i].size(); j++) {
				if (cellIndex == (m_district.zones[i][j].y * m_district.cols) + m_district.zones[i][j].x) {
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
					if (m_entities.entities[j].identifiersValues[i].value == m_entities.entities[entity].identifiersValues[i].conditions[k]) {
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

				for (int j = 0; j < m_district.zones[zone].size(); j++) {
					int index = (m_district.zones[zone][j].y * m_district.cols) + m_district.zones[zone][j].x;

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
		SetEntityCounts();
		m_collapsedCells.clear();

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

		for (int j = 0; j < m_entities.entities.size(); j++)
			cells[cellIndex + j] = false;
	}

	void DisableCellsZones(std::vector<bool>& cells, int iteration) {
		for (int i = 0; i < m_district.iterations[iteration].disabledCells.size(); i++)
			DisableCellCoordinate(cells, m_district.iterations[iteration].disabledCells[i]);

		for (int i = 0; i < m_district.iterations[iteration].disabledZones.size(); i++) {
			int zoneIndex = m_district.iterations[iteration].disabledZones[i];

			for (int j = 0; j < m_district.zones[zoneIndex].size(); j++)
				DisableCellCoordinate(cells, m_district.zones[m_district.iterations[iteration].disabledZones[i]][j]);
		}
	}

	void CarryCellCoordinate(std::vector<bool>& cells, vec2 cellCoord, int iteration) {
		int cellIndex = (cellCoord.y * m_district.cols) + cellCoord.x;

		for (int j = 0; j < m_entities.entities.size(); j++) {
			if (j != m_district.iterations[iteration - 1].cells[cellIndex])
				cells[(cellIndex * m_entities.entities.size()) + j] = false;
		}

		CollapseCell(cells, cellIndex);
	}

	void CarryCellsZones(std::vector<bool>& cells, int iteration) {
		if (iteration == 0)
			return;

		for (int i = 0; i < m_district.iterations[iteration].carriedCells.size(); i++)
			CarryCellCoordinate(cells, m_district.iterations[iteration].carriedCells[i], iteration);

		for (int i = 0; i < m_district.iterations[iteration].carriedZones.size(); i++) {
			int zoneIndex = m_district.iterations[iteration].carriedZones[i];

			for (int j = 0; j < m_district.zones[zoneIndex].size(); j++)
				CarryCellCoordinate(cells, m_district.zones[m_district.iterations[iteration].carriedZones[i]][j], iteration);
		}
	}

	void RunIteration(int iteration) {
		m_cellsToCollapse = m_district.rows * m_district.cols;

		std::vector<bool> cellsBase((m_district.rows * m_district.cols) * m_entities.entities.size(), 1);
		std::vector<bool> cellsBest((m_district.rows * m_district.cols) * m_entities.entities.size(), 1);
		std::vector<bool> cellsWorking((m_district.rows * m_district.cols) * m_entities.entities.size(), 1);

		ApplyGlobalEntityConditions(cellsBase);
		RemoveUnoccupiableCells(cellsBase);

		DisableCellsZones(cellsBase, iteration);
		CarryCellsZones(cellsBase, iteration);

		PrintDistrictDebug(m_district, m_entities, cellsBase);
		LOG("---\n");

		int currentm_collapsedCellsCount = 0;
		for (int i = 0; i < m_repeats; i++) {
			cellsWorking = cellsBase;

			int m_collapsedCellsCount = RunCollapses(cellsWorking);

			if (m_collapsedCellsCount > currentm_collapsedCellsCount) {
				cellsBest = cellsWorking;

				currentm_collapsedCellsCount = m_collapsedCellsCount;
				if (m_collapsedCellsCount == m_cellsToCollapse)
					break;
			}
		}

		SetDistrictCells(cellsBest, iteration);

		PrintDistrictDebug(m_district, m_entities, cellsBest);
		LOG("---\n");
	}

	void SetTotalEntities() {
		for (int i = 0; i < m_entities.entities.size(); i++) {
			if (m_entities.entities[i].count == 0) {
				m_totalEntities = INT_MAX;
				return;
			}
			m_totalEntities += m_entities.entities[i].count;
		}
	}

	void ReduceIdentifiers() {

	}
	
	void GenerateDistrict(EOP_Config& eop_config, int repeats) {
		m_district = eop_config.district;
		m_entities = eop_config.entities;
		m_repeats = repeats;

		SetTotalEntities();

		srand((unsigned)time(NULL)); // Random Seed.

		for (int i = 0; i < m_district.iterations.size(); i++) {
			RunIteration(i);

			ReduceIdentifiers();
		}

		eop_config.district = m_district;
		eop_config.entities = m_entities;
	}

}
