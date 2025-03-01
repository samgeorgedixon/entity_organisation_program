eop_config = eop.GetDefaultConfig()

-- Importing

sheets = {
    rooms       = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "rooms"),
    people      = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "people"),
    classes     = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "classes"),
    identifiers = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "identifiers"),
    periods     = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "periods")
}

occupiableCellsTable = {}
occupiableCellsTable[1] = {}
cellIndex = 0

classesIdentifiers = {}

function GetRoom(row, index, startIndex)
    local classesCell = eop.Trim(row[3])
    local collapsedIdentifiersTable = {}
    local positiveZoneIdentifierConditionsTable = {}

    for i = 1, #classesIdentifiers do
        positiveZoneIdentifierConditionsTable[i] = {}
    end

    for token in classesCell:gmatch("[^,]+") do
        collapsedIdentifiersTable[#collapsedIdentifiersTable + 1] = {}
        collapsedIdentifiersTable[#collapsedIdentifiersTable]["name"] = eop.Trim(token)
        collapsedIdentifiersTable[#collapsedIdentifiersTable]["value"] = "T"

        local identifierIndex = eop.GetIdentifierIndexes(classesIdentifiers, eop.Trim(token))[1]
        
        positiveZoneIdentifierConditionsTable[identifierIndex] = {}
        positiveZoneIdentifierConditionsTable[identifierIndex][1] = "T"
    end
    if #collapsedIdentifiersTable == 0 and #classesCell > 0 then
        collapsedIdentifiersTable[#collapsedIdentifiersTable + 1] = {}
        collapsedIdentifiersTable[#collapsedIdentifiersTable]["name"] = eop.Trim(classesCell)
        collapsedIdentifiersTable[#collapsedIdentifiersTable]["value"] = "T"

        local identifierIndex = eop.GetIdentifierIndexes(classesIdentifiers, eop.Trim(classesCell))[1]
        
        positiveZoneIdentifierConditionsTable[identifierIndex] = {}
        positiveZoneIdentifierConditionsTable[identifierIndex][1] = "T"
    end

    local seatCount = tonumber(row[2])
    local cellsTable = {}

    for j = 1, seatCount do
        occupiableCellsTable[1][cellIndex + j] = true

        cellsTable[j] = {}
        cellsTable[j]["x"] = cellIndex + j - 1
        cellsTable[j]["y"] = 0
    end
    cellIndex = cellIndex + seatCount

    return {
        id = index - startIndex + 1,
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
    if index - startIndex + 1 > #identfierConditions then
        for i = #identfierConditions + 1, index - startIndex do
            identfierConditions[i] = {}
        end
    end

    identfierConditions[index - startIndex + 1] = eop.StringToValuePairList(row[2], ")")

    return {
        name = row[1],
        iterationCount = 0,

        relitiveCellConditions = {},
        relitiveZoneConditions = { 0 }
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

    local disabledZonesStrings = eop.StringToStringList(row[3])
    local disabledZonesTable = {}

    for i = 1, #disabledZonesStrings do
        local zoneId = -1

        for j = 1, #zonesTable do
            if zonesTable[j]["name"] == disabledZonesStrings[i] then
                zoneId = j - 1
            end
        end
        if zoneId ~= -1 then
            disabledZonesTable[#disabledZonesTable + 1] = zoneId
        end
    end

    return {
        name = row[1],
        hide = false,
        disableDropIterationCount = false,

        disabledCells = {},
        disabledZones = disabledZonesTable,
        disabledIdentifiers = {},

        carriedCells = {},
        carriedZones = {},
        carriedIdentifiers = {},

        disabledZoneCollapseIdentifiers = disabledZoneCollapseIdentifiersTable
    }
end

function GetPerson(row, index, startIndex)
    local identifiers = {}
 
    identifiers[1] = {}
    identifiers[1]["value"] = row[1]
    identifiers[1]["conditions"] = eop.StringToStringList(row[2])

    for i = 3, #row do
        identifiers[#identifiers + 1] = {}
        identifiers[#identifiers]["value"] = row[i]
        identifiers[#identifiers]["conditions"] = {}
    end

    return {
        count = 1,

        entityCellConditions = {},
        entityZoneConditions = {},

        identifiersValues = identifiers
    }
end

function SetIdentifierCondition(person, index, startIndex)
    for i = 1, #identfierConditions do
        for j = 1, #identfierConditions[i] do
            if identfierConditions[i][j]["name"] == person["identifiersValues"][#classesTable + 1 + i]["value"] then

                person["identifiersValues"][#classesTable + 1 + i]["conditions"][#person["identifiersValues"][#classesTable + 1 + i]["conditions"] + 1] = identfierConditions[i][j]["value"]
            elseif identfierConditions[i][j]["value"] == person["identifiersValues"][#classesTable + 1 + i]["value"] then
                
                person["identifiersValues"][#classesTable + 1 + i]["conditions"][#person["identifiersValues"][#classesTable + 1 + i]["conditions"] + 1] = identfierConditions[i][j]["name"]
            end
        end
    end

    return person
end

-- Exporting

function ExportTimetableDay(eop_config_res, day)
    local timetable = {}

    timetable[1] = eop.CheckTableNil(timetable[1])
    timetable[2] = eop.CheckTableNil(timetable[2])
    timetable[3] = eop.CheckTableNil(timetable[3])
    
    timetable[1][1] = "Timetable: Day - " .. day
    timetable[2][1] = "Name"
    timetable[2][2] = "Classes"
    
    timetable[3][1] = ""
    
    local offset = 0
    local across = 2

    for i = 1, #eop_config_res["district"]["iterations"] do
        local iteration = eop_config_res["district"]["iterations"][i]
        
        if iteration["hide"] == false then
            timetable[3][across - offset] = iteration["name"]
            timetable[3][across + 1 - offset] = ""
        else
            offset = offset + 1
        end
        across = across + 2
    end
    
    local identifierIndexes = eop.GetIdentifierIndexes(eop_config_res["entities"]["identifiers"], eop.identifiers)

    for i = 1, #eop_config_res["district"]["zones"] do
        local zone = eop_config_res["district"]["zones"][i]
    
        timetable[i + 3] = eop.CheckTableNil(timetable[i + 3])
    
        timetable[i + 3][1] = zone["name"]
    
        local offset = 0
        local across = 2

        for j = 1, #eop_config_res["district"]["iterations"] do
            local iteration = eop_config_res["district"]["iterations"][j]
    
            if iteration["hide"] == false then
                timetable[i + 3][across - offset] = iteration["zoneCollapsedIdentifiers"][i]

                local data = ""

                for l = 1, #zone["cells"] do
                    local cellIndex = ((zone["cells"][l]["y"] * eop_config_res["district"]["cols"]) + zone["cells"][l]["x"]) + 1
                    local entityId = tonumber(iteration["cells"][cellIndex])

                    if entityId == -1 then
                        data = data
                    elseif #identifierIndexes == 0 then
                        data = data .. ", " .. tostring(entityId)
                    else
                        for k = 1, #identifierIndexes - 1 do
                            data = data .. eop_config_res["entities"]["entities"][entityId + 1]["identifiersValues"][identifierIndexes[k]]["value"]
                            data = data .. "-"
                        end
                        
                        data = data .. eop_config_res["entities"]["entities"][entityId + 1]["identifiersValues"][identifierIndexes[#identifierIndexes]]["value"]
                        data = data .. ", "
                    end
                end

                timetable[i + 3][across + 1 - offset] = data
            else
                offset = offset + 1
            end
            across = across + 2
        end
    end
    
    eop.ExportSheetTable(eop.exportSpreadsheetFilePath, "timetable_day-" .. day, timetable)
end

-- Running

classesTable = eop.ForEachItemRun(sheets["classes"], GetClass, 3)
identifiersTable = eop.ForEachItemRun(sheets["identifiers"], GetIdentifier, 3)

classesIdentifiers = eop.ConcatenateTable(classesTable, identifiersTable)

nameIdentifier = { {
        name = "name",
        iterationCount = 0,
        relitiveCellConditions = {},
        relitiveZoneConditions = { 0 }
    }
}
classesIdentifiers = eop.ConcatenateTable(nameIdentifier, classesIdentifiers)

zonesTable = eop.ForEachItemRun(sheets["rooms"], GetRoom, 3)

peopleTable = eop.ForEachItemRun(sheets["people"], GetPerson, 4)
peopleTable = eop.ForEachItemRun(peopleTable, SetIdentifierCondition, 1)

eop_config = {
    district = {
        rows = 1,
        cols = cellIndex,

        occupiableCells = occupiableCellsTable,
        
        zones = zonesTable,
        iterations = eop.ForEachItemRun(sheets["periods"], GetPeriod, 3),
    },

    entities = {
        identifiers = classesIdentifiers,
        entities = peopleTable
    }
}

eop_config = eop.EvaluateEOP_Config(eop_config)

ExportTimetableDay(eop_config, 1)
