int	dbg;
int	lp[40];	/* stack for things like piles and matrices */
int	ct;	/* pointer to lp */
int	used[100];	/* available registers */
int	ps;	/* dflt init pt size */
int	ft;	/* dflt font */
int	first;
extern	int	fout, fin;
int	ifile;
int	linect;	/* line number in file */
int	eqline;	/* line where eqn started */
int	svargc;
char	**svargv;
int	eht[100];
int	ebase[100];
int	tht[30];
int	tbase[30];
int	ptr;
struct	{
	char *nptr;
	char *sptr;
} deftab[100];
int	eqnreg, eqnht;
int	lefteq, righteq;
int	lastchar;	/* last character read by lex */
