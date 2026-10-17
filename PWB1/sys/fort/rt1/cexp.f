	function cexp(z1)
	complex cexp, z1, z2
	dimension dummy(2)
	equivalence(a,z2,dummy(1)),(b,dummy(2))

	z2 = z1
	c = exp(a)
	a = c * cos(b)
	b = c * sin(b)
	cexp = z2
	return
	end
