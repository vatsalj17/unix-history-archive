# include "stdio.h"
# include "ctype.h"
extern FILE *in;
extern int endpush, labels, sort, bare, keywant;
#ifdef	MFLAG
extern int mflag;
#endif
extern char *smallcaps;
extern char comname;
extern char *keystr;
extern int authrev;
extern int nmlen, dtlen;
extern char *data[], **search;
extern int refnum;
extern char *reftable[];
extern char *rtp, reftext[];
extern char labc[];
extern int sep;
extern char tfile[];
extern char gfile[];
extern char ofile[];
extern char hidenam[];
extern char *Ifile; 
extern char *Index;
extern int Iline;
extern FILE *fo, *ftemp;
union ptr {
	unsigned *a; 
	long *b;
};
extern union ptr master;
extern long *hpt;
extern FILE *fa;
extern FILE *fb;
extern FILE *fc;
extern char *qitem[];
extern int lmaster, reached;
extern int iflong;
extern int nfound;
extern char *sinput;
extern char text[];
extern char tagout[];
extern char *sinput;
extern char *Index;
# define FLAG 003
#ifdef	MFLAG
#define KEYLET	005
#endif
/*
 * limits:
 */
# define NRFTXT 4000		/* chars in tags for all refs (-e,s) */
# define NRFTBL 500		/* total refs (-e,s) */
# define NTFILE 20		/* chars in tempfile name */
# define LLINE 200		/* max input line length */
# define QLEN 300		/* chars in query (imprecise citation) */
#define NQUERY	30		/* number of words in query */
# define TAGLEN 50		/* chars in a tag */
# define NSERCH 20		/* number of indexes to search */
#define LLIST	100		/* length of lists of tag pointers in *.ib */
#define TXTLEN	800		/* number of chars in reference */
#define NFLD	30		/* number of % fields, from reffile or input */
# define NFLAB 4000		/* total chars in label list (-l,m) */
# define NREF	500		/* number of refs for labels */
