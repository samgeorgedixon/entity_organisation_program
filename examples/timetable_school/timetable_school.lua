eop_config = eop.GetDefaultConfig()

occupiableCells = {}
occupiableCells[1] = {}
cellIndex = 0

function GetRoom(row, index, startIndex)
    zone["name"] = 

    local classesCell = eop.Trim(row[3])
    local collapsedIdentifiersTable = {}
    local positiveZoneIdentifierConditionsTable = {}

    for token in classesCell:gmatch("[^,]+") do
        collapsedIdentifiersTable[#collapsedIdentifiersTable + 1] = {}
        collapsedIdentifiersTable[#collapsedIdentifiersTable]["name"] = eop.Trim(token)
        collapsedIdentifiersTable[#collapsedIdentifiersTable]["value"] = "T"

        positiveZoneIdentifierConditionsTable[#positiveZoneIdentifierConditionsTable + 1] = 
    end
    if #collapsedIdentifiersTable == 0 and #classesCell > 0 then
        collapsedIdentifiersTable[#collapsedIdentifiersTable + 1] = {}
        collapsedIdentifiersTable[#collapsedIdentifiersTable]["name"] = eop.Trim(classesCell)
        collapsedIdentifiersTable[#collapsedIdentifiersTable]["value"] = "T"
    end

    local seatCount = tonumber(row[2])
    local cellsTable = {}

    for j = 1, seatCount do
        occupiableCells[1][cellIndex + j] = true

        cellsTable[j] = {}
        cellsTable[j]["x"] = cellIndex + j - 1
        cellsTable[j]["y"] = 1
    end
    cellIndex = cellIndex + seatCount

    return {
        id = index - startIndex + 1
        name = row[1],

        cells = cellsTable,
        collapsedIdentifiers = collapsedIdentifiersTable,

        positiveZoneIdentifierConditions = positiveZoneIdentifierConditionsTable,
        negativeZoneIdentifierConditions = {}
    }
end

countPerPersonPerWeek = {}

function GetClass(row, index, startIndex)
    countPerPersonPerWeek[index - startIndex + 1] = tonumber(row[2])

    return {
        name = row[1],
        iterationCount = tonumber(row[3]),

        relitiveCellConditions = {},
        relitiveZoneConditions = {},
    }
end

identfierConditions = {}

function GetIdentifier(row, index, startIndex)
    identfierConditions[index - startIndex + 1] = eop.StringToValuePairList(row[2], ")")

    return {
        name = row[1],
        iterationCount = 0,

        relitiveCellConditions = {},
        relitiveZoneConditions = {},
    }
end

function GetPeriod(row, index, startIndex)
    local disabledClassesCell = row[2]
    local disabledZoneCollapseIdentifiersTable = {}

    for token in disabledClassesCell:gmatch("[^,]+") do
        disabledZoneCollapseIdentifiersTable[#disabledZoneCollapseIdentifiersTable + 1] = {}
        disabledZoneCollapseIdentifiersTable[#disabledZoneCollapseIdentifiersTable]["name"] = eop.Trim(token)
        disabledZoneCollapseIdentifiersTable[#disabledZoneCollapseIdentifiersTable]["value"] = "T"
    end
    if #disabledZoneCollapseIdentifiersTable == 0 and #disabledClassesCell > 0 then
        disabledZoneCollapseIdentifiersTable[#disabledZoneCollapseIdentifiersTable + 1] = {}
        disabledZoneCollapseIdentifiersTable[#disabledZoneCollapseIdentifiersTable]["name"] = eop.Trim(disabledClassesCell)
        disabledZoneCollapseIdentifiersTable[#disabledZoneCollapseIdentifiersTable]["value"] = "T"
    end

    return {
        name = row[1],
        hide = false,
        disableDropIterationCount = false,

        disabledCells = {},
        disabledZones = {},
        disabledIdentifiers = {},

        carriedCells = {},
        carriedZones = {},
        carriedIdentifiers = {},

        disabledZoneCollapseIdentifiers = disabledZoneCollapseIdentifiersTable
    }
end

-- Running



eop_config = {
    district = {
        rows = 1,
        cols = 0,

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
