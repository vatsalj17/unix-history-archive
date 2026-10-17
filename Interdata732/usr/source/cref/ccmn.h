EXTERN struct	tempent {
		char	*beg;
		int	ct;
		char	term;
	} temp[30];

EXTERN char	lbuf[5];
EXTERN int	cflag;
EXTERN int	t1;

EXTERN int	level;
EXTERN int	hlevel;
EXTERN int	dlevel;
EXTERN int	xtrn;

EXTERN int	tp[5];

EXTERN struct	{
		int	cl[NUMC];
	} tab[NUMS];

EXTERN int	coll();
EXTERN int	save();
EXTERN int	out();
EXTERN int	asym();
EXTERN int	asw();
EXTERN int	csym();
EXTERN int	csw();
EXTERN int	incl();
EXTERN int	decl();
EXTERN int	sk2();
EXTERN int	sk();
EXTERN int	tabs();
EXTERN int	semi();

EXTERN char	line[132];
EXTERN int	l;
EXTERN int	lno;
EXTERN char	c;
EXTERN int	cursl;
EXTERN char	curs[9];
EXTERN int	curfl;
EXTERN char	curf[10];

EXTERN int	usw;
EXTERN int	xsw;
EXTERN int	only;
extern int	cross;
EXTERN int	file;
EXTERN int	cs;
EXTERN int	(*flag[8])();
EXTERN int	fl;
EXTERN char	gch[8];

EXTERN struct	htab	{
		int	hsiz;
		int	ssiz;
		int	nsym;
		int	curb;
		int	*hptr;
		char	*symt;
		};

EXTERN struct	htab	itab;
EXTERN struct	htab	xtab;

EXTERN int	ipsp[PTRI];
EXTERN char	issp[CHARI];
EXTERN int	xpsp[PTRX];
EXTERN char	xssp[CHARX];

EXTERN int	ib1[259];
EXTERN int	ib2[259];

EXTERN char	*ibuf;
EXTERN char	*ibuf1;
EXTERN char	*ibuf2;

extern char	mone;
extern int	order;
EXTERN int	ssw;
EXTERN int	type;
EXTERN char	*utmp;
