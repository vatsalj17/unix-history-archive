#include	<local-system>

/*
 *	mv [-d] file1 file2
 *
 *	unlink file2
 *	link file1 file2
 *	unlink file1
 */

struct sbuf
{
	int dev;
	int inum;
	int imode;
#ifdef	AUSAM
	unsigned uid;
#endif
	char nlink;
#ifndef	AUSAM
	char uid;
	char gid;
#endif
	char siz0;
	char siz1;
	int addr[8];
	int adate[2];
	int mdate[2];
};

int stbuf[42];
char strbuf[70];

main(argc,argv)
int argc;
register char **argv;
{
	char *argp1,	*argp2,	*argp3,	*argp4;
	char *p,	*p1,	*p2;
	char place[100];
	int b,	i,	status;
#ifdef	AUSAM
	unsigned u_id;
#endif	AUSAM

	/*
	 *	check for correct number of arguments
	 */
	if(argc != 3)
	{
		prints(2, "Usage: mv name1 name2\n");
		return 1;
	}
	/*
	 *	is there anything to do?
	 */
	argp3 = argv[1];
	argp4 = argv[2];
#ifndef	AUSAM
	if( stat(argv[1], stbuf) == -1)
#else
	if( newstat(argv[1], stbuf) == -1)
#endif	AUSAM
	{
		perror( argv[1] );
		return 1;
	}
	/*
	 *	yes, there is a source.
	 *	check whether file or directory
	 */
	if( (stbuf[0].imode & 060000 ) == 040000)
	{
		/*
		 *	The source is a directory, so
		 *	we do lots of checking and
		 *	messing around so as not to get
		 *	into trouble.  This patch of
		 *	code contains administrative
		 *	policies rather than system
		 *	restrictions.
		 */
#ifndef	AUSAM
		if( stat(argv[2], stbuf) != -1)
#else
		if( newstat(argv[2], stbuf) != -1)
#endif	AUSAM
		{
			prints(2, "Directory target exists.\n");
			return 1;
		}
		argp1 = argv[1];
		argp2 = argv[2];
		while(*argp1 == *argp2)
		{
			argp1++;
			if(*argp2++ == 0)
			{
				prints(2, "???\n");
				return 1;
			}
		}
		while(*argp1)if(*argp1++ == '/')
		{
			prints(2, "Directory rename only\n");
			return 1;
		}
		while(*argp2)if(*argp2++ == '/')
		{
			prints(2, "Directory rename only\n");
			return 1;
		}
		if(*--argp1 == '.')
		{
			prints(2, "Can't move . or .. !\n");
			return 1;
		}
		if( access("",2) )
		{
			perror(".");
			exit(1);
		}
	}
	else
	{
		/*
		 *	the source is a file.
		 */
#ifndef	AUSAM
		setuid(getuid());
		if( stat(argp4, &stbuf[2]) != -1 )
#else
		setuid(u_id = getreal());
		if(newstat(argp4, &stbuf[2]) != -1)
#endif	AUSAM
		{
			if( (stbuf[2].imode & 060000) == 040000 )
			{
				argp2 = strbuf;
				while(*argp2++ = *argp4++);
				argp2[-1] = '/';
				argp4 = argv[1];
				argp1 = argv[1];
				while(*argp4)
				{
					if(*argp4++ == '/')
						argp1 = argp4;
				}
				while(*argp2++ = *argp1++);
				argp4 = strbuf;
			}
#ifndef	AUSAM
			if(stat(argp4, &stbuf[2]) != -1)
#else
			if(newstat(argp4, &stbuf[2]) != -1)
#endif	AUSAM
			{
				if((stbuf[0]==stbuf[2]) && (stbuf[1]==stbuf[3]))
				{
					prints(2, "Files are identical.\n");
					return 1;
				}
				if( access(argp4,2) )
				{
					printf("%s: mode %o: (y-n) ", argp4,
						stbuf[2].imode & 07777);
					i = b = getchar();
					while(b != '\n' && b != '\0')
						b = getchar();
					if(i != 'y')
						return 0;
				}
				if(unlink(argp4) < 0)
				{
					perror(argp4);
					return 1;
				}
			}
		}
	}
	if(link(argp3, argp4) == -1)
	{
		i = fork();
		if(i == -1)
		{
			perror("try again");
			return 1;
		}
		if(i)
		{
			while(waitx(&status) != i);
		}
		else
		{
			p = place;
			p1 = p;
			while(*p++ = *argp3++);
			p2 = p;
			while(*p++ = *argp4++);
			execl("/bin/cp","cp", p1, p2, 0);
			perror("cp");
			exit(1);
		}
		if((status & 0377) != 0)
		{
			prints(2, "?\n");
			return 1;
		}
		if(status != 0)
			return 1;
	}
	if(unlink(argp3) < 0)
	{
		perror(argp3);
		return 1;
	}
}
