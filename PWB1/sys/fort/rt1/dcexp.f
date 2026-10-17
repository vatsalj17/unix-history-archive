	function cexp(z1)
	double complex cexp, z1, z2
	double precision dummy(2), a, b
	equivalence(a,z2,dummy(1)),(b,dummy(2))

	z2 = z1
	c = dexp(a)
	a = c * dcos(b)
	b = c * dsin(b)
	cexp = z2
	return
	end
