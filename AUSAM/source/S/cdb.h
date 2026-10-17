#
/*

	C debugger header

*/


#define	DSP	0
#define	ISP	1
#define	NBKP	10
#define	SYMSIZ	12*400
#define	BADJST	01
#define	NGRPS	10
#define QUIT	3

int	fcore;
int	errno;
int	fsym;
int	symoff;
char	*lp;
int	errflg;
int	symlen;
int	symct;
int	symcor;
int	symbuf[SYMSIZ];
int	*symptr;
struct {
	int	loc;
	int	ins;
	int	count;
	int	flag;
} bkptl[NBKP];
struct	grpmem	{
	char	*adr;
	int	mode;
	int	offset;
	char	*fcnnam;
	char	*varnam;
	struct	grpmem	*link;
} membr;
struct	{
	struct	grpmem	*memptr;
} group[NGRPS];
int	lastbp;
char	symbol[8];
int	symflg;
int	symval;
char	tsym[8];
char	fsymbol[10];
char	ssymbol[8];
int	ssymflg;
int	ssymval;
int	signo;
char	line[128];
int	regbuf[512];
char	*rtsize;
int	loccsv;
int	locsr5;
#define	RUSER	1
#define	RIUSER	2
#define	WUSER	4
#define	WIUSER	5
#define	RUREGS	3
#define	WUREGS	6
#define	SETTRC	0
#define	CONTIN	7
#define	EXIT	8

#define	ps	-1
#define	pc	-2
#define	sp	-6
#define	r5	-9
#define	r4	-10
#define	r3	-11
#define	r2	-12
#define	r1	-5
#define	r0	-3

struct sfregs {
	int	junk[2];
	int	fpsr;
	float	sfr[6];
};

struct lfregs {
	int	junk[2];
	int	fpsr;
	double	lfr[6];
};

int	dot;
int	tdot;
int	modifier;
int	callist[50];
int	entpt[50];
int	callev;
int	pid;
int	adrflg;
int	idsep;
int	pokeval;
int	autof;
int	ssflg;
int	recur;
int	litflg;
int	is_auto;
char	*fcnstr;

