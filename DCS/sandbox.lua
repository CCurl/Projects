-- UT: a simple Utility class.
-- NOTE: the default side is BLUE

UT = { side = coalition.side.BLUE }

UT.isRed  = function(side) return (side == coalition.side.RED)  end
UT.isBlue = function(side) return (side == coalition.side.BLUE) end
UT.msg = function(str) trigger.action.outText(str, 30) end

UT.getGroupsForSide = function(side)
    if (not side) then side = UT.side end
    if (UT.isRed(side))  then return coalition.getGroups(1) end -- RED
    if (UT.isBlue(side)) then return coalition.getGroups(2) end -- BLUE
    return coalition.getGroups(0) -- NEUTRAL
end

UT.dumpUnits = function(group)
    UT.msg("  Units: ")
    for i, un in pairs(Group.getUnits(group)) do
        UT.msg("  - " .. Unit.getName(un))
    end
end

UT.dumpGroups = function(color, groups)
    UT.msg(color .. " groups:")
    for i, gp in pairs(groups) do
        UT.msg("- " .. Group.getName(gp))
        UT.dumpUnits(gp)
    end
end

UT.dumpRedGroups = function()  UT.dumpGroups("RED",  UT.getGroupsForSide(coalition.side.RED))  end
UT.dumpBlueGroups = function() UT.dumpGroups("BLUE", UT.getGroupsForSide(coalition.side.BLUE)) end

UT.findGroup = function(side, name)
    local groups = UT.getGroupsForSide(side)
    for i, gp in pairs(groups) do
        if (gp.getName(gp) == name) then return gp end
    end
    return nil
end

UT.msg("--- Utilities loaded ---")
