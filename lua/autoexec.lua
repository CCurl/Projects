-- note: globals are in _G
-- use require "autoexec" to load this file

ut = {}
ut.dump = function(t) if t ~= nil then for k,v in pairs(t) do print(k,v) end end end
ut.size = function(t) local n=0 for _ in pairs(t) do n=n+1 end return n end

ut.copy = function (orig)
    local orig_type = type(orig)
    local obj
    if orig_type == 'table' then
        obj = {}
        for orig_key, orig_value in pairs(orig) do
            if type(orig_value) == 'table' then
                obj[orig_key] = ut.copy(orig_value)
            else
                obj[orig_key] = orig_value
            end
        end
    else -- number, string, boolean, etc
        obj = orig
    end
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
