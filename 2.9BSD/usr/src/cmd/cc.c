static	char	sccsid[] = "@(#)cc.c	2.6";	/*	SCCS id keyword	*/
#
# include <stdio.h>
# include <ctype.h>
# include <signal.h>
# include <whoami.h>

/* cc command */

# define MAXINC 10
# define MAXFIL 100
# define MAXLIB 100
# define MAXOPT 100
char	*tmp0;
char	*tmp1;
char	*tmp2;
char	*tmp3;
char	*tmp4;
char	*tmp5;
char	*outfile;
# define CHSPACE 1000
char	ts[CHSPACE+50];
char	*tsa = ts;
char	*tsp = ts;
char	*av[50];
char	*clist[MAXFIL];
char	*llist[MAXLIB];
int	pflag;
int	sflag;
int	cflag;
int	eflag;
int	iflag;
int	vflag;
int	exflag;
int	oflag;
int	proflag;
#ifdef MENLO_OVLY
int	ovlyflag;
	/*
	 * Predefined constant always true for an overlay compilation.
	 * Useful for sorting through include files...
	 */
#define	OVL_DEF	"-DC_OVERLAY"
#define OVLIBC	"-lovc"
#endif MENLO_OVLY
#define LD	"/bin/ld"
#define AS	"/bin/as"
#define LIBC	"-lc"
int	noflflag;
char	*chpass ;
char	*npassname ;
char	pass0[20] = "/lib/c0";
char	pass1[20] = "/lib/c1";
char	pass2[20] = "/lib/c2";
char	passp[20] = "/lib/cpp";
char	*pref = "/lib/crt0.o";
char	*copy();
char	*setsuf();
char	*strcat();
char	*strcpy();

main(argc, argv)
char *argv[]; 
{
	char *t;
	char *savetsp;
	char *assource;
	char **pv, *ptemp[MAXOPT], **pvt;
	int nc, nl, i, j, c, f20, nxo, na;
	int idexit();

	i = nc = nl = f20 = nxo = 0;
	setbuf(stdout, (char *)NULL);
	pv = ptemp;
	while(++i < argc) {
		if(*argv[i] == '-') switch (argv[i][1]) {
		default:
			goto passa;
		case 'S':
			sflag++;
			cflag++;
			break;
		case 'o':
			if (++i < argc) {
				outfile = argv[i];
				if ((c=getsuf(outfile))=='c'||c=='o') {
					error("Would overwrite %s", outfile);
					exit(8);
				}
			}
			break;
		case 'O':
			oflag++;
			break;
		case 'p':
			proflag++;
			break;
#ifdef MENLO_OVLY
		case 'V':
			ovlyflag++;
			*pv++ = OVL_DEF;
			if (pv >= ptemp+MAXOPT) {
				error("Too many DIUC options", (char *)NULL);
				--pv;
			}
			break;
#endif MENLO_OVLY
		case 'E':
			exflag++;
		case 'P':
			pflag++;
			*pv++ = argv[i];
		case 'c':
			cflag++;
			break;

		case 'f':
			noflflag++;
			if (npassname || chpass)
				error("-f overwrites earlier option", (char *)NULL);
			npassname = "/lib/f";
			chpass = "1";
			break;

		case '2':
			if(argv[i][2] == '\0')
				pref = "/lib/crt2.o";
			else {
				pref = "/lib/crt20.o";
				f20 = 1;
			}
			break;
		case 'D':
		case 'I':
		case 'U':
		case 'C':
			*pv++ = argv[i];
			if (pv >= ptemp+MAXOPT) {
				error("Too many DIUC options", (char *)NULL);
				--pv;
			}
			break;
		case 't':
			if (chpass)
				error("-t overwrites earlier option", (char *)NULL);
			chpass = argv[i]+2;
			if (chpass[0]==0)
				chpass = "012p";
			break;

		case 'B':
			if (npassname)
				error("-B overwrites earlier option", (char *)NULL);
			npassname = argv[i]+2;
			if (npassname[0]==0)
				npassname = "/usr/src/cmd/c/o";
			break;

		case 'v':
			vflag++;
			break;

		case 'i':
			iflag++;
			break;
		} 
		else {
passa:
			t = argv[i];
			if((c=getsuf(t))=='c' || c=='s'|| exflag) {
				clist[nc++] = t;
				if (nc>=MAXFIL) {
					error("Too many source files", (char *)NULL);
					exit(1);
				}
				t = setsuf(t, 'o');
			}
			if (nodup(llist, t)) {
				llist[nl++] = t;
				if (nl >= MAXLIB) {
					error("Too many object/library files", (char *)NULL);
					exit(1);
				}
				if (getsuf(t)=='o')
					nxo++;
			}
		}
	}
	if (npassname && chpass ==0)
		chpass = "012p";
	if (chpass && npassname==0)
		npassname = "/usr/src/cmd/c/";
	if (chpass)
		for (t=chpass; *t; t++) {
			switch (*t) {
			case '0':
				strcpy (pass0, npassname);
				strcat (pass0, "c0");
				continue;
			case '1':
				strcpy (pass1, npassname);
				strcat (pass1, "c1");
				continue;
			case '2':
				strcpy (pass2, npassname);
				strcat (pass2, "c2");
				continue;
			case 'p':
				strcpy (passp, npassname);
				strcat (passp, "cpp");
				continue;
			}
		}
	if (noflflag)
		pref = proflag ? "/lib/fmcrt0.o" : "/lib/fcrt0.o";
	else if (proflag)
		pref = "/lib/mcrt0.o";
	if(nc==0)
		goto nocom;
	if (pflag==0) {
		char FD;

		tmp0 = copy("/tmp/ctm0XXXXX");
		mktemp (tmp0);
		FD = creat (tmp0);
		if (FD < 0)
		{
			error("cc: cannot create temp", NULL);
			exit(1);
		}
		close (FD);
	}
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, idexit);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, idexit);
	(tmp1 = copy(tmp0))[8] = '1';
	(tmp2 = copy(tmp0))[8] = '2';
	(tmp3 = copy(tmp0))[8] = '3';
	if (oflag)
		(tmp5 = copy(tmp0))[8] = '5';
	if (pflag==0)
		(tmp4 = copy(tmp0))[8] = '4';
	pvt = pv;
	for (i=0; i<nc; i++) {
		if (nc>1 || vflag)
			printf("%s:\n", clist[i]);
		if (getsuf(clist[i])=='s') {
			assource = clist[i];
			goto assemble;
		} 
		else
			assource = tmp3;
		if (pflag)
			tmp4 = setsuf(clist[i], 'i');
		savetsp = tsp;
		av[0] = "cpp";
		av[1] = clist[i];
		av[2] = exflag ? "-" : tmp4;
		na = 3;
		for(pv=ptemp; pv <pvt; pv++)
			av[na++] = *pv;
		av[na++]=0;
		if(vflag)
			doecho(passp, av);
		if (callsys(passp, av)) {
			cflag++;
			eflag++;
			continue;
		}
		av[1] = tmp4;
		tsp = savetsp;
		av[0]= "c0";
		if (pflag) {
			cflag++;
			continue;
		}
		na = 2;
		av[na++] = tmp1;
		av[na++] = tmp2;
		if (proflag)
			av[na++] = "-P";
#ifdef MENLO_OVLY
		if (ovlyflag)
			av[na++] = "-V";
#endif
		av[na++] = 0;
		if(vflag)
			doecho(pass0, av);
		if (callsys(pass0, av)) {
			cflag++;
			eflag++;
			continue;
		}
		av[0] = "c1";
		av[1] = tmp1;
		av[2] = tmp2;
		if (sflag)
			assource = tmp3 = setsuf(clist[i], 's');
		av[3] = tmp3;
		if (oflag)
			av[3] = tmp5;
		na = 4;
#ifdef MENLO_OVLY
		if (ovlyflag)
			av[na++] = "-V";
#endif
		av[na++] = 0;
		if(vflag)
			doecho(pass1, av);
		if(callsys(pass1, av)) {
			cflag++;
			eflag++;
			continue;
		}
		if (oflag) {
			av[0] = "c2";
			av[1] = tmp5;
			av[2] = tmp3;
			av[3] = 0;
			if(vflag)
				doecho(pass2, av);
			if (callsys(pass2, av)) {
				unlink(tmp3);
				tmp3 = assource = tmp5;
			} 
			else
				unlink(tmp5);
		}
		if (sflag)
			continue;
assemble:
		av[0] = "as";
		av[1] = "-u";
		na = 2;
#ifdef	MENLO_OVLY
		if (ovlyflag)
			av[na++] = "-V";
#endif	MENLO_OVLY
		av[na++] = "-o";
		av[na++] = setsuf(clist[i], 'o');
		av[na++] = assource;
		av[na++] = 0;
		cunlink(tmp1);
		cunlink(tmp2);
		cunlink(tmp4);
		if(vflag)
			doecho(AS, av);
		if (callsys(AS, av) > 1) {
			cflag++;
			eflag++;
			continue;
		}
	}
nocom:
	if (cflag==0 && nl!=0) {
		i = 0;
		av[0] = "ld";
		av[1] = "-X";
		av[2] = pref;
		j = 3;
		if (noflflag) {
			j = 4;
			if (iflag)
				av[3] = "-lfpsim_sep";
			else
				av[3] = "-lfpsim";
		}
		if (outfile) {
			av[j++] = "-o";
			av[j++] = outfile;
		}
		while(i<nl)
			av[j++] = llist[i++];
		if(f20)
			av[j++] = "-l2";
		else {
#ifdef MENLO_OVLY
			av[j++] = ovlyflag ? OVLIBC : LIBC;
#else
			av[j++] = LIBC;
#endif MENLO_OVLY
		}
		av[j++] = 0;
		if(vflag)
			doecho(LD, av);
		eflag |= callsys(LD, av);
		if (nc==1 && nxo==1 && eflag==0)
			cunlink(setsuf(clist[0], 'o'));
	}
	dexit();
}

idexit()
{
	eflag = 100;
	dexit();
}

dexit()
{
	if (!pflag) {
		cunlink(tmp1);
		cunlink(tmp2);
		if (sflag==0)
			cunlink(tmp3);
		cunlink(tmp4);
		cunlink(tmp5);
		cunlink(tmp0);
	}
	exit(eflag);
}

error(s, x)
char *s, *x;
{
	fprintf(exflag?stderr:stdout, s, x);
	putc('\n', exflag? stderr : stdout);
	cflag++;
	eflag++;
}




getsuf(as)
char as[];
{
	register int c;
	register char *s;
	register int t;

	s = as;
	c = 0;
	while(t = *s++)
		if (t=='/')
			c = 0;
		else
			c++;
	s -= 3;
	if (c<=14 && c>2 && *s++=='.')
		return(*s);
	return(0);
}

char *
setsuf(as, ch)
char *as;
{
	register char *s, *s1;

	s = s1 = copy(as);
	while(*s)
		if (*s++ == '/')
			s1 = s;
	s[-1] = ch;
	return(s1);
}

callsys(f, v)
char f[], *v[]; 
{
	int t, status;

	if ((t=fork())==0) {
		execv(f, v);
		printf("Can't find %s\n", f);
		exit(100);
	} else
		if (t == -1) {
			printf("Try again\n");
			return(100);
		}
	while(t!=wait(&status))
		;
	if (t = status&0377) {
		if (t!=SIGINT) {
			printf("Fatal error in %s\n", f);
			eflag = 8;
		}
		dexit();
	}
	return((status>>8) & 0377);
}

char *
copy(as)
char *as;
{
	char *malloc();
	register char *otsp, *s;

	otsp = tsp;
	s = as;
	while (*tsp++ = *s++)
		;
	if (tsp > tsa+CHSPACE) {
		tsp = tsa = malloc(CHSPACE+50);
		if (tsp==NULL) {
			error("no space for file names", (char *)NULL);
			dexit();
		}
	}
	return(otsp);
}

nodup(l, os)
char **l, *os;
{
	register char *t, *s;
	register int c;

	s = os;
	if (getsuf(s) != 'o')
		return(1);
	while(t = *l++) {
		while(c = *s++)
			if (c != *t++)
				break;
		if (*t=='\0' && c=='\0')
			return(0);
		s = os;
	}
	return(1);
}

cunlink(f)
char *f;
{
	if (f==NULL)
		return;
	unlink(f);
}

doecho(s, v)
char *s;
register char **v;
{
	printf ("%s", s);
	while (*v)
		printf (" %s", *v++);
	putchar ('\n');
	fflush (stdout);
}
