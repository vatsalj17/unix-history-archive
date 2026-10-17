# include "stdio.h"
# include "refer.h"
FILE *in = stdin;
int endpush 0;
int labels 0;
int keywant 0;
#ifdef	MFLAG
int mflag 0;
#endif
int sort 0;
int bare 0;
int authrev 0;
char *smallcaps "";
char *keystr "AD";
int nmlen 0, dtlen 0;
char *data[NSERCH];
char **search data;
int refnum 0;
char reftext[NRFTXT];
char *reftable[NREF];
char *rtp reftext;
int sep '\n';
char tfile[NTFILE];
FILE *fo = stdout;
FILE *ftemp = stdout;
char ofile[NTFILE];
char gfile[NTFILE];
char hidenam[NTFILE];
char *Ifile "standard input";
int Iline 0;
char text[TXTLEN];
char tagout[TAGLEN];
char *sinput;
char *Index;
int nfound;
