#ifndef MixedSizes			/* int and word are one and the same */
#if IntSize == 32
#define CIntSize		   32	/* Number of bits in cset word */
#define LogIntSize		    5	/* log of IntSize */
#define MinShort	      0100000	/* smallest short integer */
#define MaxShort	       077777	/* largest short integer */
#define MinLong 	020000000000L	/* smallest long integer */
#define MaxLong 	017777777777L	/* largest long integer */
#define LogHuge 		   39	/* maximum base-10 exp+1 of real */
#define GranSize		 1024	/* storage allocation granule size */
#define MaxStrLen	     07777777	/* maximum string length */
/*
 * Cset initialization macros.
 */
#define twd(w0, w1)	((w0)&0xffff | (w1)<<16)
#define cset_display(w0,w1,w2,w3,w4,w5,w6,w7,w8,w9,wa,wb,wc,wd,we,wf) \
	{twd(w0,w1),twd(w2,w3),twd(w4,w5),twd(w6,w7), \
	 twd(w8,w9),twd(wa,wb),twd(wc,wd),twd(we,wf)}

#define F_Nqual 0x80000000		/* set if NOT string qualifier */
#define F_Var	0x40000000		/* set if variable */
#define F_Tvar	0x20000000		/* set if trapped variable */
#define F_Ptr	0x10000000		/* set if value field is pointer */
#endif IntSize == 32

#if IntSize == 16
#define CIntSize	   16		/* Number of bits in cset word */
#define LogIntSize	    4		/* log of CIntSize */
#define MinLong 020000000000L		/* smallest long integer */
#define MaxLong 017777777777L		/* largest long integer */
#define MinShort      0100000		/* smallest short integer */
#define MaxShort       077777		/* largest short integer */
#define LogHuge 	   39		/* maximum base-10 exp +1 of float number */
#define GranSize	   64		/* storage allocation granule size */
#define MaxStrLen      077777		/* maximum string length */
/*
 * Cset initialization macros.
 */
#define cset_display(w0,w1,w2,w3,w4,w5,w6,w7,w8,w9,wa,wb,wc,wd,we,wf) \
	{w0,w1,w2,w3,w4,w5,w6,w7,w8,w9,wa,wb,wc,wd,we,wf}

#define F_Nqual     0x8000		/* set if NOT string qualifier */
#define F_Var	    0x4000		/* set if variable */
#define F_Tvar	    0x2000		/* set if trapped variable */
#define F_Ptr	    0x1000		/* set if value field is pointer */

#endif IntSize == 16

#else MixedSizes
#define CIntSize		   16	/* Number of bits in cset word */
#define LogIntSize		    4	/* log of IntSize */
#define MinShort	      0100000	/* smallest short integer */
#define MaxShort	       077777	/* largest short integer */
#define MinLong 	020000000000L	/* smallest long integer */
#define MaxLong 	017777777777L	/* largest long integer */
#define LogHuge 		   39	/* maximum base-10 exp+1 of real */
#define GranSize		 1024	/* storage allocation granule size */
#define MaxStrLen	     07777777	/* maximum string length */

/*
 * Cset initialization macros.
 */
#define cset_display(w0,w1,w2,w3,w4,w5,w6,w7,w8,w9,wa,wb,wc,wd,we,wf) \
	{w0,w1,w2,w3,w4,w5,w6,w7,w8,w9,wa,wb,wc,wd,we,wf}

#define F_Nqual		0x80000000L	/* set if NOT string qualifier */
#define F_Var		0x40000000L	/* set if variable */
#define F_Tvar		0x20000000L	/* set if trapped variable */
#define F_Ptr		0x10000000L	/* set if value field is pointer */

#endif MixedSizes
