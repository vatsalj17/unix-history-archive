#define	MAX_WORDS	(200)		/* 200 words per paragraph */

/*
 *	dvi specific definitions
 */
#define	pt_per_in	(72)
#define	sp_per_pt	(1<<16)

#define	scale_num	(25400000)	/* scaling factor - sp to 10e-7 m. */
#define	scale_denom	(473628672)

#define	TEX_ID		(2)		/* TeX version 2 */


#define	WD_WORD	(0x01)
#define	WD_TAB	(0x02)
#define	WD_DTAB (0x04)
#define	WD_UL	(0x08)

#define	TABMAX	10			/* can you believe this? */



#define	SET0	0
#define	PUT1	133
#define	PUTRULE	137
#define	NOP	138
#define	BOP	139
#define	EOP	140
#define	PUSH	141
#define	POP	142
#define	RIGHT1	143
#define	RIGHT2	144
#define	RIGHT3	145
#define	RIGHT4	146
#define	W0	147
#define	W1	148
#define	X0	152
#define	X1	153
#define	X2	154
#define	X3	155
#define	X4	156
#define	DOWN1	157
#define	DOWN2	158
#define	DOWN3	159
#define	DOWN4	160
#define	Y0	161
#define	Y1	162
#define	Y2	163
#define	Y3	164
#define	Y4	165
#define	Z0	166
#define	Z1	167
#define	FONT0	171
#define	FONT1	235
#define	XXX1	239
#define	DFONT1	243
#define	PRE	247
#define	POST	248
#define	POSTPOST 249


#define	pt_to_sp(x)	((x) * sp_per_pt)
#define	mpt_to_sp(x)	(pt_to_sp(x)/1000)
#define	min_to_sp(x)	(pt_to_sp( (((x) * pt_per_in) / 1000) ))
#define	fix_to_sp(x)	(((x>>3)+1)>>1)
#define	pix_to_sp(x)	pt_to_sp((((double)x) / 80) * pt_per_in)

#define	sp_to_in(x)	((((double)x) / sp_per_pt) / pt_per_in )

#define	out4(v)		(out( outfd, 4, (long)(v) ))
#define	out3(v)		(out( outfd, 3, (long)(v) ))
#define	out2(v)		(out( outfd, 2, (long)(v) ))
#define	out1(v)		(out( outfd, 1, (long)(v) ))

#define	outcmd(c)	(out1(c))

/*
 *	type defininitions
 */

typedef	struct
	{
	short	wd_flags;
	SP	wd_length;
	int	wd_pos;		
	int	wd_end;	
	} WORD;
#define	wd_tabcnt	wd_pos

typedef struct
	{
	SP	p_h;		/* current horizontal position */
	SP	p_v;		/* current vertical position */
	SP	p_w;		/* delta */
	SP	p_x;		/* delta */
	SP	p_y;		/* delta */
	SP	p_z;		/* delta */
	} POS;

/*
 *	external variables
 */

extern	FILE	* outfd;
