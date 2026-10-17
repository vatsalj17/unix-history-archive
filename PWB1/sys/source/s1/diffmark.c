#
/*	diffmark 1.4 of 3/15/77 : make margin characters from diff output
 *
 *	diffmark [-abstring -aestring -cbstring -cestring -dbstring -destring
 *	markedfile]
 */
static char SCCSID[]	"@(#)diffmark.c	1.4";
char	*code[6] {	/* code names for mark strings, correspond to mark */
	"ab",		/* append begin */
	"ae",		/* append end */
	"cb",		/* change begin */
	"ce",		/* change end */
	"db",		/* delete begin */
	"de"		/* delete end */
};

char *mark[6] {		/* default marks, corresponding to code array */
	".mc |",	/* ab */
	".mc",		/* ae */
	".mc |",	/* cb */
	".mc",		/* ce */
	".mc *",	/* db */
	".mc"		/* de */
};
char	*wfile	"";	/* ptr to name of file to be written (if nonnull) */
char	dot[]	".";	/* for terminating delete converted to change */

#define	INSIZE	512
char	inline[INSIZE];	/* input line buffer */
char	*lp;		/* addr last char before nl; else addr of nl(getline)*/


main(argc,argv)
int argc; char **argv;
{
	extern fin,fout;
	fin = dup(0);		/* buffer input ... */
	fout = dup(1);		/* and output */
	getargs(argc,argv);
	while(getline()) {
		switch (*lp) {
	case 'a':
			copyac(mark[0],mark[1]);
			break;
	case 'c':
			copyac(mark[2],mark[3]);
			break;
	case 'd':
			if (*mark[4] || *mark[5]) {	/* if either mark exists */
				*lp = 'c';	/* change d to c */
				copyin();	/* output modified line */
				putstr(mark[4]);
				putstr(mark[5]);
				putstr(dot);
			}
			else
				copyin();	/* copy unmodified line */
			break;
	default:
			abend("input not from diff");
		}
	}

	if (*wfile) {	/* if nonnull name add w name to output */
		putchar('w');
		putchar(' ');
		putstr(wfile);
	}
	flush();
	exit(0);
}


/*	getargs: scan all arguments (6 codes, 1 file name */
getargs(argc,argv)
int argc;char **argv;
{
	register char *p,*pc;
	register int i;
	argv++;
	while(--argc) {
		p = *argv++;
		if (*p != '-')
			wfile = p;
		else {
			p++;	/* skip over - */
			for(i = 0;i <= 5; i++) {
				pc = code[i];
				if (*p == *pc && p[1] == pc[1]) {
					mark[i] = p+2;
					break;
				}
			}
		}
	}
	return;
}


/*	getline: fill inline with next input line; set up addr of char before
	nl in lp (or nl itself if null line); return 1 if data, 0 on eof */
getline()
{
	register char *ip,c;
	ip = inline;
	while(ip < inline+INSIZE && (*ip++ = c = getchar()))
		if (c == '\n')
			break;
	lp = ip-2;	/* move back to last char before nl */
	if (c == '\n')
		return(1);	/* normal */
	if (lp >= inline+INSIZE)
		abend("line too long");
	return(0);		/* eof */
}


/*	copyac: copy lines in a or c sequence, adding marks as needed */
copyac(begin,end)
char *begin,*end;
{
	copyin();	/* a or c command */
	putstr(begin);
	while(getline()) {
		if (inline[0] == '.' && inline[1] == '\n') {
			putstr(end);
			copyin();
			break;
		}
		else
			copyin();
	}
	return;
}


/*	putstr: output string only if nonnull, add nl to end */
putstr(string)
char *string;
{
	if (*string) {
		while(*string)
			putchar(*string++);
		putchar('\n');
	}
	return;
}


/*	copyin: copy line from inline to output */
copyin()
{
	register char *p;
	p = inline;
	do
		putchar(*p);
	while(*p++ != '\n');
	return;
}


/*	abend: write message, show bad input, exit abnormally */
abend(mesg)
char *mesg;
{
	register char *p;
	register int i;
	while(*mesg)
		write(2,mesg++,1);
	write(2,"\n",1);
	for(p = inline; p <= inline+72;) {
		write(2,p,1);
		if (*p++ == '\n')
			break;
	}
	exit(1);
}
