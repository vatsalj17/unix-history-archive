/*
 *	tabs: set tab stops at terminal. 2.7 of 4/15/77
 *	tabs [tabspec] [+f] [+m[n]] [+ln] +t[type]] [+q]
 */

/*	max # columns used (needed for GSI) */
#define NCOLS 158
#define NTABS 41	/* max # tabs +1 (to be set) */
#define	NTABSCL	21	/* max # tabs + 1 that will be cleared */
#define ESC	033
#define CLEAR	'2'
#define SET	'1'
#define TAB	'\t'
#define CR	'\r'
#define NMG	0	/* no margin setting */
#define GMG	1	/* GSI300S margin */
#define TMG	2	/* TERMINET margin */
#define DMG	3	/* DASI450 margin */

#define TCLRLN	0	/* long, repetitive, general tab clear */

char omit[]	"omitted";		/* +t option omitted entirely */
char tclrsh[] {ESC,CLEAR,CR,0};	/* short sequence for most terminals */
char tclrss[] {ESC,TAB,ESC,CLEAR,CR,0};	/* short including GSIS */
char tclrgs[] {ESC,TAB,CR,0};		/* short, knowing GSIS for sure */
char	tclr40[]	{ESC, 'R', CR, 0};	/* TTY 40/2 */
char *termtab[] {	/* terminal handling info, see struct ttab */
	"",	tclrsh,	158,	132,	NMG,	/* +t by itself */
	omit,	TCLRLN,	158,	132,	TMG,	/* +t omitted entirely */
	"gsi",	TCLRLN,	158,	132,	NMG,	/* old GSI */
	"300",	TCLRLN,	158,	132,	NMG,	/* old GSI */
	"gsis",	tclrgs,	158,	132,	GMG,	/* GSI 300S */
	"300S",	tclrgs,	158,	132,	GMG,	/* GSI 300S */
	"450",	tclrsh,	158,	132,	DMG,	/* DASI 450/DIABLO 1620 */
	"1620",	tclrsh,	158,	132,	DMG,	/* DASI 450/DIABLO 1620 */
	"tn",	tclrsh,	118,	118,	TMG,	/* TERMINET */
	"hp",	TCLRLN,	 80,	 80,	NMG,	/* HP2640A */
	"40-2",	tclr40,	 80,	 80,	NMG,	/* TTY 40/2 */
	0
};

struct ttab {		/* for termtab */
	char *tcode;	/* +tcode */
	char *tclr;	/* char sequence to clear tabs and return carriage */
	int tmaxtab;	/* maximum allowed position */
	int tdefmax;	/* default maximum, if +l omitted */
	int tmarg;	/* type of margin setting allowed */
} *tt	termtab;


int maxtab;	/* max tab value for repetitive spec, from +l, default 132 */
int margin;	/* margin (for TN and GSIS), from +m, default 10 */
int margflg;	/* >0 ==> +m option used, 0 ==> not */
char *terminal	omit;		/* type of terminal used */
char *tabspec	"-8";	/* default tab specification */
char fsuprs;	/* from +f, 0 ==> print tabs if --file, 1 ==> suppress */
char	quick;	/* from +q, 0 ==> clear tabs, 1 ==> omit clear */

int	ttyold[3];	/* tty table */
int	ttysave;	/* save for modes */
int	istty;		/* 1 ==> is actual tty */

static char SCCSID[] "@(#)tabs.c	2.7";
main(argc,argv)
int argc;
char *argv[];
{
	int tabvect[NTABS];	/* build tab list here */
	char *scan;	/* scan pointer to next char */
	int endup();
	signal(2,&endup);	/* catch interrupts */
	if (gtty(1, ttyold) == 0) {
		ttysave = ttyold[2];
		istty++;
	}
	tabvect[0] = 0;	/* mark as not yet filled in */
	while (--argc > 0) {
		scan = *++argv;
		if (*scan == '+')
			switch (*++scan) {
			case 'f':
				fsuprs = 1;
				break;
			case 'l':
				scan++;
				maxtab = getnum(&scan);
				break;
			case 'm':
				margflg++;
				scan++;
				if (*scan)
					margin = getnum(&scan);
				else
					margin = 10;
				break;
			case 'T':
			case 't':
				terminal = ++scan;
				break;
			case 'q':
				quick++;
				break;
			}
		else
			tabspec = scan;		/* save tab specification */
	}
	termadj();	/* check terminal type and alter parameters */
	scantab(tabspec,tabvect,0);
	if (!tabvect[0])
		repetab("8",tabvect);
	settabs(tabvect);
	endup();
	exit(0);
}

/*	scantab: scan 1 tabspec & return tab list for it */

scantab(scan,tabvect,level)
char *scan;
int tabvect[NTABS], level;
{
	register char c;
	if (*scan == '-')
		if ((c = *++scan) == '-')
			filetab(++scan,tabvect,level);
		else if (c >= '0' && c <= '9')
			repetab(scan,tabvect);
		else if (stdtab(scan,tabvect))
			abend("unknown tab code");
		else;
	else
		arbitab(scan,tabvect);
	return;
}

/*	repetab: scan and set repetitve tabs, 1+n, 1+2*n, etc */

repetab(scan,tabvect)
char *scan;
int tabvect[NTABS];
{
	register int incr, i, tabn;
	int limit;
	incr = getnum(&scan);
	tabn = 1;
	limit = (maxtab-1)/(incr?incr:1)-1; /* # last actual tab */
	if (limit>NTABS-2)
		limit = NTABS-2;
	for (i = 0; i<=limit; i++)
		tabvect[i] = tabn =+ incr;
	tabvect[i] = 0;
	return;
}

/*	arbitab: handle list of arbitrary tabs */

arbitab(scan,tabvect)
char *scan;
int tabvect[NTABS];
{
	register int i, t, last;
	char c;
	last = 0;
	for (i = 0; i<NTABS-1;) {
		if ((c = *scan) == '+') {
			scan++;		/* +n ==> increment, not absolute */
			if (t = getnum(&scan))
				tabvect[i++] = last =+ t;
			else abend("illegal increment");
		}
		else {
			if ((t = getnum(&scan)) > last)
				tabvect[i++] = last = t;
			else abend("illegal tabs");
		}
		if (*scan++ != ',') break;
	}
	if (last > NCOLS)
		abend("illegal tabs");
	tabvect[i] = 0;
	return;
}

/*	filetab: copy tabspec from existing file */
#define CARDSIZ	132
filetab(scan,tabvect,level)
char *scan;
int tabvect[NTABS];
{
	register int length, i;
	register char c;
	int fildes;
	char card[CARDSIZ];	/* buffer area for 1st card in file */
	char state, found;
	char *temp;
	if (level)
		abend("file indirection");
	if ((fildes = open(scan,0)) < 0)
		abend("can't open");
	length = read(fildes,card,CARDSIZ);
	close(fildes);
	found = state = 0;
	scan = 0;
	for (i = 0; i<length && (c = card[i]) != '\n'; i++) {
		switch (state) {
		case 0:
			state = (c == '<'); break;
		case 1:
			state = (c == ':')?2:0; break;
		case 2:
			if (c == 't')
				state = 3;
			else if (c == ':')
				state = 6;
			else if (c != ' ')
				state = 5;
			break;
		case 3:
			if (c == ' ')
				state = 2;
			else {
				scan = &card[i];
				state = 4;
			}
			break;
		case 4:
			if (c == ' ') {
				card[i] = '\0';
				state = 5;
			}
			else if (c == ':') {
				card[i] = '\0';
				state = 6;
			}
			break;
		case 5:
			if (c == ' ')
				state = 2;
			else if (c == ':')
				state = 6;
			break;
		case 6:
			if (c == '>') {
				found = 1;
				goto done;
			}
			else state = 5;
			break;
		}
	}
done:
	if (found && scan != 0) {
		scantab(scan,tabvect,1);
		temp = scan;
		while (*++temp);
		*temp = '\n';
		if (fsuprs == 0)
			write(1,scan,++temp-scan);
	}
	else scantab("-8",tabvect,1);
	return;
}

/*	termadj: check terminal type; adjust margin & maxtab if needed.
	set global tt -> right entry in termtab */
termadj()
{
	register struct ttab *t;
	for(t =tt; t->tcode; t++) {
		if (equal(terminal,t->tcode))
			break;
	}
	if (!t->tcode)
		t = tt;		/* unrecognizable = +t by self */
	if (maxtab == 0)
		maxtab = t->tdefmax;	/* use default, since +l omitted */
	else if (maxtab > t->tmaxtab)
		t = t->tmaxtab;		/* too big, use max allowed */
	tt = t;
	return;
}

char	*cleartabs();
/*	settabs: set actual tabs at terminal */
/*	note: this code caters to necessities of handling GSI and
	other terminals in a consistent way. */

settabs(tabvect)
int tabvect[NTABS];
{
	char setbuf[250];	/* 2+3*NTABS+2+NCOLS+NTABS (approx) */
	register char *p;		/* ptr for assembly in setbuf */
	register int *curtab;		/* ptr to tabvect item */
	int i, previous, nblanks;
	if (istty) {
		ttyold[2] =& 0777757;
		stty(1,ttyold);	/* turn off cr-lf map */
	}
	p = setbuf;
	*p++ = CR;
	if (!quick)
		p = cleartabs(p, tt->tclr);

	if (margflg)
		switch(tt->tmarg) {
		case GMG:	/* GSI300S */
		/* NOTE: the 300S appears somewhat odd, in that there is
		a column 0, but there is no way to do a direct tab to it.
		The sequence ESC 'T' '\0' jumps to column 27 and prints
		a '0', without changing the margin. */
			*p++ = ESC;
			*p++ = 'T';	/* setup for direct tab */
			if (margin =& 0177)	/* normal case */
				*p++ = margin;
			else {			/* +m0 case */
				*p++ = 1;	/* column 1 */
				*p++ = '\b';	/* column 0 */
			}
			*p++ = margin;	/* direct horizontal tab */
			*p++ = ESC;
			*p++ = '0';	/* actual margin set */
			break;
		case TMG:	/* TERMINET 300 & 1200 */
			for(i = 1; i <= margin; i++)
				*p++ = ' ';
			break;
		case DMG:	/* DASI450/DIABLO 1620 */
			*p++ = ESC;	/* direct tab ignores margin */
			*p++ = '\t';
			*p++ = (margin & 0177) + 1;
			*p++ = ESC;
			*p++ = '9';
			break;
		}

/*
 *	actual setting: at least terminals do this consistently!
 */
	previous = 1; curtab = tabvect;
	while ((nblanks = *curtab-previous) >= 0 &&
		previous + nblanks <= maxtab) {
		for (i = 1; i <= nblanks; i++) *p++ = ' ';
		previous = *curtab++;
		*p++ =ESC;
		*p++ = SET;
	}
	*p++ = CR;
	if (equal(tt->tclr, tclr40))
		*p++ = '\n';	/* TTY40/2 needs LF, not just CR */
	write(1, setbuf, p - setbuf);
	return;
}

/*	cleartabs(pointer to buffer, pointer to clear sequence */
char *cleartabs(p, qq)
register char *p;
char *qq;
{
	register i;
	register char *q;
	q = qq;
        if (q == TCLRLN) {      /* if repetitive sequence */
		*p++ = CR;
		for(i = 0; i < NTABSCL - 1; i++) {
			*p++ = TAB;
			*p++ = ESC;
			*p++ = CLEAR;
		}
		*p++ = CR;
	}
	else {
		while(*p++ = *q++);	/* copy table sequence */
		p--;			/* adjust for null */
		if (qq == tclr40) {	/* TTY40 extra delays needed */
			*p++ = '\0';
			*p++ = '\0';
			*p++ = '\0';
			*p++ = '\0';
		}
	}
	return(p);
}
/*	getnum: scan and convert number, return zero if none found */
/*	set scan ptr to addr of ending delimeter */
getnum(scan1)
char **scan1;
{
	register int n;
	register char c, *scan;
	n = 0;
	scan = *scan1;
	while ((c = *scan++) >= '0' && c <= '9') n = n * 10 + c -'0';
	*scan1 = --scan;
	return(n);
}

/*	abend: terminate processing with message to terminal */
abend(arg)
char *arg;
{
	register char *temp;
	temp = arg;
	while (*++temp);	/* get length */
	*temp = '\n';
	endup();
	write(1,arg,temp+1-arg);
	exit(1);
}

/*	endup: make sure tty mode reset & exit */
endup()
{
	if (istty) {
		ttyold[2] = ttysave;
		stty(1,ttyold);	/* reset cr-lf to previous */
	}
}

/*	stdtabs: standard tabs table
	format: option code letter(s), null, tabs, null */
char stdtabs[] {
'a',	0,1,10,16,36,72,0,			/* IBM 370 Assembler */
'a','2',0,1,10,16,40,72,0,			/* IBM Assembler alternative*/
'c',	0,1,8,12,16,20,55,0,			/* COBOL, normal */
'c','2',0,1,6,10,14,49,0,			/* COBOL, crunched*/
'c','3',0,1,6,10,14,18,22,26,30,34,38,42,46,50,54,58,62,67,0,
						/* crunched COBOL, many tabs */
'f',	0,1,7,11,15,19,23,0,			/* FORTRAN */
'p',	0,1,5,9,13,17,21,25,29,33,37,41,45,49,53,57,61,0, /* PL/I */
's',	0,1,10,55,0,				/* SNOBOL */
'u',	0,1,12,20,44,0,				/* UNIVAC ASM */
0};

/*	stdtab: return tab list for any "canned" tab option.
	entry: option points to null-terminated option string
		tabvect points to vector to be filled in
	exit: return(0) if legal, tabvect filled, ending with zero
		return(-1) if unknown option
*/
stdtab(option,tabvect)
char option[];
int tabvect[];
{
	register char *sp;
	tabvect[0] = 0;
	sp = stdtabs;
	while (*sp) {
		if (equal(option,sp)) {
			while (*sp++);		/* skip to 1st tab value */
			while (*tabvect++ = *sp++);	/* copy, make int */
			return(0);
		}
		while(*sp++);	/* skip to 1st tab value */
		while(*sp++);		/* skip over tab list */
	}
	return(-1);
}


/*	equal: string comparison, return 1 if equal, 0 otherwise */
equal(s1,s2)
register char	*s1,*s2;
{
	register char c;
	while((c = *s1++) == *s2++ && c);
	if (c == *--s2)
		return(1);
	return(0);
}
