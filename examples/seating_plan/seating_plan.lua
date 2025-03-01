eop_config = eop.GetDefaultConfig()

sheets = {
    room    = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "room"),
    people    = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "people"),
    zones = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "zones"),
}

-- Importing

zoneCells = {}
function GetOccupiableCells(row, index, startIndex)
    local occupiableCellsRow = {}

    for i = 1, #row do
        if row[i] ~= "" then
            occupiableCellsRow[i] = true
            
            local zoneIndex = tonumber(row[i])
            zoneCells[zoneIndex] = eop.CheckTableNil(zoneCells[zoneIndex])

            zoneCells[zoneIndex][#zoneCells[zoneIndex] + 1] = {}
            zoneCells[zoneIndex][#zoneCells[zoneIndex]]["x"] = i - 1
            zoneCells[zoneIndex][#zoneCells[zoneIndex]]["y"] = index - startIndex
        else
            occupiableCellsRow[i] = false
        end
    end

    return occupiableCellsRow
end

function GetIdentifiers(row)
    local identifiers = {}

    identifiers[1] = eop.CheckIdentifierNil()
    identifiers[1]["name"] = "name"
    identifiers[1]["relitiveCellConditions"] = eop.StringToVec2List(row[2])

    for i = 3, #row do
        identifiers[i - 1] = eop.CheckIdentifierNil()
        identifiers[i - 1]["name"] = row[i]
    end
    return identifiers
end

function GetPerson(row, index, startIndex)
    local identifierValuesTable = {}

    identifierValuesTable[1] = {}
    identifierValuesTable[1]["value"] = row[1]
    identifierValuesTable[1]["conditions"] = eop.StringToStringList(row[2])

    local j = 3
    while j <= #row do
        local identifier = {}

        identifier["value"] = row[j]
        identifier["conditions"] = {}

        identifierValuesTable[#identifierValuesTable + 1] = identifier
	    j = j + 1
    end

    return {
        count = 1,

        entityCellConditions = {},
        entityZoneConditions = {},

        identifiersValues = identifierValuesTable
    }
end

function GetZone(row, index, startIndex)
    negativeZoneIdentifierConditionsTable = {}

    local j = 3;
    while j <= #row do
        negativeZoneIdentifierConditionsTable[j - 2] = eop.StringToStringList(row[j])
        j = j + 1
    end

    return {
        id = tonumber(row[1]),
        name = row[2],

        cells = zoneCells[tonumber(row[1])],
        collapsedIdentifiers = {},

        positiveZoneIdentifierConditions = {},
        negativeZoneIdentifierConditions = negativeZoneIdentifierConditionsTable
    }
end

function ExportIteration(iteration)
    if iteration["hide"] == true then
        return
    end
    local identifierIndexes = eop.GetIdentifierIndexes(eop_config["entities"]["identifiers"], eop.identifiers)

    local iterationSheet = {}
    
    iterationSheet[1] = eop.CheckTableNil(iterationSheet[1])
    iterationSheet[1][1] = "Organised Seating Plan"
    
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
    eop.ExportSheetTable(eop.exportSpreadsheetFilePath, iteration["name"], iterationSheet)
end

-- Running

eop_config = {
    district = {
        rows = #sheets["room"] - 1,
        cols = #sheets["room"][1],
        
        occupiableCells = eop.ForEachItemRun(sheets["room"], GetOccupiableCells, 2),
        
        zones = eop.ForEachItemRun(sheets["zones"], GetZone, 4),
        iterations = {
            {
                name = "seating_plan",
                hide = false,
                disableDropIterationCount = false,
                
                disabledCells = {},
                disabledZones = {},
                disabledIdentifiers = {},
                
                carriedCells = {},
                carriedZones = {},
                carriedIdentifiers = {},
                
                disabledZoneCollapseIdentifiers = {}
            }
        }
    },

    entities = {
        identifiers = GetIdentifiers(sheets["people"][3]),
        entities = eop.ForEachItemRun(sheets["people"], GetPerson, 4)
    }
}

eop_config = eop.EvaluateEOP_Config(eop_config)

ExportIteration(eop_config["district"]["iterations"][1])
