# include "e.h"
#include "e.def"

#define	TBLSIZE	100

tbl	*keytbl[TBLSIZE];	/* key words */
tbl	*restbl[TBLSIZE];	/* reserved words */
tbl	*deftbl[TBLSIZE];	/* user-defined names */

struct {
	char	*key;
	int	keyval;
} keyword[]	={
	"sub", 	SUB, 
	"sup", 	SUP, 
	".EN", 	EOF, 
	"from", 	FROM, 
	"to", 	TO, 
/*	"sum", 	SUM,  removed by stan */
	"hat", 	HAT, 
	"vec", VEC, 
	"dyad", DYAD, 
	"dot", 	DOT, 
	"dotdot", 	DOTDOT, 
	"bar", 	BAR, 
	"tilde", 	TILDE, 
	"under", 	UNDER, 
/*	"prod", 	PROD, 
	"int", 	INT, 
	"integral", 	INT, 
	"union", 	UNION, 
	"inter", 	INTER,    removed by stan */
	"pile", 	PILE, 
	"lpile", 	LPILE, 
	"cpile", 	CPILE, 
	"rpile", 	RPILE, 
	"over", 	OVER, 
	"sqrt", 	SQRT, 
	"above", 	ABOVE, 
	"size", 	SIZE, 
	"font", 	FONT, 
	"fat", FAT, 
	"roman", 	ROMAN, 
	"italic", 	ITALIC, 
	"bold", 	BOLD, 
	"left", 	LEFT, 
	"right", 	RIGHT, 
	"delim", 	DELIM, 
	"define", 	DEFINE, 

#ifdef	NEQN	/* make ndefine synonym for define, tdefine a no-op */

	"tdefine",	TDEFINE,
	"ndefine",	DEFINE,

#else		/* tdefine = define, ndefine = no-op */

	"tdefine", 	DEFINE, 
	"ndefine", 	NDEFINE, 

#endif

	"gsize", 	GSIZE, 
	".gsize", 	GSIZE, 
	"gfont", 	GFONT, 
	"include", 	INCLUDE, 
	"up", 	UP, 
	"down", 	DOWN, 
	"fwd", 	FWD, 
	"back", 	BACK, 
	"mark", 	MARK, 
	"lineup", 	LINEUP, 
	"matrix", 	MATRIX, 
	"col", 	COL, 
	"lcol", 	LCOL, 
	"ccol", 	CCOL, 
	"rcol", 	RCOL, 
	0, 	0
};

struct {
	char	*res;
	char	*resval;
} resword[]	= {
	"alpha",	"\\(*a",
	"beta",		"\\(*b",
	"gamma",	"\\(*g",
	"delta",	"\\(*d",
	"epsilon",	"\\(*e",
	"zeta",		"\\(*z",
	"eta",		"\\(*y",
	"theta",	"\\(*h",
	"iota",		"\\(*i",
	"kappa",	"\\(*k",
	"lambda",	"\\(*l",
	"mu",		"\\(*m",
	"nu",		"\\(*n",
	"xi",		"\\(*c",
	"omicron",	"\\(*o",
	"pi",		"\\(*p",
	"rho",		"\\(*r",
	"sigma",	"\\(*s",
	"tau",		"\\(*t",
	"upsilon",	"\\(*u",
	"phi",		"\\(*f",
	"chi",		"\\(*x",
	"psi",		"\\(*q",
	"omega",	"\\(*w",
	"ALPHA",	"\\(*A",
	"BETA",		"\\(*B",
	"GAMMA",	"\\(*G",
	"DELTA",	"\\(*D",
	"EPSILON",	"\\(*E",
	"ZETA",		"\\(*Z",
	"ETA",		"\\(*Y",
	"THETA",	"\\(*H",
	"IOTA",		"\\(*I",
	"KAPPA",	"\\(*K",
	"LAMBDA",	"\\(*L",
	"MU",		"\\(*M",
	"NU",		"\\(*N",
	"XI",		"\\(*C",
	"OMICRON",	"\\(*O",
	"PI",		"\\(*P",
	"RHO",		"\\(*R",
	"SIGMA",	"\\(*S",
	"TAU",		"\\(*T",
	"UPSILON",	"\\(*U",
	"PHI",		"\\(*F",
	"CHI",		"\\(*X",
	"PSI",		"\\(*Q",
	"OMEGA",	"\\(*W",
	">",		"\\fG>\\fP",
	"<",		"\\fG<\\fP",
	">=",		"\\(>=",
	"<=",		"\\(<=",
	"==",		"\\(==",
	"!=",		"\\(!=",
	"+-",		"\\(+-",
	"-+",		"\\fSl\\fP",
	"->",		"\\(->",
	"<-",		"\\(<-",
	"<<",		"\\fSG\\fP",
	">>",		"\\fSg\\fP",
	"><",		"\\fSI\\fP",
	"<>",		"\\fSi\\fP",
	"=~",		"\\fSK\\fP",
	"!<",		"\\fSM\\fP",
	"!>",		"\\fSm\\fP",
	"inf",		"\\(if",
	"infinity",	"\\(if",
	"partial",	"\\(pd",
	"half",		"\\fR1\\(sl2\\fP",
	"prime",	"\\fR\\(aa\\fP",
	"approx",	"\\(ap",
	"propor",	"\\(pt",
	"nothing",	"",
	"cdot",		"\\fG,\\fP",
	"times",	"\\(mu",
	"divide",	"\\(di",
	"del",		"\\(gr",
	"grad",		"\\(gr",
	"laplace",	"\\fSv\\fP",
	"box",		"\\(sq",
	"...",		"\\fG,,,\\fP",
	",...,",	",\\fG,,,\\fP,",
	"sum",		"\\fGF\\fP",
    "int",      "\\b'\\(ti\\(bi'",
    "bint",     "\\b'\\(ti\\(bv\\(bi'",
	"prod",		"\\fGW\\fP",
	"union",	"\\fST\\fP",
	"inter",	"\\fSt\\fP",
	"subset",	"\\fSU\\fP",
	"supset",	"\\fSu\\fP",
	"member",	"\\fS\\zU\\v'.15m'-\\v'-.15m'\\fP",
	"empty",	"\\fR\\o'O/'\\fP",
	"ang",		"\\fSS\\fP",
	"lang",		"\\fSq\\fP",
	"rang",		"\\fSQ\\fP",
	"perp",		"\\fSY\\fP",
	"scriptl",	"\\fSs\\fP",
	"norm",		"\\fSy\\fP",
	"uarrow",	"\\fS^\\fP",
	"larrow",	"\\fSw\\fP",
	"rarrow",	"\\fSW\\fP",
	"darrow",	"\\fS\\(dl\\fP",
	"dagger",	"\\(dg",
	"sub0",		"\\fS0\\fP",
	"sub1",		"\\fS1\\fP",
	"sub2",		"\\fS2\\fP",
	"sub3",		"\\fS3\\fP",
	"sub4",		"\\fS4\\fP",
	"sub5",		"\\fS5\\fP",
	"sub6",		"\\fS6\\fP",
	"sub7",		"\\fS7\\fP",
	"sub8",		"\\fS8\\fP",
	"sub9",		"\\fS9\\fP",
	"sup0",		"\\fS{\\fP",
	"sup1",		"\\fS!\\fP",
	"sup2",		"\\fS\"\\fP",
	"sup3",		"\\fS#\\fP",
	"sup4",		"\\fS$\\fP",
	"sup5",		"\\fS%\\fP",
	"sup6",		"\\fS&\\fP",
	"sup7",		"\\fS\\(aa\\fP",
	"sup8",		"\\fS(\\fP",
	"sup9",		"\\fS)\\fP",
	"ciplus",	"\\fRO\\E*\\(esN+*\\fP",
	"citimes",	"\\fR\\zo\\fP\\fS<\\fP",
	"cidot",	"\\fR\\zO\\fP\\fG,\\fP",
	"=wig",		"\\fSK\\fP",
	"-wig",		"\\fSk\\fP",
	"=dot",		"=\\v'-0.35m'\\h'-\\w*=\\fG,*u/2u'\\fG,\\v'0.35m'\\fP",
	"-dot",		"-\\v'-0.3m'\\h'-\\w*-\\fG,*u/2u'\\fG,\\v'0.3m'\\fP",
	"=hat",		"\\fSn\\fP",
/*	"bstar",	"\\fR\\o'X|-'\\fP",	*/
	"oppA",		"\\fRV\\E'\\(esN-'\\fP",
	"andsign",	"\\fG/\\h'-26u'\\(ss\\fP",
	"orsign",	"\\fG\\(ss\\h'-26u'/\\fP",
	"oppE",
"\\fG\\z_\\v'50u'\\z_\\v'50u'\\z_\\v'-100u'\\h'40u'|\\h'-140u'\\(ss\\fP",
	"nexist",
"\\fG\\z_\\v'50u'\\z_\\v'50u'\\z_\\v'-100u'\\h'40u'|\\h'-140u'\\(ss\\fP",
	"exist",
"\\fG\\z_\\v'50u'\\z_\\v'50u'\\z_\\v'-100u'\\h'40u'|\\h'-40u'\\fP",
	"abs",		"\\fG&\\fP",
	"nie",		"\\fSN\\fP\\E'\\fG\\(esN/'\\fP",
	"angst",	"\\fGA\\h'-100u'\\v'-40u';\\v'40u'\\fP",
	"star",		"*",
	"hbar",		"\\fGf\\fP",
	"<->",		"\\fSw\\E*\\(esNW*\\fP",
	"<=>",		"<=>",
	"3dot",	"\\fG\\z.\\v'-50u'\\z.\\v'-50u'.\\v'100u'\\fP",
	"thf",  "\\fG\\h'-34u'.\\h'-60u'\\v'-60u'.\\v'60u'\\h'-60u'.\\h'-46u'\\fP",
	"quarter",	"\\fR1\\(sl4\\fP",
	"3quarter",	"\\fR3\\(sl4\\fP",
	"degree",	"\\fG;\\fP",
	"circle",	"\\fRO\\fP",
/*	"blot",		"\\fR\\o'HIX0'\\fP",	*/
	"vbar",		"\\(br",
/*	"bullet",	"\\fR\\(bu\\fP",	*/
	"curse",	"\\@!\\#$\\(st",
	"lfl",		"\\(lf",
	"rfl",		"\\(rf",
	"lcl",		"\\(lc",
	"rcl",		"\\(rc",
	"lbs",		"\\fG?\\fP",
	"and",		"\\fRand\\fP",
	"for",		"\\fRfor\\fP",
	"if",		"\\fRif\\fP",
	"Re",		"\\fRRe\\fP",
	"Im",		"\\fRIm\\fP",
	"sin",		"\\fRsin\\fP",
	"cos",		"\\fRcos\\fP",
	"tan",		"\\fRtan\\fP",
	"arc",		"\\fRarc\\fP",
	"sinh",		"\\fRsinh\\fP",
	"coth",		"\\fRcoth\\fP",
	"tanh",		"\\fRtanh\\fP",
	"cosh",		"\\fRcosh\\fP",
	"lim",		"\\fRlim\\fP",
	"log",		"\\fRlog\\fP",
	"max",		"\\fRmax\\fP",
	"min",		"\\fRmin\\fP",
	"ln",		"\\fRln\\fP",
	"exp",		"\\fRexp\\fP",
	"det",		"\\fRdet\\fP",
	0,	0
};

tbl *lookup(tblp, name, defn)	/* find name in tbl. if defn non-null, install */
tbl **tblp;
char *name, *defn;
{
	register tbl *p;
	register int h;
	register char *s = name;
	char *malloc();

	for (h = 0; *s != '\0'; )
		h += *s++;
	h %= TBLSIZE;

	for (p = tblp[h]; p != NULL; p = p->next)
		if (strcmp(name, p->name) == 0) {	/* found it */
			if (defn != NULL)
				p->defn = defn;
			return(p);
		}
	/* didn't find it */
	if (defn == NULL)
		return(NULL);
	p = (tbl *) malloc(sizeof (tbl));
	if (p == NULL)
		error(FATAL, "out of space in lookup");
	p->name = name;
	p->defn = defn;
	p->next = tblp[h];
	tblp[h] = p;
	return(p);
}

init_tbl()	/* initialize all tables */
{
	int i;

	for (i = 0; keyword[i].key != NULL; i++)
		lookup(keytbl, keyword[i].key, keyword[i].keyval);
	for (i = 0; resword[i].res != NULL; i++)
		lookup(restbl, resword[i].res, resword[i].resval);
}
