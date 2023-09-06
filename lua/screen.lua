local M = {}

M.cls        = function()    io.write("\027[2J") end
M.clrEOL     = function()    io.write("\027[K") end
M.toXY       = function(x,y) io.write("\027[", y, ";", x, "H") end
M.curOn      = function(n)   io.write("\027[?25h") end
M.curOff     = function(n)   io.write("\027[?25l") end
M.curUp      = function(n)   io.write("\027[", n or "", "A") end
M.curDown    = function(n)   io.write("\027[", n or "", "B") end
M.curSave    = function()    io.write("\027[7") end
M.curRestore = function()    io.write("\027[8") end

return M
