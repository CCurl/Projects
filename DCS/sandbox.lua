/*

coalition.side = {
   NEUTRAL = 0
   RED = 1
   BLUE = 2
}

Group.Category = {
  AIRPLANE      = 0
  HELICOPTER    = 1
  GROUND        = 2
  SHIP          = 3
  TRAIN         = 4
}

*/

ut = { side=nil }

ut.isRed = function(side) return (side == coalition.side.RED) end
ut.isBlue = function(side) return (side == coalition.side.BLUE) end

ut.dumpGroups(color) = function(groups)
    env.info(color .. " groups:")
    for i, gp in pairs(groups) do
        env.info(Group.getName(gp))
    end
end

ut.dumpRedGroups() = function()
    dump(ut.getGroups(coalition.side.RED))
end

ut.dumpBlueGroups() = function()
    dump(ut.getGroups(coalition.side.BLUE))
end

ut.getGroups(side) = function()
    local groups = {}
    if (!side) then side = ut.side end
    if (ut.isRed(side)) then
        groups = coalition.getGroups(coalition.side.RED)
    end
    if (ut.isBlue(side)) then
        groups = coalition.getGroups(coalition.side.RED)
    end
    return groups;
end

ut.findGroup = function(side, name)
    local groups = ut.getGroups(side)
    for i, gp in pairs(groups) do
        if (Group.getName() == name) then return gp end
    end
    return nil
end
