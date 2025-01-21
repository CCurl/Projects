-- note: globals are in _G
-- use require "autoexec" to load this file

ut = {}
ut.dump = function(t) if t ~= nil then for k,v in pairs(t) do print(k,v) end end end
ut.size = function(t) local n=0 for _ in pairs(t) do n=n+1 end return n end

-- Clone a table (object)
-- Returns the cloned table (object)
ut.clone = function (orig)
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

ut.edit = function()
	os.execute("nvim .")
end

ut.RL = function()
	package.loaded["autoexec"] = nil
	package.loaded["screen"] = nil
	require("autoexec")
end

scr = require("screen")
