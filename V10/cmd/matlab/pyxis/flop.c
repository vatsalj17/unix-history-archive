struct {
    long sym, syn[4], buf[256], char_, flp[2], fin, fun, lhs, rhs, ran[2];
} com_;

 typedef union {
	long L[2];
	double d;
	} dL;

 static long mask[26] = {
	0xffffffff, 0xfffffff0,
	0xffffffff, 0xffffff00,
	0xffffffff, 0xfffff000,
	0xffffffff, 0xffff0000,
	0xffffffff, 0xfff00000,
	0xffffffff, 0xff000000,
	0xffffffff, 0xf0000000,
	0xffffffff, 0x00000000,
	0xfffffff0, 0x00000000,
	0xffffff00, 0x00000000,
	0xfffff000, 0x00000000,
	0xffff0000, 0x00000000,
	0xfff00000, 0x00000000};

 double
flop_(x)
 register dL *x;
{
	register long k;
	register dL *m;

	++com_.flp[0];
	k = com_.flp[1];
	if (k <= 0)
		return x->d;
	if (k >= 14)
		return 0.;
	m = (dL *)&mask[(k - 1)<<1];
	x->L[0] &= m->L[0];
	x->L[1] &= m->L[1];
	return x->d;
	}
