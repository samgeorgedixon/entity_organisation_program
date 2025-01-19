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

#ifdef EOP_DISABLE_LOGGING

	void PrintEOP_ConfigIteration(const EOP_Config& eopConfig, std::string identifiers, int iteration) {

	}

	void PrintEOP_Config(const EOP_Config& eopConfig, std::string identifiers) {

	}

#else

	void PrintEOP_ConfigIteration(const EOP_Config& eopConfig, std::string identifiers, int iteration) {
		std::vector<int> identifierIndexes = GetIdentifierIndexes(eopConfig, identifiers);

		int j = 0;

		EOP_LOG("Row: - ");
		for (int i = 0; i < eopConfig.district.rows * eopConfig.district.cols; i++) {
			int id = eopConfig.district.iterations[iteration].cells[i];

			if (id == -1) {
				EOP_LOG("NA : ");
			}
			else {
				for (int k = 0; k < identifierIndexes.size(); k++) {
					EOP_LOG(eopConfig.entities.entities[id].identifiersValues[identifierIndexes[k]].value);
					if (k < identifierIndexes.size() - 1) {
						EOP_LOG(", ");
					}
				}
				EOP_LOG(" : ");
			}

			j++;
			if (j >= eopConfig.district.cols && i + 1 < eopConfig.district.rows * eopConfig.district.cols) {
				EOP_LOG("\nRow: - ");
				j = 0;
			}
		}
		EOP_LOG("\n");
	}

	void PrintEOP_Config(const EOP_Config& eopConfig, std::string identifiers) {
		for (int i = 0; i < eopConfig.district.iterations.size(); i++) {
			PrintEOP_ConfigIteration(eopConfig, identifiers, i);

			if (i != eopConfig.district.iterations.size() - 1) {
				EOP_LOG("---\n");
			}
		}
	}

#endif

	void SetZonesSheet(const EOP_Config& eopConfig, OpenXLSX::XLWorksheet& zonesSheet) {
		zonesSheet.cell(1, 1).value() = "Organised Zones";

		zonesSheet.cell(2, 1).value() = "ID";
		zonesSheet.cell(2, 2).value() = "Name";
		zonesSheet.cell(2, 3).value() = "Collapsed Identifiers";

		int hiddenCount = 0;
		for (int i = 0; i < eopConfig.district.iterations.size(); i++) {
			if (eopConfig.district.iterations[i].hide) {
				hiddenCount++;
				continue;
			}
			zonesSheet.cell(3, (i + 3) - hiddenCount).value() = eopConfig.district.iterations[i].name;
		}
		hiddenCount = 0;
		
		for (int i = 0; i < eopConfig.district.zones.size(); i++) {
			zonesSheet.cell(i + 4, 1).value() = std::to_string(i);
			zonesSheet.cell(i + 4, 2).value() = eopConfig.district.zones[i].name;

			for (int j = 0; j < eopConfig.district.iterations.size(); j++) {
				if (eopConfig.district.iterations[j].hide) {
					hiddenCount++;
					continue;
				}
				if (i >= eopConfig.district.iterations[j].zoneCollapsedIdentifiers.size()) {
					continue;
				}
				
				zonesSheet.cell(i + 4, (j + 3) - hiddenCount).value() = eopConfig.district.iterations[j].zoneCollapsedIdentifiers[i];
			}
			hiddenCount = 0;
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
			EOP_LOG("Unable to Write File : " << filePath);
			return 0;
		}
		doc.close();

		return 1;
	}

}
