extern	char	*p;
extern	char	clabel[ ];
extern	char	buf[ ];
extern	char	literals[16][8];

extern	char	hextab[ ];
extern	char	dectab[ ];
extern	char	octtab[ ];
extern	char	alptab[ ];
extern	char	oprtab[ ];

extern	char	(*fn)();
extern	char	*avec[ ];

extern	char	*lvec[ ];

extern	int	tp;
extern	int	nslot;
extern	int	nlit;
extern	int	nfmt;
extern	int	nops;
extern	int	nsym;
extern	int	nline;
extern	int	nerr;
extern	int	sym;
extern	int	mem;
extern	char	mem4[ ];
extern	int	fd;

extern	struct	node	tree[ ];
extern	struct	tbl	s2[ ];
extern	struct	tbl	parse[ ];
extern	struct	fd	format[ ];
extern	struct	od	*opcode;
extern	struct	st	symtab[ ];
extern	struct	ht	head;


extern	struct	tbl	e1[NE1];
extern	struct	tbl	s1[NS1];
extern	struct	tbl	s3[NS3];
