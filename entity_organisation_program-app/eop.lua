local eop = {}

function eop.GetDefaultConfig()
    config = {}
    
    config["district"] = {};
    config["district"]["rows"] = 0;
    config["district"]["cols"] = 0;

    config["district"]["occupiableCells"] = {};
    config["district"]["zones"] = {};
    config["district"]["iterations"] = {};
    
    config["entities"] = {};
    config["entities"]["entities"] = {};
    config["entities"]["identifiers"] = {};
    
    return config
end

function eop.CheckTableNil(table)
    if table == nil then
        table = {}
    end
    return table
end

function eop.CheckZoneNil(zone)
    if zone == nil then
        zone = {}
        zone["name"] = ""

        zone["cells"] = {}
        zone["collapsedIdentifiers"] = {}
        zone["positiveZoneIdentifierConditions"] = {}
        zone["negativeZoneIdentifierConditions"] = {}
    end
    return zone
end
function eop.CheckEntityNil(entity)
    if entity == nil then
        entity = {}
        entity["count"] = 0

        entity["entityCellConditions"] = {}
        entity["entityZoneConditions"] = {}
        entity["identifiersValues"] = {}
    end
    return entity
end
function eop.CheckIdentifierNil(identifier)
    if identifier == nil then
        identifier = {}
        identifier["name"] = ""
        identifier["iterationCount"] = 0
        
        identifier["relitiveCellConditions"] = {}
        identifier["relitiveZoneConditions"] = {}
    end
    return identifier
end
function eop.CheckIterationNil(iteration)
    if iteration == nil then
        iteration = {}
        iteration["name"] = ""
        iteration["hide"] = false
        iteration["disableDropIterationCount"] = false

        iteration["disabledCells"] = {}
        iteration["disabledZones"] = {}
        iteration["disabledIdentifiers"] = {}
        iteration["disabledZoneCollapseIdentifiers"] = {}

        iteration["carriedCells"] = {}
        iteration["carriedZones"] = {}
        iteration["carriedIdentifiers"] = {}

        iteration["disabledZoneCollapseIdentifiers"] = {}
    end
    return iteration
end

function eop.CheckEqualString(value, equal)
    if value == equal then
        return true
    end
    return false
end

function eop.Trim(str)
    return string.gsub(str, '^%s*(.-)%s*$', '%1')
end

function eop.StringToIntList(value)
    local intList = {}
    if value == "" then
        return intList
    end

    value = eop.Trim(value)

    for token in value:gmatch("[^,]+") do
        table.insert(intList, tonumber(token))
    end
    if #intList == 0 and #value > 0 then
        table.insert(intList, tonumber(value))
    end

    return intList
end
function eop.StringToStringList(value)
    local values = {}

    if value == "" then
        return values
    end

    value = eop.Trim(value)

    for token in value:gmatch("[^,]+") do
        values[#values + 1] = eop.Trim(token)
    end
    if #values == 0 and #value > 0 then
        values[#values + 1] = eop.Trim(value)
    end

    return values
end
function eop.StringSquareBracketToStringList(value)
    local values = {}

    if value == "" then
        return values
    end

    value = eop.Trim(value)

    for token in value:gmatch("[^%[%]]+") do
        values[#values + 1] = eop.Trim(token)
    end
    if #values == 0 and #value > 0 then
        values[#values + 1] = eop.Trim(value)
    end

    return values
end
function eop.StringToVec2List(value)
    local vec2List = {}

    if value == "" then
        return vec2List
    end

    value = eop.Trim(value)

    local i = 1
    for token in value:gmatch("%b()") do
        token = token:sub(2, -2)
        local x, y = token:match("([^,]+),([^,]+)")

        vec2List[i] = {}
        vec2List[i]["x"] = tonumber(x)
        vec2List[i]["y"] = tonumber(y)

        i = i + 1
    end

    return vec2List
end
function eop.StringToValuePairList(value, bound)
    local valuePairList = {}

    if value == "" then
        return valuePairList
    end

    value = eop.Trim(value)

    local i = 1
    for token in value:gmatch("[^" .. bound .. "]+") do
        local name, val = token:match("^.(.-),(.*)")
 
        valuePairList[i] = {}
        valuePairList[i]["name"] = eop.Trim(name)
        valuePairList[i]["value"] = eop.Trim(val)

        i = i + 1
    end

    return valuePairList
end

return eop
