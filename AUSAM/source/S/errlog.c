#
/*
 *	system error logger
 *
 *	- reads from system error emitting device
 *	  and writes to error log file.
 *
 *	- will buffer output & continue reading
 *	  if write returns error, then sleeps
 *	  until error goes away.
 *
 *	- will complain if errors too frequent.
 *
 *	- will ask for crash details on startup
 *	  if relevant.
 *
 *		    by
 *	       piers lauder
 *	DEPT. OF COMPUTER SCIENCE
 *	   UNIVERSITY OF SYDNEY
 *		 feb 1978
 */ 

#include	<local-system>
#include	<stat.h>

#define	NOTNICE		-90	/* run at extremely high priority */
#define	DATESTAMP	10	/* don't date stamp more often than this (secs.) */
#define	MAXERR		10	/* max. allowable errors in DATESTAMP period */
#define	SIGTERMINATE	14


char errlogf[]	"/usr/adm/errlog";
char errdevf[]	"/dev/errlog";
char openf[]	"cannot open %s\n";
char panicm[]	"\nerrlogger - too many errors!\n";

int totalerrs;
int ou;
char inbuf[80];
char oubuf[512];
char *oubufp oubuf;
long this, last, time();


main(argc, argv)
char **argv;
{
	register in, n;
	register *bp;
	int finish();
	struct statb sb;

	close(2);
	if((in = open(errdevf, 0)) < 0)
	{
		printf(openf, errdevf);
		return(-1);
	}
	if((ou = open(errlogf, 2)) < 0)
	{
		printf(openf, errlogf);
		return(-1);
	}
	bp = inbuf;
	fstat(ou, &sb);
	if(sb.i_size1 < 2 && sb.i_size0 == 0)
		goto crash;
	seek(ou, -2, 2);
	read(ou, bp, 2);
	if(*bp != -1)
	{	/* crashed */ 
    crash:
		printf("please type in why system crashed\n(^d to end)\n");
		copyout("\nSYSTEM CRASH\n", 14);
		readit(0);
		copyout("\n", 1);
	}
	else
	{
		seek(ou, -2, 2);
		copyout("\n\n", 2);
		seek(ou, -2, 2);
	}
	close(0);

	nice(NOTNICE);

	if(fork())
	{
		printf("error logger started\n");
		return 0;
	}

	signal(2, 1);
	signal(3, 1);
	signal(SIGTERMINATE, finish);

	last = 0;
	totalerrs = 0;

	readit(in);

	finish();
}


readit(in)
register in;
{
	register n;
	register errcount = 0;

	while((n = read(in, inbuf, (sizeof inbuf)-1)) > 0)
		if(n > 1)
		{
			if(++errcount > MAXERR)
			{
				printf(panicm);
				sleep(DATESTAMP);
			}
			totalerrs++;
			if((((this = time())-last) > DATESTAMP) && (in || last == 0))
			{
				copyout(ctime(this), 25);
				last = this;
				errcount = 0;
			}

			if(inbuf[n-1] != '\n')
				inbuf[n++] = '\n';

			copyout(inbuf, n);
		}
}


finish()
{
	copyout("\377\377", 2);	/* system termination */ 
	printf("error logger completed\n");
	if(totalerrs)
		printf("%d errors logged\n", totalerrs);
	exit(0);
}


copyout(cp, n)
register char *cp;
register n;
{
	register char *bp = oubufp;
	int z;

	while(bp < &oubuf[sizeof oubuf] && n--)
		*bp++ = *cp++;

	oubufp = oubuf;
	z = bp-oubuf;

	while(write(ou, oubuf, z) != z)
		if(n > 0)
			sleep(1);
		else
		{
			oubufp = bp;
			break;
		}

	if(n > 0)
		copyout(cp, n);

}


#ifdef	SPRINTF
#define		NUMBERS

#include	<sprintf.h>
#endif	SPRINTF
