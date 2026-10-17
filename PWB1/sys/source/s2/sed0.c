#include "sed.h"

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

main(argc, argv)
char	*argv[];
{
	register char	*p;
	extern fout, fin;

	eargc = argc;
	eargv = argv;

	fin = dup(0);
	fout = dup(1);
	flush();
	aptr = abuf;
	lab = labtab + 1;	/* 0 reserved for end-pointer */
	rep = ptrspace;
	rep->ad1 = respace;
	lbend = &linebuf[LBSIZE];
	ptrend = &ptrspace[PTRSIZE];
	reend = &respace[RESIZE];
	labend = &labtab[LABSIZE];
	lnum = 0;
	pending = 0;
	depth = 0;
	spend = linebuf;

	while (--eargc > 0 && (++eargv)[0][0]=='-')
		switch (eargv[0][1]) {

		case 'n':
			nflag++;
			continue;

		case 'f':
			close(fin);
			if(eargc-- <= 0)	exit(2);

			if((fin = open(*++eargv, 0)) < 0) {
				printf2("Cannot open pattern-file: %s\n",
					*eargv);
				exit(2);
			}

			fcomp();
			close(fin);
			continue;

		case 'e':
			eflag++;
			fcomp();
			eflag = 0;
			continue;

		case 'g':
			gflag++;
			continue;

		default:
			printf("Unknown flag: %c\n", eargv[0][1]);
			continue;
		}

	if(rep == ptrspace) {
		eargv--;
		eargc++;
		eflag++;
		fcomp();
		eargv++;
		eargc--;
		eflag = 0;
	}

	if(depth)
		printf2("Too many {'s");

	labtab->address = rep;

	dechain();

/*	abort();	/*DEBUG*/

	if(eargc <= 0)
		execute(0);
	else while(--eargc >=0) {
		execute(*eargv++);
	}
	flush();
	exit(0);
}
fcomp()
{

	register char	*p, *op, *tp;
	struct reptr	*pt, *pt1;
	int	i;
	struct label	*lpt;

	op = lastre;

	if(rline(linebuf) <0)	return(0);
	if(*linebuf == '#' && linebuf[1] == 'n')
		nflag = 1;
	else {
		cp = linebuf;
		goto comploop;
	}

	for(;;) {
		if(rline(linebuf) < 0)	break;

		cp = linebuf;

comploop:
/*	printf("cp: %s\n", cp);	/*DEBUG*/
		while(*cp == ' ' || *cp == '\t')	cp++;
		if(*cp == '\0')		continue;

		p = address(rep->ad1);
		if(p == -1)	printf2(CGMES);

		if(p == rep->ad1) {
			if(op)
				rep->ad1 = op;
			else
				printf2("First RE may not be null\n");
		} else if(p == 0) {
			p = rep->ad1;
			rep->ad1 = 0;
		} else {
			op = rep->ad1;
			if(*cp == ',' || *cp == ';') {
				cp++;
				if((rep->ad2 = p) > reend)
					printf2(TMMES);
				p = address(rep->ad2);
				if(p == -1 || p == 0)	printf2(CGMES);
				if(p == rep->ad2)
					rep->ad2 = op;
				else
					op = rep->ad2;

			} else
				rep->ad2 = 0;
		}

		if(p > reend)
			printf2("Too much text: %s\n", linebuf);

gvloop:
		switch(*cp++) {

			default:
				printf2("Unrecognized command: %s\n",
					linebuf);

			case '{':
				rep->command =| JCOM;
				rep->re1 = rep + 2;

				if(++rep >= ptrend)
					printf2("Too many commands\n");

				rep->ad1 = 0;
				rep->command = JCOM;
				cmpend[depth++] = &rep->re1;
				if(++rep >= ptrend)
					printf2("Too many commands\n");

				rep->ad1 = p;
				if(*cp == '\0')	continue;

				goto comploop;

			case '}':
				if(rep->ad1)
					printf2(AD0MES);

				if(--depth < 0)
					printf2("Too many }'s\n");
				*cmpend[depth] = rep;

				rep->ad1 = p;
				continue;

			case '=':
				rep->command =| EQCOM;
				if(rep->ad2)	printf2(AD1MES);
				break;

			case ':':
				if(rep->ad1)	printf2(AD0MES);

				while(*cp++ == ' ');
				cp--;


				tp = lab->asc;
				while((*tp++ = *cp++))
					if(tp >= &(lab->asc[8]))
						printf2(LTL);
				*--tp = '\0';

				if(lpt = search(lab)) {
					if(lpt->address)
						printf2("Duplicate labels: %s\n",
							linebuf);
				} else {
					lab->chain = 0;
					lpt = lab;
					if(++lab >= labend)
						printf2("Too many labels: %s\n",
							linebuf);
				}
				lpt->address = rep;
				rep->ad1 = p;

				continue;

			case 'a':
				rep->command =| ACOM;
				if(rep->ad2)	printf2(AD1MES);
				if(*cp == '\\')	cp++;
				if(*cp++ != ('\n'))
					printf2(CGMES);
				rep->re1 = p;
				p = text(rep->re1);
				break;
			case 'c':
				rep->command =| CCOM;
				if(*cp == '\\')	cp++;
				if(*cp++ != ('\n'))
					printf2(CGMES);
				rep->re1 = p;
				p = text(rep->re1);
				break;
			case 'i':
				rep->command =| ICOM;
				if(rep->ad2)	printf2(AD1MES);
				if(*cp == '\\')	cp++;
				if(*cp++ != ('\n'))
					printf2(CGMES);
				rep->re1 = p;
				p = text(rep->re1);
				break;

			case 't':
				rep->command =| TCOM;
				goto jtcommon;

			case 'j':
				rep->command =| JCOM;
jtcommon:
				while(*cp++ == ' ');
				cp--;

				if(*cp == '\0') {
					if(pt = labtab->chain) {
						while(pt1 = pt->re1)
							pt = pt1;
						pt->re1 = rep;
					} else
						labtab->chain = rep;
					break;
				}
				tp = lab->asc;
				while((*tp++ = *cp++))
					if(tp >= &(lab->asc[8]))
						printf2(LTL);
				cp--;
				*--tp = '\0';

				if(lpt = search(lab)) {
					if(lpt->address) {
						rep->re1 = lpt->address;
					} else {
						pt = lpt->chain;
						while(pt1 = pt->re1)
							pt = pt1;
						pt->re1 = rep;
					}
				} else {
					lab->chain = rep;
					lab->address = 0;
					if(++lab >= labend)
						printf2("Too many labels: %s\n",
							linebuf);
				}
				break;

			case 'n':
				rep->command =| NCOM;
				break;

			case 'N':
				rep->command =| CNCOM;
				break;

			case 'p':
				rep->command =| PCOM;
				break;

			case 'P':
				rep->command =| CPCOM;
				break;

			case 'r':
				rep->command =| RCOM;
				if(rep->ad2)	printf2(AD1MES);
				if(*cp++ != ' ')
					printf2(CGMES);
				rep->re1 = p;
				p = text(rep->re1);
				break;

			case 'd':
				rep->command =| DCOM;
				break;

			case 'D':
				rep->command =| CDCOM;
				rep->re1 = ptrspace;
				break;

			case 'q':
				rep->command =| QCOM;
				if(rep->ad2)	printf2(AD1MES);
				break;

			case 'l':
				rep->command =| LCOM;
				break;

			case 's':
				rep->command =| SCOM;
				seof = *cp++;
				rep->re1 = p;
				p = compile(rep->re1);
				if(p == -1)	printf2(CGMES);
				if(p == rep->re1) {
					rep->re1 = op;
				} else {
					op = rep->re1;
				}
				if(snlf)	rep->nlf = 1;

				if((rep->rhs = p) > reend)
					printf2(TMMES);

				if((p = compsub(rep->rhs)) == -1)
					printf2(CGMES);
				break;

			case 'w':
				rep->command =| WCOM;
				if(*cp++ != ' ')
					printf2(CGMES);
				if(nfiles >= 10)
					printf2("Too many files in w commands\n");

				text(fname[nfiles]);
				for(i = nfiles - 1; i >= 0; i--)
					if(cmp(fname[nfiles],
						fname[i]) == 0) {
						rep->fcode = fcode[i];
						goto done;
					}

				if((rep->fcode = creat(fname[nfiles],0777))
					< 0)
					printf2("Cannot create %s\n",
						fname[nfiles]);
				fcode[nfiles++] = rep->fcode;
				break;

			case 'g':
				if(rep->command)
					printf2(CGMES);
				rep->command = GCOM;
				seof = *cp++;
				rep->re2 = p;
				p = compile(rep->re2);
				if(p == -1)
					printf2(CGMES);

				if(p == rep->re2) {
					rep->re2 = op;
				} else {
					op = rep->re2;
				}

				if(p > reend)
					printf2(TMMES);
				goto gvloop;

			case 'v':
				if(rep->command)
					printf2(CGMES);
				rep->command = VCOM;
				seof = *cp++;
				rep->re2 = p;
				p = compile(rep->re2);
				if(p == -1)
					printf2(CGMES);

				if(p == rep->re2) {
					rep->re2 = op;
				} else {
					op = rep->re2;
				}

				if(p > reend)
					printf2(TMMES);
				goto gvloop;

		}
		if(*cp == 'g') {
			cp++;
			rep->gfl++;
		} else if(gflag)
			rep->gfl++;

		if(*cp == 'p') {
			cp++;
			rep->pfl = 1;
		}

		if(*cp == 'P') {
			cp++;
			rep->pfl = 2;
		}

		if(*cp == 'w') {
			cp++;
			if(*cp++ !=  ' ')
				printf2(CGMES);
			if(nfiles >= 10)
				printf2("Too many files in w commands\n");

			text(fname[nfiles]);
			for(i = nfiles - 1; i >= 0; i--)
				if(cmp(fname[nfiles],fname[i]) == 0) {
					rep->fcode = fcode[i];
					goto done;
				}
			if((rep->fcode=creat(fname[nfiles],0777)) < 0)
				printf2("cannot open %s\n", fname[nfiles]);
			fcode[nfiles++] = rep->fcode;

		}
done:
		if(++rep >= ptrend)
			printf2("Too many commands, last: %s\n", linebuf);

		rep->ad1 = p;

		if(*cp++ != '\0')
			printf2(CGMES);

	}
	rep->command = 0;
	lastre = op;
}
compsub(rhsbuf)
char	*rhsbuf;
{
	register char	*p, *q;

	p = rhsbuf;
	q = cp;
	for(;;) {
		if((*p = *q++) == '\\') {
			*p = *q++;
			if(*p > numbra + '0' && *p <= '9')
				return(-1);
			*p++ =| 0200;
			continue;
		}
		if(*p == seof) {
			*p++ = '\0';
			cp = q;
			return(p);
		}
		if(*p++ == '\0') {
			return(-1);
		}

	}
}
printf2(s, a, b)
{
	extern fout;
	flush();
	fout = 2;
	printf(s, a, b);
	flush();
	exit(2);
}
compile(expbuf)
char	*expbuf;
{
	register c;
	register char *ep, *sp;
	char	neg;
	char *lastep;
	int cclcnt;
	int	closed;
	char	bracket[NBRA], *bracketp;

	if(*cp == seof) {
		cp++;
		return(expbuf);
	}

	ep = expbuf;
	lastep = 0;
	bracketp = bracket;
	closed = numbra = 0;
	snlf = 0;
	sp = cp;
	if (*sp == '^') {
		*ep++ = 1;
		sp++;
	} else {
		*ep++ = 0;
	}
	for (;;) {
		if (ep >= &expbuf[ESIZE]) {
			cp = sp;
			return(-1);
		}
		if((c = *sp++) == seof) {
			if(bracketp != bracket) {
				cp = sp;
				return(-1);
			}
			cp = sp;
			*ep++ = CEOF;
			return(ep);
		}
		if(c != '*')
			lastep = ep;
		switch (c) {

		case '\\':
			if((c = *sp++) == '(') {
				if(numbra >= NBRA) {
					cp = sp;
					return(-1);
				}
				*bracketp++ = numbra;
				*ep++ = CBRA;
				*ep++ = numbra++;
				continue;
			}
			if(c == ')') {
				if(bracketp <= bracket) {
					cp = sp;
					return(-1);
				}
				*ep++ = CKET;
				*ep++ = *--bracketp;
				closed++;
				continue;
			}

			if(c >= '1' && c <= '9') {
				if((c =- '1') >= closed)
					return(-1);
	
				*ep++ = CBACK;
				*ep++ = c;
				continue;
			}
			if(c == '\n') {
				cp = sp;
				return(-1);
			}
			if(c == 'n') {
				c = '\n';
			}
			goto defchar;

		case '\0':
			continue;
		case '\n':
			cp = sp;
			return(-1);

		case '.':
			*ep++ = CDOT;
			continue;

		case '*':
			if (lastep==0)
				goto defchar;
			if(*lastep == CKET) {
				cp = sp;
				return(-1);
			}
			*lastep =| STAR;
			continue;

		case '$':
			if (*sp != seof)
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			if(&ep[17] >= &expbuf[ESIZE])
				printf2("RE too long: %s\n", linebuf);

			*ep++ = CCL;

			neg = 0;
			if((c = *sp++) == '^') {
				neg = 1;
				c = *sp++;
			}

			do {
				if(c == '\0')	printf2(CGMES);

				if(c == '\\') {
					switch(c = *sp++) {
						case 'n':
							c = '\n';
							break;
					}
				}

				ep[c >> 3] =| bittab[c & 07];
			} while((c = *sp++) != ']');

			if(neg)
				for(cclcnt = 0; cclcnt < 16; cclcnt++)
					ep[cclcnt] =^ -1;
			ep[0] =& 0376;

			ep =+ 16;

			continue;

		defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
}
rline(lbuf)
{
	register char	*p, *q;
	static	saveq;

	p = lbuf - 1;

	if(eflag) {
		if(eflag > 0) {
			eflag = -1;
			if(eargc-- <= 0)
				exit(2);
			q = *++eargv;
			while(*++p = *q++) {
				if(*p == '\\') {
					if((*++p = *q++) == '\0') {
						saveq = 0;
						return(-1);
					} else
						continue;
				}
				if(*p == '\n') {
					*p = '\0';
					saveq = q;
					return(1);
				}
			}
			saveq = 0;
			return(1);
		}
		if((q = saveq) == 0)	return(-1);

		while(*++p = *q++) {
			if(*p == '\\') {
				if((*++p = *q++) == '0') {
					saveq = 0;
					return(-1);
				} else
					continue;
			}
			if(*p == '\n') {
				*p = '\0';
				saveq = q;
				return(1);
			}
		}
		saveq = 0;
		return(1);
	}

	while(*++p = getchar())
		if(*p == '\\')
			*++p = getchar();
		else if(*p == '\n') {
			*p = '\0';
			return(1);
		}
		else if(*p == '\0')
			p--;
	return(-1);
}

address(expbuf)
char	*expbuf;
{
	register char	*rcp;
	long	lno;

	if(*cp == '$') {
		cp++;
		*expbuf++ = CEND;
		*expbuf++ = CEOF;
		return(expbuf);
	}

	if(*cp == '/') {
		seof = '/';
		cp++;
		return(compile(expbuf));
	}

	rcp = cp;
	lno = 0;

	while(*rcp >= '0' && *rcp <= '9')
		lno = lno*10 + *rcp++ - '0';

	if(rcp > cp) {
		*expbuf++ = CLNUM;
		*expbuf++ = nlno;
		tlno[nlno++] = lno;
		if(nlno >= NLINES)
			printf2("Too many line numbers\n");
		*expbuf++ = CEOF;
		cp = rcp;
		return(expbuf);
	}
	return(0);
}
cmp(a, b)
char	*a,*b;
{
	register char	*ra, *rb;

	ra = a - 1;
	rb = b - 1;

	while(*++ra == *++rb)
		if(*ra == '\0')	return(0);
	return(1);
}

text(textbuf)
char	*textbuf;
{
	register char	*p, *q;

	p = textbuf;
	q = cp;
	for(;;) {
		if((*p = *q++) == '\\') {
			*p++ = *q++;
			continue;
		}

		if(*p++ == '\0') {
			cp = --q;
			return(p);
		}
	}
}


search(ptr)
struct label	*ptr;
{
	struct label	*rp;

	rp = labtab;
	while(rp < ptr) {
		if(cmp(rp->asc, ptr->asc) == 0)
			return(rp);
		rp++;
	}

	return(0);
}


dechain()
{
	struct label	*lptr;
	struct reptr	*rptr, *trptr;

	for(lptr = labtab; lptr < lab; lptr++) {

		if(lptr->address == 0)
			printf2("Undefined label: %s\n", lptr->asc);

		if(lptr->chain) {
			rptr = lptr->chain;
			while(trptr = rptr->re1) {
				rptr->re1 = lptr->address;
				rptr = trptr;
			}
			rptr->re1 = lptr->address;
		}
	}
}

