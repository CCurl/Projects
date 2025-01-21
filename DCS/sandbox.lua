-- UT: a simple set of utilities

UT = { neutral = 0, red = 1, blue = 2 }

-- Compare side to coalition sides
UT.isRed  = function(side) return (side == UT.red)  end
UT.isBlue = function(side) return (side == UT.blue) end

-- Output a message to the screen
UT.msg = function(str) trigger.action.outText(str, 30) end

-- Between function (inclusive)
UT.btw = function(n, lo, hi)
    if (n < lo) then return false end
    if (n > hi) then return false end
    return true
end

-- Clone a table (object)
-- Returns the cloned table (object)
UT.clone = function (orig)
    local obj = {}
	for key, val in pairs(orig) do
         -- A "class" table has "__index" equal to itself
		if (val == orig) then obj[key] = obj
		else obj[key] = type(val) == 'table' and ut.clone(val) or val
		end
	end
    -- An instance of a "class" has a metatable
	setmetatable(obj, getmetatable(orig))
    return obj
end

-- Get the Groups for a coalition/side
-- if side is nil, it defaults to BLUE
-- Returns: a table of Groups
UT.getGroups = function(side)
    if (side == nil) then side = UT.blue end
    if (UT.btw(side, 0, 2)) then return coalition.getGroups(side) end
    return {}
end

-- Dump the units in a single Group
UT.dumpUnits = function(group)
    UT.msg("  Units: ")
    for i, un in pairs(Group.getUnits(group)) do
        UT.msg("  - " . i .. ": " .. Unit.getName(un))
    end
end

-- Dump a single Group
UT.dumpGroups = function(color, groups)
    UT.msg(color .. " groups:")
    for i, gp in pairs(groups) do
        UT.msg("- " .. i .. ": " .. Group.getName(gp))
        UT.dumpUnits(gp)
    end
end

-- Functions to dump Groups
UT.dumpRedGroups = function()  UT.dumpGroups("RED",  UT.getGroups(UT.red))  end
UT.dumpBlueGroups = function() UT.dumpGroups("BLUE", UT.getGroups(UT.blue)) end

-- Find a single Group on a given side with name <name>
-- Returns: a Group if found, else nil
UT.findGroup = function(side, name)
    local groups = UT.getGroups(side)
    for i, gp in pairs(groups) do
        if (Group.getName(gp) == name) then return gp end
    end
    return nil
end

-- Find all Groups on a given side starting with <prefix>
-- Returns: a table/array, possibly empty
UT.findGroupsWithPrefix = function(side, prefix)
    local groups = UT.getGroups(side)
    local ret = {}
    for i, gp in pairs(groups) do
        s,e = string.find(Group.getName(gp), prefix, 1)
        if (s == 1) then table.insert(ret, gp) end
    end
    return ret
end

UT.msg("- RED: " .. coalition.side.RED .. " -")
UT.msg("- BLUE: " .. coalition.side.BLUE .. " -")
UT.msg("--- Utilities loaded ---")
