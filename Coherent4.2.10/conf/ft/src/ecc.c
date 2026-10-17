/* ecc.c */
int	checkEcc();

/*
eccT0 - log2
eccT1 - alog2
eccT2 - mpy by 02
eccT3 - mpy by c0
eccT4 - mpy by c3
*/

extern	unsigned char	eccT0[];		/* ecc tables		*/
extern	unsigned char	eccT1[];
extern	unsigned char	eccT2[];
extern	unsigned char	eccT3[];
extern	unsigned char	eccT4[];

static	unsigned char	mpy();			/* declare functions	*/
static	unsigned char	dvd();


static	unsigned char *	pbfr;			/* adr buffer		*/
static	int	nbfr;			/* num of bfrs in block	*/

static	unsigned char	s0, s1, s2;		/* syndromes		*/
static	int  	l0, l1, l2;		/* locations		*/
static	unsigned char	L0, L1, L2;		/* 2** locations	*/
static	unsigned char	M0, M1, M2;		/* c3** locations	*/

static	unsigned char	m00, m01, m02;		/* inverse matrix	*/
static	unsigned char	m10, m11, m12;
static	unsigned char	m20, m21, m22;
static	unsigned char	det;

static	unsigned char	n00, n01, n10, n11;	/* 2nd inverse matrix	*/

static	unsigned char	e0, e1, e2;		/* error values		*/
static	unsigned char	c0, c1;			/* check error values	*/

/************************************************************************/
/*	multiply							*/
/************************************************************************/
static	unsigned char
mpy( m0, m1 )
unsigned char	m0;
unsigned char	m1;
{
	register int m2;

	if ( m0 == 0 || m1 == 0 )
		return( 0 );
	m2 = eccT0[ m0 ] + eccT0[ m1 ];
	if ( m2 >= 255 )
		m2 -= 255;
	return( eccT1[ m2 ] );
}

/************************************************************************/
/*	divide								*/
/************************************************************************/
static	unsigned char
dvd( m0, m1 )
unsigned char	m0;
unsigned char	m1;
{
	register int	m2;

	if( m0 == 0 )
		return( 0 );
	m2 = eccT0[ m0 ] - eccT0[ m1 ];
	if( m2 < 0 )
		m2 += 255;
	return( eccT1[ m2 ] );
}
/************************************************************************/
/*	3 error case							*/
/************************************************************************/
/*									*/
/*	syndrome equations						*/
/*									*/
/*	| 1  1	1|   |e0|   |s0|					*/
/*	|L0 L1 L2|   |e1| = |s1|					*/
/*	|M0 M1 M2|   |e2|   |s2|					*/
/*									*/
/*	inversing matrix gives error equations				*/
/*									*/
/*	|e0|	|L1*M2+M1*L2  M1+M2  L1+L2|  |s0|			*/
/*	|e1| =	|L0*M2+M0*M2  M0+M2  L0+L2|  |s1|			*/
/*	|e2|	|L0*M1+M0*L1  M0+M1  L0+L1|  |s2|			*/
/*	       -----------------------------------			*/
/*	       L0*M1+L0*M2+L1*M0+L1*M2+L2*M0+L2*M1			*/
/************************************************************************/

static unsigned char ae, af, bd, bf, cd, ce;	/* temps */
static unsigned char aebd, afcd, bfce;

static int
fix3()					/* 3 error case			*/
{
	unsigned char *	p0;
	int	d0;
	int	d1;

	L0 = eccT1[ 31 - l0 ];	/* generate powers of 2 (Lx) 	*/
	M0 = eccT1[ 224 + l0 ];	/*  and C3 (Mx) [ C3 = 1/2 ]	*/
	L1 = eccT1[ 31 - l1 ];
	M1 = eccT1[ 224 + l1 ];
	L2 = eccT1[ 31 - l2 ];
	M2 = eccT1[ 224 + l2 ];

	ae = mpy( L0, M1 );		/* generate inverse matrix	*/
	af = mpy( L0, M2 );
	bd = mpy( L1, M0 );
	bf = mpy( L1, M2 );
	cd = mpy( L2, M0 );
	ce = mpy( L2, M1 );
	aebd = ae ^ bd;
	afcd = af ^ cd;
	bfce = bf ^ ce;
	det = aebd ^ afcd ^ bfce;
	m00 = dvd( bfce, det );
	m01 = dvd( M1 ^ M2, det );
	m02 = dvd( L1 ^ L2, det );
	m10 = dvd( afcd, det );
	m11 = dvd( M0 ^ M2, det );
	m12 = dvd( L0 ^ L2, det );
	m20 = dvd( aebd, det );
	m21 = dvd( M0 ^ M1, det );
	m22 = dvd( L0 ^ L1, det );

	for ( d1 = 0; d1 < 1024; ++d1 ){
		p0 = pbfr;			/* generate syndromes */
		s2 = s1 = s0 = 0;
		for ( d0 = 0; d0 < nbfr; ++d0 ){
			s0 ^= *p0;
			s1 = eccT2[ s1 ] ^ *p0;
			s2 = eccT4[ s2 ] ^ *p0;
			p0 += 1024;
		}

		/* generate error values */
		e0 = mpy( m00, s0 ) ^ mpy( m01, s1 ) ^ mpy( m02, s2 );
		e1 = mpy( m10, s0 ) ^ mpy( m11, s1 ) ^ mpy( m12, s2 );
		e2 = mpy( m20, s0 ) ^ mpy( m21, s1 ) ^ mpy( m22, s2 );

		/* correct errors */
		*( pbfr + l0 * 1024 ) ^= e0;
		*( pbfr + l1 * 1024 ) ^= e1;
		*( pbfr + l2 * 1024 ) ^= e2;

		pbfr++;
		/* bump to next column */
	}	
	/* indicate ok */
	return( 1 );
}

/************************************************************************/
/*	2 error case							*/
/************************************************************************/
/*									*/
/*	syndrome equations						*/
/*									*/
/*	| 1  1|   |e0|	 |s0|						*/
/*	|L0 L1|   |e1| = |s1|						*/
/*	|M0 M1| 	 |s2|						*/
/*									*/
/*	using 2 of the 3 possible inverses:				*/
/*									*/
/*	|e0|	 |L1  1|  |s0|	      |M1  1|  |s0|			*/
/*	|e1|  =  |L0  1|  |s1|	  =   |M0  1|  |s2|			*/
/*		 -------------	      -------------			*/
/*		     L0+L1		  M0+M1 			*/
/************************************************************************/

static int
fix2()
{
	unsigned char *	p0;
	int	d0;
	int	d1;

	L0 = eccT1[ 31 - l0 ];	/* generate powers of 2 (Lx) &	*/
	M0 = eccT1[ 224 + l0 ];	/*  and C3 (Mx) [C3 = 1/2]	*/
	L1 = eccT1[ 31 - l1 ];
	M1 = eccT1[ 224 + l1 ];

	det = L0 ^ L1;			/* generate inverse matrix	*/
	m00 = dvd( L1, det );
	m01 = dvd( 1, det );
	m10 = dvd( L0, det );
	m11 = m01;

	det = M0 ^ M1;			/* generate check inv matrix	*/
	n00 = dvd( M1, det );
	n01 = dvd( 1, det );
	n10 = dvd( M0, det );
	n11 = n01;

	for( d1 = 0; d1 < 1024; ++d1 ){
		/* generate syndromes */
		p0 = pbfr;
		s2 = s1 = s0 = 0;
		for( d0 = 0; d0 < nbfr; ++d0 ){
			s0 ^= *p0;
			s1 = eccT2[ s1 ] ^ *p0;
			s2 = eccT4[ s2 ] ^ *p0;
			p0 += 1024;
		}

		/* generate error values */
		e0 = mpy( m00, s0) ^ mpy( m01, s1 );
		e1 = mpy( m10, s0) ^ mpy( m11, s1 );

		/* generate check values */
		c0 = mpy( n00, s0) ^ mpy( n01, s2 );
		c1 = mpy( n10, s0) ^ mpy( n11, s2 );

		/* exit if uncorrectable */
		if ( e0 != c0 || e1 != c1 )
			return( 0 );

		/* correct errors */
		*( pbfr + l0 * 1024 ) ^= e0;	
		*( pbfr + l1 * 1024 ) ^= e1;

		pbfr++;
		/* bump to next column */
	}
	/* indicate ok */
	return( 1 );
}

/************************************************************************/
/*	1 error case							*/
/*	also could be 2 errors, 1 known location			*/
/************************************************************************/
/*									*/
/*	syndrome equations						*/
/*									*/
/*	| 1|	      |s0|						*/
/*	|L0|   |e0| = |s1|						*/
/*	|M0|	      |s2|						*/
/*									*/
/*	e0 = s0 = s1/L0 = s2/M0						*/
/************************************************************************/

static unsigned char a, b, c;		/* temps */

static int
fix1()
{
	unsigned char *	p0;
	int	d0;
	int	d1;

	/* generate powers of 2 (Lx) and */
	L0 = eccT1[ 31 - l0 ];
	/*  and C3 (Mx) [C3 = 1/2] */
	M0 = eccT1[ 224 + l0 ];

	for( d1 = 0; d1 < 1024; ++d1 ){
		/* generate syndromes */
		p0 = pbfr;
		s2 = s1 = s0 = 0;
		for( d0 = 0; d0 < nbfr; ++d0 ){
			s0 ^= *p0;
			s1 = eccT2[ s1 ] ^ *p0;
			s2 = eccT4[ s2 ] ^ *p0;
			p0 += 1024;
		}

		/* generate error value */
		e0 = s0;
		/* generate check values */
		c0 = dvd( s1, L0 );
		c1 = dvd( s2, M0 );

		/* if not 1 error case */
		if( e0 != c0 || e0 != c1 ){
			/*   test for 2 error case */
			a = mpy( M0, s0 ) ^ s2;
			/*   and generate l1 */
			b = mpy( M0, s1 ) ^ mpy( L0, s2 );
			c = mpy( L0, s0 ) ^ s1;
			if( !a )
				return( 0 );
			L1 = dvd( b, a ) ^ L0;
			L2 = dvd( dvd( c, a ), L0 );
			if( L1 != L2 )
				return( 0 );
			l1 = eccT0[ L1 ];
			if( l1 > 31 )
				return( 0 );
			l1 = 31 - l1;
			/* handle as 2 error case */
			return( fix2() );	
		}

		/* correct error */
		*( pbfr + l0 * 1024 ) ^= e0;

		/* bump to next column */
		pbfr++;
	}
	/* indicate ok */
	return( 1 );
}

/************************************************************************/
/*	1 error case, location unknown					*/
/************************************************************************/
/*									*/
/*	syndrome equations						*/
/*									*/
/*	| 1|	      |s0|						*/
/*	|L0|   |e0| = |s1|						*/
/*	|M0|	      |s2|						*/
/*									*/
/*	location equations						*/
/*									*/
/*	L0 = s1/s0 = s0/s2						*/
/*	l0 = log2[L0]							*/
/*									*/
/*	error equation							*/
/*									*/
/*	e0 = s0								*/
/************************************************************************/

static int
fix0()
{
	unsigned char *	p0;
	int	d0;
	int	d1;

	for ( d1 = 0; d1 < 1024; ++d1 ){
		/* generate syndromes */
		p0 = pbfr;
		s2 = s1 = s0 = 0;
		for( d0 = 0; d0 < nbfr; ++d0 ){
			s0 ^= *p0;
			s1 = eccT2[ s1 ] ^ *p0;
			s2 = eccT4[ s2 ] ^ *p0;
			p0 += 1024;
		}

		/* br if no error */
		if( ( s0 | s1 | s2 ) ){
			/* generate l0 or fail */
			L0 = dvd( s1, s0 );
			L1 = dvd( s0, s2 );
			if ( L0 != L1 )
				return( 0 );
			l0 = eccT0[ L0 ];
			if( l0 > 31 )
				return( 0 );
			l0 = 31 - l0;

			/* generate error value */
			e0 = s0;

			/* correct error */
			*( pbfr + l0 * 1024 ) ^= e0;
		}

		/* bump to next column */
		pbfr++;
	}
	/* indicate ok */
	return( 1 );
}

/************************************************************************/
/*	decodeEcc	correct a block 					*/
/************************************************************************/
int
decodeEcc( adr, nb, errc, err0, err1, err2 )
unsigned char *	adr;	/* adr of bfr		*/
unsigned char	nb; 	/* # blocks in bfr	*/
unsigned char	errc;	/* error count		*/
unsigned char	err0;	/* location of errors	*/
unsigned char	err1;	/* location of errors	*/
unsigned char	err2;	/* location of errors	*/
{
	unsigned char *	p0;
	int	d0;

	/* globalize inputs	*/
	pbfr = adr;
	nbfr = (int)nb;
	l0 = (int)err0;
	l1 = (int)err1;
	l2 = (int)err2;
	switch(errc){
	case 0:
		if( !checkEcc( adr, nb ) )
			return( 1 );
		return( fix0() );
	case 1:
		return( fix1() );
	case 2:
		return( fix2() );
	case 3:
		return( fix3() );
	default:
		break;
	}
	return( 0 );
}
