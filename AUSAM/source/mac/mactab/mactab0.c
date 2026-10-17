#include	"../mac/mac.h"
#include	"mactab.h"

extern	plit();
extern	plab();
extern	phed();
extern	pops();
extern	pfmt();
extern	parg();
extern	pend();



char	*p;
char	clabel[64];
char	buf[256];
char	literals[NLIT][8];

char	hextab[ ]	"0123456789abcdef";
char	dectab[ ]	"0123456789";
char	octtab[ ]	"01234567";
char	alptab[ ]	"abcdefghijklmnopqrstuvwxyz@._";
char	oprtab[ ]	"+-*/%><~&|";

char	(*fn)();
char	*avec[ ]	{
	&plit,		/* literals */
	&plab,		/* pre-def labels */
	&phed,		/* header */
	&pops,		/* opcodes */
	&pfmt,		/* formats */
	&parg,		/* instr args */
	&pend,		/* output form code */
	0
	};

char	*lvec[ ]	{
	"literals",
	"labels",
	"header",
	"opcodes",
	"formats",
	"args",
	"end",
	0
	};

int	tp	0;
int	nslot	0;
int	nlit	0;
int	nfmt	0;
int	nops	0;
int	nsym	0;
int	nline	0;
int	nerr	0;
int	sym;
int	mem;
char	mem4[4];
int	fd;

struct	node	tree[NTREE];
struct	tbl	s2[NTABLE];
struct	tbl	parse[NTABLE+NTREE];
struct	fd	format[NFMT];
struct	od	*opcode;
struct	st	symtab[NSYM];			/* 8 pre-defined labels */
struct	ht	head;

/*
 *
 *	Pre-defined sections of the parser.
 *
 *	E1:	expression evaluater
 *	S1:	part of the source line before the args
 *	S3:	part of the line after the args.
 *
 */


struct	tbl	e1[NE1]	{

	{	-OPR,	0,	2,	2	},
	{	-OPR,	1,	2,	OOPR	},
	{	LBL,	NUL,	6,	OLBL	},
	{	CON,	NUL,	6,	OCON	},
	{	-CHR,	'!',	6,	OCHR	},
	{	MCH,	NUL,	NUL,	GOTO	},

	{	OPR,	0,	NUL,	OOPR	},		/* 6 */
	{	MCH,	0,	NUL,	RETN	},

	};

struct	tbl	s1[NS1]	{

	{	LBL,	NUL,	6,	ALBL	},		/* label tag */
	{	EOL,	NUL,	NUL,	OREC	},
	{	EOF,	NUL,	NUL,	ENDP	},
	{	COM,	NUL,	10,	NOOP	},
	{	SPA,	NUL,	12,	NOOP	},
	{	MCH,	NUL,	NUL,	OERR	},

	{	SPA,	NUL,	12,	NOOP	},		/*  6 */
	{	COM,	NUL,	10,	NOOP	},
	{	EOL,	NUL,	NUL,	OREC	},
	{	MCH,	NUL,	NUL,	OERR	},

	{	EOL,	NUL,	NUL,	OREC	},		/* 10 */
	{	MCH,	NUL,	NUL,	OERR	},

	{	LBL,	NUL,	16,	DOPV	},		/* 12 */
	{	EOL,	NUL,	NUL,	OREC	},
	{	COM,	NUL,	10,	NOOP	},
	{	MCH,	NUL,	NUL,	OERR	},

	{	SPA,	NUL,	20,	NOOP	},		/* 16 */
	{	EOL,	NUL,	NUL,	OREC	},
	{	COM,	NUL,	10,	NOOP	},
	{	MCH,	NUL,	NUL,	OERR	},

								/* 20 */
	};

struct	tbl	s3[NS3]	{

	{	SPA,	NUL,	1,	NOOP	},
	{	COM,	NUL,	2,	NOOP	},
	{	EOL,	NUL,	NUL,	OREC	},
	{	MCH,	NUL,	NUL,	OERR	},

							/* 4 */
	};
