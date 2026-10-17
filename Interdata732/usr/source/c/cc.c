/* C command */

char *tmp0;
char *tmp1;
char *tmp2;
char *tmp3;
char *tmp4;
char *tmp5;
char *asm;
char ts[1000];
char *tsp ts;
char *av[50];
char *clist[50];
char *llist[50];
char *plist[50];
int pflag;
int sflag;
int cflag;
int oflag;
int proflag;
int mflag;
int	vflag;
int exfail;
char *passp "/lib/cpp";
char *pass0 "/lib/c0";
char *pass1 "/lib/c1";
char *pass2 "/lib/c2";
char *pref "/lib/crt0.o";
char *ofile;

main(argc, argv)
char *argv[]; {
	register char *t;
	register i, j, c;
	int nc, nl, np, f20, nxo;
	int dexit();

	i = nc = nl = np = f20 = nxo = 0;
	while(++i < argc) {
		if(*argv[i] == '-')
			switch (argv[i][1]) {
				default:
					goto passa;
				case 'S':
					sflag++;
					cflag++;
					break;
				case 'O':
					oflag++;
					break;
				case 'p':
					proflag++;
					pref = "/lib/mcrt0.o";
					break;
				case 'm':
					mflag++;
					break;

				case 'c':
					cflag++;
					break;


				case '2':
					if(argv[i][2] == '\0')
						pref = "/lib/crt2.o";
					else {
						pref = "/lib/crt20.o";
						f20 = 1;
					}
					break;
				case 'T':
					if (argv[i][2]=='p')
						passp = "/usr/c/cpp";
					if (argv[i][2]=='0')
						pass0 = "/usr/c/c0";
					if (argv[i][2]=='1')
						pass1 = "/usr/c/c1";
					if (argv[i][2]=='2')
						pass2 = "/usr/c/c2";
					break;

				case 'v':
					vflag++;
					break;

				/* change default output file */
				case 'o':
					if (argv[i][2])
						ofile = &argv[i][2];
					else if (i+1<argc && *argv[i+1]!='-')
						ofile = argv[++i];
					break;

				/* Preprocessor flags */
				case 'P':
					pflag++;
					cflag++;
					plist[np++] = argv[i];
					break;
				case 'D':
				case 'U':
				case 'I':
					if (argv[i][2])
						plist[np++] = argv[i];
					else if (i+1<argc && *argv[i+1]!='-') {
						plist[np++] = cat(argv[i], argv[i+1]);
						i++;
					}
					break;

			}
		else {
		passa:
			t = argv[i];
			if((c=getsuf(t))=='c' || c=='s') {
				clist[nc++] = t;
				t = setsuf(t, 'o');
			}
			if (nodup(llist, t)) {
				llist[nl++] = t;
				if (getsuf(t)=='o')
					nxo++;
			}
		}
	}
	if(nc==0)
		goto nocom;
	if (pflag==0) {
		tmp0 = copy("/tmp/ctm0a");
		while((c=open(tmp0, 0))>=0) {
			close(c);
			tmp0[9]++;
		}
		while((creat(tmp0, 0400))<0)
			tmp0[9]++;
	}
	if ((signal(2, 1) & 01) == 0)
		signal(2, &dexit);
	(tmp1 = copy(tmp0))[8] = '1';
	(tmp2 = copy(tmp0))[8] = '2';
	(tmp3 = copy(tmp0))[8] = '3';
/***
	if (oflag)
		(tmp5 = copy(tmp0))[8] = '5';
***/
	if (pflag==0)
		(tmp4 = copy(tmp0))[8] = '4';

    for (i=0; i<nc; i++) {
	if (nc>1)
		printf("%s:\n", clist[i]);
	if (getsuf(clist[i]) == 's')
		asm = clist[i];
	else {
		av[0] = "cpp";
		for (j=1; j<=np; j++)
			av[j] = plist[j-1];
		av[j++] = clist[i];
		if (pflag)
			tmp4 = setsuf(clist[i], 'i');
		av[j++] = tmp4;
		av[j++] = 0;
		exfail = callsys(passp, av);
		if (pflag || exfail)
			continue;
		av[0] = "c0";
		av[1] = tmp4;
		av[2] = tmp1;
		av[3] = tmp2;
		j = 4;
		if (proflag)
			av[j++] = "-p";
		if (mflag)
			av[j++] = "-m";
		av[j] = 0;
		if (callsys(pass0, av)) {
			cflag++;
			continue;
		}
		av[0] = "c1";
		av[1] = tmp1;
		av[2] = tmp2;
		if (sflag)
			tmp3 = setsuf(clist[i], 's');
		av[3] = tmp3;
/***
		if (oflag)
			av[3] = tmp5;
***/
		av[4] = 0;
		if(callsys(pass1, av)) {
			cflag++;
			continue;
		}
/***
		if (oflag) {
			av[0] = "c2";
			av[1] = tmp5;
			av[2] = tmp3;
			av[3] = 0;
			callsys(pass2, av);
			unlink(tmp5);
		}
***/
		asm = tmp3;
	}
	if (sflag)
		continue;
	cunlink(tmp1);
	cunlink(tmp2);
	cunlink(tmp4);
	av[0] = "as";
	j = 1;
	av[j++] = "-o";
	av[j++] = setsuf(clist[i], 'o');
	av[j++] = asm;
	av[j] = 0;
	if (callsys("/bin/as", av))
		cflag++;
	cunlink(tmp3);
	}
nocom:
	if (cflag==0 && nl!=0) {
		i = 0;
		av[0] = "ld";
		av[1] = "-x";	/* Throw away all local symbols for now */
		j = 2;
		if (ofile) {
			av[j++] = "-o";
			av[j++] = ofile;
		}
		av[j++] = pref;
		while(i<nl)
			av[j++] = llist[i++];
		if(f20)
			av[j++] = "-l2";
		else {
			av[j++] = "-lc";
			av[j++] = "-l";
		}
		av[j++] = 0;
		callsys("/bin/ld", av);
		if (nc==1 && nxo==1)
			cunlink(setsuf(clist[0], 'o'));
	}
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
	exit();
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
	s =- 3;
	if (c<=14 && c>2 && *s++=='.')
		return(*s);
	return(0);
}

setsuf(as, ch)
char as[];
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
char f[], *v[]; {
	int t, status;
	register char *p, *prog;

	if (vflag) {
		prog = f;
		for (p = f; t = *p++; )
			if (t == '/')
				prog = p;
		printf("-- %s\n", prog);
	}
	if ((t=fork())==0) {
		execv(f, v);
		printf("Can't find %s\n", f);
		exit(1);
	} else
		if (t == -1) {
			printf("Try again\n");
			return(1);
		}
	while(t!=wait(&status));
	if ((t=(status&0377)) != 0 && t!=14) {
		if (t!=2)		/* interrupt */
			printf("Fatal error in %s\n", f);
		dexit();
	}
	return((status>>8) & 0377);
}

copy(as)
char as[];
{
	register char *otsp, *s;

	otsp = tsp;
	s = as;
	while(*tsp++ = *s++);
	return(otsp);
}

cat(s, t)
register char *s, *t;
{
	register char *otsp;

	otsp = tsp;
	while (*tsp = *s++)
		tsp++;
	while (*tsp++ = *t++)
		;
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
	if (f==0)
		return(0);
	return(unlink(f));
}
