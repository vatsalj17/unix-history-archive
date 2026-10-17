#include "sed.h"

execute(file)
{
	extern	fout;
	register char *p1, *p2;
	register	c;
	int	i, fi;
	char	cflag;

	if (file) {
		if ((f = open(file, 0)) < 0) {
			printf2("Can't open %s\n", file);
		}
	} else
		f = 0;

	ebp = ibuf;
	cbp = ibuf;

	if(pending) {
		rep = pending;
		cflag = 1;
		pending = 0;
		goto com;
	}

	for(;;) {
		if((spend = gline(linebuf)) == -1) {
			close(f);
			return(0);
		}

		for(rep = ptrspace; rep->command; ) {

			p1 = rep->ad1;
			p2 = rep->ad2;
			cflag = 0;

			if(p1) {

				if(rep->inar) {
					cflag++;
					if(*p2 == CEND) {
						p1 = 0;
						goto com;
					}
					if(*p2 == CLNUM) {
						c = p2[1];
						if(lnum == tlno[c]) {
							rep->inar = 0;
						}
						if(lnum > tlno[c])
							rep->inar =
							cflag = 0;

						goto com;
					}
					if(match(p2, 0) || dolflag){
						rep->inar = 0;
					}
				} else if(*p1 == CEND) {
					if(dolflag)	cflag++;
					goto com;
				}else if(*p1 == CLNUM) {
					c = p1[1];
					if(lnum == tlno[c]) {
						cflag++;
						if(p2)
							rep->inar = 1;
						goto com;
					}
					if(lnum > tlno[c]) {
						rep++;
						continue;
					}
				} else {
					if(match(p1, 0)) {
						cflag++;
						if(p2)
							rep->inar = 1;
					}
				}
			} else
				cflag++;
com:
			if(cflag)	command();
			if(delflag)
				break;

			if(jflag) {
				jflag = 0;
				rep = rep->re1;
			} else
				rep++;

		}
		if(!nflag && !delflag)
			printf("%s\n", linebuf);

		if(aptr > abuf) {
			arout();
		}

		delflag = 0;

	}
}
match(expbuf, gf)
char	*expbuf;
{
	register char	*p1, *p2, c;

	if(gf) {
		if(*expbuf)	return(0);
		p1 = linebuf;
		p2 = genbuf;
		while(*p1++ = *p2++);
		locs = p1 = loc2;
	} else {
		p1 = linebuf;
		locs = 0;
	}

	p2 = expbuf;
	if(*p2++) {
		loc1 = p1;
		return(advance(p1, p2));
	}

	/* fast check for first character */

	if(*p2 == CCHR) {
		c = p2[1];
		do {
			if(*p1 != c)
				continue;
			if(advance(p1, p2)) {
				loc1 = p1;
				return(1);
			}
		} while(*p1++);
		return(0);
	}

	do {
		if(advance(p1, p2)) {
			loc1 = p1;
			return(1);
		}
	} while(*p1++);
	return(0);
}
advance(alp, aep)
{
	register char *lp, *ep, *curlp;
	char	c;
	char *nextep, *bbeg;
	int	ct;

/*	printf("*lp = %c, %o\n*ep = %c, %o\n", *lp, *lp, *ep, *ep);	/*DEBUG*/

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

	case CNL:
	case CDOL:
		if (*lp==0)
			continue;
		return(0);

	case CEOF:
		loc2 = lp;
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
			if(lp == locs)	break;
			if (advance(lp, ep))
				return(1);
		} while (lp-- > curlp);
		return(0);

	default:
		printf2("RE botch\n");
	}
}
substitute()
{
	if(match(rep->re1, 0) == 0)	return(0);

	sflag = 1;
	dosub(rep->rhs);

	if(rep->gfl) {
		while(*loc2) {
			if(match(rep->re1, 1) == 0) break;
			dosub(rep->rhs);
		}
	}
	return(1);
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
dosub(rhsbuf)
char	*rhsbuf;
{
	register char *lp, *sp, *rp;
	int c;

	lp = linebuf;
	sp = genbuf;
	rp = rhsbuf;
	while (lp < loc1)
		*sp++ = *lp++;
	while(c = *rp++) {
		if (c=='&') {
			sp = place(sp, loc1, loc2);
			continue;
		} else if (c<0 && (c =& 0177) >='1' && c < NBRA+'1') {
			sp = place(sp, braslist[c-'1'], braelist[c-'1']);
			continue;
		}
		*sp++ = c&0177;
		if (sp >= &genbuf[LBSIZE])
			printf2("output line too long.\n");
	}
	lp = loc2;
	loc2 = sp + linebuf - genbuf;
	while (*sp++ = *lp++)
		if (sp >= &genbuf[LBSIZE]) {
			printf2("Output line too long.\n");
		}
	lp = linebuf;
	sp = genbuf;
	while (*lp++ = *sp++);
	spend = lp;
}
place(asp, al1, al2)
{
	register char *sp, *l1, *l2;

	sp = asp;
	l1 = al1;
	l2 = al2;
	while (l1 < l2) {
		*sp++ = *l1++;
		if (sp >= &genbuf[LBSIZE])
			printf2("Output line too long.\n");
	}
	return(sp);
}

command()
{
	register int	i;
	register char	*p1, *p2;

	if(rep->command & GCOM)
		if(match(rep->re2, 0) == 0)
			return;
	if(rep->command & VCOM)
		if(match(rep->re2, 0) == 1)
			return;

	switch(rep->command & ~(GCOM | VCOM)) {

		case ACOM:
			*aptr++ = rep;
			if(aptr >= &abuf[ABUFSIZE]) {
				printf2("Too many appends after line %ld\n",
					lnum);
			}
			*aptr = 0;
			break;

		case CCOM:
			delflag = 1;
			if(!rep->inar)
				printf("%s\n", rep->re1);
			break;
		case DCOM:
			delflag++;
			break;
		case CDCOM:
			p1 = p2 = linebuf;

			while(*p1 != '\n') {
				if(*p1++ == 0) {
					delflag++;
					return(0);
				}
			}

			p1++;
			while(*p2++ = *p1++);
			spend = p2;
			jflag++;
			break;

		case EQCOM:
			printf("%ld\n", lnum);
			break;


		case ICOM:
			printf("%s\n", rep->re1);
			break;

		case JCOM:
			jflag = 1;
			break;

		case LCOM:
			break;

		case NCOM:
			if(!nflag)
				printf("%s\n", linebuf);

			if(aptr > abuf)
				arout();
			if((spend = gline(linebuf)) == -1) {
				pending = rep;
				delflag = 1;
			}

			break;
		case CNCOM:
			if(aptr > abuf)
				arout();
			spend[-1] = '\n';
			if((spend = gline(spend)) == -1) {
				pending = rep;
				delflag = 1;
			}
			break;


		case PCOM:
			if(nflag)
				printf("%s\n", linebuf);
			break;
		case CPCOM:
			if(nflag) {
	cpcom:
				p1 = linebuf;
				while(*p1 != '\n' && *p1 != '\0')
					p1++;
				i = *p1;
				*p1 = '\0';
				printf("%s\n", linebuf);
				*p1 = i;
			}
			break;

		case QCOM:
			if(!nflag)
				printf("%s\n", linebuf);
			if(aptr > abuf)	arout();
			flush();
			exit(0);
		case RCOM:

			*aptr++ = rep;
			if(aptr >= &abuf[ABUFSIZE])
				printf2("Too many reads after line%ld\n",
					lnum);

			*aptr = 0;

			break;

		case SCOM:
			i = substitute();
			if(i && nflag && rep->pfl)
				if(rep->pfl == 1)
					printf("%s\n", linebuf);
				else
					goto cpcom;
			if(i && rep->fcode)
				goto wcom;
			break;

		case TCOM:
			if(sflag == 0)	break;
			sflag = 0;
			jflag = 1;
			break;

		wcom:
		case WCOM:
			i = 0;
			while(linebuf[i++]);
			linebuf[--i] = '\n';
			write(rep->fcode, linebuf, i + 1);
			linebuf[i] = '\0';
			break;
	}

}

gline(address)
char	*address;
{
	register char	*p1, *p2;
	register	c;
	p1 = address;
	p2 = cbp;
	for (;;) {
		if (p2 >= ebp) {
			if ((c = read(f, ibuf, 512)) <= 0) {
				return(-1);
			}
			p2 = ibuf;
			ebp = ibuf+c;
		}
		if ((c = *p2++) == '\n') {
			if(p2 >=  ebp) {
				if((c = read(f, ibuf, 512)) <= 0) {
					close(f);
					if(eargc == 0)
							dolflag = 1;
				}

				p2 = ibuf;
				ebp = ibuf + c;
			}
			break;
		}
		if(c)
		if(p1 < lbend)
			*p1++ = c;
	}
	lnum++;
	*p1++ = 0;
	cbp = p2;

	return(p1);
}
ecmp(a, b, count)
char	*a, *b;
{
	while(count--)
		if(*a++ != *b++)	return(0);
	return(1);
}

arout()
{
	int	i, fi;
	aptr = abuf - 1;
	while(*++aptr) {
		if(((*aptr)->command & ~(GCOM|VCOM))==ACOM)
			printf("%s\n", (*aptr)->re1);
		else {
			if((fi = open((*aptr)->re1, 0))
				< 0)
				continue;
			flush();
			while((i = read(fi, genbuf, 512))
				== 512)
				write(fout, genbuf, 512);
			if(i)	write(fout, genbuf, i);
			close(fi);
		}
	}
	aptr = abuf;
	*aptr = 0;
}

