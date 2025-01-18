#include "export_eop_config.h"

#include "OpenXLSX/OpenXLSX.hpp"

namespace eop {

	std::vector<int> GetIdentifierIndexes(const EOP_Config& eopConfig, std::string identifiers) {
		std::vector<int> identifiersIndexes;

		identifiers.erase(std::remove(identifiers.begin(), identifiers.end(), ' '), identifiers.end());
		std::stringstream ss(identifiers);

		int j = 0;
		std::string token;
		while (getline(ss, token, ',')) {
			for (int i = 0; i < eopConfig.entities.identifiers.size(); i++) {
				if (eopConfig.entities.identifiers[i].name == token) {
					identifiersIndexes.push_back(i);
				}
			}
			j++;
		}

		if (identifiers != "" && j == 0) {
			for (int i = 0; i < eopConfig.entities.identifiers.size(); i++) {
				if (eopConfig.entities.identifiers[i].name == token) {
					identifiersIndexes.push_back(i);
				}
			}
		}
		return identifiersIndexes;
	}

	void SetZonesSheet(const EOP_Config& eopConfig, OpenXLSX::XLWorksheet& zonesSheet) {
		zonesSheet.cell(1, 1).value() = "Organised Zones";

		zonesSheet.cell(2, 1).value() = "ID";
		zonesSheet.cell(2, 2).value() = "Name";
		zonesSheet.cell(2, 3).value() = "Collapsed Identifiers";

		for (int i = 0; i < eopConfig.district.iterations.size(); i++) {
			if (eopConfig.district.iterations[i].hide) {
				continue;
			}
			zonesSheet.cell(3, i + 3).value() = eopConfig.district.iterations[i].name;
		}

		for (int i = 0; i < eopConfig.district.zones.size(); i++) {
			zonesSheet.cell(i + 4, 1).value() = std::to_string(i);
			zonesSheet.cell(i + 4, 2).value() = eopConfig.district.zones[i].name;

			for (int j = 0; j < eopConfig.district.iterations.size(); j++) {
				if (i >= eopConfig.district.iterations[j].zoneCollapsedIdentifiers.size() || eopConfig.district.iterations[j].hide) {
					continue;
				}
				zonesSheet.cell(i + 4, j + 3).value() = eopConfig.district.iterations[j].zoneCollapsedIdentifiers[i];
			}
		}
	}

	void SetIterationSheet(const EOP_Config& eopConfig, OpenXLSX::XLWorksheet& iterationSheet, const std::vector<int>& identifierIndexes, int iteration) {
		iterationSheet.cell(1, 1).value() = "Organisation of Entities: " + eopConfig.district.iterations[iteration].name;
		
		for (int i = 0; i < eopConfig.district.rows; i++) {
			for (int j = 0; j < eopConfig.district.cols; j++) {
				std::string data = "";
				
				int entityId = eopConfig.district.iterations[iteration].cells[(i * eopConfig.district.cols) + j];
				
				if (entityId == -1) {
					data = "";
				}
				
				else if (identifierIndexes.size() == 0) {
					data = std::to_string(entityId);
				}
				else {
					for (int k = 0; k < identifierIndexes.size() - 1; k++)
						data += eopConfig.entities.entities[entityId].identifiersValues[identifierIndexes[k]].value + ", ";
					data += eopConfig.entities.entities[entityId].identifiersValues[identifierIndexes[identifierIndexes.size() - 1]].value;
				}

				if (data == "") {
					continue;
				}

				iterationSheet.cell(i + 2, j + 1).value() = data;
			}
		}
	}

	void SetIterationSheets(const EOP_Config& eopConfig, OpenXLSX::XLWorkbook& workbook, const std::vector<int>& identifierIndexes) {
		for (int i = 0; i < eopConfig.district.iterations.size(); i++) {
			if (eopConfig.district.iterations[i].hide) {
				continue;
			}
			std::string itertationName = eopConfig.district.iterations[i].name;

			OpenXLSX::XLWorksheet iterationSheet;
			if (!workbook.sheetExists(itertationName)) {
				workbook.addWorksheet(itertationName);
			}
			iterationSheet = workbook.sheet(itertationName);

			SetIterationSheet(eopConfig, iterationSheet, identifierIndexes, i);
		}
	}

	int ExportEOP_ConfigXLSX(std::string filePath, const EOP_Config& eopConfig, std::string identifiers) {
		std::vector<int> identifierIndexes = GetIdentifierIndexes(eopConfig, identifiers);

		filePath = filePath.substr(0, filePath.size() - 5) + "-org.xlsx";

		OpenXLSX::XLDocument doc;

		std::ifstream file;
		file.open(filePath);

		if (file.good()) {
			file.close();
			doc.open(filePath);
		}
		else {
			file.close();
			doc.create(filePath);
		}

		OpenXLSX::XLWorkbook wbk = doc.workbook();

		OpenXLSX::XLWorksheet zonesSheet;
		if (!wbk.sheetExists("zones")) {
			wbk.addWorksheet("zones");
		}
		zonesSheet = wbk.sheet("zones");

		SetZonesSheet(eopConfig, zonesSheet);

		SetIterationSheets(eopConfig, wbk, identifierIndexes);

		try {
			doc.save();
		} catch (...) {
			return 0;
		}
		doc.close();

		return 1;
	}

}
