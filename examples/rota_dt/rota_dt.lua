eop_config = eop.GetDefaultConfig()

sheets = {
    rooms   = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "rooms"),
    classes = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "classes"),
    lessons = eop.ImportSheetTable(eop.importSpreadsheetFilePath, "lessons")
}

-- Importing

lessonsTermFullType = {}

function GetLesson(row, index, startIndex)
    local iterationCountValue = 1

    if eop.Low(eop.Trim(row[2])) == "f" then
        lessonsTermFullType[#lessonsTermFullType + 1] = true
        iterationCountValue = 2
    elseif eop.Low(eop.Trim(row[2])) == "h" then
        lessonsTermFullType[#lessonsTermFullType + 1] = false
    end

    return {
        name = row[1],
        iterationCount = iterationCountValue,
    
        relitiveCellConditions = {},
        relitiveZoneConditions = {}
    }
end

occupiableCellsTable = {}
occupiableCellsTable[1] = {}
cellIndex = 0

zonesTable = {}

identfiersTable = {}

function GetRoom(row, index, startIndex)
    local lessonsCell = eop.Trim(row[2])

    local collapsedIdentifiersTableHalf = {}
    local positiveZoneIdentifierConditionsTableHalf = {}
    local negativeZoneIdentifierConditionsTableHalf = {}

    local collapsedIdentifiersTableFull = {}
    local positiveZoneIdentifierConditionsTableFull = {}
    local negativeZoneIdentifierConditionsTableFull = {}

    for i = 1, #identfiersTable do
        positiveZoneIdentifierConditionsTableHalf[i] = {}
        positiveZoneIdentifierConditionsTableFull[i] = {}

        negativeZoneIdentifierConditionsTableHalf[i] = {}
        negativeZoneIdentifierConditionsTableFull[i] = {}
    end

    for token in lessonsCell:gmatch("[^,]+") do
        local identifierIndex = eop.GetIdentifierIndexes(identfiersTable, eop.Trim(token))[1]

        if lessonsTermFullType[identifierIndex - 3] == true then
            collapsedIdentifiersTableFull[#collapsedIdentifiersTableFull + 1] = {}
            collapsedIdentifiersTableFull[#collapsedIdentifiersTableFull]["name"] = eop.Trim(token)
            collapsedIdentifiersTableFull[#collapsedIdentifiersTableFull]["value"] = "T"
            
            positiveZoneIdentifierConditionsTableFull[identifierIndex] = {}
            positiveZoneIdentifierConditionsTableFull[identifierIndex][1] = "T"

            negativeZoneIdentifierConditionsTableHalf[identifierIndex] = {}
            negativeZoneIdentifierConditionsTableHalf[identifierIndex][1] = "T"
        else
            collapsedIdentifiersTableHalf[#collapsedIdentifiersTableHalf + 1] = {}
            collapsedIdentifiersTableHalf[#collapsedIdentifiersTableHalf]["name"] = eop.Trim(token)
            collapsedIdentifiersTableHalf[#collapsedIdentifiersTableHalf]["value"] = "T"
            
            positiveZoneIdentifierConditionsTableHalf[identifierIndex] = {}
            positiveZoneIdentifierConditionsTableHalf[identifierIndex][1] = "T"

            negativeZoneIdentifierConditionsTableFull[identifierIndex] = {}
            negativeZoneIdentifierConditionsTableFull[identifierIndex][1] = "T"
        end
    end
    if #collapsedIdentifiersTableHalf == 0 and #collapsedIdentifiersTableFull == 0 and #lessonsCell > 0 then
        local identifierIndex = eop.GetIdentifierIndexes(identfiersTable, eop.Trim(lessonsCell))[1]

        if lessonsTermFullType[identifierIndex - 3] == true then
            collapsedIdentifiersTableFull[#collapsedIdentifiersTableFull + 1] = {}
            collapsedIdentifiersTableFull[#collapsedIdentifiersTableFull]["name"] = eop.Trim(lessonsCell)
            collapsedIdentifiersTableFull[#collapsedIdentifiersTableFull]["value"] = "T"
            
            positiveZoneIdentifierConditionsTableFull[identifierIndex] = {}
            positiveZoneIdentifierConditionsTableFull[identifierIndex][1] = "T"

            negativeZoneIdentifierConditionsTableHalf[identifierIndex] = {}
            negativeZoneIdentifierConditionsTableHalf[identifierIndex][1] = "T"
        else
            collapsedIdentifiersTableHalf[#collapsedIdentifiersTableHalf + 1] = {}
            collapsedIdentifiersTableHalf[#collapsedIdentifiersTableHalf]["name"] = eop.Trim(lessonsCell)
            collapsedIdentifiersTableHalf[#collapsedIdentifiersTableHalf]["value"] = "T"
            
            positiveZoneIdentifierConditionsTableHalf[identifierIndex] = {}
            positiveZoneIdentifierConditionsTableHalf[identifierIndex][1] = "T"

            negativeZoneIdentifierConditionsTableFull[identifierIndex] = {}
            negativeZoneIdentifierConditionsTableFull[identifierIndex][1] = "T"
        end
    end
    
    occupiableCellsTable[1][cellIndex + 1] = true
    occupiableCellsTable[1][cellIndex + 2] = true
    occupiableCellsTable[1][cellIndex + 3] = false

    cellIndex = cellIndex + 3

    zonesTable[#zonesTable + 1] = {
        id = #zonesTable,
        name = row[1] .. "-h",

        cells = { { x = cellIndex - 3, y = 0 } },
        collapsedIdentifiers = collapsedIdentifiersTableHalf,

        positiveZoneIdentifierConditions = positiveZoneIdentifierConditionsTableHalf,
        negativeZoneIdentifierConditions = negativeZoneIdentifierConditionsTableHalf
    }
    zonesTable[#zonesTable + 1] = {
        id = #zonesTable,
        name = row[1] .. "-f",

        cells = { { x = cellIndex - 2, y = 0 } },
        collapsedIdentifiers = collapsedIdentifiersTableFull,

        positiveZoneIdentifierConditions = positiveZoneIdentifierConditionsTableFull,
        negativeZoneIdentifierConditions = negativeZoneIdentifierConditionsTableFull
    }
end

function GetClass(row, index, startIndex)
    local identifiers = {}

    identifiers[#identifiers + 1] = {} -- Name
    identifiers[#identifiers]["value"] = row[1]
    identifiers[#identifiers]["conditions"] = {}

    identifiers[#identifiers + 1] = {} -- Time
    identifiers[#identifiers]["value"] = row[2]
    identifiers[#identifiers]["conditions"] = {}

    identifiers[#identifiers + 1] = {} -- All
    identifiers[#identifiers]["value"] = "T"
    identifiers[#identifiers]["conditions"] = { "T" }

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

times = {}

function GetIterations(zones, entities, termCount)
    local iterations = {}

    for i = 1, #entities do
        times[entities[i]["identifiersValues"][2]["value"]] = entities[i]["identifiersValues"][2]["value"]
    end

    local fullZones = {}

    for i = 1, #zones do
        if i % 2 == 0 then
            fullZones[#fullZones + 1] = i
        end
    end

    local index = 1
    local halfTerm = 1

    for i = 1, termCount do
        for key, time in pairs(times) do
            iterations[index] = eop.CheckIterationNil(iterations[index])
            iterations[index]["name"] = "half-term-" .. halfTerm .. "_" .. time

            iterations[index]["disabledIdentifiers"] = {}

            local j = 1
            for keyIdentifier, timeIdentifier in pairs(times) do
                if key ~= keyIdentifier then
                    iterations[index]["disabledIdentifiers"][j] = {}
                    iterations[index]["disabledIdentifiers"][j]["name"] = "time"
                    iterations[index]["disabledIdentifiers"][j]["value"] = timeIdentifier

                    j = j + 1
                end
            end

            index = index + 1
        end

        halfTerm = halfTerm + 1

        for key, time in pairs(times) do
            iterations[index] = eop.CheckIterationNil(iterations[index])
            iterations[index]["name"] = "half-term-" .. halfTerm .. "_" .. time

            iterations[index]["disabledIdentifiers"] = {}

            local j = 1
            for keyIdentifier, timeIdentifier in pairs(times) do
                if key ~= keyIdentifier then
                    iterations[index]["disabledIdentifiers"][j] = {}
                    iterations[index]["disabledIdentifiers"][j]["name"] = "time"
                    iterations[index]["disabledIdentifiers"][j]["value"] = timeIdentifier

                    j = j + 1
                end
            end

            iterations[index]["carriedZones"] = { { "half-term-" .. halfTerm - 1 .. "_" .. time, fullZones } }

            index = index + 1
        end

        halfTerm = halfTerm + 1
    end

    return iterations
end

-- Exporting

function ExportRota(termCount)
    rotaSheet = {}

    rotaSheet[1] = eop.CheckTableNil(rotaSheet[1])
    rotaSheet[2] = eop.CheckTableNil(rotaSheet[2])
    rotaSheet[3] = eop.CheckTableNil(rotaSheet[3])
    
    rotaSheet[1][1] = "Organised Zones"
    rotaSheet[2][1] = "Class"
    rotaSheet[2][2] = "Half Terms"
    
    rotaSheet[3][1] = ""

    local halfTerm = 1
    local timesCount = 0

    for key, time in pairs(times) do
        timesCount = timesCount + 1
    end

    for i = 1, #eop_config["entities"]["entities"] do
        rotaSheet[3 + i] = eop.CheckTableNil(rotaSheet[3 + i])
        rotaSheet[3 + i][1] = eop_config["entities"]["entities"][i]["identifiersValues"][1]["value"]

        for j = 1, termCount * 2 do
            rotaSheet[3 + i][1 + j] = "NA"
        end
    end

    for i = 1, #eop_config["district"]["iterations"] do
        local roomId = 0

        for j = 1, #eop_config["district"]["iterations"][i]["cells"] do
            if j % 3 == 0 then
                roomId = roomId - 1
            end
            roomId = roomId + 1

            local classId = eop_config["district"]["iterations"][i]["cells"][j] + 1

            local room = string.sub(eop_config["district"]["zones"][roomId]["name"], 1, -3)

            local lesson = string.sub(eop_config["district"]["iterations"][i]["zoneCollapsedIdentifiers"][roomId], 1, -4)

            if classId ~= 0 then
                rotaSheet[3 + classId] = eop.CheckTableNil(rotaSheet[3 + classId])

                rotaSheet[3 + classId][halfTerm + 1] = room .. ", " .. lesson
            end
        end

        if i % timesCount == 0 then
            halfTerm = halfTerm + 1
        end
    end
    
    eop.ExportSheetTable(eop.exportSpreadsheetFilePath, "rota", rotaSheet)
end

-- Running

defaultIdentifiers = {
    {
        name = "name",
        iterationCount = 0,

        relitiveCellConditions = {},
        relitiveZoneConditions = {}
    },
    {
        name = "time",
        iterationCount = 0,

        relitiveCellConditions = {},
        relitiveZoneConditions = {}
    },
    {
        name = "all",
        iterationCount = 0,

        relitiveCellConditions = { { x = 1, y = 0 }, { x = -1, y = 0 } },
        relitiveZoneConditions = {}
    }
}

lessons = eop.ForEachItemRun(sheets["lessons"], GetLesson, 3)

identfiersTable = eop.ConcatenateTable(defaultIdentifiers, lessons)

eop.ForEachItemRun(sheets["rooms"], GetRoom, 3)

entitiesTable = eop.ForEachItemRun(sheets["classes"], GetClass, 4)

eop_config = {
    district = {
        rows = 1,
        cols = cellIndex,

        occupiableCells = occupiableCellsTable,

        zones = zonesTable,
        iterations = GetIterations(zonesTable, entitiesTable, 3)
    },

    entities = {
        identifiers = identfiersTable,
        entities = entitiesTable
    }
}

eop_config = eop.EvaluateEOP_Config(eop_config)

ExportRota(3)
