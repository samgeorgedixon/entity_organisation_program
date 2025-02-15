#include "import_lua_config.h"

#include "OpenXLSX/OpenXLSX.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace eop {

	std::vector<std::vector<std::string>> ImportSheetTable(std::string filePath, std::string sheetName) {
		std::vector<std::vector<std::string>> table;
		
		std::ifstream file;
		file.open(filePath);

		if (!file.good()) {
			EOP_LOG("Unable to Open File: " << filePath);

			file.close();
			return {};
		}
		file.close();

		OpenXLSX::XLDocument doc;
		doc.open(filePath);

		OpenXLSX::XLWorkbook wbk = doc.workbook();
		OpenXLSX::XLWorksheet sheet = wbk.sheet(sheetName);

		int rowCount = 0;
		int colCount = 0;

		int i = 0;
		for (i = 0; i < sheet.rowCount(); i++) {
			if (sheet.cell(i + 1, 1).value().getString()[0] == '-') {
				break;
			}
		}
		rowCount = i;
		for (i = 0; i < sheet.columnCount(); i++) {
			if (sheet.cell(1, i + 1).value().getString()[0] == '-') {

				break;
			}
		}
		colCount = i;

		for (int i = 0; i < rowCount; i++) {
			table.push_back(std::vector<std::string>(colCount, ""));

			for (int j = 0; j < colCount; j++) {
				table[i][j] = sheet.cell(i + 1, j + 1).value().getString();
			}
		}

		doc.close();

		return table;
	}

	std::vector<int> lua_ConvertVectorIntTable(const sol::table& table) {
		std::vector<int> converted;
		for (int i = 0; i < table.size(); i++) {
			converted.push_back(table[i + 1].get<int>());
		}
		return converted;
	}
	std::vector<std::string> lua_ConvertVectorStringTable(const sol::table& table) {
		std::vector<std::string> converted;
		for (int i = 0; i < table.size(); i++) {
			converted.push_back(table[i + 1].get<std::string>());
		}
		return converted;
	}
	std::vector<vec2> lua_ConvertVectorVec2Table(const sol::table& table) {
		std::vector<vec2> converted;
		for (int i = 0; i < table.size(); i++) {
			converted.push_back({ table[i + 1]["x"].get<int>(), table[i + 1]["y"].get<int>() });
		}
		return converted;
	}
	std::vector<value_pair> lua_ConvertVectorValuePairTable(const sol::table& table) {
		std::vector<value_pair> converted;
		for (int i = 0; i < table.size(); i++) {
			converted.push_back({ table[i + 1]["name"].get<std::string>(), table[i + 1]["value"].get<std::string>() });
		}
		return converted;
	}

	EOP_Config lua_ConvertEOP_ConfigTable(const sol::table& eopConfigTable) {
		EOP_Config eopConfig;

		eopConfig.district.rows = eopConfigTable["district"]["rows"].get<int>();
		eopConfig.district.cols = eopConfigTable["district"]["cols"].get<int>();

		// Import Cells
		for (int i = 0; i < eopConfig.district.rows; i++) {
			for (int j = 0; j < eopConfig.district.cols; j++) {
				eopConfig.district.occupiableCells.push_back(eopConfigTable["district"]["occupiableCells"][i + 1][j + 1].get_or(false));
			}
		}

		// Import Zones
		const sol::table& zones = eopConfigTable["district"]["zones"].get<sol::table>();
		for (int i = 0; i < zones.size(); i++) {
			Zone zone;

			zone.name = zones[i + 1]["name"];

			zone.cells = lua_ConvertVectorVec2Table(zones[i + 1]["cells"].get<sol::table>());
			zone.collapsedIdentifiers = lua_ConvertVectorValuePairTable(zones[i + 1]["collapsedIdentifiers"].get<sol::table>());

			const sol::table& positiveZoneIdentifierConditions = zones[i + 1]["positiveZoneIdentifierConditions"].get<sol::table>();

			for (int j = 0; j < positiveZoneIdentifierConditions.size(); j++) {
				zone.positiveZoneIdentifierConditions.push_back(lua_ConvertVectorStringTable(positiveZoneIdentifierConditions[j + 1].get<sol::table>()));
			}

			const sol::table& negativeZoneIdentifierConditions = zones[i + 1]["negativeZoneIdentifierConditions"].get<sol::table>();

			for (int j = 0; j < negativeZoneIdentifierConditions.size(); j++) {
				zone.negativeZoneIdentifierConditions.push_back(lua_ConvertVectorStringTable(negativeZoneIdentifierConditions[j + 1].get<sol::table>()));
			}
			eopConfig.district.zones.push_back(zone);
		}

		// Import Entities
		const sol::table& entities = eopConfigTable["entities"]["entities"].get<sol::table>();
		for (int i = 0; i < entities.size(); i++) {
			Entity entity;

			entity.count = entities[i + 1]["count"].get<int>();

			entity.entityCellConditions = lua_ConvertVectorVec2Table(entities[i + 1]["entityCellConditions"].get<sol::table>());
			entity.entityZoneConditions = lua_ConvertVectorIntTable(entities[i + 1]["entityCellConditions"].get<sol::table>());

			const sol::table& identifiersValues = entities[i + 1]["identifiersValues"].get<sol::table>();
			for (int j = 0; j < identifiersValues.size(); j++) {
				IdentifierEntry identifierEntry;

				identifierEntry.value = identifiersValues[j + 1]["value"].get<std::string>();
				identifierEntry.conditions = lua_ConvertVectorStringTable(identifiersValues[j + 1]["conditions"].get<sol::table>());

				entity.identifiersValues.push_back(identifierEntry);
			}

			eopConfig.entities.entities.push_back(entity);
		}

		// Import Identifiers
		const sol::table& identifiers = eopConfigTable["entities"]["identifiers"].get<sol::table>();
		for (int i = 0; i < identifiers.size(); i++) {
			Identifier identifier;

			identifier.name = identifiers[i + 1]["name"].get<std::string>();
			identifier.iterationCount = identifiers[i + 1]["iterationCount"].get<int>();

			identifier.relitiveCellConditions = lua_ConvertVectorVec2Table(identifiers[i + 1]["relitiveCellConditions"].get<sol::table>());
			identifier.relitiveZoneConditions = lua_ConvertVectorIntTable(identifiers[i + 1]["relitiveZoneConditions"].get<sol::table>());

			eopConfig.entities.identifiers.push_back(identifier);
		}

		std::vector<value_pair> sp = lua_ConvertVectorValuePairTable(eopConfigTable["district"]["iterations"][1]["disabledIdentifiers"]);

		// Import Iterations
		const sol::table& iterations = eopConfigTable["district"]["iterations"].get<sol::table>();
		for (int i = 0; i < iterations.size(); i++) {
			Iteration iteration;

			iteration.name = iterations[i + 1]["name"].get<std::string>();

			iteration.hide = iterations[i + 1]["hide"].get_or(false);
			iteration.disableDropIterationCount = iterations[i + 1]["disableDropIterationCount"].get_or(false);

			iteration.disabledCells = lua_ConvertVectorVec2Table(iterations[i + 1]["disabledCells"].get<sol::table>());
			iteration.disabledZones = lua_ConvertVectorIntTable(iterations[i + 1]["disabledZones"].get<sol::table>());
			iteration.disabledIdentifiers = lua_ConvertVectorValuePairTable(iterations[i + 1]["disabledIdentifiers"].get<sol::table>());

			const sol::table& carriedCells = iterations[i + 1]["carriedCells"].get<sol::table>();
			for (int j = 0; j < carriedCells.size(); j++) {
				iteration.carriedCells.push_back({ carriedCells[j + 1][1].get<std::string>(), lua_ConvertVectorVec2Table(carriedCells[j + 1][2].get<sol::table>()) });
			}

			const sol::table& carriedZones = iterations[i + 1]["carriedZones"].get<sol::table>();
			for (int j = 0; j < carriedZones.size(); j++) {
				iteration.carriedZones.push_back({ carriedZones[j + 1][1].get<std::string>(), lua_ConvertVectorIntTable(carriedZones[j + 1][2].get<sol::table>()) });
			}

			const sol::table& carriedIdentifiers = iterations[i + 1]["carriedIdentifiers"].get<sol::table>();
			for (int j = 0; j < carriedIdentifiers.size(); j++) {
				iteration.carriedIdentifiers.push_back({ carriedIdentifiers[j + 1][1].get<std::string>(), lua_ConvertVectorValuePairTable(carriedIdentifiers[j + 1][2].get<sol::table>()) });
			}

			iteration.disabledZoneCollapseIdentifiers = lua_ConvertVectorValuePairTable(iterations[i + 1]["disabledZoneCollapseIdentifiers"].get<sol::table>());

			eopConfig.district.iterations.push_back(iteration);
		}

		return eopConfig;
	}

	bool m_importResult = true, m_exportResult = true;

	std::string m_importSpreadsheetFilePath = "", m_exportSpreadsheetFilePath = "";
	std::string m_identifiers = "";

	int m_depth = 1;
	bool m_fullRandom = false, m_entitiesRandom = true;

	void lua_EvaluateEOP_Config(sol::table eopConfigTable) {
		EOP_Config eop_config = lua_ConvertEOP_ConfigTable(eopConfigTable);

		if (eop_config.district.rows == 0 && eop_config.district.cols == 0) {
			m_importResult = false; return;
		}

		EvaluateEOP_Config(eop_config, m_depth, m_fullRandom, m_entitiesRandom);

		PrintEOP_Config(eop_config, m_identifiers);

		int outRes = ExportEOP_ConfigXLSX(m_exportSpreadsheetFilePath, eop_config, m_identifiers);

		if (!outRes) {
			m_exportResult = false; return;
		}
	}

	std::pair<bool, bool> RunLuaConfig(std::string luaConfigFilePath, std::string importSpreadsheetFilePath, std::string exportSpreadsheetFilePath, int depth, bool fullRandom, bool entitiesRandom, std::string identifiers) {
		m_importSpreadsheetFilePath = importSpreadsheetFilePath;
		m_exportSpreadsheetFilePath = exportSpreadsheetFilePath;

		m_depth = depth;
		m_fullRandom = fullRandom;
		m_entitiesRandom = entitiesRandom;
		m_identifiers = identifiers;

		sol::state lua;
		lua.open_libraries(sol::lib::base, sol::lib::io, sol::lib::math, sol::lib::table, sol::lib::string);

		lua.require_file("eop", "eop.lua");

		lua["eop"]["spreadsheetFilePath"] = importSpreadsheetFilePath;

		lua["eop"]["ImportSheetTable"] = &ImportSheetTable;
		lua["eop"]["EvaluateEOP_Config"] = &lua_EvaluateEOP_Config;

		m_importResult = true;
		m_exportResult = true;

		try {
			lua.safe_script_file(luaConfigFilePath);
		}
		catch (const sol::error& e) {
			EOP_LOG(e.what() << "\n");
			return {};
		}

		return { m_importResult, m_exportResult };
	}

}
