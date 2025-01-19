#include "import_eop_config.h"

#include "OpenXLSX/OpenXLSX.hpp"

namespace eop {

	std::vector<vec2> FindCellConditions(std::string value) {
		std::vector<vec2> cellConditions;

		value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
		std::stringstream ss(value);

		std::string token;
		while (getline(ss, token, ')')) {
			int x = std::stoi(token.substr(1).substr(0, token.find(",")));
			int y = std::stoi(token.substr(1).substr(token.find(",")));

			cellConditions.push_back({ x, y });
		}
		return cellConditions;
	}
	std::vector<int> FindZoneConditions(std::string value) {
		std::vector<int> zoneConditions;

		value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
		std::stringstream ss(value);

		std::string token;
		while (getline(ss, token, ',')) {
			zoneConditions.push_back(std::stoi(token));
		}

		if (zoneConditions.size() == 0 && zoneConditions.size() != 0) {
			zoneConditions.push_back(std::stoi(value));
		}
		return zoneConditions;
	}
	std::vector<std::pair<std::string, std::string>> FindIdentifierPairs(std::string value) {
		std::vector<std::pair<std::string, std::string>> collapsedIdentifiers;

		value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
		std::stringstream ss(value);

		std::string token;
		while (getline(ss, token, ')')) {
			std::string identifier = token.substr(1).substr(0, token.find(",") - 1);
			std::string value = token.substr(1).substr(token.find(","));

			collapsedIdentifiers.push_back({ identifier, value });
		}
		return collapsedIdentifiers;
	}
	std::vector<std::string> FindCommaValues(std::string value) {
		std::vector<std::string> values;

		value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
		std::stringstream ss(value);

		std::string token;
		while (getline(ss, token, ',')) {
			values.push_back(token);
		}

		if (values.size() == 0 && values.size() != 0) {
			values.push_back(value);
		}
		return values;
	}

	District ImportDistrictSheet(const OpenXLSX::XLWorksheet& districtSheet, const OpenXLSX::XLWorksheet& iterationsSheet, const OpenXLSX::XLWorksheet& zonesSheet) {
		District district;

		// Find Rows and Cols.
		for (int i = 1; i < districtSheet.columnCount() + 1; i++) {
			if (districtSheet.cell(2, i).value().getString()[0] == '-') {
				district.cols = i - 1;
				break;
			}
		}
		for (int i = 1; i < districtSheet.rowCount() + 1; i++) {
			if (districtSheet.cell(i, 1).value().getString()[0] == '-') {
				district.rows = i - 2;
				break;
			}
		}

		// Collect Cell Data.
		for (int i = 0; i < district.rows; i++) {
			for (int j = 0; j < district.cols; j++) {
				std::string cell = districtSheet.cell(i + 2, j + 1).value().getString();
				cell.erase(std::remove(cell.begin(), cell.end(), ' '), cell.end());
				
				if (cell == "") {
					district.occupiableCells.push_back(false);
					continue;
				}
				else {
					district.occupiableCells.push_back(true);
				}
				
				int zone = std::stoi(cell);
		
				while (district.zones.size() <= zone) {
					district.zones.push_back({});
				}
				district.zones[zone].cells.push_back({ j, i });
			}
		}

		// Extract Iterations Data.
		int i = 3;
		while (iterationsSheet.cell(i, 1).getString()[0] != '-') {
			if (iterationsSheet.cell(i, 1).getString() == "") {
				i++; continue;
			}
			std::string name = iterationsSheet.cell(i, 1).getString();

			bool hiden = false;
			if (iterationsSheet.cell(i, 2).getString() == "T" || iterationsSheet.cell(i, 2).getString() == "t") {
				hiden = true;
			}
			
			std::vector<vec2>	disabledCells = FindCellConditions(iterationsSheet.cell(i, 3).getString());
			std::vector<int>	disabledZones = FindZoneConditions(iterationsSheet.cell(i, 4).getString());
			std::vector<std::pair<std::string, std::string>> disabledIdentifiers = FindIdentifierPairs(iterationsSheet.cell(i, 5).getString());

			std::vector<vec2>	carriedCells = FindCellConditions(iterationsSheet.cell(i, 6).getString());
			std::vector<int>	carriedZones = FindZoneConditions(iterationsSheet.cell(i, 7).getString());
			std::vector<std::pair<std::string, std::string>> carriedIdentifiers = FindIdentifierPairs(iterationsSheet.cell(i, 8).getString());

			std::vector<std::pair<std::string, std::string>> disabledZoneCollapseIdentifiers = FindIdentifierPairs(iterationsSheet.cell(i, 9).getString());

			district.iterations.push_back
			({ name, hiden, disabledCells, disabledZones, disabledIdentifiers, carriedCells, carriedZones, carriedIdentifiers, disabledZoneCollapseIdentifiers });
			i++;
		}

		// Extract Zone Data.
		i = 5;
		while (zonesSheet.cell(i, 1).getString()[0] != '-') {
			if (zonesSheet.cell(i, 1).getString() == "") {
				i++; continue;
			}
			std::vector<std::vector<std::string>> positiveZoneIdentifierConditions;
			std::vector<std::vector<std::string>> negativeZoneIdentifierConditions;

			int j = 4;
			while (zonesSheet.cell(3, j).getString()[0] != '-') {
				positiveZoneIdentifierConditions.push_back(FindCommaValues(zonesSheet.cell(i, j).getString()));
				negativeZoneIdentifierConditions.push_back(FindCommaValues(zonesSheet.cell(i, j + 1).getString()));

				j += 2;
			}
			int zone = std::stoi(zonesSheet.cell(i, 1).getString());

			district.zones[zone].name = zonesSheet.cell(i, 2).getString();
			district.zones[zone].collapsedIdentifiers = FindIdentifierPairs(zonesSheet.cell(i, 3).getString());
			district.zones[zone].positiveZoneIdentifierConditions = positiveZoneIdentifierConditions;
			district.zones[zone].negativeZoneIdentifierConditions = negativeZoneIdentifierConditions;

			i++;
		}

		return district;
	}

	Entities ImportEntitiesSheet(const OpenXLSX::XLWorksheet& entitiesSheet, const OpenXLSX::XLWorksheet& identifiersSheet) {
		Entities entities;

		// Extract Entity Data.
		int i = 5;
		while (entitiesSheet.cell(i, 1).getString()[0] != '-') {
			if (entitiesSheet.cell(i, 1).getString() == "") {
				i++; continue;
			}
			Entity entity;

			entity.count = std::stoi(entitiesSheet.cell(i, 1).getString());

			entity.entityCellConditions = FindCellConditions(entitiesSheet.cell(i, 2).getString());
			entity.entityZoneConditions = FindZoneConditions(entitiesSheet.cell(i, 3).getString());
			
			int j = 4;
			while (entitiesSheet.cell(3, j).getString()[0] != '-') {
				IdentifierEntry identifier;
				
				identifier.value = entitiesSheet.cell(i, j).getString();
				identifier.conditions = FindCommaValues(entitiesSheet.cell(i, j + 1).getString());

				entity.identifiersValues.push_back(identifier);
				j += 2;
			}

			entities.entities.push_back(entity);
			i++;
		}

		// Extract Identifiers Data.
		i = 3;
		while (identifiersSheet.cell(i, 1).getString()[0] != '-') {
			if (identifiersSheet.cell(i, 1).getString() == "") {
				i++; continue;
			}
			std::string name = identifiersSheet.cell(i, 1).getString();

			int iterationCount = std::stoi(FindCommaValues(identifiersSheet.cell(i, 2).getString())[0]);

			std::vector<vec2>	relitiveCellConditions = FindCellConditions(identifiersSheet.cell(i, 3).getString());
			std::vector<int>	relitiveZoneConditions = FindZoneConditions(identifiersSheet.cell(i, 4).getString());

			entities.identifiers.push_back({ name, iterationCount, relitiveCellConditions, relitiveZoneConditions });
			i++;
		}

		return entities;
	}

	EOP_Config ImportEOP_ConfigXLSX(std::string filePath) {
		EOP_Config eopConfig;

		std::ifstream file;
		file.open(filePath);

		if (!file.good()) {
			EOP_LOG("Unable to Open File: " << filePath);

			file.close();
			return eopConfig;
		}
		file.close();

		OpenXLSX::XLDocument doc;
		doc.open(filePath);

		OpenXLSX::XLWorkbook wbk = doc.workbook();

		eopConfig.district = ImportDistrictSheet(wbk.sheet("district"), wbk.sheet("iterations"), wbk.sheet("zones"));
		eopConfig.entities = ImportEntitiesSheet(wbk.sheet("entities"), wbk.sheet("identifiers"));

		doc.close();

		return eopConfig;
	}

}
