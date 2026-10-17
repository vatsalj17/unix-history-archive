#
/*
 *	IFEXPR:
 *
 *
 *	if[n]defs extended to handle expressions rather than the
 *	mention of a single name, binary operators supported are
 *	| (or) and & (and), ! (not) is only unary operator supported.
 *	Nested expressions are supported using ( and ).
 *	Precedence of evaluation is !(not) &(and) |(or).
 *	ifndef adopts an unusual convention with respect to ots effect
 *	i.e.
 *		#ifndef	a|b
 *	 is equivalent to
 *		#ifdef !(a|b)
 *
 *	#else added.
 *	i.e.
 *		#ifdef abc
 *			.....
 *		#endif
 *		#ifndef abc
 *			.....
 *		#endif
 *	is equivalent to
 *		#ifdef abc
 *			.....
 *		#else
 *			.....
 *		#endif
 *
 *			ANDREW HUME & IAN JOHNSTONE
 *			17-08-77
 *			 4-04-78
 *
 *	BETTER_FILES:
 *
 *	use 'mktemp' to create temporary files
 *	this solves duplicate tmp file problem.
 *			GREG ROSE
 *			30-03-78
 *
 *	LIST:
 *
 *	Produce line numbered listings of the source that C is actually
 *	going to compile - in other words take note of if[n]defs etc...
 *	Line numbers that appear on listing file (*.l) are actual line numbers
 *	in the file in which the line appears. Include files may also be listed.
 *	Of course characters in strings, character constants, defines,
 *	macros and comments are all handled as you would expect - PROPERLY
 *
 *	-L[O][I]
 *		L produce listing
 *		O listing only - no compilation, no .i file etc...
 *		I include included files in listing
 *		V list all lines not generating code
 *		   ( no line numbers/listing levels )
 *
 *	sample:
 *		0001 -- main()
 *		0002 0- {
 *		0003 -- 	int a;
 *		0004 -0 }
 *
 *	errors:
 *		too many '}'s (unmatched) give rise to an error
 *		message "too many '}'s"
 *	COMMENT:
 *	A SIDE EFFECT of this alteration is that comments are now handle
 *	in a more consistent way with respect to line numbering, e.g.
 *		0001 --     <<<error>>> ; /* a very
 *		0002 --			     long
 *		0003 --			     comment */
 *	under the old cc the error diagnostic for the above error would refer
 *	to line 3 (yes it did) under the new scheme it will refer to line 1
 *	(a little more sensible).
 *			IAN JOHNSTONE
 *			 3-04-78
 *
 *	OUTPUT:
 *
 *	For those of you who are sick and tired of " cc -o apple apple.c "
 *	then this mod is for you.  If -o is used then a quick check is made
 *	to determine if a output file has been specified if none is present
 *	then the name (no suffix) of the first mentioned file (*.s,*.o,*.c)
 *	appearing after the option is used as the output file. NOTE the entire
 *	pathname is not used only that portion appearing after the last '/'
 *	and before the . of the suffix. So "cc -o apple.c" is equivalent to
 *	the above.
 *			IAN JOHNSTONE
 *			 3-04-78
 *
 *	OPT:
 *
 *	IF you've always wanted to print out the stats from the C optimiser
 *	you now can ( yes virginia there is a santa claus ).
 *	All that need be done is to place any non-null char after '-O'
 *	and bingo as it were.
 *			IAN JOHNSTONE
 *			 4-04-78
 *
 */

#define IFEXPR			/***/
#define BETTER_FILES		/***/
#define LIST			/***/
#define OUTPUT			/***/
#define OPT			/***/
/*#define TEST	/* to include some test code - have a look if ya like */
#ifdef LIST
#define COMMENT
#endif


/* C command */

# define SBSIZE 10000
# define MAXINC 10
char	sbf[SBSIZE];
#ifdef	LIST
char	itoabuf[20];
#endif	LIST
char	*tmp0;
char	*tmp1;
char	*tmp2;
char	*tmp3;
char	*tmp4;
char	*tmp5;
char	*outfile;
# define CHSPACE 1000
char	ts[CHSPACE+50];
# define EXPSIZE 500
char	*tsa ts;
char	*tsp ts;
char	*av[50];
char	*clist[50];
char	*llist[50];
int	instring;
#ifdef	COMMENT
int	incomment;
#endif
int	pflag;
int	sflag;
#ifdef	LIST
int	llflag;		/* listing flag */
#define	LLLIS	01 	/* produce listing */
#define	LLINCL	02 	/* list includes too */
#define	LLONLY	04 	/* listing only */
#define	LLVERB	08	/* list all lines even those not gening code */
#endif	LIST
int	cflag;
int	eflag;
int	exflag;
int	oflag;
int	proflag;
int	depth;
int	*ibuf;
int	*ibufs[MAXINC];
int	ifno;
int	*obuf;
extern int	fout;
char	*lp;
char	*line;
char *predef;
char *sprefix "/usr/include";
int	lineno[MAXINC];
int	exfail;
struct symtab {
	char	name[8];
	char	*value;
} *symtab;
# define symsiz 400
struct symtab *defloc;
struct symtab *incloc;
struct symtab *eifloc;
struct symtab *ifdloc;
struct symtab *ifnloc;
struct symtab *unxloc;
struct symtab *lneloc;
struct symtab *prdloc;
#ifdef	IFEXPR
struct symtab *elsloc;
char trufls[100];
int tflvl -1;
       /*
	* the following two variables values are altered by getline
	* in response to #ifdefs,#ifndefs,#endifs and #elses it may
	* encounter. The net effect of theses changes is to control
 	* the compilation of the desired program segments:
	*
	* if( flslvl != 0 ) then no compile - just flush input
	*		    until an appropriate #endif of #else
	*		    is encountered.
	*		    flslvl is in effect the number of FALSE
	*		    nested #ifdefs or #ifndefs
	* if( flslvl == 0 ) compile all input.
	*		    trulvl is in effect the number of TRUE
	*		    nested #ifdefs or #ifndefs
	*/
#endif	IFEXPR
int	trulvl;
int	flslvl;
char	*stringbuf;
char	*pass0 "/lib/c0";
char	*pass1 "/lib/c1";
char	*pass2 "/lib/c2";
char	*pref "/lib/crt0.o";

main(argc, argv)
char *argv[]; {
	char *t;
	char *savetsp;
	char *assource;
	int nc, nl, i, j, c, f20, nxo;
	int dexit();

	i = nc = nl = f20 = nxo = 0;
	while(++i < argc) {
		if(*argv[i] == '-') switch (argv[i][1]) {
			default:
				goto passa;
			case 'S':
				sflag++;
				cflag++;
				break;
#ifdef	LIST
			case 'L':
				llflag=LLLIS;
				t = &argv[i][2];
				while( c = *t++ ) {
					switch( c ) {
					case 'i':
					case 'I':
						llflag =| LLINCL;
						break;
					case 'o':
					case 'O':
						llflag =| LLONLY;
						pflag++;
						cflag++;
						break;
					case 'v':
					case 'V':
						llflag =| LLVERB;
						break;
					}
				}
				break;
#endif	LIST
			case 'o':
				if (++i < argc) {
					outfile = argv[i];
#ifndef	OUTPUT
					if ((t=getsuf(outfile))=='c'||t=='o') {
						error("Would overwrite %s", outfile);
						exit(8);
#endif	OUTPUT
#ifdef	OUTPUT
					if ((t=getsuf(outfile))=='c'||t=='o'||*argv[i]=='-') {
						i--;
						outfile = -1;
#endif	OUTPUT
					}
				}
				break;
			case 'O':
#ifndef	OPT
				oflag++;
#endif	OPT
#ifdef	OPT
				oflag=1;
				if( argv[i][2] != 0 ) oflag=2;
#endif	OPT
				break;
			case 'p':
				proflag++;
				pref = "/lib/mcrt0.o";
				break;
			case 'E':
				exflag++;
			case 'P':
					pflag++;
			case 'c':
				cflag++;
				break;

			case 'f':
				pref = "/lib/fcrt0.o";
				pass0 = "/lib/fc0";
				pass1 = "/lib/fc1";
				break;

			case 'D':
				predef = argv[i]+2;
				break;
			case 'I':
				sprefix=argv[i]+2;
				break;
#ifdef	TEST
			case '2':
				if(argv[i][2] == '\0')
					pref = "/lib/crt2.o";
				else {
					pref = "/lib/crt20.o";
					f20 = 1;
				}
				break;
			case 't':
				if (argv[i][2]=='0')
					pass0 = "/sys/c/c0";
				if (argv[i][2]=='1')
					pass1 = "/sys/c/c1";
				if (argv[i][2]=='2')
					pass2 = "/sys/c/c2";
				break;

			case 'B':
				pass0 = "/sys/c/oc0";
				pass1 = "/sys/c/oc1";
				break;
#endif	TEST
		} else {
		passa:
			t = argv[i];
#ifdef	LIST
			if( ((c=getsuf(t))=='h') && (llflag&LLONLY) ) {

				clist[nc++] = t;
			} else
#endif	LIST
			if((c=getsuf(t))=='c' || c=='s'|| exflag) {
				clist[nc++] = t;
				t = setsuf(t, 'o');
			}
			if (nodup(llist, t)) {
				llist[nl++] = t;
				if (getsuf(t)=='o') {
					nxo++;
#ifdef	OUTPUT
					if( outfile == -1 ) {
				/* no output file specified to this point
				   but '-o' specified so use filename of first
				   pathname encountered for output, regardless
				   of whether is was a .s or .c or .o pathname */
						t = outfile = setsuf( t , 0 );
						while( *t++ ) ;
						t[-2] = 0;
					}
#endif	OUTPUT
				}
			}
		}
	}
	if(nc==0)
		goto nocom;
	if (pflag==0) {
#ifndef	BETTER_FILES
		tmp0 = copy("/tmp/ctm0a");
#endif	BETTER_FILES
#ifdef	BETTER_FILES
		tmp0 = mktemp( copy("/tmp/ctm0.XXXXX"));
#endif	BETTER_FILES
		while((c=open(tmp0, 0))>=0) {
			close(c);
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
	for (i=0; i<nc; i++) {
		if (nc>1)
			printf("%s:\n", clist[i]);
		if (getsuf(clist[i])=='s') {
			assource = clist[i];
			goto assemble;
		} else
			assource = tmp3;
		av[0] = "c0";
#ifndef	LIST
		if (pflag)
#endif	LIST
#ifdef	LIST
		if( pflag && !(llflag&LLONLY) )
#endif	LIST
			tmp4 = setsuf(clist[i], 'i');
		savetsp = tsp;
		av[1] = expand(clist[i]);
		tsp = savetsp;
		if (pflag || exfail)
			continue;
		if (av[1] == 0) {
			cflag++;
			eflag++;
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
#ifndef	OPT
			av[1] = tmp5;
			av[2] = tmp3;
			av[3] = 0;
#endif	OPT
#ifdef	OPT
			j = 1;
			if( oflag==2 ) av[j++] = "-";
			av[j++] = tmp5;
			av[j++] = tmp3;
			av[j++] = 0;

#endif	OPT
			if (callsys(pass2, av)) {
				cflag++;
				eflag++;
				continue;
			}
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

# define LINELEN 500
#ifdef	LIST
char lline[LINELEN+8] "nnnn --  ";
char *llinep;
int llevel 0;
#endif	LIST
char *fnames[MAXINC];

expand(file)
char *file;
{
	int ob[259];
	struct symtab stab[symsiz];
	char ln[LINELEN];
	register int c;
	register char *rlp;
#ifdef	LIST
	int llbuf[259];
#endif	LIST

	exfail = 0;
	ifno=0;
	if (ibufs[0]==0)
		ibufs[0] = sbrk(518);
	ibuf=ibufs[0];
	fnames[ifno] = file;
	if (fopen(file, ibuf)<0) {
		error("No file %s", file);
		return(file);
		}
	obuf = ob;
	symtab = stab;
	for (c=0; c<symsiz; c++) {
		stab[c].name[0] = '\0';
		stab[c].value = 0;
	}
	insym(&defloc, "define");
	insym(&incloc, "include");
	insym(&eifloc, "endif");
	insym(&ifdloc, "ifdef");
	insym(&ifnloc, "ifndef");
	insym(&unxloc, "unix");
	insym(&lneloc, "line");
#ifdef	IFEXPR
	insym(&elsloc, "else");
#endif	IFEXPR
	if (predef)
	insym(&prdloc, predef);
	stringbuf = sbf;
	trulvl = 0;
	flslvl = 0;
	line  = ln;
	lineno[0] = 1;
	if (exflag==0) {
#ifdef	LIST
		if( (llflag&LLONLY)==0 )  {
#endif	LIST
			if (fcreat(tmp4, obuf) < 0) {
				printf("Can't creat %s\n", tmp4);
				dexit();
			}
#ifdef	LIST
		} else obuf[0] = -1;	/* lets not bother with the i/o */
					/* a liitle kludgy but.... */
					/* about a zillion tests required otherwise */
#endif	LIST
	} else {
		obuf = &fout;
		fout = dup(1);
	}
#ifdef	LIST
	if( llflag ) {
		if( fcreat(setsuf(file,'l'),llbuf) < 0 ) {
			printf("Can't create %s\n",setsuf(file,'l'));
			dexit();
		}
	}
#endif	LIST
	puts("# 1 \"");
	puts(file);
	puts("\"\n");
#ifdef	LIST
	llinep = &lline[8];
	llevel = -1;
#endif	LIST
	while(getline()) {
#ifdef	LIST
		putline(llbuf);
#endif	LIST
		if (ln[0] != '#' && flslvl==0)
			for (rlp = line; c = *rlp++;)
				putc(c, obuf);
		putc('\n', obuf);
	}
	for(rlp=line; c = *rlp++;)
			putc(c,obuf);
#ifdef	LIST
	if( llflag ) {
		fflush(llbuf);
		close( llbuf[0] );
	}
#endif	LIST
#ifdef	IFEXPR
	if( flslvl || trulvl ) errexit("missing endif");
#endif	IFEXPR
	fflush(obuf);
	if (obuf != &fout)
	close(obuf[0]);
	close(ibuf[0]);
	return(tmp4);
}

#ifdef	LIST
putline(llbuf)
int llbuf[259];
{
	static char levelchars[] "?0123456789abcdefghijklmnopqrstuvwxyz";
	register char *rlp;
	register int c;
	register int cs;
	int css;
	int blevel,elevel;
	int blflag,elflag;

	css = ifno;
	while( lineno[css]-1 == 0 ) css--;	/* cope with includes */
	if( (flslvl!=0) || (llflag==0) ) {
		if( llflag&LLVERB ) {
	verbose:	if( !(llflag&LLINCL) && ifno != 0)
				goto done;
			pfnm( css , llbuf );
			for( cs = 0 ; cs <8 ; cs ++ )
				putc( ' ' , llbuf);
			for (rlp = &lline[8]; rlp<llinep;)
				putc( *rlp++ , llbuf);
		}
		goto done;
	}
	if( ( (ifno!=0) && (lineno[ifno]>1) ) && !(llflag&LLINCL) ) goto done;
	blflag = elflag = 1;
	if( lline[8] == '#' ) {
		for( rlp = &lline[9]; *rlp == ' ' || *rlp == '\t'; rlp++ ) ;
		if( eq("include", rlp) || eq("define", rlp))
			goto skip;
		if( llflag&LLVERB )
			goto verbose;
		else
			goto done;
	}
	blevel = - (elevel = 32767) ;
	cs = 0;
	for (rlp = line; c = *rlp++;) {
	   if( cs ) {
		   if( cs == c ) cs=0;
		   else if( c == '\\' ) rlp++;
	   } else if( c=='\'' || c=='"' ) cs = c;
		  else if( c=='{' ) { blflag=0; if( ++llevel>blevel ) blevel=llevel; }
		       else if( c=='}' ) { elflag=0; if( --llevel<elevel ) elevel=llevel+1; }
	}
	if( elevel<0 )
		error("too many '}'s ");
 skip:
	pfnm( css , llbuf );
	itoa( lineno[css]-1 );
	lline[0] = itoabuf[15];
	lline[1] = itoabuf[16];
	lline[2] = itoabuf[17];
	lline[3] = itoabuf[18];
	lline[5] = blflag ? '-' : levelchars[(blevel<0)||(blevel>(sizeof levelchars-1)) ? 0 : blevel+1];
	lline[6] = elflag ? '-' : levelchars[(elevel<0)||(elevel>(sizeof levelchars-1)) ? 0 : elevel+1];
	for (rlp =lline; rlp<llinep;)
		putc( *rlp++ , llbuf);
 done:
	llinep = &lline[8];
}

pfnm( cs , llbuf )
register int cs;
int *llbuf;
{
	register char c,*rlp;
	if( llflag&LLINCL ) {
		/* the following is a triffle inefficient but 
		   it don't happen that often so forget it  	*/
		cs = rlp = fnames[cs];
		while( *rlp ) if( *rlp++ == '/' ) cs = rlp;
		rlp = cs;
		c = 0;
		while( *rlp ) { c++ ; putc( *rlp++ , llbuf ); }
		if( c<8 ) putc( '\t' , llbuf );
		putc( '\t' , llbuf );
	}
}

eq(s1,s2)
register char *s1,*s2;
{
	register char c;

	while( (c = *s1++) == *s2++ );
	return( c==0 );
}
#endif	LIST

getline()
{
	register int c, sc, state;
	struct symtab *np;
	char *namep, *filname, *cp;
#ifdef	IFEXPR
	int ifval[40];
	char ifop[40];
	int *ifv;
	char *ifo;

	ifv = ifval;
	*(ifo = ifop) = 0;
#endif	IFEXPR

	lp = line;
	*lp = '\0';
	state = 0;
	if ((c=getch()) == '#')
		state = 1;
	while (c!='\n' && c!='\0') {
		if ('a'<=c && c<='z' || 'A'<=c && c<='Z' || c=='_') {
			namep = lp;
			sch(c);
			while ('a'<=(c=getch()) && c<='z'
			      ||'A'<=c && c<='Z'
			      ||'0'<=c && c<='9' 
			      ||c=='_')
				sch(c);
			sch('\0');
			lp--;
			if (state>3) {
#ifndef	IFEXPR
				if (flslvl==0 &&(state+!lookup(namep,-1)->name[0])==5)
					trulvl++;
				else
					flslvl++;
		out:
				while (c!='\n' && c!= '\0')
					c = getch();
				return(c);
#endif	IFEXPR
#ifdef	IFEXPR
				*++ifv = lookup(namep,-1)->name[0] ? 1 : 0 ;
				if(*ifo == '!') {
					*ifv = !*ifv;
					ifo--;
				}
				if(*ifo == '&') {
					ifv--;	*ifv =& ifv[1];
					ifo--;
				}
				continue;
#endif	IFEXPR
			}
			if (state==3) /* include */
				if (*namep != '"' && *namep != '<')
					{
					error("Bad include syntax", 0);
					state=1;
					}
			if (state!=2 || flslvl==0)
				{
				ungetc(c);
				np = lookup(namep, state);
				c = getch();
				}
			if (state==1) {
				if (np==defloc)
					state = 2;
				else if (np==incloc)
					state = 3;
				else if (np==ifnloc)
					state = 4;
				else if (np==ifdloc)
					state = 5;
				else if (np==eifloc) {
					if (flslvl)
						--flslvl;
					else if (trulvl)
						--trulvl;
					else { errback("If-less #endif"); dexit(); }
					tflvl--;
					goto out;
				}
#ifdef	IFEXPR
				else if (np==elsloc) {
					if (flslvl) {
						--flslvl;
					} else if (trulvl) {
						--trulvl;
					} else { errback("If-less #else"); dexit(); }
					trufls[tflvl]= !trufls[tflvl];
					if( flslvl==0 && trufls[tflvl] )
						trulvl++;
					 else
						flslvl++;
					goto out;
				}
#endif	IFEXPR
				else if (np==lneloc)
					{
					puts("# ");
					lp=line;
					for(; c !='\n' && c != '\0'; c=getch())
						if (!pflag || exflag)
							sch(c);
					sch('\0');
					return(c);
					}
				else {
					errback("Undefined control");
#ifdef	IFEXPR
			out:
#endif	IFEXPR
					while (c!='\n' && c!='\0')
						c = getch();
					return(c);
				}
			} else if (state==2) {
				if (flslvl)
					goto out;
				np->value = stringbuf;
				if (c != '\n' && c != 0)
					{
					savch(c);
					while ((c=getch())!='\n' && c!='\0')
						savch(c);
					}
				savch('\0');
				return(1);
			}
			continue;
		} else if ((sc=c) == '\'' || sc== '"' || (state==3 && sc== '<')) {
			sch(sc);
			filname = lp;
			if (sc== '<')
				{
				sc= '>';
				for(cp=sprefix; *cp; cp++)
					*lp++ = *cp;
				*lp++= '/';
				}
			instring++;
			while ((c=getch())!=sc && c!='\n' && c!='\0') {
				sch(c);
				if (c=='\\')
					sch(getch());
			}
			instring = 0;
			if (flslvl)
				goto out;
			if (state==3) {
				if (flslvl)
					goto out;
				*lp = '\0';
				while ((c=getch())!='\n' && c!='\0');
				if (ifno+1 >=MAXINC)
					error("Unreasonable include nesting",0);
				if(ibufs[++ifno]==0)
					ibufs[ifno]=sbrk(518);
				if (fopen(filname, ibufs[ifno])<0) {
					ifno--;
					errback("Missing file %s", filname);
					dexit();
				} else
					ibuf = ibufs[ifno];
				puts("\n# 1 \"");
				puts(filname);
				puts("\"");
				lineno[ifno]=1;
				fnames[ifno] = copy(filname);
				return(c);
			}
		}
#ifdef	IFEXPR
			else if(state > 3) switch(c)
				{
				case '(':
				case '|':
				case '&':
				case '!':
						*++ifo = c;
						break;
				case ' ':
				case '\t':
						break;
				case ')':
						for(;*ifo == '|'; ifo--) {
							ifv--;	*ifv =| ifv[1];
						}
						if(*ifo == '(')
							ifo--;
						else errexit("'(' expected");
						if(*ifo == '!') {
							*ifv = !*ifv;
							ifo--;
						}
						if(*ifo == '&') {
							ifv--;	*ifv =& ifv[1];
							ifo--;
						}
						break;
				default:

						errexit("illegal char '%o' oct",c);
						break;
				}
#endif	IFEXPR
		sch(c);
		c = getch();
	}
	sch('\0');
#ifdef	IFEXPR
	for(;*ifo == '|'; ifo--) {
		ifv--;	*ifv =| ifv[1];
	}
	if(state == 4 || state == 5)
	if( flslvl==0 & (trufls[++tflvl]=((state +  !(*ifv--))==5)) )
		trulvl++;
	else
		flslvl++;
	if( tflvl == (sizeof trufls - 1) ) { 
		errback("Excessive nesting if ifdefs\n");
		dexit();
	}
	if((ifv != ifval) || (ifo != ifop))
		{
		errexit("if%cdef expression error",state==4 ? 'n' : 0);
		}
	state = 0;
#endif	IFEXPR
	if (state>1)
		errback("Control syntax");
	return(c);
}
#ifdef	IFEXPR
errexit(s,x)
{
	lineno[ifno]--;
	error(s,x);
	dexit();
}
#endif	IFEXPR

insym(sp, namep)
struct symtab **sp;
char *namep;
{
	register struct symtab *np;

	*sp = np = lookup(namep, 1);
	np->value = np->name;
}

error(s, x)
{
	int olfout;
	olfout = fout;
	flush();
	if (exflag)
		fout=2;
	if (fnames[ifno])
		printf("%s: %d: ", fnames[ifno], lineno[ifno]);
	printf(s, x);
	putchar('\n');
	flush();
	fout=olfout;
	exfail++;
	cflag++;
	eflag++;
}
errback(s,x)
{
	lineno[ifno]--;
	error(s,x);
	lineno[ifno]++;
}

sch(c)
{
	register char *rlp;

	rlp = lp;
	if (rlp==line+LINELEN-2)
		error("Line overflow");
	*rlp++ = c;
	if (rlp>line+LINELEN-1)
		rlp = line+LINELEN-1;
	lp = rlp;
}

savch(c)
{
	*stringbuf++ = c;
	if (stringbuf-sbf < SBSIZE)
		return;
	error("Too much defining");
	dexit();
}

getch()
{
	register int c;

#ifdef	COMMENT
	if( incomment ) goto commentprocessing;
#endif	COMMENT
loop:
	if ((c=getc1())=='/' && !instring) {
		if ((c=getc1())!='*')
			{
			ungetc(c);
			return('/');
			}
		for(;;) {
			incomment++;
#ifdef	COMMENT
  commentprocessing:
#endif	COMMENT
			c = getc1();
		cloop:
			switch (c) {

			case '\0':
				return('\0');

			case '*':
				if ((c=getc1())=='/') {
#ifdef	COMMENT
					incomment = 0;
#endif	COMMENT
					goto loop;
				}
				goto cloop;

			case '\n':
#ifdef	COMMENT
				return('\n');
#endif	COMMENT
#ifndef	COMMENT
				putc('\n', obuf);
				continue;
#endif	COMMENT
			}
		}
	}
	return(c);
}
char pushbuff[EXPSIZE];
char *pushp pushbuff;
ungetc(c)
	{
	*++pushp = c;
	if (pushp>pushbuff+EXPSIZE) {
		error("too much backup");
		dexit(8);
		}
	}

getc1()
{
	register c;
	register flag = 0;

	if (*pushp !=0)
		return(*pushp--);
	depth=0;
	if ((c = getc(ibuf)) < 0 && ifno>0) {
		close(ibuf[0]);
		ibuf = ibufs[--ifno];
		puts("\n# ");
		puts(itoa(lineno[ifno]));
		puts(" \"");
		puts(fnames[ifno]);
		puts("\"\n");
		c = getc1();
		flag++;
		if (c=='\n') lineno[ifno]--;
	}
	if (c<0)
		return(0);
	if (c=='\n' )
		lineno[ifno]++;
#ifdef	LIST
	if( (llflag) && (flag==0) ) *llinep++ = c;
#endif	LIST
	return(c);
}

lookup(namep, enterf)
char *namep;
{
	register char *np, *snp;
	register struct symtab *sp;
	int i, c, around;
	np = namep;
	around = i = 0;
	while (c = *np++)
		i =+ c;
	i =% symsiz;
	sp = &symtab[i];
	while (sp->name[0]) {
		snp = sp;
		np = namep;
		while (*snp++ == *np)
			if (*np++ == '\0' || np==namep+8) {
				if (!enterf)
					subst(namep, sp);
				return(sp);
			}
		if (++sp >= &symtab[symsiz])
			if (around++)
				{
				error("too many defines");
				dexit();
				}
			else
			sp = symtab;
	}
	if (enterf>0) {
		snp = namep;
		for (np = &sp->name[0]; np < &sp->name[8];)
			if (*np++ = *snp)
				snp++;
	}
	return(sp);
}
char revbuff[200], *bp;
backsch(c)
	{
	if (bp-revbuff > 200)
		error("Excessive define looping", bp--);
	*bp++ = c;
	}

subst(np, sp)
char *np;
struct symtab *sp;
{
	register char *vp;
	int macflg;

	lp = np;
	bp = revbuff;
	if (depth++>100)
		{
		error("define recursion loop\n");
		return;
		}
	if ((vp = sp->value) == 0)
		return;
	macflg= (*vp == '(');
	/* arrange that define unix unix still
	has no effect, avoiding rescanning */
	while (blank(*vp))
		vp++;
	if (streq(sp->name,vp))
		{
		while (*vp)
			sch(*vp++);
		return;
		}
	if (macflg)
		expdef(vp);
	else
		while (*vp)
			backsch(*vp++);
	while (bp>revbuff)
		ungetc(*--bp);
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
expdef(proto)
char *proto;
{
	char buffer[EXPSIZE], *parg[20], *pval[20], name[20], *cspace, *wp;
	char protcop[EXPSIZE], *pr;
	int narg, k, c;
	pr = protcop;
	while (*pr++ = *proto++);
	if (pr>protcop+EXPSIZE){
		error("define prototype too big");
		dexit(8);
	}
	proto= protcop;
	for (narg=0; (parg[narg] = token(&proto)) != 0; narg++) ;
	/* now scan input */
	cspace = buffer;
	while ((c=getch()) == ' ');
	if (c != '(') {
		error("defined function requires arguments");
		return;
	}
	ungetc(c);
	for(k=0; pval[k] = coptok(&cspace, buffer+EXPSIZE); k++);
	if (k!=narg) {
		error("define argument mismatch");
		return;
	}
	while (c= *proto++) {
		if (!letter(c))
			backsch(c);
		else {
			wp = name;
			*wp++ = c;
			while (letnum(*proto))
			*wp++ = *proto++;
			*wp = 0;
			for (k=0; k<narg; k++)
			if(streq(name,parg[k]))
			break;
			wp = k <narg ? pval[k] : name;
			while (*wp) backsch(*wp++);
		}
	}
}

token(cpp)
char **cpp;
{
	char *val;
	int stc;

	stc = **cpp;
	*(*cpp)++ = '\0';
	if (stc==')') return(0);
	while (**cpp == ' ') (*cpp)++;
	for (val = *cpp; (stc= **cpp) != ',' && stc!= ')'; (*cpp)++) {
		if (!letnum(stc) || (val == *cpp && !letter(stc))) {
			error("define prototype argument error");
			return(0);
		}
	}
	return(val);
}

coptok (cpp, clim)
char **cpp, *clim;
{
	char *val;
	int stc, stop,paren;
	paren = stop = 0;
	val = *cpp;
	if (getch() == ')') return(0);
	while (((stc = getch()) != ',' && stc != ')' ) || paren > 0 || stop >0)  {
		if (stc == '\0') {
			error("non terminated macro call");
			val = 0;
			break;
		}
		if (stop == 0 && (stc == '"' || stc == '\'')) stop = stc;
		else if (stc==stop) stop = 0;
		if ( stc == '\\') {
			stc = getch();
			if (stop>0 || (stc != ',' && stc != '\\'))
				*(*cpp)++ = '\\';
				*(*cpp)++ = stc;
		} else {
			*(*cpp)++ = stc;
			if (stop==0) {
				if (stc == '(') paren++;
				if (stc == ')') paren--;
			}
		}
		if (*cpp >= clim) {
			error("define argument too long",0);
			dexit(8);
		}
	}
	*(*cpp)++ = 0;
	ungetc(stc);
	return(val);
}

letter(c)
{
	if ((c >= 'a' && c <= 'z') ||
	    (c >= 'A' && c <= 'Z') ||
	    (c == '_'))
	    return (1);
	else
	    return(0);
}

letnum(c)
{
	if (letter(c) || (c >= '0' && c <= '9'))
	  return(1);
	else
	  return(0);
}

streq(s,t)
char *s, *t;
{
	int c;
	while ( (c= *s++) == *t++)
		if (c==0) return(1);
	return(0);
}

puts(s)
char *s;
{
	int c;
	if (pflag && !exflag) return;
	while (c= *s++)
		putc(c, obuf);
}

itoa(n)
{
	char *sp;
#ifndef	LIST
	static char sb[20];
	sp = sb+19;
#endif	LIST
#ifdef	LIST
	sp = itoabuf;
	while( sp<itoabuf+19 ) *sp++ = '0';
#endif	LIST
	*sp=0;
	if (n<=0)
		*--sp = '0';
	else
	while (n) {
		*--sp = '0'+ n%10;
		n=n/10;
	}
	return(sp);
}

blank(c)
{
	return(c==' ' || c== '\t');
}
