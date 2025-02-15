eop_config = eop.GetDefaultConfig()

-- Import District Sheet
districtSheet = eop.ImportSheetTable(eop.spreadsheetFilePath, "district")

eop_config["district"]["rows"] = #districtSheet - 1
eop_config["district"]["cols"] = #districtSheet[1]

for i = 1, #districtSheet - 1 do
    eop_config["district"]["occupiableCells"][i] = {}

    for j = 1, #districtSheet[2] do
        if districtSheet[i + 1][j] ~= "" then
            eop_config["district"]["occupiableCells"][i][j] = true            
            
            local zoneIndex = tonumber(districtSheet[i + 1][j]) + 1
            local zone = eop.CheckZoneNil(eop_config["district"]["zones"][zoneIndex])

            zone["cells"][#zone["cells"] + 1] = {}
            zone["cells"][#zone["cells"]]["x"] = j - 1
            zone["cells"][#zone["cells"]]["y"] = i - 1

            eop_config["district"]["zones"][zoneIndex] = zone
        else
            eop_config["district"]["occupiableCells"][i][j] = false
        end
    end
end

-- Import Entities Sheet
entitiesSheet = eop.ImportSheetTable(eop.spreadsheetFilePath, "entities")

for i = 1, #entitiesSheet - 4 do
    if entitiesSheet[i + 4][1] ~= "" then
        local entityIndex = #eop_config["entities"]["entities"] + 1

        local entity = eop.CheckEntityNil(eop_config["entities"]["entities"][entityIndex])

        entity["count"] = tonumber(entitiesSheet[i + 4][1])

        entity["entityCellConditions"] = eop.StringToVec2List(entitiesSheet[i + 4][2])
        entity["entityZoneConditions"] = eop.StringToIntList(entitiesSheet[i + 4][3])

        local j = 4;
        while j <= #entitiesSheet[1] do
            local identifier = {}

            identifier["value"] = entitiesSheet[i + 4][j]
            identifier["conditions"] = eop.StringToStringList(entitiesSheet[i + 4][j + 1])

            entity["identifiersValues"][#entity["identifiersValues"] + 1] = identifier
	        j = j + 2
        end

        eop_config["entities"]["entities"][entityIndex] = entity
    end
end

-- Import Identifiers Sheet
identifiersSheet = eop.ImportSheetTable(eop.spreadsheetFilePath, "identifiers")

for i = 1, #identifiersSheet - 2 do
    if identifiersSheet[i + 2][1] ~= "" then
        local identifierIndex = #eop_config["entities"]["identifiers"] + 1

        local identifier = eop.CheckIdentifierNil(eop_config["entities"]["identifiers"][identifierIndex])

        identifier["name"] = identifiersSheet[i + 2][1]
        identifier["iterationCount"] = tonumber(identifiersSheet[i + 2][2])

        identifier["relitiveCellConditions"] = eop.StringToVec2List(identifiersSheet[i + 2][3])
        identifier["relitiveZoneConditions"] = eop.StringToIntList(identifiersSheet[i + 2][4])

        eop_config["entities"]["identifiers"][identifierIndex] = identifier
    end
end

-- Import Iterations Sheet
iterationsSheet = eop.ImportSheetTable(eop.spreadsheetFilePath, "iterations")

for i = 1, #iterationsSheet - 2 do
    if iterationsSheet[i + 2][1] ~= "" then
        local iterationIndex = #eop_config["district"]["iterations"] + 1

        local iteration = eop.CheckIterationNil(eop_config["district"]["iterations"][iterationIndex])

        iteration["name"] = iterationsSheet[i + 2][1]

        iteration["hide"] = eop.CheckEqualString(string.lower(iterationsSheet[i + 2][2]), "t")
        iteration["disableDropIterationCount"] = eop.CheckEqualString(string.lower(iterationsSheet[i + 2][3]), "t")

        iteration["disabledCells"] = eop.StringToVec2List(iterationsSheet[i + 2][4])
        iteration["disabledZones"] = eop.StringToIntList(iterationsSheet[i + 2][5])
        iteration["disabledIdentifiers"] = eop.StringToValuePairList(iterationsSheet[i + 2][6], ")")

        -- Get Carried Cells
        local carriedCellsStringList = eop.StringSquareBracketToStringList(iterationsSheet[i + 2][7])

        local j = 1;
        while j <= #carriedCellsStringList do
            local carriedCellStringList = eop.StringToStringList(carriedCellsStringList[j])

            iteration["carriedCells"][j] = {}
            iteration["carriedCells"][j][1] = carriedCellStringList[1]
            iteration["carriedCells"][j][2] = eop.StringToVec2List(carriedCellStringList[2])

            j = j + 1
        end

        -- Get Carried Zones
        local carriedZonesStringList = eop.StringSquareBracketToStringList(iterationsSheet[i + 2][8])

        j = 1;
        while j <= #carriedZonesStringList do
            local carriedZoneStringList = eop.StringToStringList(carriedZonesStringList[j])

            iteration["carriedZones"][j] = {}
            iteration["carriedZones"][j][1] = carriedZoneStringList[1]
            iteration["carriedZones"][j][2] = eop.StringToIntList(carriedZoneStringList[2])

            j = j + 1
        end

        -- Get Carried Identifiers
        local carriedIdentifiersStringList = eop.StringSquareBracketToStringList(iterationsSheet[i + 2][9])

        j = 1;
        while j <= #carriedIdentifiersStringList do
            local first, second = carriedIdentifiersStringList[j]:match("([^,]*),?(.*)")

            print(first)
            print(second)
            
            iteration["carriedIdentifiers"][j] = {}
            iteration["carriedIdentifiers"][j][1] = first
            iteration["carriedIdentifiers"][j][2] = eop.StringToValuePairList(second, ")")

            print(iteration["carriedIdentifiers"][j][2][1]["name"])
            print(iteration["carriedIdentifiers"][j][2][1]["value"])

            j = j + 1
        end

        iteration["disabledZoneCollapseIdentifiers"] = eop.StringToValuePairList(iterationsSheet[i + 2][10], ")")

        eop_config["district"]["iterations"][iterationIndex] = iteration
    end
end

-- Import Zones Sheet
zonesSheet = eop.ImportSheetTable(eop.spreadsheetFilePath, "zones")

for i = 1, #zonesSheet - 4 do
    if zonesSheet[i + 4][1] ~= "" then
        local zoneIndex = tonumber(zonesSheet[i + 4][1])

        local zone = eop.CheckZoneNil(eop_config["district"]["zones"][zoneIndex + 1])

        zone["id"] = zoneIndex
        zone["name"] = zonesSheet[i + 4][2]

        zone["collapsedIdentifiers"] = eop.StringToValuePairList(zonesSheet[i + 4][3], ")")

        local j = 4;
        while j <= #zonesSheet[1] do
            zone["positiveZoneIdentifierConditions"][#zone["positiveZoneIdentifierConditions"] + 1] = eop.StringToStringList(zonesSheet[i + 4][j])
            zone["negativeZoneIdentifierConditions"][#zone["negativeZoneIdentifierConditions"] + 1] = eop.StringToStringList(zonesSheet[i + 4][j + 1])

	        j = j + 2
        end

        eop_config["district"]["zones"][zoneIndex + 1] = zone
    end
end

eop.EvaluateEOP_Config(eop_config)
