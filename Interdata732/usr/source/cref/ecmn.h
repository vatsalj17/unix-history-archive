EXTERN int	count;
EXTERN int	page;
EXTERN struct	{
		char	*beg;
		int	ct;
		char	term;
	} temp[30];

EXTERN int	pn;
EXTERN int	word;
EXTERN int	hsw;
EXTERN int	t1;

EXTERN int	level;
EXTERN int	hlevel;
EXTERN int	dlevel;
EXTERN int	xtrn;

EXTERN int	tp[2];

EXTERN struct	{
		int	cl[NUMC];
	} tab[NUMS];

EXTERN int	coll();
EXTERN int	save();
EXTERN int	out();
EXTERN int	hyphen();
EXTERN int	hyp1();
EXTERN int	hyp2();
EXTERN int	pno();
EXTERN int	error();

EXTERN char	line[300];
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
EXTERN int	cross;
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

EXTERN int	ipsp[PTRI];
EXTERN char	issp[CHARI];

EXTERN int	ib1[131];

EXTERN char	*ibuf;
EXTERN char	*ibuf1;

extern char	mone;
EXTERN int	order;
EXTERN int	ssw;
EXTERN int	type;
EXTERN char	*utmp;
