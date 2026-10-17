#
/*
 *	Interdata -> Unix object code converter
 */

/* Relocation bits */
#define	RABS	00
#define	RTEXT	02
#define	RDATA	04
#define	REXT	010
#define	RHI	01

/* Symbol types */
#define	SABS	01
#define	STEXT	02
#define	SDATA	03
#define	SEXT	040

/* a.out header */

struct {
	int mword;
	int tsize;
	int dsize;
	int bsize;
	int ssize;
	int entrypt;
	int unused;
	int rflag;
} hdr;

/* External reference chain */
struct chain {
	struct chain *next;	/* link to next entry		*/
	int	defloc;		/* location of reference	*/
	int	refseg;		/* segment referred to		*/
				/* (ext ref: -(symbol no)	*/
	int	refloc;		/* location referred to		*/
				/* (ext ref: offset)		*/
};

/* Segment information */
struct segment {
	int loc;		/* location counter		*/
	int maxloc;		/* highest location so far	*/
	int offset[2];		/* file offsets - code, reloc	*/
	int nchar;		/* no. chars in buffers		*/
	struct chain *first;	/* chain of ext refs		*/
	char obuf[2][512];	/* buffers - code, reloc bits	*/
} pure, impure;

/* buffer for symbol table */
int	symbuf[128];
int	nsym;			/* no. words in buf		*/
int	symoff;			/* file offset of sym tab	*/
int	symno;			/* next available sym number	*/

/* table of common blocks */
#define	NCOM	32
struct combuf {
	int com_name[2];	/* name of common block */
	int com_sym;		/* symbol number */
	int com_len;		/* length of block */
	int com_off;		/* offset of beginning of block */
} combuf[NCOM];

struct combuf	*lastcom combuf;
int	bdata;			/* block data flag */

struct segment	*curseg	{ &impure };
struct chain	*top	{ &end	};
struct chain	*avail	{ &end	};

/* tricky structure for accessing bytes of a word */
struct	{ char byte[];	};

int	ofile;

main(argc, argv)
char *argv[];
{
	if (argc != 3)
		error("Usage: cvo infile outfile");
	close(0);
	if (open(argv[1], 0) != 0)
		error("Can't find input file");
	if ((ofile = creat(argv[2], 0666)) < 0)
		error("Can't create output file");

	pass1();

	/* If block data program, make another pass through input */
	if (bdata) {
		seek(0, 0, 0);
		close(ofile);
		if ((ofile = creat(argv[2], 0666)) < 0)
			error("can't create output file");
		blkdata();
	}

	putbuf(&pure);
	putbuf(&impure);
	putsymbuf();
	hdr.ssize = symno * 16;
	hdr.mword = 0407;
	seek(ofile, 0, 0);
	write(ofile, &hdr, sizeof hdr);
	if (bdata == 0)
		pass2();
}

pass1()
{
	register c, n;
	register struct combuf *cp;
	int name[2], word;

	readobj();

	for(;;) switch (c = cget()) {

	case 1:		/* End of program */
		return;

	case 2:		/* Reset sequence number */
		break;

	case 3:		/* Block data initialization */
		cp = getcom();
		cp->com_off = 1;	/* flag for second pass */
		bdata++;
		skip(3);
		break;

	case 5:		/* Pure address */
		curseg = &pure;
		goto addr;

	case 6:		/* Impure address */
		curseg = &impure;
addr:
		if ((n = get(3) - curseg->loc) != 0)
			org(n);
		break;

	case 11:	/* Common reference */
		cp = getcom();
		curseg->loc =- 4;
		dchain(-cp->com_sym, get(3));
		curseg->loc =+ 4;
		break;

	case 13:	/* Entry */
		name[0] = get(4);
		name[1] = get(4);
		symbol(name);
		symno++;
		switch (cget()) {
		case 4:
			putsym(SEXT+SABS, get(3));
			break;
		case 5:
			putsym(SEXT+STEXT, get(3));
			break;
		case 6:
			putsym(SEXT+SDATA, get(3) + hdr.tsize);
			break;
		default:
			error("Unknown entry symbol type");
		}
		break;

	case 14:	/* Common definition */
		cp = putcom();
		symno++;
		cp->com_len = n = get(3);
		putsym(SEXT, n);
		break;

	case 15:	/* Label */
		skip(8);
		break;

	case 16:	/* 3 bytes abs, 3 bytes pure */
		put(get(2), RABS);
	case 9:		/* 4 bytes pure */
		word = n = get(4);
		dchain(&pure, n&03777777);
		if (c==9)
			word.byte[0] = '\0';
		put(word>>16, RTEXT+RHI);
		put(n&0177777, RTEXT);
		break;

	case 17:	/* 3 bytes abs, 3 bytes impure */
		put(get(2), RABS);
	case 10:	/* 4 bytes impure */
		word = n = get(4);
		dchain(&impure, n&03777777);
		n = word.byte[0];
		word =+ hdr.tsize;
		word.byte[0] = c == 10 ? '\0' : n;
		put((n=word)>>16, RDATA+RHI);
		put(n&0177777, RDATA);
		break;

	case 18:	/* Entry address */
	case 19:	/* Chain definition address */
		skip(4);
		break;

	case 21:	/* 2 bytes abs, 2 bytes pure */
		put(get(2), RABS);
	case 7:		/* 2 bytes pure */
		put(get(2), RTEXT);
		break;

	case 22:	/* 2 bytes abs, 2 bytes impure */
		put(get(2), RABS);
	case 8:		/* 2 bytes impure */
		put(get(2) + hdr.tsize, RDATA);
		break;

	case 12:	/* EXTRN */
		name[0] = get(4);
		name[1] = get(4);
		symbol(name);
		putsym(SEXT, 0);
		switch (cget()) {
		case 5:
			mkchain(&pure, get(3));
			break;
		case 6:
			mkchain(&impure, get(3));
			break;
		default:
			error("Unknown extrn address type");
		}
		symno++;
		break;

	case 24:	/* Length of impure & pure segments */
		n = get(3);
		reset(n, get(3));
		break;

	case 25:	/* Perform fullword chain */
	case 27:	/* No-op	*/
		break;

	/*
	 * New code (undocumented!) produced by CAL-R05
	 *	- code 31 apparently means 'one byte absolute'
	 *	  It will be handled correctly only if followed immediately
	 *	  by an 'org' or another 1-byte code
	 */
	case 31:	/* 1 byte absolute? */
		put1byte(get(1));
		break;

	default:	/* Absolute data */
		if ((c =- 31) <= 0 || c > 60)
			error("Unknown control item");
		if (bdata)
			skip(c*2);
		else while (c--)
			put(get(2), RABS);

	}
}

/*
 * Define a common block and write its name to the symbol table
 * (returns pointer to common table entry)
 */
putcom()
{
	int name[2];
	register struct combuf *cp;

	if ((cp = lastcom) >= &combuf[NCOM])
		error("Too many common blocks");
	cp->com_name[0] = get(4);
	cp->com_name[1] = get(4);
	cp->com_sym = symno;
	symbol(cp->com_name);
	lastcom = ++cp;
	return(cp - 1);
}

/*
 * Look up a common block name & return a pointer to its common table entry
 */
getcom()
{
	int name[2];
	register struct combuf *cp;

	name[0] = get(4);
	name[1] = get(4);
	for (cp = combuf; cp < lastcom; cp++)
		if (cp->com_name[0] == name[0] && cp->com_name[1] == name[1])
			return(cp);
	error("Undefined common block reference");
}

/*
 * Link an entry into chain of possible external references
 * (sorted by location).
 */
dchain(seg, loc)
struct segment *seg;
{
	register struct chain *ch, *lastch, *av;

	if ((seg > 0) && loc >= seg->maxloc)		/* ignore forward refs */
		return;
	if ((av = avail) >= top)
		brk(top =+ 64);
	lastch = &curseg->first;
	for (ch = curseg->first; ch; ch = ch->next) {
		if (ch->defloc > curseg->loc)
			break;
		lastch = ch;
	}
	av->next = ch;
	lastch->next = av;
	av->defloc = curseg->loc;
	av->refseg = seg;
	av->refloc = loc;
	avail = ++av;
}

/*
 * Make chain of external references
 */
mkchain(seg, loc)
struct segment *seg;
{
	register struct chain *ch;

	for (ch = seg->first; ch;) {
		if (ch->defloc > loc)
			break;
		if (ch->defloc == loc && (ch->refseg > 0)) {
			seg = ch->refseg;
			loc = ch->refloc;
			ch->refseg = -symno;
			ch->refloc = 0;
			ch = seg->first;
			continue;
		}
		ch = ch->next;
	}
	seg->loc = loc;
	curseg = seg;
	dchain(-symno, 0);
}

pass2()
{
	register n;

	n = sizeof hdr;
	fixup(&pure, n, 0);
	fixup(&impure, n =+ hdr.tsize, 0);
	fixup(&pure, n =+ hdr.dsize, 1);
	fixup(&impure, n =+ hdr.tsize, 1);
}

fixup(seg, off, flag)
struct segment *seg;
{
	register struct chain *ch;
	static zero, s;

	for (ch = seg->first; ch; ch = ch->next)
		if (ch->refseg <= 0) {
			if (flag) {
				seek(ofile, off + ch->defloc, 0);
				s = (-ch->refseg)<<20 | REXT+RHI<<16
					| (-ch->refseg)<<4 | REXT;
				write(ofile, &s, sizeof s);
			} else {
				seek(ofile, off + ch->defloc + 1, 0);
				write(ofile, &ch->refloc.byte[1], 3);
			}
		}
}

/*
 * Read next record from object file
 */
char inbuff[126];
char *inp;

readobj()
{
	if (read(0, inbuff, 126) != 126)
		error("Input file i/o error");
	inp = &inbuff[4];
}

/*
 * Get next command code from object file
 */
cget()
{
	register n;

	if ((n = get(1)) == 0) {
		readobj();
		n = get(1);
	}
	return(n);
}

/*
 * Get next n bytes (1 - 4) from object file
 */
get(n)
{
	register i, w;

	w = *inp++;
	for (i=1; i<n; i++) {
		w =<< 8; w =| *inp++;
	}
	return(w);
}

/*
 * Skip next n bytes in object file
 */
skip(n)
{
	inp =+ n;
}

/*
 * Print error message and exit
 */
error(s)
char *s;
{
	char *p;

	for (p=s; *p; p++)
		;
	write(1, s, p-s);
	write(1, "\n", 1);
	exit(1);
}

/*
 * Initialize offsets & buffers
 */
reset(dsize, tsize)
{
	register n;

	hdr.dsize = dsize;
	hdr.tsize = tsize;

	pure.offset[0] = n = sizeof hdr;
	impure.offset[0] = n =+ tsize;
	pure.offset[1] = n =+ dsize;
	impure.offset[1] = n =+ tsize;
	impure.maxloc = impure.loc = pure.nchar = impure.nchar = 0;
	curseg = &impure;

	symoff = n =+ dsize;
	nsym = 0;
}

/*
 * Put a halfword of code & corresponding relocation bits to output
 */
put(code, rel)
{
	register struct segment *seg;
	register i;

	seg = curseg;
	if (seg->nchar >= 512)
		putbuf(seg);
	i = seg->nchar;

	seg->obuf[0][i] = code>>8;
	seg->obuf[0][i+1] = code&0377;
	seg->obuf[1][i] = rel>>8;
	seg->obuf[1][i+1] = rel&0377;

	seg->nchar = i+2;
	seg->loc =+ 2;
	if (seg->loc > seg->maxloc)
		seg->maxloc = seg->loc;
}

/*
 * Put out 1 absolute byte
 *	(This had better be followed by an org)
 */
put1byte(code)
{
	register struct segment *seg;
	register i;

	seg = curseg;
	if (seg->nchar >= 512)
		putbuf(seg);
	i = seg->nchar;

	seg->obuf[0][i] = code;
	seg->obuf[1][i] = 0;

	seg->nchar = ++i;
	seg->loc++;
	if (seg->loc > seg->maxloc)
		seg->maxloc = seg->loc;
}

/*
 * Adjust location in current segment by offset of (off)
 */
org(off)
{
	register struct segment *seg;
	register n;

	seg = curseg;
	if ((n = off) > 0 && seg->loc >= seg->maxloc && n < 512) {
		while (n--)
			put1byte(0);
		return;
	}
	putbuf(seg);
	seg->offset[0] =+ n;
	seg->offset[1] =+ n;
	seg->loc =+ n;
	if (seg->loc > seg->maxloc)
		seg->maxloc = seg->loc;
}

/*
 * Flush output buffers
 */
putbuf(seg)
{
	register n, i;

	if ((n = seg->nchar) <= 0)
		return;
	for (i=0; i<=1; i++) {
		seek(ofile, seg->offset[i], 0);
		write(ofile, seg->obuf[i], n);
		seg->offset[i] =+ n;
	}
	seg->nchar = 0;
}

/*
 * Copy a symbol name to the output file
 */
symbol(name)
char *name;
{
	int sym[2];
	register c, i;
	register char *p, *q;

	p = sym;	q = name;
	for (i=8; i>0; i--) {
		if ((c = *q++) == ' ')
			c = '\0';
		*p++ = c;
	}
	putsym(sym[0], sym[1]);
}

/*
 * Write two words to the symbol table
 */
putsym(s1, s2)
{
	if (nsym >= 128)
		putsymbuf();
	symbuf[nsym++] = s1;
	symbuf[nsym++] = s2;
}

/*
 * Flush the symbol table buffer
 */
putsymbuf()
{
	if (nsym <= 0)
		return;
	seek(ofile, symoff, 0);
	write(ofile, symbuf, nsym*4);
	symoff =+ nsym*4;
	nsym = 0;
}

/*
 * Second pass for block data program --
 *	turn initialized common blocks to data
 */
blkdata()
{
	register struct combuf *cp;
	register n, c;

	/* find total length of commons */
	n = 0;
	for (cp = combuf; cp < lastcom; cp++)
		if (cp->com_off) {
			cp->com_off = n;
			n =+ cp->com_len;
			cp->com_len = 0;
		}

	/* reset buffers & re-process input */
	reset(n, 0);

	readobj();
	for (;;) switch (c = cget()) {

	case 1:		/* End of program */
		return;

	case 2:		/* Reset sequence number */
		break;

	case 3:		/* block data initialization */
		cp = getcom();
		org(cp->com_off + get(3) - curseg->loc);
		break;

	case 14:	/* Common definition */
		cp = getcom();
		skip(3);
		symbol(cp->com_name);
		if (cp->com_len)
			putsym(SEXT, cp->com_len);
		else
			putsym(SEXT+SDATA, cp->com_off);
		break;

	case 24:	/* Length of impure & pure segments */
		skip(6);
		break;

	default:
		if ((c =- 31) <= 0 || c > 60)
			error("Illegal control item in block data program");
		while (c--)
			put(get(2), RABS);
	}
}
