W = io.write
timer = os.clock

isPrime = function(n)
	for i = 3, math.sqrt(n), 2 do
		if (n % i) == 0 then
			return false
		end
	end
	return true
end

numPrimes = function(n)
	local c = 4
	for i = 11, n, 2 do
		if isPrime(i) then
			c = c + 1
		end
	end
	return c
end

mil = function(n)
	return n * 1000 ^ 2
end

b1 = function(n)
	local s = timer()
	return numPrimes(n), timer() - s
end
