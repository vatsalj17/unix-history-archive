#include "em.h"



donothing()
{
	char t1, t2;

	t1 = rhsbuf[0];
	t2 = rhsbuf[1];
	rhsbuf[0] = '&';
	rhsbuf[1] = 0;
	dosub();
	rhsbuf[0] = t1;
	rhsbuf[1] = t2;
}

confirmed()
{
#ifdef	BETTER_SWAP
	register char *ptr;
#endif	BETTER_SWAP
	int	ch;
	int	oldglobp;

	if (xflag)
	{
#ifndef	BETTER_SWAP
		putnls(linebuf);
#else
		col = 0;
		for (ptr = genbuf; ptr < ssp; putchar(*ptr++));
		for (ptr = slp; *ptr; putchar(*ptr++));
		putchar('\n');
#endif	BETTER_SWAP
		underline();
		oldglobp = globp;
		globp = 0;
		ch = getchar();
		if (ch != '\n')
		{
			while (getchar() != '\n');
			if (ch != CONFIRM) putnls("? '.' to confirm");
		}
		globp = oldglobp;
		return(ch == CONFIRM);
	}
	return(1);
}

underline()
{
	int	i1, i2;
	register int	i, j;

	i1 = calccol(loc1);
	i2 = calccol(loc2);
	if (!listf && i1>i2) { i = i1; i1 = i2; i2 = i; }
	j = i1 & ~7;
	for (i = 0; i < j; i =+ 8) write(1, "\t", 1);
	for (; i < i1; i++) write(1, " ", 1);
	while (i != i2)
	{
		if (++i >= (LENGTH - 1) && listf) { write(1, "\r", 1); i = 1; }
		write(1, "^", 1);
	}
}

screensplit()
{
	register a;

	a = LENGTH - 2;		/* as our VDUs do funny things in the last column position! */
	col = 0;
	while (a--) putchar('-');
	putchar('\n');
}

				/*
				 * Find actual column number
				 */
calccol(ll)
char	*ll;
{
	register int	i;
	register char	*p;

	i = 0;
#ifndef	BETTER_SWAP
	for (p = linebuf; *p!='\0' && p<ll; p++)
	{
		if (*p=='\t' && !listf) i = (i+8) & ~7;
		else if (*p=='\b' && !listf) i--; else i++;
	}
#else
	for (p = genbuf; p < ssp; p++)
	{
		if (*p=='\t' && !listf) i = (i+8) & ~7;
		else if (*p=='\b' && !listf) i--; else i++;
	}
	for (p = slp; *p!='\0' && p<ll; p++)
	{
		if (*p=='\t' && !listf) i = (i+8) & ~7;
		else if (*p=='\b' && !listf) i--; else i++;
	}
#endif	BETTER_SWAP
	if (listf) i =% (LENGTH - 2);
	return(i);
}

compsub()
{
#ifndef	MISC
	register seof, c;
#else
	register c;
	register char *rp;
	int seof;
#endif
	register char *p;
	int gsubf;

	gsubf = 0;
	if ((seof = getchar()) == '\n') error(20);
	compile(seof);
#ifndef	MISC
	p = rhsbuf;
#else
	rp = rhsbuf;
	p = linebuf;
#endif
	for (;;)
	{
		c = getchar();
#ifdef	MISC
		if (c == '%')
		{
			if (*rp == 0) error(32);
			while (*p++ = *rp++) if (p >= &linebuf[LBSIZE/2]) error(27);
			rp = rhsbuf;
			p--;
			continue;
		}
		if (c==BACKSL)
		{
			if ((c = getchar()) >= '1' && c < NBRA + '1' && c >= nbra + '1') error(32);
			c =| 0200;
		}
		if (c=='\n' && (globp == 0 || *globp == 0)) error(20);
#else
		if (c==BACKSL) c = getchar() | 0200;
		if (c=='\n' && globp == 0) error(20);
#endif
		if (c==seof) break;
		*p++ = c;
#ifndef	MISC
		if (p >= &rhsbuf[LBSIZE/2]) error(27);
#else
		if (p >= &linebuf[LBSIZE/2]) error(27);
#endif
	}
	*p++ = 0;
#ifdef	MISC
	for (p = linebuf; *rp++ = *p++; );
#endif
	if (((peekc = getchar()) | 040) == 'g') { peekc = 0; gsubf =| 1; }
	if (((peekc = getchar()) | 040) == 'n') { peekc = 0; gsubf =| 2; }
	newline();
	return(gsubf);
}

getsub()
{
	register char *p1, *p2;

	p1 = linebuf;
	if ((p2 = linebp) == 0) return(EOF);
	while (*p1++ = *p2++);
	linebp = 0;
	return(0);
}

dosub()
{
	register char *lp, *sp, *rp;
	int c;

#ifndef	BETTER_SWAP
	lp = linebuf;
	sp = genbuf;
#else
	lp = slp;
	sp = ssp;
#endif	BETTER_SWAP
	rp = rhsbuf;
	while (lp < loc1) *sp++ = *lp++;
	while (c = *rp++)
	{
		if (c=='&') { sp = place(sp, loc1, loc2); continue; }
		if (c<0 && (c =& 0177) >='1' && c < NBRA+'1')
		{
			sp = place(sp, braslist[c-'1'], braelist[c-'1']);
			continue;
		}
		*sp++ = c;
		if (sp >= &genbuf[LBSIZE]) error(27);
	}
#ifndef	BETTER_SWAP
	lp = loc2;
	loc2 = sp + linebuf - genbuf;
	while (*sp++ = *lp++) if (sp >= &genbuf[LBSIZE]) error(27);
	lp = linebuf;
	sp = genbuf;
	while (*lp++ = *sp++);
#else
	slp = loc2;
	ssp = sp;
#endif	BETTER_SWAP
}

place(asp, al1, al2)
{
	register char *sp, *l1, *l2;

	sp = asp;
	l1 = al1;
	l2 = al2;
	if (sp > &genbuf[LBSIZE - (l2 - l1)]) error(27);
	while (l1 < l2) *sp++ = *l1++;
	return(sp);
}

move(cflag)
{
	register int *adt, *ad1, *ad2;
	int getcopy();

	setdot();
	nonzero();
	if ((adt = address())==0) error(23);
	newline();
	ad1 = addr1;
	ad2 = addr2;
	if (cflag) { ad1 = dol; append(getcopy, ad1++); ad2 = dol; }
	ad2++;
	if (adt<ad1)
	{
		dot = adt + (ad2-ad1);
		if ((++adt)==ad1) return;
		reverse(adt, ad1);
		reverse(ad1, ad2);
		reverse(adt, ad2);
	}
	else if (adt >= ad2)
	{
		dot = adt++;
		reverse(ad1, ad2);
		reverse(ad2, adt);
		reverse(ad1, adt);
	}
	else error(23);
}

reverse(aa1, aa2)
{
	register int *a1, *a2, t;

	a1 = aa1;
	a2 = aa2;
	for (;;)
	{
		t = *--a2;
		if (a2 <= a1) return;
		*a2 = *a1;
		*a1++ = t;
	}
}

getcopy()
{
	if (addr1 > addr2) return(EOF);
	getline(*addr1++);
	return(0);
}

compile(aeof)
{
	register eof, c;
	register char *ep;
	char *lastep;
	char bracket[NBRA], *bracketp;
#ifndef	MISC
	int nbra;
#endif
	int cclcnt;
#ifdef	AUTOW
	char	tempbuf[ESIZE];
	int	savcirc;
	char	*ep1;
#endif	AUTOW
#ifdef	PRINT_STRING
	char	fstch;
#endif	PRINT_STRING

	ep = expbuf;
	eof = aeof;
	bracketp = bracket;
	if ((c = getchar()) == eof) { if (*ep==0) error(20); return; }
	nbra = 0;
	lastep = 0;
#ifdef	AUTOW
	ep1 = tempbuf;
	if (*ep) while (*ep != CEOF) *ep1++ = *ep++;
	*ep1++ = CEOF;
	ep = expbuf;
	savcirc = circfl;
#endif	AUTOW
	circfl = 0;
	if (c=='^') { c = getchar(); circfl++; }
#ifdef	SEARCH_STRING
	if (c=='\n' && *ep) error(20);
#endif	SEARCH_STRING
	peekc = c;
	for (;;)
	{
		if (ep >= &expbuf[ESIZE]) goto cerror;
		c = getchar();
		if (c==eof) { *ep++ = CEOF; return; }
		if (c!='*') lastep = ep;
		switch (c)
		{

	    case BACKSL:if ((c = getchar())=='(')
			{
				if (nbra >= NBRA) goto cerror;
#ifndef	AUTOW
				*bracketp++ = nbra;
#endif	AUTOW
				*ep++ = CBRA;
#ifdef	AUTOW
				*bracketp++ = nbra;
#endif	AUTOW
				*ep++ = nbra++;
				continue;
			}
			if (c == ')')
			{
#ifndef	AUTOW
				if (bracketp <= bracket) goto cerror;
#endif	AUTOW
				*ep++ = CKET;
#ifdef	AUTOW
				if (bracketp <= bracket) goto cerror;
#endif	AUTOW
				*ep++ = *--bracketp;
				continue;
			}
			*ep++ = CCHR;
			if (c=='\n') goto cerror;
			*ep++ = c;
			continue;

	    case '.':	*ep++ = CDOT;
			continue;

	    case '\n':	goto cerror;

	    case '*':	if (lastep == 0)
			{
				lastep = ep;
				goto defchar;
			}
			if (*lastep==CBRA || *lastep==CKET)
#ifndef	AUTOW
				error(20);
#else
				goto cerror;
#endif	AUTOW
			*lastep =| STAR;
			continue;

	    case '$':	if ((peekc=getchar()) != eof) goto defchar;
			*ep++ = CDOL;
			continue;

	    case '[':	*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c=getchar()) == '^')
			{
				c = getchar();
				ep[-2] = NCCL;
			}
			do
			{
				if (c=='\n') goto cerror;
#ifdef	PRINT_STRING
				if (c == '-' && cclcnt > 1)
				{
					if ((c = getchar()) == '\n' || c <= ep[-1] ||
						ep >= &expbuf[ESIZE-2]) goto cerror;
					*ep++ = 0; *ep++ = c; cclcnt =+ 2;
					continue;
				}
				else if (c == BACKSL && (c = getchar()) == '\n') goto cerror;
#endif	PRINT_STRING
				*ep++ = c;
				cclcnt++;
				if (ep >= &expbuf[ESIZE]) goto cerror;
			} while ((c = getchar()) != ']');
			lastep[1] = cclcnt;
			continue;

#ifdef	MISC
	    case '%':	if (*(ep1 = rhsbuf) == 0) { *expbuf = 0; error(32); }
			while (*ep1)
			{
				*ep++ = CCHR;
				*ep++ = *ep1++ & 0177;
				if (ep >= &expbuf[ESIZE]) goto cerror;
			}
			continue;
#endif

	    case '&':	if (*tempbuf == CEOF) { *expbuf = 0; error(32); }
			if (savcirc)
				if (ep == expbuf) circfl++;
				else { lastep = ep; *ep++ = CCHR; *ep++ = '^'; }
			ep1 = tempbuf;
			while ((*ep = *ep1++) != CEOF)
			{
				if (ep >= &expbuf[ESIZE]) goto cerror;
				switch (*ep)
				{

			    case CBRA:	ep1++;
					if (nbra >= NBRA) goto cerror;
					*bracketp++ = nbra;
					*++ep = nbra++;
					break;

			    case CKET:	ep1++;
					if (bracketp <= bracket) goto cerror;
					*++ep = *--bracketp;
					break;

			    case CCL:
			    case NCCL:
			    case CCL|STAR:
			    case NCCL|STAR:
					*++ep = *ep1++;
					break;

			    case CDOT:
			    case CDOT|STAR:
			    case CCHR:
			    case CCHR|STAR:
					lastep = ep++;
					continue;

			    default:	ep++;
					continue;
				}

				ep++;
				lastep = ep-2;
			}
			if (ep[-1] == CDOL)
				if ((peekc = getchar()) != eof)
				{
					ep[-1] = CCHR;
					*ep++ = '$';
				}
			continue;

	    defchar:
	    default:	*ep++ = CCHR;
			*ep++ = c;
		}
	}
cerror:	expbuf[0] = 0;
	error(20);
}

#ifdef	PRINT_STRING
printstring()
{
	register char *ep, flag;
	register int count;
	static char empty[] "empty";

	listf++;
	write(1, "& = ", col = 4);
	if (*expbuf)
	{
		putchar('/');
		if (circfl) putchar('^');
		for (ep = expbuf; *ep != CEOF; )
			switch(*ep++)
			{
	
		    case CCHR:	putchar(*ep++); continue;
	
		    case CCHR|STAR:
				putchar(*ep++); putchar('*'); continue;
	
		    case CBRA:	ep++; putchar(BACKSL); putchar('('); continue;
	
		    case CKET:	ep++; putchar(BACKSL); putchar(')'); continue;
	
		    case CDOT:	putchar('.'); continue;
	
		    case CDOT|STAR:
				putchar('.'); putchar('*'); continue;
	
		    case CDOL:	putchar('$'); continue;
	
		    case NCCL|STAR:
		    case CCL|STAR:
		    case NCCL:
		    case CCL:	flag = *(ep-1); count = *ep++; putchar('[');
				if ((flag & ~STAR) == NCCL) putchar('^');
				while (--count)
					if (*ep=='\0') { ep++; putchar('-'); }
					else putchar(*ep++);
				putchar(']');
				if (flag & STAR) putchar('*');
			}
		putnls("/");
	}
	else putnls(empty);
	write(1, "% = ", col = 4);
	if (*rhsbuf)
	{
		putchar('/');
		for (ep = rhsbuf; *ep; )
		{
			if ((flag = *ep++) < 0)
			{
				putchar('\\');
				flag =& 0177;
			}
			putchar(flag);
		}
		putnls("/");
	}
	else putnls(empty);
	write(1, "! = ", col = 4);
	if (*unixbuffer)
	{
		putchar('/');
		for (ep = unixbuffer; *ep; putchar(*ep++));
		putnls("/");
	}
	else putnls(empty);
	listf = 0;
}
#endif	PRINT_STRING

execute(gf, addr)
int *addr;
{
	register char *p1, *p2, c;

	if (gf)
	{
		if (circfl) return(0);
#ifndef	BETTER_SWAP
		p1 = linebuf;
		p2 = genbuf;
		while (*p1++ = *p2++);
#endif	BETTER_SWAP
		locs = p1 = loc2;
	}
	else
	{
		if (addr==zero) return(0);
		p1 = getline(*addr);
		locs = 0;
	}
	p2 = expbuf;
	if (circfl) { loc1 = p1; return(advance(p1, p2)); }
	if (*p2==CCHR)				/* fast check for first character */
	{
		c = p2[1];
		do
		{
			if (*p1!=c) continue;
			if (advance(p1, p2)) { loc1 = p1; return(1); }
		} while (*p1++);
		return(0);
	}
	do					/* regular algorithm */
		if (advance(p1, p2)) { loc1 = p1; return(1); }
	while (*p1++);
	return(0);
}

advance(alp, aep)
{
	register char *lp, *ep, *curlp;
	char *nextep;

	lp = alp;
	ep = aep;
	for (;;)
		switch (*ep++)
		{

	    case CCHR:	if (*ep++ == *lp++) continue; return(0);
	
	    case CDOT:	if (*lp++) continue; return(0);
	
	    case CDOL:	if (*lp==0) continue; return(0);
	
	    case CEOF:	loc2 = lp; return(1);
	
	    case CCL:	if (cclass(ep, *lp++, 1)) { ep =+ *ep; continue; } return(0);
	
	    case NCCL:	if (cclass(ep, *lp++, 0)) { ep =+ *ep; continue; } return(0);
	
	    case CBRA:	braslist[*ep++] = lp; continue;
	
	    case CKET:	braelist[*ep++] = lp; continue;
	
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
			while (cclass(ep, *lp++, ep[-1]==(CCL|STAR)));
			ep =+ *ep;
	
		star:	do
			{
				lp--;
				if (lp==locs) break;
				if (advance(lp, ep)) return(1);
			} while (lp > curlp);
			return(0);
	
	    default:	error(20);
		}
}

cclass(aset, ac, af)
{
	register char *set, c;
	register n;

	set = aset;
	if ((c = ac) == 0) return(0);
	n = *set++;
#ifndef	PRINT_STRING
	while (--n) if (*set++ == c) return(af);
#else
	while (--n)
		if (*set == '\0')
		{
			if (c > set[-1] && c <= set[1]) return(af);
			set++; set++; n--;
		}
		else if (*set++ == c) return(af);
#endif	PRINT_STRING
	return(!af);
}

putd()
{
	register r;

	r = count % 10;
	if (count =/ 10) putd();
	putchar(r + '0');
}

puts(as)
{
	register char *sp;

	sp = as;
	while (*sp) putchar(*sp++);
}

putnls(as)
{

	col = 0;
	puts(as);
	putchar('\n');
}

char	line[70];
char	*linp	line;

putchar(ac)
{
	register char *lp;
	register c;

	lp = linp;
	c = ac;
	if (listf)
	{
		col++;
		if (col >= LENGTH - 1) { col = 1; *lp++ = BACKSL; *lp++ = '\n'; }
		if (c=='\t') { c = '>'; goto esc; }
		if (c=='\b')
		{
			c = '<';
		esc:	*lp++ = '-';
			*lp++ = '\b';
			*lp++ = c;
			goto out;
		}
		if (c<' ' && c!= '\n')
		{
			*lp++ = BACKSL;
			*lp++ = (c>>3)+'0';
			*lp++ = (c&07)+'0';
			col =+ 2;
			goto out;
		}
	}
	*lp++ = c;
out:	if(c == '\n' || lp >= &line[64])
	{
		linp = line;
		write(1, line, lp-line);
		return;
	}
	linp = lp;
}

#ifdef	GETPID
/*
 * Get process ID routine if system call is unavailable.
 */
getpid()
{
	register f;
	int b[1];

	if((f = open("/dev/kmem", 0)) < 0) return(-1);
	seek(f, 0140074, 0);			/* u_procp */
	read(f, b, 2);
	seek(f, b[0]+8, 0);			/* p_pid */
	read(f, b, 2);
	close(f);
	return(b[0]);
}
#endif

op(inglob)
{
	register int	*a1;
	register char	*lp, *sp;
	char	seof, ch;
	int	t, nl;
	int	getopen();
	int	getnil();

	threshold = genbuf + margin;
	savethresh = 0;

	ch = peekc = getchar();
	switch (ch)
	{

    case BACKSL:t = 1;
		delete();
		addr2 = addr1;
		break;

    case ';':
    case '+':	t = 0;
		break;

    case '-':	t =1;
		break;

    default:	goto normal;
	}

	peekc = 0;
	if (addr1 != addr2) error(23);
	oflag = 0;
	append(getnil, addr2-t);
	addr1 = addr2 =- (t-1);
	setdot();
	nonzero();

normal:	if (addr1 == zero) error(23);
	seof = getchar();
	if (seof == '\n') { loc2 = linebuf-1; seof = '\0'; }
	else compile(seof);
	setraw();	/* terminal into raw mode*/

	for (a1 = addr1; a1 <= addr2; a1++)
	{
		if (seof != '\0')
		{
			if (execute(0, a1) == 0) continue;
		}
		else getline(*a1);
		putnls("\\\r");
		sp = genbuf;
		inglob =| 01;
		for (lp = linebuf; lp < loc2;)
		{
			putch(*lp);
			*sp++ = *lp++;
		}
		lnp = lp;
		gnp = sp;

#ifdef	NICE_EXIT
		oldline = *a1; thestart = a1; nline = 0;
#endif	NICE_EXIT
		oflag = gopen();	/* open the current line */
		retag(*a1 = putline(linebuf), *a1);	/* write revised line */
		nl = append(getopen, a1);
		a1 =+ nl;
		addr2 =+ nl;
#ifdef	AUTOW
		if (writewaiting)
		{
			delaywrite = writewaiting = 0;
			catchclock();
			delaywrite++;
		}
#endif	AUTOW
	}
	setcook();	/* terminal into cooked mode */
	putchar('\n');
	if (inglob == 0) error(21);
}

getnil()
{
	if (oflag == EOF) return(EOF);
	linebuf[0] = '\0';
	oflag = EOF;
	return(0);
}

setraw()
{
	if (gtty(0, tty) == -1) error(3);
	savetty = tty[2];
	tty[2] =| RAW;
	stty(0, tty);
}

setcook()
{
	tty[2] = savetty;
	stty(0, tty);
}

inword(c)
char c;
{
	if (c>='0' && c<='9') return(1);
	c =& 0137;				/* convert to upper case */
	if (c>='A' && c<='Z') return(1);
	return(0);
}

rescan()
{
	register char *lp, *sp;

	if (savethresh) { threshold = savethresh; savethresh = 0; }
	lp = linebuf;
	sp = genbuf;
	while ((*lp++ = *sp++) != '\0')
		if (lp > linebuf+LBSIZE) { *(--lp) = '\0'; break; }
}

				/*
				 * Leaves revised line in linebuf,
				 * returns 0 if more to follow,
				 * EOF if last line.
				 */
gopen()
{
	register char *lp, *sp, *rp;
	char ch, *br, *pr;
	int tabs;
	int retcode, savint, pid, rpid;
#ifdef	MISC
	char erase, kill;

	kill = tty[1] >> 8 & 0177;
	erase = tty[1] & 0177;
#endif

	lp = lnp;
	sp = gnp;
	tabs = 0;
	for (rp = genbuf; rp < sp; rp++) if (*rp == CTRLI) tabs =+ TABSET;

	for (;;)
	{
#ifdef	MISC
		if ((ch = getchar()) == RUBOUT)
		{
			*thestart++ = oldline;
			if (nline)			/* some lines added */
			{
				addr1 = thestart;
				addr2 = thestart + nline - 1;
				if (addr2 == dol) delete(); else { delete(); dot--; }
			}
			putnls("\\\r");
			setcook();
			lastc = '\n';
			error(20);		/* no return */
		}
		if (ch == kill)
		{
			sp = genbuf;		/* delete displayed line backward */
			tabs = 0;
			goto verify;
		}
		if (ch == erase) goto erasechr;
		if (ch >= ' ')			/* not a control chr */
		{
			*--lp = ch;
			goto forward;
		}
		switch(ch)			/* what's left */
#else
		switch (ch = getchar())
#endif
		{

	    case CTRLD:
	    case ESCAPE:	/* close the line (see case '\n' also) */
	    close:
			putb(lp);
			while (*sp++ = *lp++);
			rescan();
			return(EOF);

	    case CTRLA:				/* verify line */
	    verify:	putnls("\\\r");
			*sp = '\0';
			putb(genbuf);
			continue;

	    case CTRLB:				/* back a word */
			if (sp == genbuf) goto backquery;
			while ((*--lp = *--sp) == SPACE)
				if (sp < genbuf) goto out;
			if (inword(*sp))
			{
				while (inword(*--lp = *--sp))
					if (sp < genbuf) goto out;
				if (*sp == SPACE)
					while ((*--lp = *--sp) == SPACE)
						if (sp < genbuf) goto out;
			}
			else while (sp >= genbuf && !inword(*sp))
					if ((*lp-- = *sp--) == CTRLI) tabs =- TABSET;
	    out:	sp++;
			lp++;
			goto verify;

	    case CTRLC:
	    case CTRLQ:				/* forward one char */
			if (*lp == 0) goto backquery;
			putch(*lp);
	    forward:	if (*lp==SPACE && sp+tabs > threshold)
			{
				putch('\r');
				ch = '\n';
				putch(ch);
				lp++;
				*sp++ = ch;
				br = sp;
				break;
			}
			if (*lp == CTRLI) tabs =+ TABSET;
			*sp++ = *lp++;				/* one character */
			if (sp+tabs == threshold) putch(BELL);
			continue;

	    case CTRLE:	putb(lp);
			goto verify;

	    case CTRLF:	while (*lp++);			/* delete forward */
			lp--;
			goto verify;

#ifndef	MISC
	    case CTRLH:	help();				/* help */
			goto verify;
#endif

	    case CTRLP:	while (*lp != '\0')		/* skip to eol */
			{
				if (*lp == CTRLI) tabs =+ TABSET;
				putch(*sp++ = *lp++);
			}
			continue;

	    case CTRLR:					/* margin release */
			if (threshold-genbuf < LBSIZE-40)
			{
				savethresh = threshold;
				threshold = genbuf+LBSIZE-40;
			}
			else goto backquery;
			continue;

	    case CTRLS:	while (*sp++ = *lp++);		/* re-set to start of line */
			rescan();
			lp = linebuf;
			sp = genbuf;
			tabs = 0;
			goto verify;

	    case CTRLT:					/* backup one character */
			if (sp == genbuf) goto backquery;
			if ((*--lp = *--sp) == CTRLI) tabs =- TABSET;
			goto verify;

	    case CTRLV:	rp = sp;			/* verify spelling */
#ifndef	NOSPELL
			pr = unixbuffer + UNIXBUFL - 2;
			*pr = 0;
			while (*(--rp) == SPACE);
			while (inword(*rp) && rp >= genbuf) *--pr = *rp--;
			if (*pr == 0) goto backquery;
			putnls("!!");
			setcook();
			if ((pid = fork()) == 0)
			{
				signal(SIGHUP, onhup);
				signal(SIGQUIT, onquit);
				execl("/bin/spell", "spell", pr, 0);
#else
				putchar('\n');
#endif
				putnls("Sorry, can't spell today");
#ifndef	NOSPELL
				exit(1);
			}
			savint = signal(SIGINTR, 1);
			do rpid = wait(&retcode); while (rpid!=pid && rpid!=-1);
			signal(SIGINTR, savint);
			setraw();
			putnls("!!");
#endif	NOSPELL
			goto verify;

	    case CTRLW:					/* forward one word */
			if (*lp == '\0') goto backquery;
			while (*lp == SPACE) putch(*sp++ = *lp++);
			if (inword(*lp))
			{
				while (inword(*lp))
				{
					putch(*sp++ = *lp++);
					if (sp+tabs == threshold) putch(BELL);
				}
				if (*lp == SPACE)
				{
					if (sp+tabs > threshold)
					{
						ch = '\n';
						lp++;
						*sp++ = ch;
						br = sp;
						putch('\r');
						putch('\n');
					}
					if (*lp == SPACE)
						while (lp[1] == SPACE)
							putch(*sp++ = *lp++);
				}
			}
			else while (*lp && !inword(*lp))
				{
					if (*lp == CTRLI) tabs =+ TABSET;
					putch(*sp++ = *lp++);
					if (sp+tabs == threshold) putch(BELL);
				}
			break;

	    case CTRLZ:					/* delete a word */
			if (sp == genbuf) goto backquery;
			while (*--sp == SPACE) if (sp < genbuf) goto zout;
			if (inword(*sp))
			{
				while (inword(*--sp)) if (sp < genbuf) goto zout;
				if (*sp == SPACE)
					while (*--sp == SPACE) if (sp < genbuf) goto zout;
			}
			else while (sp>=genbuf && !inword(*sp))
					if (*sp-- == CTRLI) tabs =- TABSET;
	    zout:	sp++;
			goto verify;

#ifndef	MISC
	    case '@':	sp = genbuf;			/* delete displayed line backward */
			tabs = 0;
			goto verify;
#endif

#ifndef	MISC
	    case RUBOUT:
#ifdef	NICE_EXIT
			*thestart++ = oldline;
			if (nline)		/* some lines added */
			{
				addr1 = thestart;
				addr2 = thestart + nline - 1;
				if (addr2 == dol) delete(); else { delete(); dot--; }
			}
#endif	NICE_EXIT
			putnls("\\\r");
			setcook();
			lastc = '\n';
			error(20);
#endif	MISC

	    case CTRLX:
#ifndef	MISC
	    case '#':	if (sp == genbuf) goto backquery;
#else
	    erasechr:	if (sp == genbuf) goto backquery;
#endif
			if (*--sp == CTRLI) tabs =- TABSET;
			if (ch == CTRLX) goto verify;
			continue;

	    case '\n':
	    case '\r':				/*
						 * split line; actually handled at
						 * end of switch block
						 */
			ch = '\n';
			*sp++ = ch;
			br = sp;
			break;

	    case '\0':	continue;

	    case BACKSL:			/* special symbols */
			ch = getchar();
			if (ch!=BACKSL && ch!='#' && ch!='@')
				if (savetty & UCASE)
					switch (ch)
					{
	
				    case '(':	ch = '{'; break;
	
				    case ')':	ch = '}'; break;
	
				    case '!':	ch = '|'; break;
	
				    case '^':	ch = '~'; break;
	
				    case '\'':	ch = '`'; break;
	
				    default:	if (ch>='a' && ch<='z') ch =- 040;
						else { peekc = ch; ch = BACKSL; }
						break;
					}
				else { peekc = ch; ch = BACKSL; }
#ifndef	MISC
	    default:	*(--lp) = ch;
			goto forward;
#else
	    case CTRLI:
	    case CTRLG:			/* allow for bell, tab, form-feed */
	    case CTRLL:	*--lp = ch;
			goto forward;

	    default:	help();
			goto verify;
#endif
		}

		if (ch == '\n')
		{
								/* split line */
/*			if (*(br-1) != '\n') putnls("!!");	/* debugging only */
			lnp = sp;
			while (*sp++ = *lp++);			/* move the rest over */
			brp = linebuf + (br - genbuf);
			lnp = linebuf + (lnp - br);
			rescan();
			*(brp - 1) = '\0';
			return(0);
		}
		else continue;

backquery:	putch(BELL);
	}				/* end of forloop block */
}					/* end of gopen */


				/*
				 * calls gopen, deals with multiple lines etc.
				 */
getopen()
{
	register char *lp, *sp;

	if (oflag == EOF) return(EOF);

				/* otherwise, multiple lines */

	lp = linebuf;
	sp = brp;
	while (*lp++ = *sp++);			/* move it down */
	sp = genbuf;
	lp = linebuf;
	while (lp < lnp) *sp++ = *lp++;
	gnp = sp;
					/* should check whether empty line returned */
	oflag = gopen();
	return(0);
}

putch(ch)
char ch;
{
	write(1, &ch, 1);
}

putb(ptr)
char *ptr;	/*display string */
{
	register char *p;

	p = ptr;
	if (*p == '\0') return;
	while (*(++p));
	write(1, ptr, p-ptr);
}

help()
{
	putnls("\n");
#ifdef	MISC
	putnls("          erase and kill characters unchanged");
#endif
	putnls("	^A	display Again		^Q, ^C	next character");
	putnls("	^B	backup word		^R	release margin");
	putnls("	ESCAPE				^S	re-scan from Start");
	putnls("	or ^D	close line and exit	^V	verify spelling");
	putnls("	^E	display to End		^W	next Word");
	putnls("	^F	delete line Forward	^Z	delete word");
#ifndef	MISC
	putnls("	^H	Help			# or ^X delete character");
#else
	putnls("	^O	Help			^X      delete character");
#endif
	putnls("	^P	skip to End		^T	backup one character");
#ifndef	MISC
	putnls("	RUBOUT	exit unchanged		@	delete line backward");
#else
	putnls("	RUBOUT	exit unchanged");
#endif
	putnls("\n	Other characters (including RETURN) inserted as typed");
}
