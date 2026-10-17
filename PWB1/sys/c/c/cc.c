#
# include "stdio.h"
/* C command */

# define SBSIZE 10000
# define MAXINC 10
char	*tmp0;
char	*tmp1;
char	*tmp2;
char	*tmp3;
char	*tmp4;
char	*tmp5;
char	*outfile;
# define CHSPACE 1000
char	ts[CHSPACE+50];
char	*tsa ts;
char	*tsp ts;
char	*av[50];
char	*clist[50];
char	*llist[50];
int	pflag;
int	sflag;
int	cflag;
int	eflag;
int	exflag;
int	oflag;
int	proflag;
int	exfail;
char	*pass0 ;
char	*pass1 ;
char	*pass2 ;
char	*passp ;
char	*pref "/lib/crt0.o";

main(argc, argv)
char *argv[]; {
	char *t;
	char *savetsp;
	char *assource;
	char **pv, ptemp[20], **pvt;
	int nc, nl, i, j, c, f20, nxo, na;
	int dexit();

	i = nc = nl = f20 = nxo = 0;
	passp = "/lib//cpp";
	pass0 = "/lib//c0";
	pass1 = "/lib//c1";
	pass2 = "/lib//c2";
	if(*argv[0] != 'c')
		passp[5] = pass0[5] = pass1[5] = pass2[5] = *argv[0];
	pv = ptemp;
	while(++i < argc) {
		if(*argv[i] == '-') switch (argv[i][1]) {
			case 0:
				argv[i] = "./-.c";
				goto passa;
			default:
				goto passa;
			case 'S':
				sflag++;
				cflag++;
				break;
			case 'o':
				if (++i < argc) {
					outfile = argv[i];
					if ((t=getsuf(outfile))=='c'||t=='o') {
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
				pref = "/lib/mcrt0.o";
				break;
			case 'E':
				exflag++;
			case 'P':
				pflag++;
				*pv++ = argv[i];
			case 'c':
				cflag++;
				break;

			case 'T':
				pref = "/usr/lib/tcrt0.o";
				break;
			case 'f':
				pref = "/lib/fcrt0.o";
				pass0 = "/lib/fc0";
				pass1 = "/lib/fc1";
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
				*pv++ = argv[i];
				break;
			case 't':
				for(t=argv[i]+2; *t; t++)
				switch(t)
					{
					case '0':
						pass0= "/sys/c/c0"; continue;
					case '1':
						pass1= "/sys/c/c1"; continue;
					case '2':
						pass2 = "/sys/c/c2"; continue;
					case 'p':
						passp = "/sys/c/cpp"; continue;
					}
				break;

			case 'B':
				pass0 = "/sys/c/oc0";
				pass1 = "/sys/c/oc1";
				passp = "/sys/c/ocpp";
				break;
		} else {
		passa:
			t = argv[i];
			if((c=getsuf(t))=='c' || c=='s'|| exflag) {
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
		while((c=fopen(tmp0, "r")) != NULL) {
			fclose(c);
			tmp0[9]++;
		}
		while((creat(tmp0, 0400))<0)
			tmp0[9]++;
	}
	if ((signal(2, 1) & 01) == 0)	/* interrupt */
		signal(2, dexit);
	if ((signal(15, 1) & 01) == 0)	/* terminate */
		signal(15, dexit);
	(tmp1 = copy(tmp0))[8] = '1';
	(tmp2 = copy(tmp0))[8] = '2';
	(tmp3 = copy(tmp0))[8] = '3';
	if (oflag)
		(tmp5 = copy(tmp0))[8] = '5';
	if (pflag==0)
		(tmp4 = copy(tmp0))[8] = '4';
	pvt = pv;
	for (i=0; i<nc; i++) {
		if (nc>1)
			printf("%s:\n", clist[i]);
		if (getsuf(clist[i])=='s') {
			assource = clist[i];
			goto assemble;
		} else
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
		if (callsys(passp, av))
			{exfail++; eflag++;}
		av[1] =tmp4;
		tsp = savetsp;
		av[0]= "c0";
		if (pflag || exfail)
			{
			cflag++;
			continue;
			}
		av[2] = tmp1;
		av[3] = tmp2;
		if (proflag) {
			av[4] = "-P";
			av[5] = 0;
		} else
			av[4] = 0;
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
		av[4] = 0;
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
			if (callsys(pass2, av)) {
				unlink(tmp3);
				tmp3 = assource = tmp5;
			} else
				unlink(tmp5);
		}
		if (sflag)
			continue;
	assemble:
		av[0] = "as";
		av[1] = "-u";
		av[2] = "-o";
		av[3] = setsuf(clist[i], 'o');
		av[4] = assource;
		av[5] = 0;
		cunlink(tmp1);
		cunlink(tmp2);
		cunlink(tmp4);
		if (callsys("/bin/as", av) > 1) {
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
		if (outfile) {
			av[j++] = "-o";
			av[j++] = outfile;
		}
		while(i<nl)
			av[j++] = llist[i++];
		if(f20)
			av[j++] = "-l2";
		else {
			av[j++] = "-lc";
			av[j++] = "-l";
		}
		av[j++] = 0;
		eflag =| callsys("/bin/ld", av);
		if (nc==1 && nxo==1 && eflag==0)
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
	exit(eflag);
}

error(s, x)
{
	fprintf(exflag?stderr:stdout , s, x);
	putc('\n', exflag? stderr : stdout);
	exfail++;
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
	s =- 3;
	if (c<=14 && c>2 && *s++=='.')
		return(*s);
	return(0);
}

setsuf(as, ch)
char as[];
{
	register char *s, *s1, *s2;

	s = s1 = s2 = copy(as);
	while(*s)
		if (*s++ == '/')
			s1 = s;
	s[-1] = ch;
	if (strcmp(as,"./-.c"))
		return(s1);
	else
		return(s2);
}

callsys(f, v)
char f[], *v[]; {
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
	while(t!=wait(&status));
	if ((t=(status&0377)) != 0 && t!=14) {
		if (t!=2)		/* interrupt */
			{
			printf("Fatal error in %s\n", f);
			eflag = 8;
			}
		dexit();
	}
	return((status>>8) & 0377);
}

copy(as)
char as[];
{
	register char *otsp, *s;
	int i;

	otsp = tsp;
	s = as;
	while(*tsp++ = *s++);
	if (tsp >tsa+CHSPACE)
		{
		tsp = tsa = i = alloc(CHSPACE+50);
		if (i== -1){
			error("no space for file names");
			dexit(8);
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
	if (f==0)
		return(0);
	return(unlink(f));
}
