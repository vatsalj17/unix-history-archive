/* Ratfor-Fortran command */

extern int fin, fout;
char	ts[500];
char	*tsp	ts;
char	*av[50];
char	*rlist[50];
int	nr	0;
char	*llist[50];
int	nl	0;
int	bdcount	0;	/* count block data files generated */
int	rflag	0;
int	vflag	1;
int	fflag	0;
int	cflag	0;
int    sflag	0;
char 	*complr "/usr/bin/fort";
char 	*ratfor "/usr/bin/ratinc";

main(argc, argv)
char *argv[]; {
	register char *t;
	register int i, j, c;
	int dexit();

	for(i=0; ++i < argc; ) {
		if(*argv[i] == '-')
			switch (argv[i][1]) {
				default:
					goto passa;
					break;
				case 'v':
					vflag = 0;
					break;
				case 'r':
					rflag = fflag = cflag = 1;
					break;
				case 'f':
					fflag = 1;
					break;
				case 'c':
					cflag = 1;
					break;
				case 'S':
					sflag = 1;
					break;
			}
		else {
	   passa:
			t = argv[i];
			if( (c=getsuf(t))=='r' )
				ratcomp(t);
			else
				llist[nl++] = t;
		}
	}
	if(rflag)
		dexit();
	if ((signal(2, 1) & 01) == 0)
		signal(2, &dexit);
/***		do fortran compile assemble and link    ***/
	if ( fortcomp() == 0 && fflag == 0 ) {
		for ( i=0; i<nr; i++)
			cunlink(rlist[i]);
	}
	dexit();
}

dexit()
{
	cunlink("ratjunk");
	exit(0);
}
texit()
{
	printf(" syntax errors -- please list ratjunk\n");
	exit(0);

}

ratcomp(s) char *s; {
	int status;
	register int t;
	if(vflag)
		printf("%s:\n",s);
	av[0] = ratfor;
	av[1] = "-6";		/* set continuation in col 5 & 6 */
	av[2] = s;
	av[3] = 0;
	if( (t=fork())==0 ){
		close(1);
		fout = creat("ratjunk", 0666);
		execv(ratfor, av);
		fout = 2;
		error("can't ratfor\n");
		exit(1);
	}
	while( t!=wait(&status) );
	if( (t=(status&0377)) != 0 && t!=14 )
		texit();		/***  temp  ***/
	t = (status>>8) & 0377;
	if( t )
/***		return(++cflag);****/
		texit();		/***  temp  ***/
	splitup();
}

fortcomp(){
	register int t;
	register int j;
	register int i;
	j=1;
	av[0] = complr;
	if ( cflag ) 
		av[j++] = "-c";
	if ( sflag )
		av[j++] = "-S";
	for ( i=0; i<nr; i++) {
		av[j++] = rlist[i];
		if ( vflag )
			printf("    %s\n",rlist[i]);
	}
	for ( i=0; i<nl; i++ ) 
		av[j++] = llist[i];
	av[j++] = "-lr";
	av[j] = 0;
	if( callsys(complr, av) )
		return(1);
	return(0);
}

getsuf(s)
char s[];
{
	int c;
	char t, *os;

	c = 0;
	os = s;
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

setsuf(s, ch)
char s[];
{
	char *os;

	os = s;
	while(*s++);
	s[-2] = ch;
	return(os);
}


callsys(f, v)
char f[], *v[]; {
	int i, t, status;

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
	t = (status>>8) & 0377;
	return(t);
}

copy(s)
char s[]; {
	char *otsp;
	if ( tsp > &ts[500] )
		error("too many files \n");
	otsp = tsp;
	while(*tsp++ = *s++);
	return(otsp);
}



cunlink(f)
char *f;
{
	if (f==0)
		return(0);
	return(unlink(f));
}

splitup(){
	char in[200], fname[20];
	int buf[259];
	int i,fd,c;
	if( (fin=open("ratjunk", 0)) < 0)
		error("can't open ratjunk\n");
	while( gets(in) ){
		getname(in, fname);
		savename(fname);
		if( (fd = fcreat(fname, buf)) < 0)
			error("can't open %s", fname);
		puts(in,buf);
		while( ! endcard(in) ){
			gets(in);
			puts(in,buf);
		}
		fflush(buf);
		close(fd);
	}
	close(fin);
}

gets(s) char *s; {
	int c;
	while( (*s++=c=getchar()) != '\n' && c != '\0' );
	*s = '\0';
	return(c);
}

puts(s,b) char *s; int *b; {
	while( *s )
		putc(*s++, b);
}

savename(s) char *s; {
	rlist[nr++] = copy(s);
}

getname(s,f) char *s,*f; {
	int i,j,c;
   loop:
	while( *s == ' ' || *s == '\t' )
		s++;
	if( compar(s,"subroutine") ){ s =+ 10; goto bot; }
	else if( compar( s,"function") ){ s =+ 8; goto bot; }
	else if( compar(s,"real") ){ s =+ 4; goto loop; }
	else if( compar(s,"integer") ){ s =+ 7; goto loop; }
	else if( compar(s,"logical") ){ s =+ 7; goto loop; }
	else if( compar(s,"double") ){ s =+ 6; goto loop; }
	else if( compar(s,"precision") ){ s =+ 9; goto loop; }
	else if( compar(s,"complex") ){ s =+ 7; goto loop; }
	else if( compar(s,"block") ){
		s = "blockdata ";
		s[9] = (bdcount++) + '0';
		goto bot;
	}
	else {
		for(i=0; f[i]="MAIN.f"[i]; i++);
		return;
	}
   bot:
	while( *s == ' ' || *s == '\t' )
		s++;
	for(i=0; alphanum(s[i]); i++)
		f[i] = s[i];
	f[i++] = '.';
	f[i++] = 'f';
	f[i++] = '\0';
}

compar(s,t) char *s,*t; {
	while( *t )
		if( *s++ != *t++ )
			return(0);
	return(1);
}

alphanum(c) int c; {
	return( (c>='a' && c<='z')
		|| (c>='A' && c<='Z')
		|| (c>='0' && c<='9') );
}

endcard(s) char *s; {
	if( *s==0 )
		return(1);
	while( *s==' ' || *s=='\t' )
		s++;
	if( *s!='e' || *(s+1)!='n' || *(s+2)!='d' || *(s+3)!='\n' )
		return(0);
	return(1);
}

error(s1, s2){
	fout = 1;
	printf(s1,s2);
	putchar('\n');
	flush(1);
	cflag++;
}
