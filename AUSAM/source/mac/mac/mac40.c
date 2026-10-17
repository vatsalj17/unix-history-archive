#include	"mac.h"

extern	int	prdc1();
extern	int	prdc2();
extern	int	prdc3();
extern	int	prdc4();
extern	int	prnull();
extern	int	prequ();
extern	int	prglobal();
extern	int	pralign();
extern	int	prorg();
extern	int	prseg();
extern	int	prds();
extern	int	prend();
extern	int	prdcs();
extern	int	prstruc();
extern	int	prends();

extern	int	pedc1();
extern	int	pedc2();
extern	int	pedc3();
extern	int	pedc4();
extern	int	pealign();
extern	int	peorg();
extern	int	peseg();
extern	int	peds();
extern	int	peend();
extern	int	petitle();
extern	int	penull();
extern	int	pedcs();



int	nline		0;		/* nth input line */
int	lline		0;		/* nth listing line */
int	lpage		0;		/* page number */
int	nsyms		0;		/* # of symbols */
int	errcount	0;		/* # errors */
char	flags[26];			/* start up options */
int	afd;				/* a.out fd */

char	*p;				/* char line pointer */
char	buf[MAXBUF];			/* line buffer */
char	clabel[64];			/* identifiers & strings */
char	ctitle[32];			/* title buffer */
char	*literals;			/* literal labels */

struct	fd	*memory;		/* start of brk'd core & fmt tab */
struct	st	*symtab;		/* pointer to symbol table */
char		*opcode;		/* pointer to opcode table */
struct	tbl	*parse;			/* start of parser table */
char		*endcore;		/* end of core pointer */
char		*coreptr;		/* pointer to next free byte */
int		*code;			/* pointer to assembled code */

int	sym;				/* next symbol */
int	mem;				/* member of symbol */
int	oprstac[16];			/* general opr. stack */
int	relstac[16];			/* relocatability word */
int	*cp;				/* pointer to object */
int	nstruc	0;			/* structure offset counter */

char	(*fn)();

int	eof;				/* eof indicator flag */
int	unit;				/* io unit */
int	lcntr;				/* curr. locn counter */
int	length;				/* length of cur. instr. */

int	reloc;				/* current reloc factor */
int	mreloc;				/* internal reloc word */

char	*plabel[ ]	{		/* pseudo op labels */
	0,
	0,
	0,
	0,
	"equ",
	"global",
	"align",
	"org",
	"seg",
	"ds",
	"end",
	"title",
	"eject",
	"dc",
	"struc",
	"ends",
	0
	};

int	pradr[ ]	{
	&prdc1,
	&prdc2,
	&prdc3,
	&prdc4,
	&prequ,
	&prglobal,
	&pralign,
	&prorg,
	&prseg,
	&prds,
	&prend,
	&prnull,		/* title */
	&prnull,		/* eject */
	&prdcs,			/* special dc */
	&prstruc,
	&prends,
	0
	};

int	peadr[ ]	{
	&pedc1,
	&pedc2,
	&pedc3,
	&pedc4,
	&penull,		/* equ */
	&penull,		/* global */
	&pealign,
	&peorg,
	&peseg,
	&peds,
	&peend,
	&petitle,
	&penull,		/* eject */
	&pedcs,
	&penull,		/* struc 14 */
	&penull,		/* ends */
	0
	};

char	hextab[ ]	"0123456789abcdef";
char	octtab[ ]	"01234567";
char	dectab[ ]	"0123456789";
char	alptab[ ]	"abcdefghijklmnopqrstuvwxyz._@";
char	oprtab[ ]	"+-*/%><~&|";

struct	st	*fchar[sizeof (alptab)];	/* first char indexes */
struct	ht	head;				/* header record */

struct	lt	locn[LCOUNT];
struct	bt	ibuf;
struct	it	intercode;


#ifdef		PDP11

int	bitmask[ ]	{

	0x0000,	0x0001,	0x0003,	0x0007,	0x000f,
		0x001f,	0x003f,	0x007f,	0x00ff,
		0x01ff,	0x03ff,	0x07ff,	0x0fff,
		0x1fff,	0x3fff,	0x7fff,	0xffff

	};

#endif


#ifdef		INTERDATA

int	bitmask[ ]	{

	0x00000000,  0x00000001,  0x00000003,  0x00000007,  0x0000000f,
		     0x0000001f,  0x0000003f,  0x0000007f,  0x000000ff,
		     0x000001ff,  0x000003ff,  0x000007ff,  0x00000fff,
		     0x00001fff,  0x00003fff,  0x00007fff,  0x0000ffff,
		     0x0001ffff,  0x0003ffff,  0x0007ffff,  0x000fffff,
		     0x001fffff,  0x003fffff,  0x007fffff,  0x00ffffff,
		     0x01ffffff,  0x03ffffff,  0x07ffffff,  0x0fffffff,
		     0x1fffffff,  0x3fffffff,  0x7fffffff,  0xffffffff

	};

#endif
