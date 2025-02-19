eop_config = eop.GetDefaultConfig()

-- Set Iteration
eop_config["district"]["iterations"][1] = eop.CheckIterationNil()
eop_config["district"]["iterations"][1]["name"] = "seating_plan"

-- Import Room Sheets
roomSheet = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "room")

eop_config["district"]["rows"] = #roomSheet - 1
eop_config["district"]["cols"] = #roomSheet[1]

for i = 1, #roomSheet - 1 do
    eop_config["district"]["occupiableCells"][i] = {}

    for j = 1, #roomSheet[2] do
        if roomSheet[i + 1][j] ~= "" then
            eop_config["district"]["occupiableCells"][i][j] = true
            
            local zoneIndex = tonumber(roomSheet[i + 1][j])
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

-- Import People Sheet
peopleSheet = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "people")

-- Import Identifiers
eop_config["entities"]["identifiers"][1] = eop.CheckIdentifierNil()
eop_config["entities"]["identifiers"][1]["name"] = "name"
eop_config["entities"]["identifiers"][1]["relitiveCellConditions"] = eop.StringToVec2List(peopleSheet[3][2])

for i = 3, #peopleSheet[1] do
    eop_config["entities"]["identifiers"][i - 1] = eop.CheckIdentifierNil()
    eop_config["entities"]["identifiers"][i - 1]["name"] = peopleSheet[3][i]
end

-- Import People
for i = 1, #peopleSheet - 3 do
    if peopleSheet[i + 3][1] ~= "" then
        local entityIndex = #eop_config["entities"]["entities"] + 1

        local entity = eop.CheckEntityNil(eop_config["entities"]["entities"][entityIndex])

        entity["count"] = 1
        entity["entityCellConditions"] = {}
        entity["entityZoneConditions"] = {}

        entity["identifiersValues"][1] = {}
        entity["identifiersValues"][1]["value"] = peopleSheet[i + 3][1]
        entity["identifiersValues"][1]["conditions"] = eop.StringToStringList(peopleSheet[i + 3][2])

        local j = 3
        while j <= #peopleSheet[1] do
            local identifier = {}

            identifier["value"] = peopleSheet[i + 3][j]
            identifier["conditions"] = {}

            entity["identifiersValues"][#entity["identifiersValues"] + 1] = identifier
	        j = j + 1
        end

        eop_config["entities"]["entities"][entityIndex] = entity
    end
end

-- Import Zones Sheet
zonesSheet = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "zones")

for i = 1, #zonesSheet - 3 do
    if zonesSheet[i + 3][1] ~= "" then
        local zoneIndex = tonumber(zonesSheet[i + 3][1])

        local zone = eop.CheckZoneNil(eop_config["district"]["zones"][zoneIndex])

        zone["id"] = zoneIndex
        zone["name"] = zonesSheet[i + 3][2]
        zone["collapsedIdentifiers"] = {}
        zone["negativeZoneIdentifierConditions"][1] = {}

        local j = 4;
        while j <= #zonesSheet[1] do
            zone["negativeZoneIdentifierConditions"][#zone["negativeZoneIdentifierConditions"] + 1] = eop.StringToStringList(zonesSheet[i + 3][j])
	        j = j + 1
        end

        eop_config["district"]["zones"][zoneIndex] = zone
    end
end

eop_config = eop.EvaluateEOP_Config(eop_config)

-- Export Iterations
identifierIndexes = eop.GetIdentifierIndexes(eop_config, eop.identifiers)

iterationSheet = {}
iterationSheet[1] = eop.CheckTableNil(iterationSheet[1])
iterationSheet[1][1] = "Organised Seating Plan"

iteration = eop_config["district"]["iterations"][1]

for j = 1, eop_config["district"]["rows"] do
    for k = 1, eop_config["district"]["cols"] do
        local data = ""
        local entityId = iteration["cells"][(((j - 1) * eop_config["district"]["cols"]) + k - 1) + 1]

        if entityId == -1 then
            data = ""
        elseif #identifierIndexes == 0 then
            data = tostring(entityId)
        else
            for l = 1, #identifierIndexes - 1 do
                data = data .. eop_config["entities"]["entities"][entityId + 1]["identifiersValues"][identifierIndexes[l]]["value"]
                data = data .. ", "
            end
            data = data .. eop_config["entities"]["entities"][entityId + 1]["identifiersValues"][identifierIndexes[#identifierIndexes]]["value"]
        end

        iterationSheet[j + 1] = eop.CheckTableNil(iterationSheet[j + 1])
        iterationSheet[j + 1][k] = data
    end
end

eop.ExportSheetTable(eop.exportSpreadsheetFilePath, "seating_plan", iterationSheet)
