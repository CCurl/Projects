-- Conway's Game of Life in LUA

scr = require("screen")
scr.Width  = 190
scr.Height =  45

rows=100
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

disp = function(g, gen)
    local ul = upperLeft
    scr.toXY(1, 1)
    for y = 0, scr.Height do
        local t = {}
        for x = 0, scr.Width do
            t[x] = (g[ul+x] and "*") or " "
        end
        io.write(table.concat(t),"\n")
        ul = ul + mult
    end
    print(gen or "")
end

moveTo = function(g, r, c)
    upperLeft = toNDX(r, c)
    if g then disp(g) end
end

function alive(g, ndx)
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

    if (n == 2) then return g[ndx]
    elseif (n == 3) then return true
    else return false
    end
end

oneGen = function(g,i)
    local w = {}
    for r = 0, rows do
        ndx = toNDX(r, 0)
        for c = 0, cols do
            if alive(g, ndx+c) then w[ndx+c] = true end
        end
    end
	disp(g,i)
    return w
end

life = function(g, gens)
    scr.cls()
    scr.curOff()
    for i = 1, gens do
        g = oneGen(g,i)
    end
    scr.curOn()
    return g
end

function RL()
    package.loaded["life"] = nil
    require("life")
end

g = makeRand(rows*cols*.7)
moveTo(g, 20, 20)
g = life(g, 1000)
