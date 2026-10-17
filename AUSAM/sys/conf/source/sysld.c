#

/*
 *	special link editor for pdp 11/40 systems
 *	jeff rottman
 */

#include <param.h>
#define	SIZAUT	8
#define	CALLR0	04037
#define	SIGINT	2
#define	ARCMAGIC 0177545
#define	FMAGIC	0407
#define	NMAGIC	0410
#define	IMAGIC	0411
#define	KW	2048

#define	EXTERN	040
#define	UNDEF	00
#define	ABS	01
#define	TEXT	02
#define	DATA	03
#define	BSS	04
#define	COMM	05	/* internal use only */

#define	RABS	00
#define	RTEXT	02
#define	RDATA	04
#define	RBSS	06
#define	REXT	010


#define	RELFLG	01
#define	NROUT	256
#define	NSYM	807
#define	NSYMPR	500
#define	NAUT	256	/* possible number of autoloads: 256 max */
#define	NSEG	256	/* possible number of segments: 256 max */

#define	RONLY	0400

char	premeof[] "Premature EOF";

struct	page {
	int	nuser;
	int	bno;
	int	nibuf;
	int	buff[256];
	} page[2];

struct	{
	int	nuser;
	int	bno;
	} fpage;

struct	stream {
	int	*ptr;
	int	bno;
	int	nibuf;
	int	size;
	struct page	*pno;
	};

struct stream	text;
struct stream	reloc;

struct	archdr {
	char	aname[14];
	int	atime[2];
	char	auid, agid;
	int	amode;
	int	asize[2];
	} archdr;

struct	filhdr {
	int	fmagic;
	unsigned Tsize;
	unsigned Dsize;
	unsigned Bsize;
	unsigned Ssize;
	char	*entry;
	int	pad;
	int	relflg;
	} filhdr;

struct	liblist {
	int	off;
	int	bno;
	};

struct liblist	liblist[NROUT];
struct liblist	*libp { &liblist[0] };

struct	symbol {
	char	sname[8];
	char	stype;
	char	spad;
	int	svalue;
	int	sseg;		/* segment in which symbol lies. Only for common */
	char	saut;		/* symbols autoload number */
	char	ssseg;		/* saved sseg */
	};

struct symbol	cursym;
struct symbol	symtab[NSYM];
struct symbol	*hshtab[NSYM+2];
struct symbol	*symp	{ symtab };
struct symbol	**local[NSYMPR];
struct symbol	*backed[NSYMPR];
struct symbol	**backp;
struct symbol	*p_etext;
struct symbol	*p_edata;
struct symbol	*p_end;
struct symbol	*p_eover;	/* address at which segment text ends (file), rounded up */
struct symbol	*p_eto;		/* top of onceonly segment (physical blocks) */
struct symbol	*p_eu;		/* physical address of process 0 (physical blocks) */
struct symbol	*p_etoa;	/* top on onceonly segment (virtual) */
struct symbol	*p_eres;	/* bottom of oncelonly segment (file) */
struct symbol	*entrypt;

int	Lflag;		/* list some sizes not available later */
int	xflag;		/* discard local symbols */
int	cflag;		/* list common segments */
int	Xflag;		/* discard locals starting with 'L' */
int	Sflag;		/* discard all except locals and globals*/
int	sflag;		/* discard all symbols */
int	Vflag;		/* add overlay symbols */
int	nwhere;

int	ofilfnd;
char	*ofilename "l.out";
int	infil;
char	*filname;

struct	segdat {
	unsigned tsize;		/* size of text segment */
	unsigned dsize;		/* size of data segment */
	unsigned csize;		/* size of common segment */
	unsigned bsize;		/* size of bss segment */
	unsigned spage;		/* page number where it starts */
	char	*base;		/* base address for segment text */
	} segdat[NSEG];

struct	autdat {
	int	autseg;		/* segment number in which it lies */
	char	*realad;	/* real address in kmem */
	int	nargs;
	} autdat[NAUT];

#define	NARGS	8

int	save0[]	{
	013716, 0172352, 0012037, 0172352, 0004730, 0012637, 0172352, 0000207
	};

int	save1[]	{
	016616, 0000004, 0013766, 0172352, 0000004, 0012037, 0172352, 0004730,
	005726, 0016637, 0000002, 0172352, 0000207
	};

int	save2[]	{
	016616, 0000004, 0016666, 0000002, 0000004, 0016666, 0000006, 0000002,
	013766, 0172352, 0000006, 0012037, 0172352, 0004730, 0022626, 0016637,
	000002, 0172352, 0011607
	};

int	save3[] { 
	0016601, 0000002, 0016616, 0000004, 0016666, 0000006, 0000002, 0016666,
	0000010, 0000004, 0010166, 0000006, 0013766, 0172352, 0000010, 0012037,
	0172352, 0004730, 0022626, 0016637, 0000004, 0172352, 0000176, 0000002
	};

int	save4[] { 
	0016601, 0000002, 0016616, 0000004, 0016666, 0000006, 0000002, 0016666,
	0000010, 0000004, 0016666, 0000012, 0000006, 0010166, 0000010, 0013766,
	0172352, 0000012, 0012037, 0172352, 0004730, 0022626, 0016637, 0000006,
	0172352, 0000176, 0000004
	};

int	save5[] { 
	0016601, 0000002, 0016616, 0000004, 0016666, 0000006, 0000002, 0016666,
	0000010, 0000004, 0016666, 0000012, 0000006, 0016666, 0000014, 0000010,
	0010166, 0000012, 0013766, 0172352, 0000014, 0012037, 0172352, 0004730,
	0022626, 0016637, 0000010, 0172352, 0000176, 0000006
	};

int	save6[] { 
	0016601, 0000002, 0016616, 0000004, 0016666, 0000006, 0000002, 0016666,
	0000010, 0000004, 0016666, 0000012, 0000006, 0016666, 0000014, 0000010,
	0016666, 0000016, 0000012, 0010166, 0000014, 0013766, 0172352, 0000016,
	0012037, 0172352, 0004730, 0022626, 0016637, 0000012, 0172352, 0000176,
	0000010
	};

int	save7[] { 
	0016601, 0000002, 0016616, 0000004, 0016666, 0000006, 0000002, 0016666,
	0000010, 0000004, 0016666, 0000012, 0000006, 0016666, 0000014, 0000010,
	0016666, 0000020, 0000012, 0016666, 0000020, 0000014, 0010166, 0000016,
	0013766, 0172352, 0000016, 0012037, 0172352, 0004730, 0022626, 0016637,
	0000014, 0172352, 0000176, 0000012
	};

struct	caldat {
	int	clong;
	char	*ccode;
	char	*calnam;
	char	*ccore;
	int	cused;
	} caldat[NARGS] {
	{ sizeof save0, save0, "..save0" },
	{ sizeof save1, save1, "..save1" },
	{ sizeof save2, save2, "..save2" },
	{ sizeof save3, save3, "..save3" },
	{ sizeof save4, save4, "..save4" },
	{ sizeof save5, save5, "..save5" },
	{ sizeof save6, save6, "..save6" },
	{ sizeof save7, save7, "..save7" },
	};
char	*autotbl;	/* address of core autoload table */
unsigned tsegsize;	/* total size of segments */
unsigned tsegtxt;
unsigned tsegdat;
unsigned tsegbss;
unsigned tsegcom;
int	ssize;		/* size of symbol part of file */
int	nsym;
int	naut;		/* number of autoloads */
int	mseg;		/* segment counter 2 */
int	lastseg;
int	nseg;		/* number of segments */
int	xseg;		/* the active segment */

int	torigin;
int	dorigin;
int	borigin;

int	ctrel;
int	cdrel;
int	cbrel;

int	errlev;
int	delarg	4;
char	tfname[] "/tmp/lxyyyyy";
int	toutb[259];
int	doutb[259];
int	otoutb[259];
int	odoutb[259];
int	soutb[259];

struct symbol	**lookup();
struct symbol	**slookup();

int	fout;

main(argc, argv)
char **argv;
{
	register state, *ap;
	register char *cp;
	static xargc, xargv[500];
	static char xdata[3000];

	fout = dup(1);

	read(open(argv[1], 0), xdata, sizeof xdata);
	ap = xargv;
	for (cp = xdata; *cp != '\0'; cp++) {
		if (state) {
			if (*cp==' ' || *cp=='\t' || *cp==',' || *cp=='\n') {
				*cp = 0;
				state = 0;
			}
		} else {
			if (*cp!=' ' && *cp!='\t' && *cp!=',' && *cp!='\n') {
				*ap++ = cp;
				xargc++;
				state++;
			}
		}
	}
	mainx(xargc, xargv);
	exit(0);
}

mainx(argc, argv)
char **argv;
{
	extern int delexit();
	register c;
	register char *ap, **p;
	struct symbol **hp;
	int k;

	if ((signal(SIGINT, 1) & 01) == 0)
		signal(SIGINT, delexit);
	if (argc == 1)
		exit(4);
	p = argv + 1;
	for (c = 1; c < argc; c++) {
		filname = 0;
		ap = *p++;
		if (*ap == '-') switch (ap[1]) {

		case 'o':
			if (++c >= argc)
				error(1, "Bad output file");
			ofilename = *p++;
			ofilfnd++;
			continue;

		case 'u':
		case 'e':
			if (++c >= argc)
				error(1, "Bad 'use' or 'entry'");
			if (*(hp = slookup(*p++)) == 0) {
				*hp = symp;
				enter();
			}
			if (ap[1] == 'e')
				entrypt = *hp;
			continue;

		case 'l':
			break;

		case 'x':
			xflag++;
			continue;

		case 'X':
			Xflag++;
			continue;

		case 'c':
			cflag++;
			continue;

		case 'L':
			Lflag++;
			continue;

		case 'S':
			Sflag++; 
			continue;

		case 's':
			sflag++;
			xflag++;
			continue;

		case 'N':	
			nwhere = nseg + 1;
			continue;

		case 'O':		/* a page 5 segment */
			if (++nseg >= NSEG)
				error(1, "Too many segments");
			xseg = nseg;
			continue;

		case 'R':		/* root segment */
			xseg = 0;
			continue;

		case 'A':		/* a list of autoloads */
			while (c < argc-1 && **p != '-') {
				++c;
				k = getnum(*p);
				if (k >= NARGS) {
					printf("'%s' ", p++);
					error(0, "too many args");
					continue;
				}
				if (*(hp = slookup(*p++)) == 0) {
					printf("'%s' ", p[-1]);
					error(0, "Undefined autoload");
					continue;
				}
				if ((*hp)->sseg == 0) {
					printf("'%s' ", p[-1]);
					error(0, "Not in segment");
				}
				if ((*hp)->stype != EXTERN+TEXT) {
					printf("'%s' ", p[-1]);
					error(0, "Autoload not in text");
					continue;
				}
				if (++naut >= NAUT)
					error(1, "Too many autoloads");
				caldat[k].cused++;
				autdat[naut].nargs = k;
				(*hp)->saut = naut;
			}
			continue;

		case 'T':
			continue;

		case 'V':		/* add some overlay symbols */
			Vflag++;
			continue;

		case 'C':		/* a list of root commons */
			while (c < argc-1 && **p != '-') {
				++c;
				if (*(hp = slookup(*p++)) == 0) {
					printf("'%s' ", p[-1]);
					error(0, "Undefined common");
					continue;
				}
				(*hp)->sseg = 0;
			}
			continue;
		}
		load1arg(ap);
		close(infil);
	}
	filname = 0;
	middle();
	setupout();
	p = argv+1;
	libp = liblist;
	for (c=1; c<argc; c++) {
		ap = *p++;
		if (*ap == '-')
			switch (ap[1]) {

			case 'u':
			case 'e':
			case 'o':
				++c;
				++p;
			default:
				continue;

			case 'A':
			case 'C':
				while (c < argc-1 && **p != '-') {
					++c;
					++p;
				}
				continue;

			case 'O':
				xseg = ++mseg;
				continue;

			case 'R':
				xseg = 0;
				continue;

			case 'l':
				break;
			}
		load2arg(ap);
		close(infil);
	}
	finishout();
}

getnum(s)
register char *s;
{
	register n;

	n = 0;
	while (*s)
		if (*s++ == ':') {
			s[-1] = 0;
			while (*s)
				n = n *10 + *s++ - '0';
			break;
		}
	return(n);
}

load1arg(cp)
register char *cp;
{
	register noff, nbno;

	if (!getfile(cp)) {
		load1(0, 0, 0);
		return;
	}
	nbno = 0;
	noff = 1;
	for (;;) {
		dseek(&text, nbno, noff, sizeof archdr);
		if (text.size <= 0) {
			libp->bno = -1;
			libp++;
			return;
		}
		mget(&archdr, sizeof archdr);
		if (load1(1, nbno, noff + (sizeof archdr) / 2)) {
			libp->bno = nbno;
			libp->off = noff;
			libp++;
		}
		noff =+ ((archdr.asize[1]&0777)+sizeof(archdr)+1) >> 1;
		nbno =+ (archdr.asize[1] >> 9) & 0177;
		nbno =+ (archdr.asize[0]) << 7;
		nbno =+ noff >> 8;
		noff =& 0377;
	}
}

load1(libflg, bno, off)
{
	register struct symbol *sp, **hp, ***cp;
	struct symbol *ssymp;
	int ndef, nloc, type, mtype;

	readhdr(bno, off);
	ctrel = segdat[xseg].tsize;
	cdrel =+ segdat[xseg].dsize;
	cbrel =+ segdat[xseg].bsize;
	ndef = 0;
	nloc = sizeof cursym - 4;
	cp = local;
	backp = backed;
	ssymp = symp;
	if (filhdr.relflg & RELFLG) {
		error(0, "No relocation bits");
		return(0);
	}
	off =+ (sizeof filhdr)/2 + filhdr.Tsize + filhdr.Dsize;
	dseek(&text, bno, off, filhdr.Ssize);
	while (text.size > 0) {
		mget(&cursym, (sizeof cursym) - 4);
		type = cursym.stype;
		if (Sflag) {
			mtype = type & 037;
			if (mtype==1 || mtype>4)
				continue;
		}
		if (!(type & EXTERN)) {
			if (!Xflag || cursym.sname[0]!='L')
				nloc =+ sizeof cursym - 4;
			continue;
		}
		symreloc();
		hp = lookup();
		if ((sp = *hp) == 0) {
			*hp = enter();
			*cp++ = hp;
			continue;
		}
		if (sp->stype != EXTERN+UNDEF)
			continue;
		if (cursym.stype == EXTERN+UNDEF) {
			if (cursym.svalue > sp->svalue) 
				sp->svalue = cursym.svalue;
			if (sp->sseg != xseg) {
				sp->ssseg = sp->sseg;
				sp->sseg = 0; 
				*backp++ = sp;
			}
			continue;
		}
		if (sp->svalue != 0 && cursym.stype == EXTERN+TEXT)
			continue;
		ndef++;
		sp->stype = cursym.stype;
		sp->svalue = cursym.svalue;
		sp->sseg = xseg;
	}
	if (!libflg || ndef) {
		segdat[xseg].tsize =+ filhdr.Tsize;
		segdat[xseg].dsize =+ filhdr.Dsize;
		segdat[xseg].bsize =+ filhdr.Bsize;
		ssize =+ nloc;
		return(1);
	}
/*
 * No symbols defined by this library member.
 * Rip out the hash table entries and reset the symbol table.
 */
	symp = ssymp;
	while (cp > local) 
		**--cp = 0;
	while (backp > backed) {
		backp--;
		(*backp)->sseg = (*backp)->ssseg & 0377;
	}
	return(0);
}

middle()
{
	register struct symbol *sp;
	register t, csize;
	int nund, corigin;
	long coreaddr;
	long filesize, rootsize, segsize;

	p_etext = *slookup("_etext");
	p_edata = *slookup("_edata");
	p_end = *slookup("_end");
	p_eover = *slookup("_eover");
	p_eto = *slookup("_eto");
	p_etoa = *slookup("_etoa");
	p_eu = *slookup("_eu");
	p_eres = *slookup("_eres");
/*
 * Assign common locations.
 */
	ldrsym(p_etext, segdat[0].tsize, EXTERN+TEXT);
	ldrsym(p_edata, segdat[0].dsize, EXTERN+DATA);
	ldrsym(p_end, segdat[0].bsize, EXTERN+BSS);
	if (cflag)
		printf("\fCommon      Size (seg)\n");
	for (sp = symtab; sp < symp; sp++)
		if (sp->stype==EXTERN+UNDEF && (t = sp->svalue)!=0) {
			t = (t+1) & ~01;
			sp->svalue = segdat[sp->sseg].csize;
			sp->stype = EXTERN+COMM;
			segdat[sp->sseg].csize =+ t;
			if (cflag)
				printf("%-8.8s  %6o (%u)\n", sp->sname, t, sp->sseg);
		}
/*
 * Now set symbols to their final value
 */
	autotbl = segdat[0].tsize;
	segdat[0].tsize =+ naut * SIZAUT;
	for (t = 0; t < NARGS; t++) 
		if (caldat[t].cused) {
			caldat[t].ccore = segdat[0].tsize;
			segdat[0].tsize =+ caldat[t].clong;
			addsym(caldat[t].calnam, caldat[t].ccore, TEXT);
		}
	rootsize = segdat[0].tsize + segdat[0].dsize + segdat[0].csize + segdat[0].bsize;
	filesize = segdat[0].tsize + segdat[0].dsize;


	/*	root segment may not exceed 20Kw	*/
	/*	total file text may not exceed 27Kw	*/
	/*	each segment may not exceed 4Kw		*/

	if (rootsize > 20L * KW)
		error(0, "root segment too large");
	coreaddr = rootsize;
	if (Lflag) {
		printf("\fSegment  Text  Data  Common   Bss  Total      Base        File    Core\n");
		printf("%-3u    %6o%6o  %6o%6o %6O  %5o/%-6o  %6o  %6o\n",
			0,
			segdat[0].tsize, segdat[0].dsize,
			segdat[0].csize, segdat[0].bsize,
			rootsize,
			0,0,0,0);
	}
	for (t = 1; t <= nseg; t++) {
		if (t == nwhere) {
			ldrsym(p_eu, ldiv(coreaddr + 63, 64), EXTERN+ABS);
			/*coreaddr =+ (2*USIZE + 2)*64;*/
			coreaddr =+ (3*USIZE + 3)*64;	/* enough for 3 per-process areas + 1 seg */
			ldrsym(p_eres, segdat[0].dsize + tsegsize, EXTERN+DATA);
		}
		segsize = segdat[t].tsize + segdat[t].dsize + segdat[t].csize + segdat[t].bsize;
		tsegsize =+ segsize;
		tsegtxt =+ segdat[t].tsize;
		tsegdat =+ segdat[t].dsize;
		tsegbss =+ segdat[t].bsize;
		tsegcom =+ segdat[t].csize;
		if (segsize > 4 * KW - (-coreaddr & 077))
			error(0, "segment is too large");
		segdat[t].spage = coreaddr >> 6;
		segdat[t].base = 20 * KW + (coreaddr & 077);
		if (Lflag)
			printf("%-3u    %6o%6o  %6o%6o %6O  %5o/%-6o %7O %7O\n",
				t,
				segdat[t].tsize, segdat[t].dsize,
				segdat[t].csize, segdat[t].bsize,
				segsize,
				segdat[t].spage, segdat[t].base,
				filesize, coreaddr);
		filesize =+ segsize;
		coreaddr =+ segsize;
		if (Vflag)
			segsym(t);
	}
	if (Lflag)
		printf("Segs   %6o%6o  %6o%6o %6o               %7O %7O\n",
			tsegtxt, tsegdat, tsegcom, tsegbss, tsegsize, filesize, coreaddr);
	if (filesize > 28L * KW)
		error(0, "Too much text/data");
	ldrsym(p_eover, segdat[0].dsize + tsegsize, EXTERN+DATA);
	ldrsym(p_eto, ldiv(coreaddr, 64), EXTERN+ABS);
	ldrsym(p_etoa, lrem(coreaddr, 64) + 24*KW, EXTERN+ABS);
	if (nwhere == 0) {
		ldrsym(p_eu, ldiv(coreaddr + 63, 64), EXTERN+ABS);
		ldrsym(p_eres, 0, EXTERN+DATA);
	}
	nund = 0;
	if (Lflag) 
		printf("\fAutoload   Vector     Real  Seg  Args\n");
	for (sp = symtab; sp < symp; sp++) {
		torigin = segdat[sp->sseg].base;
		dorigin = torigin + segdat[sp->sseg].tsize;
		corigin = dorigin + segdat[sp->sseg].dsize;
		borigin = corigin + segdat[sp->sseg].csize;
		switch (sp->stype) {

		case EXTERN+UNDEF:
			errlev =| 01;
			if (sp->svalue == 0) {
				if (!nund)
					printf("Undefined:\n");
				nund++;
				printf("%.8s\n", sp->sname);
			}
			break;
	
		case EXTERN+ABS:
		default:
			break;
	
		case EXTERN+TEXT:
			sp->svalue =+ torigin;
			break;
	
		case EXTERN+DATA:
			sp->svalue =+ dorigin;
			break;
	
		case EXTERN+BSS:
			sp->svalue =+ borigin;
			break;
	
		case EXTERN+COMM:
			sp->stype = EXTERN+BSS;
			sp->svalue =+ corigin;
			break;
		}
		if (sp->saut) {
			autdat[sp->saut & 0377].autseg = segdat[sp->sseg].spage;
			autdat[sp->saut & 0377].realad = sp->svalue;
			if (Vflag)
				ovrsym(sp->sname, autotbl + SIZAUT*((sp->saut & 0377) - 1), TEXT);
			if (Lflag)
				printf("%-8.8s   %6o   %6o %3d.   %2d.\n",
					sp->sname, 
					autotbl + SIZAUT*((sp->saut & 0377) - 1),
					autdat[sp->saut & 0377].realad,
					sp->sseg,
					autdat[sp->saut & 0377].nargs);
		}
		sp->spad = sp->sseg;	/* save the segment no. on the output file */
	}
	if (sflag || xflag)
		ssize = 0;
	filhdr.fmagic = FMAGIC;
	filhdr.Tsize = segdat[0].tsize;
	filhdr.Dsize = segdat[0].dsize + tsegsize;
	filhdr.Bsize = segdat[0].bsize + segdat[0].csize;
	filhdr.Ssize = sflag ? 0 : (ssize + (sizeof cursym - 4) * (symp - symtab));
	filhdr.entry = 0;
	if ((sp = entrypt) != 0) {
		if (sp->stype != EXTERN+TEXT)
			error(0, "Entry point not in text");
		else
			filhdr.entry = sp->svalue | 01;
	}
	filhdr.pad = 0;
	filhdr.relflg = 1;
	for (t = 0; t <= nseg; t++) {
		segdat[t].tsize =+ segdat[t].base;
		segdat[t].dsize =+ segdat[t].tsize + segdat[t].csize;
		segdat[t].bsize =+ segdat[t].csize;
	}
	nsym = ssize / (sizeof cursym - 4);
}

/*
 * generate symbol of form "..saven" for each of
 * argument management routines in paging scheme.
 */
addsym(name, val, type)
char	*name;
int	val;
int	type;
{
	register struct symbol	**hp;
	register struct symbol	*sp;

	hp = slookup(name);
	if (*hp == 0) {
		*hp = sp = symp;
		cursym.svalue = val;
		cursym.stype = type;
		enter();
	}
}

/*
 * generate symbol of form ".xxxx" for each of the
 * autoload vector entry-points.  In practise, the
 * initial underscore (_) is replaced with dot (.).
 */
ovrsym(name, val, type)
char	*name;
int	val;
int	type;
{
	register struct symbol	**hp;

	cp8c(name, cursym.sname);
	cursym.sname[0] = '.';
	cursym.svalue = val;
	cursym.stype = type;
	hp = lookup();
	if (*hp == 0) {
		*hp = symp;
		enter();
	}
}

/*
 * generate symbol of form "..segnnn" for each of the
 * autoload segments.
 */
segsym(t)
int	t;
{
	register int	i;
	register struct symbol	**hp;

	cp8c("..seg", cursym.sname);
	i = t;
	cursym.sname[7] = i%10 + '0';
	i =/ 10;
	cursym.sname[6] = i%10 + '0';
	i =/ 10;
	cursym.sname[5] = i + '0';
	cursym.svalue = segdat[t].spage;
	cursym.stype = ABS;
	hp = lookup();
	if (*hp == 0) {
		*hp = symp;
		enter();
	}
}

ldrsym(sp, val, type)
register struct symbol *sp;
{
	if (sp == 0)
		return;
	if (sp->stype != EXTERN+UNDEF || sp->svalue) {
		printf("%.8s: ", sp->sname);
		error(0, "Multiply defined");
		return;
	}
	sp->stype = type;
	sp->svalue = val;
	sp->sseg = 0;
}

setupout()
{
	register char *p;
	register pid;
	struct symbol *ep;

	if ((toutb[0] = creat(ofilename, 0606)) < 0)
		error(1, "Can't write output");
	pid = getpid();
	for (p = &tfname[12]; p > &tfname[7];) {
		*--p = (pid&07) + '0';
		pid =>> 3;
	}
	tcreat(doutb, 'a');
	if (!sflag || !xflag)
		tcreat(soutb, 'b');
	tcreat(otoutb, 'c');
	tcreat(odoutb, 'd');
	mput(toutb, &filhdr, sizeof filhdr);
	return;
}

tcreat(buf, letter)
int *buf;
{
	tfname[6] = letter;
	if ((buf[0] = creat(tfname, RONLY)) < 0)
		error(1, "Can't create temp");
	buf[1] = buf[2] = 0;
}

load2arg(acp)
char *acp;
{
	register char *cp;
	register struct liblist *lp;

	cp = acp;
	if (!getfile(cp)) {
		while (*cp)
			cp++;
		while (cp >= acp && *--cp != '/');
		mkfsym(++cp);
		load2(0, 0);
		return;
	}
	for (lp = libp; lp->bno != -1; lp++) {
		dseek(&text, lp->bno, lp->off, sizeof archdr);
		mget(&archdr, sizeof archdr);
		mkfsym(archdr.aname);
		load2(lp->bno, lp->off + (sizeof archdr) / 2);
	}
	libp = ++lp;
}

load2(bno, off)
{
	register struct symbol *sp;
	register int *lp, symno;
	int type, mtype;

	checkseg();
	readhdr(bno, off);
	ctrel = segdat[xseg].base;
	cdrel =+ segdat[xseg].tsize;
	cbrel =+ segdat[xseg].dsize;
/*
 * Reread the symbol table, recording the numbering
 * of symbols for fixing external references.
 */
	lp = local;
	symno = -1;
	off =+ (sizeof filhdr)/2;
	dseek(&text, bno, off+filhdr.Tsize+filhdr.Dsize, filhdr.Ssize);
	while (text.size > 0) {
		symno++;
		mget(&cursym, sizeof cursym - 4);
		symreloc();
		type = cursym.stype;
		if (Sflag) {
			mtype = type & 037;
			if (mtype==1 || mtype>4)
				continue;
		}
		if (!(type & EXTERN)) {
			if (!sflag && !xflag && (!Xflag || cursym.sname[0]!='L'))
				mput(soutb, &cursym, sizeof cursym - 4);
			continue;
		}
		if ((sp = *lookup()) == 0)
			error(1, "internal error: symbol not found");
		if (cursym.stype == EXTERN+UNDEF) {
			if (lp >= &local[NSYMPR])
				error(1, "Local symbol overflow");
			*lp++ = symno;
			*lp++ = sp;
			continue;
		}
		if (cursym.stype!=sp->stype || cursym.svalue!=sp->svalue) {
			printf("%.8s: ", cursym.sname);
			error(0, "Multiply defined");
		}
	}
	dseek(&text, bno, off, filhdr.Tsize);
	dseek(&reloc, bno, off+(filhdr.Tsize+filhdr.Dsize)/2, filhdr.Tsize);
	load2td(lp, ctrel, xseg ? otoutb : toutb);
	dseek(&text, bno, off+(filhdr.Tsize/2), filhdr.Dsize);
	dseek(&reloc, bno, off+filhdr.Tsize+(filhdr.Dsize/2), filhdr.Dsize);
	load2td(lp, cdrel, xseg ? odoutb : doutb);
	segdat[xseg].base =+ filhdr.Tsize;
	segdat[xseg].tsize =+ filhdr.Dsize;
	segdat[xseg].dsize =+ filhdr.Bsize;
}

checkseg()
{
	register f, n, *p;

	if (lastseg!=xseg && lastseg!=0) {
		fflush(odoutb);
		close(odoutb[0]);
		tfname[6] = 'd';
		f = open(tfname, 0);
		while ((n = read(f, odoutb, 512)) > 0) {
			n =>> 1;
			p = odoutb;
			do {
				putw(*p++, otoutb);
			} while (--n != 0);
		}
		close(f);
		unlink(tfname);
		tcreat(odoutb, 'd');
		if (n = segdat[lastseg].bsize >> 1)
			do {
				putw(0, otoutb);
			} while (--n != 0);
	}
	lastseg = xseg;
}

load2td(lp, creloc, b1)
int *lp;
{
	register r, t;
	int ad;

	for (ad = 0;;) {
		ad =+ 2;
	/*
	 * The pickup code is copied from "get" for speed.
	 */
		if (--text.size <= 0) {
			if (text.size < 0)
				break;
			text.size++;
			t = get(&text);
		} else if (--text.nibuf < 0) {
			text.nibuf++;
			text.size++;
			t = get(&text);
		} else
			t = *text.ptr++;
		if (--reloc.size <= 0) {
			if (reloc.size < 0)
				error(1, "Relocation error");
			reloc.size++;
			r = get(&reloc);
		} else if (--reloc.nibuf < 0) {
			reloc.nibuf++;
			reloc.size++;
			r = get(&reloc);
		} else
			r = *reloc.ptr++;
		switch (r&016) {

		case RTEXT:
			t =+ ctrel;
			if (!(r & 01) && xseg) {
				register struct autdat	*ap;
				int	spage;

				spage = segdat[xseg].spage;
				for (ap = autdat + naut; ap > autdat; ap--)
					if (t==ap->realad && spage==ap->autseg) {
						t = autotbl + SIZAUT*((ap - 1) - autdat);
						break;
					}
			}
			break;

		case RDATA:
			t =+ cdrel;
			break;

		case RBSS:
			t =+ cbrel;
			break;

		case REXT:
			{
				register struct symbol	*sp;

				sp = lookloc(lp, r);
				if (sp->stype == EXTERN+UNDEF) {
					r = (r & 01) + ((nsym + (sp - symtab)) << 4) + REXT;
					break;
				}
				if (sp->saut && t+(r&1 ? ad : 0)==0 && (xseg!=sp->sseg || !(r&01)))
					t =+ autotbl + SIZAUT*((sp->saut & 0377) - 1) - sp->svalue;
				else if (sp->sseg && xseg != (sp->sseg & 0377)) {
					printf("'%.8s' ", sp->sname);
					error(0, "Unlikely reference");
				}
				t =+ sp->svalue;
				r = (r&01) + ((sp->stype-(EXTERN+ABS))<<1);
			}
			break;
		}
		if (r&01)
			t =- creloc;
		putw(t, b1);
	}
}

finishout()
{
	register n, *w;
	register struct symbol *p;

	for (n = 1; n <= naut; n++) {
		putw(CALLR0, toutb);
		putw(caldat[autdat[n].nargs].ccore, toutb);
		putw(autdat[n].autseg, toutb);
		putw(autdat[n].realad, toutb);
	}
	for (n = 0; n < NARGS; n++)
		if (caldat[n].cused)
			for (w = caldat[n].ccode; w < caldat[n].ccode + caldat[n].clong;)
				putw(*w++, toutb);
	copy(doutb, 'a');
	xseg = 0;
	checkseg();
	copy(otoutb, 'c');
	if (!sflag) {
		if (!xflag)
			copy(soutb, 'b');
		for (p = symtab; p < symp; p++)
			mput(toutb, p, sizeof cursym - 4);
	}
	fflush(toutb);
	close(toutb[0]);
	if (!ofilfnd) {
		unlink("a.out");
		link("l.out", "a.out");
		ofilename = "a.out";
	}
	delarg = errlev;
	delexit();
}

delexit()
{
	register c;

	unlink("l.out");
	for (c = 'a'; c <= 'd'; c++) {
		tfname[6] = c;
		unlink(tfname);
	}
	if (!delarg)
		chmod(ofilename, 0777);
	flush();
	exit(delarg);
}

copy(buf, c)
int *buf;
{
	register f, *p, n;

	fflush(buf);
	close(buf[0]);
	tfname[6] = c;
	f = open(tfname, 0);
	while ((n = read(f, doutb, 512)) > 1) {
		n =>> 1;
		p = doutb;
		do {
			putw(*p++, toutb);
		} while (--n != 0);
	}
	close(f);
}

mkfsym(s)
char *s;
{

	if (sflag || xflag)
		return;
	cp8c(s, cursym.sname);
	cursym.stype = 037;
	cursym.svalue = torigin;
	mput(soutb, &cursym, sizeof cursym - 4);
}

mget(aloc, an)
int *aloc;
{
	register *loc, n;
	register *p;

	n = an;
	n =>> 1;
	loc = aloc;
	if ((text.nibuf =- n) >= 0) {
		if ((text.size =- n) > 0) {
			p = text.ptr;
			do {
				*loc++ = *p++;
			} while (--n != 0);
			text.ptr = p;
			return;
		} else
			text.size =+ n;
	}
	text.nibuf =+ n;
	do {
		*loc++ = get(&text);
	} while (--n != 0);
}

mput(buf, aloc, an)
int *aloc;
{
	register *loc;
	register n;

	loc = aloc;
	n = an>>1;
	do {
		putw(*loc++, buf);
	} while (--n != 0);
}

dseek(sp, ab, o, s)
register struct stream	*sp;
{
	register struct page *p;
	register b;
	int n;

	b = ab + ((o>>8) & 0377);
	o =& 0377;
	--sp->pno->nuser;
	if ((p = &page[0])->bno!=b && (p = &page[1])->bno!=b)
		if (p->nuser==0 || (p = &page[0])->nuser==0) {
			if (page[0].nuser==0 && page[1].nuser==0)
				if (page[0].bno < page[1].bno)
					p = &page[0];
			p->bno = b;
			seek(infil, b, 3);
			if ((n = read(infil, p->buff, 512)>>1) < 0)
				n = 0;
			p->nibuf = n;
		} else
			error(1, "No pages");
	++p->nuser;
	sp->bno = b;
	sp->pno = p;
	sp->ptr = p->buff + o;
	if (s != -1)
		sp->size = (s>>1) & 077777;
	if ((sp->nibuf = p->nibuf-o) <= 0)
		sp->size = 0;
}

get(sp)
register struct stream *sp;
{
	if (--sp->nibuf < 0) {
		dseek(sp, sp->bno+1, 0, -1);
		--sp->nibuf;
	}
	if (--sp->size <= 0) {
		if (sp->size < 0)
			error(1, premeof);
		++fpage.nuser;
		--sp->pno->nuser;
		sp->pno = &fpage;
	}
	return(*sp->ptr++);
}

getfile(cp)
register char *cp;
{
	register c;

	archdr.aname[0] = '\0';
	filname = cp;
	if (cp[0]=='-' && cp[1]=='l') {
		if (cp[2] == '\0')
			cp = "-la";
		filname = "/usr/lib/libxxxxxxxxxxxxxxx";
		for (c = 0; cp[c+2] != '\0'; c++)
			filname[c+12] = cp[c+2];
		filname[c+12] = '.';
		filname[c+13] = 'a';
		filname[c+14] = '\0';
		if ((infil = open(filname+4, 0)) >= 0) {
			filname =+ 4;
			goto OK;
		}
	}
	if ((infil = open(filname, 0)) < 0)
		error(1, "cannot open");
OK:
	page[0].bno = page[1].bno = -1;
	page[0].nuser = page[1].nuser = 0;
	text.pno = reloc.pno = &fpage;
	fpage.nuser = 2;
	dseek(&text, 0, 0, 2);
	if (text.size <= 0)
		error(1, premeof);
	return(get(&text) == ARCMAGIC);
}

struct symbol **lookup()
{
	int i;
	register struct symbol **hp;
	register char *cp, *cp1;

	i = 0;
	for (cp = cursym.sname; cp < &cursym.sname[8];)
		i = (i<<1) + *cp++;
	for (hp = &hshtab[(i & 077777)%NSYM + 2]; *hp != 0;) {
		cp1 = (*hp)->sname;
		for (cp = cursym.sname; cp < &cursym.sname[8];)
			if (*cp++ != *cp1++)
				goto no;
		break;
	    no:
		if (++hp >= &hshtab[NSYM+2])
			hp = hshtab;
	}
	return(hp);
}

struct symbol **slookup(s)
char *s;
{
	cp8c(s, cursym.sname);
	cursym.stype = EXTERN+UNDEF;
	cursym.svalue = 0;
	return(lookup());
}

enter()
{
	register struct symbol *sp;
	
	if ((sp = symp) >= &symtab[NSYM])
		error(1, "Symbol table overflow");
	cp8c(cursym.sname, sp->sname);
	sp->stype = cursym.stype;
	sp->svalue = cursym.svalue;
	sp->sseg = xseg;
	sp->saut = 0;
	symp++;
	return(sp);
}

symreloc()
{
	switch (cursym.stype) {

	case TEXT:
	case EXTERN+TEXT:
		cursym.svalue =+ ctrel;
		return;

	case DATA:
	case EXTERN+DATA:
		cursym.svalue =+ cdrel;
		return;

	case BSS:
	case EXTERN+BSS:
		cursym.svalue =+ cbrel;
		return;

	case EXTERN+UNDEF:
		return;
	}
	if (cursym.stype & EXTERN)
		cursym.stype = EXTERN+ABS;
}

error(n, s)
char *s;
{
	if (filname) {
		printf("%s", filname);
		if (archdr.aname[0])
			printf("(%.14s)", archdr.aname);
		printf(": ");
	}
	printf("%s\n", s);
	if (n)
		delexit();
	errlev = 2;
}

lookloc(lp, r)
register int	*lp;
{
	register int *clp;
	register sn;

	sn = (r >> 4) & 07777;
	for (clp = local; clp < lp; clp =+ 2)
		if (clp[0] == sn)
			return(clp[1]);
	error(1, "Local symbol botch");
}

readhdr(bno, off)
{
	register st, sd;

	dseek(&text, bno, off, sizeof filhdr);
	mget(&filhdr, sizeof filhdr);
	if (filhdr.fmagic != FMAGIC)
		error(1, "Bad format");
	st = (filhdr.Tsize+01) & ~01;
	filhdr.Tsize = st;
	cdrel = -st;
	sd = (filhdr.Dsize+01) & ~01;
	cbrel = - (st+sd);
	filhdr.Bsize = (filhdr.Bsize+01) & ~01;
}

cp8c(from, to)
char *from, *to;
{
	register char *f, *t, *te;

	f = from;
	t = to;
	te = t+8;
	while ((*t++ = *f++) && t<te);
	while (t < te)
		*t++ = 0;
}
