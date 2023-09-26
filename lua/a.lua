local x = {}

x.w = io.write

x.hi = function(m)
	x.w("hi ", m or "")
end

x.bye = function(m)
	x.w("bye ", m or "")
end

return x
