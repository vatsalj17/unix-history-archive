/*
 *	Time a command
 */

/*
 *	Modified not to use "printf" & floating point
 *	(for smaller program), and to simulate "date"
 *	if called with no arguments.
 *
 *		piers lauder -- Sep '77
 */

struct { int hiint; unsigned loint; };

#define	HZ	50

struct	{
	long	utime;
	long	stime;
	long	cutime;
	long	cstime;
} timbuf;

long	time();
int	f;

main(argc, argv)
char **argv;
{
	long start;
	register pid;
	extern fout;
	int status;

	if(argc == 1)  {
		execl( "/bin/date" , "date" , 0 );
		return;
	}

	f++;
	fout = 2;
	start = time();
	if((pid = fork()) == -1) {
		tprintf("Try again.\n");
		exit(0);
	}
	if(pid == 0) {
		argv++;
		argv[argc - 1] = 0;
		execc(argv[0], argv);
		tprintf("Command not found.\n");
		seek(0,0,2);
		exit(0);
	} else {
		signal(2, 1);
		while(pid != wait(&status));
		if(status&0377) {
			tprintf("Command terminated abnormally.\n");
			seek(0,0,2);
		}
		times(&timbuf);
		if ( (timbuf.stime = (time() - start) * HZ) == 0 )
			timbuf.stime.loint = ( timbuf.cutime.loint + timbuf.cstime.loint ) *2;
		ptime( "real " , &timbuf.stime );
		ptime( "user " , &timbuf.cutime);
		ptime( "sys  " , &timbuf.cstime);
		exit(0);
	}
}

ptime( s , t )
  char *s;
  register long *t;
{

  tprintf( s );
  f = 0;
  tprintf( "%d:%d:%d.%d\n" , (*t =/ 60 , t)->loint , (*t =/ 60 , t)->loint%60 , (*t =/ HZ , t)->loint%60 , (t->loint%HZ)*2 );
}

tprintf( s , a )
  register char *s;
  int a;
{
	register *ap = &a;
	char cc;
	register char *c = &cc;

  while ( *c = *s++ )
	if ( *c == '%' )  {
		tprint1n( (*ap)/10 );
		if ( *++s == '.' )  f++;
		tprint1n( (*ap++)%10 );
	}else
		write( 2 , f ? c : " " , 1 );
}

tprint1n( n )
  char n;
{
  write( 2 , ((n && ++f) || f) ? ( n =+ '0' , &n ) : " " , 1 );
}
