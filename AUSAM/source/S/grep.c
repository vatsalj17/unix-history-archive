/*
 * grep -- print lines matching (or not matching) a pattern
 *
 */ 

/*
 *	Changed to prevent grep exiting when it can't open a file
 *	Also told about longs and other stuff to do with the new 'C'
 *	compiler.
 *						B.Palmer Apr 79
 */

/*
 *	Added '-' option to classes, as per em ([a-d] == [abcd]), escapes
 *	are also handled. ([ab\-z] == [abz] + [-])
 *	Fixed so grep would not bomb on '*' as the pattern. 
 *
 *						Michael Rourke May 79
*/

/*
 *	Exit status returned (as per CSU):
 *
 *		0 : matches, no errors
 *		1 : no matches
 *		2 : file open error
 *		3 : RE syntax error
 *		4 : Incorrect usage
 *		5 : Program error (`RE botch')
 */

#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	NCCL	8
#define	CDOL	10
#define	CEOF	11

#define	STAR	01

#define	LBSIZE	256
#define	ESIZE	256

char	ibuf[512];
char	expbuf[ESIZE];
char	linebuf[LBSIZE+1];
int	bflag;
int	nflag;
int	cflag;
int	vflag;
int	nfile;		/* number of files */
int	circf;		/* tie expression to start of line */
int	blkno;		/* block nr - `-b' option */
int	matches;	/* at least one match */
int	errs;		/* file open error */
unsigned lnum;		/* line number */
unsigned tln;		/* number of matches - `-c' option */

main(argc, argv)
char **argv;
{
	extern fout;

	fout = dup(1);
	while (--argc > 0 && (++argv)[0][0] == '-')
		switch(argv[0][1])
		{

		case 'v':		/* find lines NOT matching */
			vflag++;
			continue;

		case 'b':		/* print out block nrs - god knows why */
			bflag++;
			continue;

		case 'c':		/* count of lines only */
			cflag++;
			continue;

		case 'n':		/* print out line number too */
			nflag++;
			continue;

		default:
			goto usage;
		}
	if (argc <= 0)
	{
    usage:	prints(2, "Usage: grep [-b][-c][-n][-v] reg-exp [files]\n");
		exit(4);
	}
	compile(*argv);
	nfile = --argc;
	if (argc <= 0)
	{
		execute(0);
		flush();
	}
	else
		while (--argc >= 0)
		{
			argv++;
			execute(*argv);
			flush();
		}
	exit(errs ? 2 : matches ? 0 : 1);
}

compile(astr)
char *astr;
{
	register c;
	register char *ep, *sp;
	char *lastep 0;
	int cclcnt;

	ep = expbuf;
	sp = astr;
	if (*sp == '^')
	{
		circf++;
		sp++;
	}
	for (;;)
	{
		if (ep >= &expbuf[ESIZE])
			goto cerror;
		if ((c = *sp++) != '*')
			lastep = ep;
		switch(c)
		{

		case '\0':
			*ep++ = CEOF;
			return;

		case '.':
			*ep++ = CDOT;
			continue;

		case '*':
			if (lastep == 0)
			{
				lastep = ep;
				goto defchar;
			}
			*lastep =| STAR;
			continue;

		case '$':
			if (*sp != '\0')
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c = *sp++) == '^')
			{
				c = *sp++;
				ep[-2] = NCCL;
			}
			do
			{
				if (c == '-' && cclcnt > 1)
				{
					if ((c = *sp++) == '\0' || c <= ep[-1] || ep >= &expbuf[ESIZE - 2]) goto cerror;
					*ep++ = '\0';
					*ep++ = c;
					cclcnt =+ 2;
					continue;
				}
				else if (c == '\\') c = *sp++;
				*ep++ = c;
				cclcnt++;
				if (c == '\0' || ep >= &expbuf[ESIZE])
					goto cerror;
			}
			while ((c = *sp++) != ']');
			lastep[1] = cclcnt;
			continue;

		case '\\':
			if ((c = *sp++) == '\0')
				goto cerror;
		defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
cerror:
	prints(2, "Regular expression syntax error\n");
	exit(3);
}

execute(file)
{
	register char *p1, *p2;
	register c;
	int f;
	char *ebp, *cbp;

	if (file)
	{
		if ((f = open(file, 0)) < 0)
		{
			flush();
			perror(file);
			errs = 1;
			return;
		}
	}
	else
		f = 0;
	ebp = ibuf;
	cbp = ibuf;
	lnum = 0;
	tln = 0;
	blkno = -1;
	for (;;)
	{
		lnum++;
		p1 = linebuf;
		p2 = cbp;
		for (;;)
		{
			if (p2 >= ebp)
			{
				if ((c = read(f, ibuf, 512)) <= 0)
				{
					close(f);
					if (cflag)
					{
						if (nfile > 1)
							printf("%s:", file);
						printf("%u\n",tln);
					}
					return;
				}
				blkno++;
				p2 = ibuf;
				ebp = ibuf+c;
			}
			if ((c = *p2++) == '\n')
				break;
			if (c)
				if (p1 < &linebuf[LBSIZE-1])
					*p1++ = c;
		}
		*p1++ = 0;
		cbp = p2;
		p1 = linebuf;
		p2 = expbuf;
		if (circf)
		{
			if (advance(p1, p2))
				goto found;
			goto nfound;
		}
		/* fast check for first character */ 
		if (*p2 == CCHR)
		{
			c = p2[1];
			do
			{
				if (*p1 != c)
					continue;
				if (advance(p1, p2))
					goto found;
			}
			while (*p1++);
			goto nfound;
		}
		/* regular algorithm */ 
		do
		{
			if (advance(p1, p2))
				goto found;
		}
		while (*p1++);
	nfound:
		if (vflag)
			succeed(file);
		continue;
	found:
		if (vflag == 0)
			succeed(file);
	}
}

advance(alp, aep)
{
	register char *lp, *ep, *curlp;
	char *nextep;

	lp = alp;
	ep = aep;
	for (;;)
		switch(*ep++)
		{

		case CCHR:
			if (*ep++ == *lp++)
				continue;
			return(0);

		case CDOT:
			if (*lp++)
				continue;
			return(0);

		case CDOL:
			if (*lp == 0)
				continue;
			return(0);

		case CEOF:
			return(1);

		case CCL:
			if (cclass(ep, *lp++, 1))
			{
				ep =+ *ep;
				continue;
			}
			return(0);

		case NCCL:
			if (cclass(ep, *lp++, 0))
			{
				ep =+ *ep;
				continue;
			}
			return(0);

		case CDOT|STAR:
			curlp = lp;
			while (*lp++);
			goto star;

		case CCHR|STAR:
			curlp = lp;
			while (*lp++ == *ep);
			ep++;
			goto star;

		case CCL|STAR:
		case NCCL|STAR:
			curlp = lp;
			while (cclass(ep, *lp++, ep[-1] == (CCL|STAR)));
			ep =+ *ep;
			goto star;

		star:
			do
			{
				lp--;
				if (advance(lp, ep))
					return(1);
			}
			while (lp > curlp);
			return(0);

		default:
			prints(2, "RE botch - seek help!\n");
			exit(5);
		}
}

cclass(aset, ac, af)
{
	register char *set, c;
	register n;

	set = aset;
	if ((c = ac) == 0)
		return(0);
	n = *set++;
	while (--n)
		if (*set == '\0')
		{
			if (c > set[-1] && c <= set[1]) return af;
			set =+ 2;
			n--;
		}
		else if (*set++ == c) return af;
	return(!af);
}

succeed(f)
{
	matches = 1;
	if (cflag)
	{
		tln++;
		return;
	}
	if (nfile > 1)
		printf("%s:", f);
	if (bflag)
		printf("%u:", blkno);
	if (nflag)
		printf("%u:", lnum);
	printf("%s\n", linebuf);
}
