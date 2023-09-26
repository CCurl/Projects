-- A very simple window system in LUA

require("screen")

Win = {}

function Win.ToXY(o, x, y)
    o.c, o.r = x, y
end

function Win.cls(o)
    o.lines = {}
    o:draw()
end

function Win.draw(o)
    scr.curSave()
    scr.curOff();

    local wr = io.write
    local ch = string.char
    scr.toXY(o.x, o.y)
    wr(ch(218)) for i=1,o.w do wr(ch(196)) end wr(ch(191))
    for i = 1, o.h do
        scr.toXY(o.x, o.y+i)
        local str = o.lines[i+o.top] or ""
        wr(ch(179))
        for j = 1, o.w do   wr(string.char(string.byte(str, j) or 32))   end
        wr(ch(179))
    end
    scr.toXY(o.x, o.y+o.h+1)
    wr(ch(192)) for i=1,o.w do wr(ch(196)) end wr(ch(217))

    scr.curRestore()
    scr.curOn();
end

function Win.setTop(o, n)
    o.top = n-1
    if (o.top < 0) then o.top = 0 end
    o:draw()
end

function Win.scroll(o, n)
    o:setTop(o.top + n + 1)
end

function Win.setLine(o, r, str, update, scroll)
    o.lines[r] = str
    if update then
        if scroll and ((o.top+o.h) < r) then
            o:setTop(o.r - o.h + 1)
        end
        o:draw()
    end
end

function Win.write(o, str, update, scroll)
    o:setLine(o.r, str, update == nil or update, scroll == nil or scroll)
    o.r = o.r + 1
end

function Win:new(x, y, h, w)
    o = {
        x=x, y=y, h=h, w=w,
        r=1, c=1, top=0,
        lines={}
    }
    setmetatable(o, self)
    self.__index = self
    return o
end

function Win.test()
    scr.cls()
    local w1 = Win:new(25,  5, 20, 15)
    local w2 = Win:new(40, 20, 10, 30)
    for i=1,50 do w1:write("w1 line #" .. i) end
    for i=1,50 do w2:write("w2 line #" .. i, false, false) end
    w2:write("end", true)
end

function Win.RL()
    package.loaded["Win"] = nil
    require("Win")
end
