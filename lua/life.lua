-- life.lua

scr = require("screen")
scr.szX = 150
scr.szY =  45

rows=300
cols=300
mult=1000000

function toNDX(r,c) return (r*mult)+c end
function toRC(ndx) return math.floor(ndx/mult), (ndx%mult) end

function makeRand(n)
    local rnd = math.random
    local g = {}
    for i = 1, n do
        g[toNDX(rnd(rows), rnd(cols))] = true
    end
    return g;
end

function neighbors(g, ndx)
    local n = 0
    local x = ndx-mult-1

    if (g[x])   then n = n + 1 end
    if (g[x+1]) then n = n + 1 end
    if (g[x+2]) then n = n + 1 end

    x = x+mult
    if (g[x])   then n = n + 1 end
    if (g[x+2]) then n = n + 1 end

    x = x+mult
    if (g[x])   then n = n + 1 end
    if (g[x+1]) then n = n + 1 end
    if (g[x+2]) then n = n + 1 end
    
    return n
end

disp = function(g)
    local wr = io.write
    local ul = upperLeft
    scr.toXY(1, 1)
    for y = 0, scr.szY do
        local t = {}
        for x = 0, scr.szX do
            t[x] = (g[ul+x] and "*") or " "
        end
        wr(table.concat(t),"\n")
        ul = ul + mult
    end
end

moveTo = function(g, r, c)
    upperLeft = toNDX(r, c)
    disp(g)
end

move = function(g, addR, addC)
    r, c = toRC(upperLeft)
    moveTo(g, r+addR, c+addC)
end

oneGen = function(g)
    w = {}
    for r = 0, rows do
        local ndx = toNDX(r, 0)
        for c = 0, cols do
            local a = false
            n = neighbors(g, ndx)
            if (n == 2) then a = g[ndx]
            elseif (n == 3) then a = true
            end
            if (a) then w[ndx] = a end
            ndx = ndx + 1
        end
    end
    return w
end

life = function(g, c)
    scr.cls()
    scr.curOff()
    for i =  1, c do
        disp(g)
        print(i)
        g = oneGen(g)
    end
    scr.curOn()
    return g
end

function RL()
    package.loaded["life"] = nil
    require("life")
end

upperLeft = toNDX(10, 10)
g = life(makeRand(rows^2*.7), 50)
