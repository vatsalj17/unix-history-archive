#include	<local-system>
#include	<stat16.h>

/*
 *	this creation just reads raw tape file specified as
 *	1st argument.  it then reports on record size etc.  on
 *	double tapemark it stops.
 */ 

#define	NERR	10	/* NERR consecutive ==> giveup */

char buf[44*512];
int flag 0;
int err 0;

catch()
{
	flag = 1;
}

main(argc, argv)
int argc;
char **argv;
{
	char register *iold, *i, *nrec;
	int infd;
	struct statbuf sbuf;



	signal(1, catch);
	if((signal(2, 1)&01) == 0)
		signal(2, catch);
	if((signal(3, 1)&01) == 0)
		signal(3, catch);
#ifdef	BASSER
	nice(10);
#endif	BASSER
	if(argc != 2)
	{
		printf("Usage: taprd file\n");
		return 1;
	}
	if((infd = open(argv[1], 0)) < 0)
	{
		perror(argv[1]);
		return 1;
	}
	fstat(infd, &sbuf);
	if((sbuf.sb_flags&IFTYP) != IFCHR)
	{
		printf("%s not a raw device !!\n", argv[1]);
		return 1;
	}
	nrec = 0;
	iold = read(infd, buf, sizeof buf);
	for(; ; )
	{
		nrec++;
		i = read(infd, buf, sizeof buf);
		if(i == iold && i == 0)
			goto fini;	/* considered end-of-tape */
		if((i == iold) && (i==017777) && (nrec==NERR) )
		{
			printf("%d consecutive errors: EOT assumed\n",NERR);
			return 1;
		}
		if((i == iold) && !flag)
			continue;
		if(iold)
		{
			if(iold != 0177777)
			{
				printf("%6.6l. * %6.6o(%6.6l.)\n",
					nrec, iold, iold);
			}
			else
			{
				printf("%6.6l. *  ERR  \n", nrec);
				err++;
				if(nrec > 1)
					return err;
			}
		}
		else
		{
			printf("\tTM\n");
		}
		nrec = 0;
		iold = i;
		if(flag)
		{
			printf("Interrupted\n");
			return 1;
		}
	}
fini:
	printf("\tEOT\n");
	return err;
}
