eop_config = eop.GetDefaultConfig()

sheets = {
    district    = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "district"),
    entities    = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "entities"),
    identifiers = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "identifiers"),
    iterations  = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "iterations"),
    zones       = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "zones")
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

function GetIdentifier(row, index, startIndex)
    return {
        name = row[1],
        iterationCount = tonumber(row[2]),
    
        relitiveCellConditions = eop.StringToVec2List(row[3]),
        relitiveZoneConditions = eop.StringToIntList(row[4])
    }
end

function GetZone(row, index, startIndex)
    local j = 4;
    local positiveZoneIdentifierConditionsTable = {}
    local negativeZoneIdentifierConditionsTable = {}
    
    while j <= #row do
        positiveZoneIdentifierConditionsTable[#positiveZoneIdentifierConditionsTable + 1] = eop.StringToStringList(row[j])
        negativeZoneIdentifierConditionsTable[#negativeZoneIdentifierConditionsTable + 1] = eop.StringToStringList(row[j + 1])
	    j = j + 2
    end

    return {
        id = tonumber(row[1]),
        name = row[2],

        cells = zoneCells[tonumber(row[1])],
        collapsedIdentifiers = eop.StringToValuePairList(row[3], ")"),

        positiveZoneIdentifierConditions = positiveZoneIdentifierConditionsTable,
        negativeZoneIdentifierConditions = negativeZoneIdentifierConditionsTable
    }
end

function GetEntity(row, index, startIndex)
    local identifiers = {}

    local j = 4;
    while j <= #row do
        identifiers[#identifiers + 1] = {}
        identifiers[#identifiers]["value"] = row[j]
        identifiers[#identifiers]["conditions"] = eop.StringToStringList(row[j + 1])
        j = j + 2
    end

    return {
        count = tonumber(row[1]),

        entityCellConditions = eop.StringToVec2List(row[2]),
        entityZoneConditions = eop.StringToIntList(row[3]),

        identifiersValues = identifiers
    }
end

function GetIteration(row, index, startIndex)
    -- Get Carried Cells
    local carriedCellsStringList = eop.StringSquareBracketToStringList(row[7])
    local carriedCellsTable = {}

    local j = 1;
    while j <= #carriedCellsStringList do
        local first, second = carriedCellsStringList[j]:match("([^,]*),?(.*)")

        carriedCellsTable[j] = {}
        carriedCellsTable[j][1] = first
        carriedCellsTable[j][2] = eop.StringToVec2List(second)

        j = j + 1
    end

    -- Get Carried Zones
    local carriedZonesStringList = eop.StringSquareBracketToStringList(row[8])
    local carriedZonesTable = {}

    j = 1;
    while j <= #carriedZonesStringList do
        local first, second = carriedZonesStringList[j]:match("([^,]*),?(.*)")

        carriedZonesTable[j] = {}
        carriedZonesTable[j][1] = first
        carriedZonesTable[j][2] = eop.StringToIntList(second)

        j = j + 1
    end

    -- Get Carried Identifiers
    local carriedIdentifiersStringList = eop.StringSquareBracketToStringList(row[9])
    local carriedIdentifiersTable = {}

    j = 1;
    while j <= #carriedIdentifiersStringList do
        local first, second = carriedIdentifiersStringList[j]:match("([^,]*),?(.*)")

        carriedIdentifiersTable[j] = {}
        carriedIdentifiersTable[j][1] = first
        carriedIdentifiersTable[j][2] = eop.StringToValuePairList(second, ")")

        j = j + 1
    end

    return {
        name = row[1],
        hide = eop.CheckEqualString(string.lower(row[2]), "t"),
        disableDropIterationCount = eop.CheckEqualString(string.lower(row[3]), "t"),

        disabledCells = eop.StringToVec2List(row[4]),
        disabledZones = eop.StringToIntList(row[5]),
        disabledIdentifiers = eop.StringToValuePairList(row[6], ")"),

        carriedCells = carriedCellsTable,
        carriedZones = carriedZonesTable,
        carriedIdentifiers = carriedIdentifiersTable,

        disabledZoneCollapseIdentifiers = eop.StringToValuePairList(row[10], ")")
    }
end

-- Exporting

function ExportZones()
    zonesSheet = {}

    zonesSheet[1] = eop.CheckTableNil(zonesSheet[1])
    zonesSheet[2] = eop.CheckTableNil(zonesSheet[2])
    zonesSheet[3] = eop.CheckTableNil(zonesSheet[3])
    
    zonesSheet[1][1] = "Organised Zones"
    zonesSheet[2][1] = "ID"
    zonesSheet[2][2] = "Name"
    zonesSheet[2][3] = "Collapsed Identifiers"
    
    zonesSheet[3][1] = ""
    zonesSheet[3][2] = ""
    
    local offset = 0
    for i = 1, #eop_config["district"]["iterations"] do
        local iteration = eop_config["district"]["iterations"][i]
    
        if iteration["hide"] == false then
            zonesSheet[3][i + 2 - offset] = iteration["name"]
        else
            offset = offset + 1
        end
    end
    
    for i = 1, #eop_config["district"]["zones"] do
        local zone = eop_config["district"]["zones"][i]
    
        zonesSheet[i + 3] = eop.CheckTableNil(zonesSheet[i + 3])
    
        zonesSheet[i + 3][1] = tostring(i)
        zonesSheet[i + 3][2] = zone["name"]
    
        local offset = 0
        for j = 1, #eop_config["district"]["iterations"] do
            local iteration = eop_config["district"]["iterations"][j]
    
            if iteration["hide"] == false then
                zonesSheet[i + 3][j + 2 - offset] = iteration["zoneCollapsedIdentifiers"][i]
            else
                offset = offset + 1
            end
        end
    end
    
    eop.ExportSheetTable(eop.exportSpreadsheetFilePath, "zones", zonesSheet)
end

function ExportIteration(iteration, index, startIndex)
    if iteration["hide"] == true then
        return
    end
    local identifierIndexes = eop.GetIdentifierIndexes(eop_config["entities"]["identifiers"], eop.identifiers)

    local iterationSheet = {}
    
    iterationSheet[1] = eop.CheckTableNil(iterationSheet[1])
    iterationSheet[1][1] = "Organisation of Entities: " .. iteration["name"]
    
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
        rows = #sheets["district"] - 1,
        cols = #sheets["district"][1],

        occupiableCells = eop.ForEachItemRun(sheets["district"], GetOccupiableCells, 2),

        zones = eop.ForEachItemRun(sheets["zones"], GetZone, 5),
        iterations = eop.ForEachItemRun(sheets["iterations"], GetIteration, 3)
    },

    entities = {
        identifiers = eop.ForEachItemRun(sheets["identifiers"], GetIdentifier, 3),
        entities = eop.ForEachItemRun(sheets["entities"], GetEntity, 5)
    }
}

eop_config = eop.EvaluateEOP_Config(eop_config)

ExportZones()
eop.ForEachItemRun(eop_config["district"]["iterations"], ExportIteration, 1)
