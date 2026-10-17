extern	int	nline;
extern	int	lline;
extern	int	lpage;
extern	int	nsyms;
extern	int	errcount;
extern	char	flags[ ];
extern	int	afd;

extern	char	*p;				/* char line pointer */
extern	char	buf[ ];				/* line buffer */
extern	char	clabel[ ];			/* identifiers */
extern	char	ctitle[ ];			/* title buffer */
extern	char	*literals;

extern	char	*opcode;			/* -> to op desc */
extern	struct	st	*symtab;		/* -> to symbol tab */
extern	struct	fd	*memory;		/* start of core */
extern	struct	tbl	*parse;			/* parser */
extern	char	*endcore;			/* end of core */
extern	char	*coreptr;			/* cur. free */
extern	int	*code;				/* start of code */

extern	int	sym;				/* next symbol */
extern	int	mem;				/* member of symbol */
extern	int	oprstac[ ];			/* gen. opr. stack */
extern	int	relstac[ ];			/* reloc factor */
extern	int	*cp;				/* pointer to object */
extern	int	nstruc;				/* struc offset counter */

extern	char	(*fn)();

extern	int	lcntr;				/* curr. locn counter */
extern	int	eof;				/* eof flag */
extern	int	length;				/* len of instr */
extern	int	unit;				/* io unit */

extern	int	reloc;
extern	int	mreloc;

extern	char	*plabel[ ];
extern	int	pradr[ ];
extern	int	peadr[ ];

extern	char	hextab[ ];
extern	char	octtab[ ];
extern	char	dectab[ ];
extern	char	alptab[ ];
extern	char	oprtab[ ];

extern	struct	st	*fchar[ ];		/* first char indexes */
extern	struct	ht	head;			/* header record */
extern	struct	lt	locn[];
extern	struct	bt	ibuf;
extern	struct	it	intercode;

extern	int	states[9][10];			/* state table */
extern	int	action[9][10];			/* actions of states */

extern	int	bitmask[ ];
