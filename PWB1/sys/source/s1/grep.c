#
/*
 * grep -- print lines matching (or not matching) a pattern
 *
 *	status returns:
 *		0 - ok, and some matches
 *		1 - ok, but no matches
 *		2 - some error
 */

#define	CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	NCCL	8
#define	CDOL	10
#define	CEOF	11
#define	CKET	12
#define	CBACK	18

#define	STAR	01

#define	LBSIZE	512
#define	ESIZE	256
#define	NBRA	9

char	ibuf[512];
char	expbuf[ESIZE];
long	lnum;
char	linebuf[LBSIZE+1];
int	bflag;
int	nflag;
int	cflag;
int	sflag;
int	vflag;
int	nfile;
int	circf;
int	blkno;
long	tln;
int	nsucc;
char	*braslist[NBRA];
char	*braelist[NBRA];
char	bittab[] {
		1,
		2,
		4,
		8,
		16,
		32,
		64,
		128
	};

int	fmon;	/* monitor file */
char	*smon;	/* monitor pointer */
char	nl	'\n';	/* monitor */;

main(argc, argv)
char **argv;
{
	extern fout;

	fout = dup(1);
	flush();
	while (--argc > 0 && (++argv)[0][0]=='-')
		switch (argv[0][1]) {

		case 'v':
			vflag++;
			continue;

		case 'b':
			bflag++;
			continue;

		case 'c':
			cflag++;
			continue;

		case 'n':
			nflag++;
			continue;
		case 's':
			sflag++;
			continue;

		default:
			printf2("Unknown flag %s\n", *argv);
			continue;
		}
	if (argc<=0)
		exit(2);
/* temporary monitoring */
/*
	if ((fmon = open("/usr/bwk/grep.pats", 2)) >= 0) {
		seek(fmon, 0, 2);
		for (smon = *argv; *smon; smon++)
			write(fmon, smon, 1);
		write(fmon, &nl, 1);
		close(fmon);
	}
*/
/* end of monitor */
	compile(*argv);
	nfile = --argc;
	if (argc<=0)
		execute(0);
	else while (--argc >= 0) {
		argv++;
		execute(*argv);
	}
	flush();
	exit(nsucc == 0);
}

compile(astr)
char *astr;
{
	register c;
	register char *ep, *sp;
	char *lastep;
	int cclcnt;
	char	bracket[NBRA], *bracketp;
	int	closed;
	char	numbra;
	char	neg;

	ep = expbuf;
	sp = astr;
	lastep = 0;
	bracketp = bracket;
	closed = numbra = 0;
	if (*sp == '^') {
		circf++;
		sp++;
	}
	for (;;) {
		if (ep >= &expbuf[ESIZE])
			goto cerror;
		if ((c = *sp++) != '*')
			lastep = ep;
		switch (c) {

		case '\0':
			*ep++ = CEOF;
			return;

		case '.':
			*ep++ = CDOT;
			continue;

		case '*':
			if (lastep==0)
				goto defchar;
			*lastep =| STAR;
			continue;

		case '$':
			if (*sp != '\0')
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			if(&ep[17] >= &expbuf[ESIZE])
				goto cerror;

			*ep++ = CCL;

			neg = 0;
			if((c = *sp++) == '^') {
				neg = 1;
				c = *sp++;
			}

			do {
				if(c == '\0')	goto cerror;

				ep[c >> 3] =| bittab[c & 07];
			} while((c = *sp++) != ']');
			if(neg) {
				for(cclcnt = 0; cclcnt < 16; cclcnt++)
					ep[cclcnt] =^ -1;
				ep[0] =& 0376;
			}

			ep =+ 16;

			continue;

		case '\\':
			if((c = *sp++) == '(') {
				if(numbra >= NBRA) {
					goto cerror;
				}
				*bracketp++ = numbra;
				*ep++ = CBRA;
				*ep++ = numbra++;
				continue;
			}
			if(c == ')') {
				if(bracketp <= bracket) {
					goto cerror;
				}
				*ep++ = CKET;
				*ep++ = *--bracketp;
				closed++;
				continue;
			}

			if(c >= '1' && c <= '9') {
				if((c =- '1') >= closed)
					goto cerror;
				*ep++ = CBACK;
				*ep++ = c;
				continue;
			}

		defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
    cerror:
	printf2("RE error\n");
}

execute(file)
{
	register char *p1, *p2;
	register c;
	int f;
	char *ebp, *cbp;

	if (file) {
		if ((f = open(file, 0)) < 0) {
			if( !sflag )
				printf2("Can't open %s\n", file);
		}
	} else
		f = 0;
	ebp = ibuf;
	cbp = ibuf;
	lnum = 0;
	tln = 0;
	blkno = -1;
	for (;;) {
		lnum++;
		if((lnum&0377) == 0)
			flush();
		p1 = linebuf;
		p2 = cbp;
		for (;;) {
			if (p2 >= ebp) {
				if ((c = read(f, ibuf, 512)) <= 0) {
					close(f);
					if (cflag) {
						if (nfile > 1)
							printf("%s:", file);
						printf("%ld\n", tln);
					}
					return;
				}
				blkno++;
				p2 = ibuf;
				ebp = ibuf+c;
			}
			if ((c = *p2++) == '\n')
				break;
			if(c)
			if (p1 < &linebuf[LBSIZE-1])
				*p1++ = c;
		}
		*p1++ = 0;
		cbp = p2;
		p1 = linebuf;
		p2 = expbuf;
		if (circf) {
			if (advance(p1, p2))
				goto found;
			goto nfound;
		}
		/* fast check for first character */
		if (*p2==CCHR) {
			c = p2[1];
			do {
				if (*p1!=c)
					continue;
				if (advance(p1, p2))
					goto found;
			} while (*p1++);
			goto nfound;
		}
		/* regular algorithm */
		do {
			if (advance(p1, p2))
				goto found;
		} while (*p1++);
	nfound:
		if (vflag)
			succeed(file);
		continue;
	found:
		if (vflag==0)
			succeed(file);
	}
}

advance(alp, aep)
{
	register char *lp, *ep, *curlp;
	char *nextep, c;
	char	*bbeg;
	int	ct;

	lp = alp;
	ep = aep;
	for (;;) switch (*ep++) {

	case CCHR:
		if (*ep++ == *lp++)
			continue;
		return(0);

	case CDOT:
		if (*lp++)
			continue;
		return(0);

	case CDOL:
		if (*lp==0)
			continue;
		return(0);

	case CEOF:
		return(1);

	case CCL:
		c = *lp++ & 0177;
		if(ep[c>>3] & bittab[c & 07]) {
			ep =+ 16;
			continue;
		}
		return(0);
	case CBRA:
		braslist[*ep++] = lp;
		continue;

	case CKET:
		braelist[*ep++] = lp;
		continue;

	case CBACK:
		bbeg = braslist[*ep];
		ct = braelist[*ep++] - bbeg;

		if(ecmp(bbeg, lp, ct)) {
			lp =+ ct;
			continue;
		}
		return(0);

	case CBACK|STAR:
		bbeg = braslist[*ep];
		ct = braelist[*ep++] - bbeg;
		curlp = lp;
		while(ecmp(bbeg, lp, ct))
			lp =+ ct;

		while(lp >= curlp) {
			if(advance(lp, ep))	return(1);
			lp =- ct;
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
		curlp = lp;
		do {
			c = *lp++ & 0177;
		} while(ep[c>>3] & bittab[c & 07]);
		ep =+ 16;
		goto star;

	star:
		if(--lp == curlp) {
			continue;
		}

		if(*ep == CCHR) {
			c = ep[1];
			do {
				if(*lp != c)
					continue;
				if(advance(lp, ep))
					return(1);
			} while(lp-- > curlp);
			return(0);
		}

		do {
			if (advance(lp, ep))
				return(1);
		} while (lp-- > curlp);
		return(0);

	default:
		printf2("RE botch\n");
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
		if (*set++ == c)
			return(af);
	return(!af);
}

printf2(s, a)
{
	extern fout;
	flush();
	fout = 2;
	printf(s, a);
	flush();
	exit(2);
}

succeed(f)
{
	nsucc = 1;
	if (cflag) {
		tln++;
		return;
	}
	if (nfile > 1)
		printf("%s:", f);
	if (bflag)
		printf("%u:", blkno);
	if (nflag)
		printf("%ld:", lnum);
	printf("%s\n", linebuf);
}

ecmp(a, b, count)
char	*a, *b;
{
	while(count--)
		if(*a++ != *b++)	return(0);
	return(1);
}

