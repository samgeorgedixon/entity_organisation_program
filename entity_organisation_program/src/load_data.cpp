#include "load_data.h"

namespace eop {
	
	std::vector<std::string> TokeniseXMLLine(std::string line) {
		if (line[0] != '<')
			return {};
		
		line = line.substr(1);
		line = line.substr(0, line.size() - 1);

		std::stringstream ss(line);
		std::string token;

		std::vector<std::string> tokens;

		while (getline(ss, token, ' ')) {
			tokens.push_back(token);
		}

		return tokens;
	}

	bool CheckXMLTag(std::string line, std::string tag) {
		std::vector<std::string> tokens = TokeniseXMLLine(line);

		if (tokens.size() == 0)
			return 0;

		if (tokens[0] == tag)
			return 1;

		return 0;
	}

	XMLLine ParseXMLLine(std::string line) {
		std::vector<std::string> tokens = TokeniseXMLLine(line);

		XMLLine xmlLine = { line, tokens[0], {} };

		for (int i = 1; i < tokens.size(); i++) {
			std::string name = tokens[i].substr(0, tokens[i].find("="));
			std::string value = tokens[i].substr(tokens[i].find("\"") + 1, tokens[i].size() - tokens[i].find("\"") - 2);

			xmlLine.variables.push_back({ name, value });
		}

		return xmlLine;
	}

	std::string FindXMLMiddle(std::string line, std::string tag) {
		int leftIndex = line.find(">") + 1;
		int rightIndex = line.find("</" + tag) - leftIndex;

		return line.substr(leftIndex, rightIndex);
	}

	std::vector<std::string> SplitXMLMiddle(std::string line, std::string tag) {
		int leftIndex = line.find(">") + 1;
		int rightIndex = line.find("</" + tag) - leftIndex;

		if (rightIndex < 0)
			return { line, line, line };

		return { line.substr(0, leftIndex), line.substr(leftIndex, rightIndex), line.substr(leftIndex + rightIndex) };
	}

	std::vector<vec2> FindCellConditions(std::string value) {
		std::vector<vec2> cellConditions;

		value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
		value.erase(std::remove(value.begin(), value.end(), '.'), value.end());
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
		value.erase(std::remove(value.begin(), value.end(), '.'), value.end());
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
		value.erase(std::remove(value.begin(), value.end(), '.'), value.end());
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
		value.erase(std::remove(value.begin(), value.end(), '.'), value.end());
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

	std::vector<std::string> ReadFileLines(std::string filePath) {
		std::ifstream file(filePath);
		if (!file.good()) {
			LOG("Unable to Open XML File: " << filePath << "\n");
			return {};
		}

		// Read All Lines.
		std::string line;
		std::vector<std::string> lines;

		while (getline(file, line)) {
			line.erase(line.begin(), std::find_if(line.begin(), line.end(), std::bind1st(std::not_equal_to<char>(), ' ')));
			if (line[0] != '<') {
				lines[lines.size() - 1] += " " + line;
			}
			else {
				lines.push_back(line);
			}
		}
		file.close();
		return lines;
	}

	EOP_Config GenerateEOP_Config(std::vector<Worksheet> worksheets) {
		Worksheet districtSheet;
		Worksheet entitiesSheet;
		Worksheet identifiersSheet;
		Worksheet iterationsSheet;
		Worksheet zonesSheet;

		District district;
		Entities entities;

		for (int i = 0; i < worksheets.size(); i++) {
			if (worksheets[i].name.substr(0, 9) == "district")
				districtSheet = worksheets[i];
			else if (worksheets[i].name.substr(0, 9) == "entities")
				entitiesSheet = worksheets[i];
			else if (worksheets[i].name.substr(0, 12) == "identifiers")
				identifiersSheet = worksheets[i];
			else if (worksheets[i].name.substr(0, 11) == "iterations")
				iterationsSheet = worksheets[i];
			else if (worksheets[i].name.substr(0, 6) == "zones")
				zonesSheet = worksheets[i];
		}

		// Extract District Data.

		// Find Rows and Cols.
		for (int i = 0; i < districtSheet.table[1].size(); i++) {
			if (districtSheet.table[1][i][0] == '-')
				district.cols = i;
		}
		for (int i = 1; i < districtSheet.table.size(); i++) {
			if (districtSheet.table[i][0][0] == '-')
				district.rows = i - 1;
		}

		// Collect Cell Data.
		for (int i = 0; i < district.rows; i++) {
			for (int j = 0; j < district.cols; j++) {
				std::string cell = districtSheet.table[i + 1][j];
				cell.erase(std::remove(cell.begin(), cell.end(), ' '), cell.end());

				if (cell[0] == '.') {
					district.occupiableCells.push_back(false);
					continue;
				} else {
					district.occupiableCells.push_back(true);
				}

				int zone = std::stoi(cell);

				while (district.zones.size() <= zone) {
					district.zones.push_back({});
				}
				district.zones[zone].cells.push_back({ j, i });
			}
		}

		// Extract Identifiers Data.
		int i = 2;
		while (identifiersSheet.table[i][0][0] != '-') {
			if (identifiersSheet.table[i][0][0] == '.') {
				i++; continue;
			}
			std::string name = identifiersSheet.table[i][0];

			int iterationCount = std::stoi(FindCommaValues(identifiersSheet.table[i][1])[0]);

			std::vector<vec2>	relitiveCellConditions = FindCellConditions(identifiersSheet.table[i][2]);
			std::vector<int>	relitiveZoneConditions = FindZoneConditions(identifiersSheet.table[i][3]);

			entities.identifiers.push_back({ name, iterationCount, relitiveCellConditions, relitiveZoneConditions });
			i++;
		}

		// Extract Iterations Data.
		i = 2;
		while (iterationsSheet.table[i][0][0] != '-') {
			if (iterationsSheet.table[i][0][0] == '.') {
				i++; continue;
			}
			std::string name = iterationsSheet.table[i][0];

			bool hiden = false;
			if (iterationsSheet.table[i][1] == "T")
				hiden = true;

			std::vector<vec2>	disabledCells = FindCellConditions(iterationsSheet.table[i][2]);
			std::vector<int>	disabledZones = FindZoneConditions(iterationsSheet.table[i][3]);
			std::vector<std::pair<std::string, std::string>> disabledIdentifiers = FindIdentifierPairs(iterationsSheet.table[i][4]);

			std::vector<vec2>	carriedCells = FindCellConditions(iterationsSheet.table[i][5]);
			std::vector<int>	carriedZones = FindZoneConditions(iterationsSheet.table[i][6]);
			std::vector<std::pair<std::string, std::string>> carriedIdentifiers = FindIdentifierPairs(iterationsSheet.table[i][7]);

			std::vector<std::pair<std::string, std::string>> disabledZoneCollapseIdentifiers = FindIdentifierPairs(iterationsSheet.table[i][8]);

			district.iterations.push_back
				({ name, hiden, disabledCells, disabledZones, disabledIdentifiers, carriedCells, carriedZones, carriedIdentifiers, disabledZoneCollapseIdentifiers });
			i++;
		}

		// Extract Zone Data.
		i = 4;
		while (zonesSheet.table[i][0][0] != '-') {
			if (zonesSheet.table[i][0][0] == '.') {
				i++; continue;
			}
			std::vector<std::vector<std::string>> positiveZoneIdentifierConditions;
			std::vector<std::vector<std::string>> negativeZoneIdentifierConditions;
			
			int j = 0;
			int index = 3;
			while (zonesSheet.table[2][j][0] != '-') {
				positiveZoneIdentifierConditions.push_back(FindCommaValues(zonesSheet.table[i][index]));
				negativeZoneIdentifierConditions.push_back(FindCommaValues(zonesSheet.table[i][index + 1]));
				j++;
				index += 2;
			}
			int zone = std::stoi(zonesSheet.table[i][0]);

			district.zones[zone].name = zonesSheet.table[i][1];
			district.zones[zone].collapsedIdentifiers = FindIdentifierPairs(zonesSheet.table[i][2]);
			district.zones[zone].positiveZoneIdentifierConditions = positiveZoneIdentifierConditions;
			district.zones[zone].negativeZoneIdentifierConditions = negativeZoneIdentifierConditions;

			i++;
		}

		// Extract Entity Data.
		i = 4;
		while (entitiesSheet.table[i][0][0] != '-') {
			if (entitiesSheet.table[i][0][0] == '.') {
				i++; continue;
			}
			Entity entity;

			entity.count = std::stoi(entitiesSheet.table[i][0]);

			entity.entityCellConditions = FindCellConditions(entitiesSheet.table[i][1]);
			entity.entityZoneConditions = FindZoneConditions(entitiesSheet.table[i][2]);

			int j = 0;
			while (entitiesSheet.table[2][j][0] != '-') {
				IdentifierEntry identifier;

				identifier.value = entitiesSheet.table[i][3 + (j * 2)];
				identifier.conditions = FindCommaValues(entitiesSheet.table[i][3 + (j * 2) + 1]);

				entity.identifiersValues.push_back(identifier);
				j++;
			}

			entities.entities.push_back(entity);
			i++;
		}

		return { district, entities };
	}

	EOP_Config LoadXMLFile(std::string filePath) {
		std::vector<std::string> lines = ReadFileLines(filePath);

		// Collect Worksheets Data.
		int worksheetIndex = 0;
		std::vector<Worksheet> worksheets;

		for (int i = 0; i < lines.size(); i++) {
			if (!CheckXMLTag(lines[i], "Worksheet"))
				continue;

			XMLLine xmlLine = ParseXMLLine(lines[i]);

			for (int i = 0; i < xmlLine.variables.size(); i++) {
				if (xmlLine.variables[i].name == "ss:Name")
					worksheets.push_back({ xmlLine.variables[i].value, {} });
			}
			i++;

			for (i; i < lines.size(); i++) {
				if (CheckXMLTag(lines[i], "Row")) {
					std::vector<std::string> row;
					i++;

					for (i; i < lines.size(); i++) {
						if (CheckXMLTag(SplitXMLMiddle(lines[i], "Cell")[0], "Cell")) {
							row.push_back(FindXMLMiddle(FindXMLMiddle(lines[i], "Cell"), "Data"));
						}
						else if (CheckXMLTag(lines[i], "Row")) {
							worksheets[worksheetIndex].table.push_back(row);
							i--; break;
						}
						else if (CheckXMLTag(lines[i], "Worksheet")) {
							worksheets[worksheetIndex].table.push_back(row);
							i--; break;
						}
						else if (CheckXMLTag(lines[i], "/Workbook")) {
							worksheets[worksheetIndex].table.push_back(row);
							i--; break;
						}
					}
				}
				else if (CheckXMLTag(lines[i], "Worksheet")) {
					i--; break;
				}
			}
			worksheetIndex++;
		}
		return GenerateEOP_Config(worksheets);
	}

	std::vector<std::string> worksheetTemplateLines = {
		"<Table ss:DefaultColumnWidth=\"66.75\" ss:DefaultRowHeight=\"30\">",
		"<Row ss:AutoFitHeight=\"0\">",
		"<Cell><Data ss:Type=\"String\"></Data></Cell>",
		"</Row>",
		"</Table>",
		"<WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\">",
		"<PageSetup>",
		"<Header x:Margin=\"0.3\"/>",
		"<Footer x:Margin=\"0.3\"/>",
		"<PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/>",
		"</PageSetup>",
		"<Unsynced/>",
		"<Print>",
		"<ValidPrinterInfo/>",
		"<PaperSizeIndex>9</PaperSizeIndex>",
		"<HorizontalResolution>600</HorizontalResolution>",
		"<VerticalResolution>600</VerticalResolution>",
		"</Print>",
		"<Zoom>100</Zoom>",
		"<Selected/>",
		"<Panes>",
		"<Pane>",
		"<Number>3</Number>",
		"<ActiveRow>3</ActiveRow>",
		"<ActiveCol>5</ActiveCol>",
		"</Pane>",
		"</Panes>",
		"<ProtectObjects>False</ProtectObjects>",
		"<ProtectScenarios>False</ProtectScenarios>",
		"</WorksheetOptions>",
		"</Worksheet>"
	};

	int FindCreateWorksheet(std::vector<std::string>& lines, std::string worksheetName) {
		int worksheetIndex = -1;
		int lineIndex = -1;
		int lastLineIndex = -1;

		for (int i = 0; i < lines.size(); i++) {
			if (CheckXMLTag(lines[i], "Worksheet")) {
				worksheetIndex++;

				XMLLine xmlLine = ParseXMLLine(lines[i]);
				for (int j = 0; j < xmlLine.variables.size(); j++) {
					if (xmlLine.variables[j].name == "ss:Name" && xmlLine.variables[j].value == worksheetName) {
						lineIndex = i;
						break;
					}
				}
				i++;
			}

			if (CheckXMLTag(lines[i], "/Worksheet"))
				lastLineIndex = i;
		}

		if (lineIndex == -1) {
			lines.insert(lines.begin() + lastLineIndex + 1, "<Worksheet ss:Name=\"" + worksheetName + "\">");
			for (int i = 0; i < worksheetTemplateLines.size(); i++) {
				lines.insert(lines.begin() + lastLineIndex + 2 + i, worksheetTemplateLines[i]);
			}
			lineIndex = lastLineIndex + 1;
		}
		return lineIndex;
	}

	void PrintTableToWorksheetLines(std::vector<std::string>& lines, int lineIndex, std::string title, std::vector<std::vector<std::string>>& table) {
		int row = -1;

		for (int i = lineIndex + 1; i < lines.size(); i++) {
			if (CheckXMLTag(lines[i], "/Table") && row < table.size()) {
				lines.insert(lines.begin() + i, "<Row ss:AutoFitHeight=\"0\">");
				lines.insert(lines.begin() + i + 1, "</Row>");
			}

			if (!CheckXMLTag(lines[i], "Row")) {
				continue;
			}
			if (row >= table.size() && row != -1) {
				break;
			}

			if (row == -1) {
				i++;
				if (!CheckXMLTag(SplitXMLMiddle(lines[i], "Cell")[0], "Cell"))
					lines.insert(lines.begin() + i, "<Cell><Data ss:Type=\"String\"></Data></Cell>");

				std::vector<std::string> splitLine = SplitXMLMiddle(lines[i], "Cell");
				lines[i] = splitLine[0] + "<Data ss:Type=\"String\">" + title + "</Data>" + splitLine[2];

				row++; continue;
			}

			i += 1;
			for (int j = 0; j < table[row].size(); j++) {
				std::string data = table[row][j];

				if (CheckXMLTag(lines[i], "Cell")) {
					std::vector<std::string> splitLine = SplitXMLMiddle(lines[i], "Cell");
					lines[i] = splitLine[0] + "<Data ss:Type=\"String\">" + data + "</Data>" + splitLine[2];
				}
				else if (CheckXMLTag(lines[i], "/Row")) {
					lines.insert(lines.begin() + i, "<Cell><Data ss:Type=\"String\">" + data + "</Data></Cell>");
				}
				i++;
			}
			row++;
		}
	}

	void SetWorksheetIterationLines(EOP_Config& eopConfig, std::vector<int>& identifierIndexes, std::vector<std::string>& lines, int iteration) {
		std::string iterationName = "org-" + eopConfig.district.iterations[iteration].name;

		int lineIndex = FindCreateWorksheet(lines, iterationName);

		std::vector<std::string>				defaultRow(eopConfig.district.cols, "");
		std::vector<std::vector<std::string>>	table(eopConfig.district.rows, defaultRow);

		// Set Table
		for (int i = 0; i < table.size(); i++) {
			for (int j = 0; j < table[i].size(); j++) {
				std::string data = "";

				int entityId = eopConfig.district.iterations[iteration].cells[(i * eopConfig.district.cols) + j];

				if (entityId == -1)
					data = ".";

				else if (identifierIndexes.size() == 0)
					data = entityId;
				else {
					for (int k = 0; k < identifierIndexes.size() - 1; k++)
						data += eopConfig.entities.entities[entityId].identifiersValues[identifierIndexes[k]].value + ", ";
					data += eopConfig.entities.entities[entityId].identifiersValues[identifierIndexes[identifierIndexes.size() - 1]].value;
				}
				table[i][j] = data;
			}
		}

		PrintTableToWorksheetLines(lines, lineIndex, "Organisation of Entities: " + iterationName.substr(4), table);
	}

	void SetWorksheetZoneLines(EOP_Config& eopConfig, std::vector<int>& identifierIndexes, std::vector<std::string>& lines) {
		std::string worksheetName = "org-zones";
		
		int lineIndex = FindCreateWorksheet(lines, worksheetName);
		std::vector<std::vector<std::string>> table;

		// Set Table
		table.push_back({ "ID", "Name", "Collapsed Identifiers" });

		std::vector<std::string> iterationsRow;

		for (int i = 0; i < eopConfig.district.iterations.size(); i++) {
			if (eopConfig.district.iterations[i].hide) {
				continue;
			}
			iterationsRow.push_back(eopConfig.district.iterations[i].name);
		}
		table.push_back(iterationsRow);
		
		for (int i = 0; i < eopConfig.district.zones.size(); i++) {
			std::vector<std::string> row = { std::to_string(i), eopConfig.district.zones[i].name };

			for (int j = 0; j < eopConfig.district.iterations.size(); j++) {
				if (i >= eopConfig.district.iterations[j].zoneCollapsedIdentifiers.size() || eopConfig.district.iterations[j].hide) {
					continue;
				}
				row.push_back(eopConfig.district.iterations[j].zoneCollapsedIdentifiers[i]);
			}
			table.push_back(row);
		}

		PrintTableToWorksheetLines(lines, lineIndex, "Organised Zones", table);
	}

	int WriteXML_EOPConfig(std::string filePath, EOP_Config& eopConfig, std::string identifier) {
		std::vector<std::string> lines = ReadFileLines(filePath);
		std::vector<std::string> worksheetLines;

		std::vector<int> identifierIndexes;

		// Backup XML File.
		std::ofstream xmlFileBakup(filePath + ".bak");

		if (xmlFileBakup.fail()) {
			xmlFileBakup.close();
			return 0;
		}

		std::ostream_iterator<std::string> backup_iterator(xmlFileBakup, "\n");
		std::copy(lines.begin(), lines.end(), backup_iterator);
		
		xmlFileBakup.close();

		identifier.erase(std::remove(identifier.begin(), identifier.end(), ' '), identifier.end());
		std::stringstream ss(identifier);

		int j = 0;
		std::string token;

		while (getline(ss, token, ',')) {
			for (int i = 0; i < eopConfig.entities.identifiers.size(); i++) {
				if (eopConfig.entities.identifiers[i].name == token)
					identifierIndexes.push_back(i);
			}
			j++;
		}

		if (identifier != "" && j == 0) {
			for (int i = 0; i < eopConfig.entities.identifiers.size(); i++) {
				if (eopConfig.entities.identifiers[i].name == token)
					identifierIndexes.push_back(i);
			}
		}
		
		SetWorksheetZoneLines(eopConfig, identifierIndexes, lines);
		
		for (int i = 0; i < eopConfig.district.iterations.size(); i++) {
			if (eopConfig.district.iterations[i].hide) {
				continue;
			}
			SetWorksheetIterationLines(eopConfig, identifierIndexes, lines, i);
		}

		// Write to XML File.
		std::ofstream xmlFile(filePath);

		if (xmlFile.fail()) {
			xmlFile.close();
			return 0;
		}

		std::ostream_iterator<std::string> output_iterator(xmlFile, "\n");
		std::copy(lines.begin(), lines.end(), output_iterator);

		xmlFile.close();

		return 1;
	}

}
