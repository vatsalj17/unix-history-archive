#
/*
 ** cp oldfile newfile
 */ 

#define	LOCKING		02010
#define	FULL_LOCK	02000

main(argc, argv)
char **argv;
{
	static int buf[256];
	int mode, omode;
	register fold;

	if(argc != 3)
	{
		prints(2, "Usage: cp oldfile newfile\n");
		return(1);
	}

	close(0);
	close(1);

	if((fold = open(argv[1], 0)) < 0)
	{
		perror(argv[1]);
		return(1);
	}

	fstat(fold, buf);
	mode = omode = buf[2];	/* remember mode of old file */ 

	/* is target a directory? */ 

	if(stat(argv[2], buf+50) != -1)
	{
		if((buf[52]&060000) == 040000)
		{
			/** a directory **/ 

			register char *bp, *p1, *p2;
			char name[200];

			p1 = argv[1];
			p2 = argv[2];
			bp = name;
			while(*bp++ = *p2++);
			bp[-1] = '/';
			p2 = bp;
			while(*bp = *p1++)
				if(*bp++ == '/')
					bp = p2;
			argv[2] = name;
			if(stat(argv[2], buf+50) == -1)
				goto create;
		}

		mode = buf[52];	/* use mode of pre-existing file */ 

		if(buf[0] == buf[50] && buf[1] == buf[51])
		{
			prints(2, "Copying file to itself.\n");
			return(1);
		}
	}

create:
	{
		register fnew, n;

		if(mode&LOCKING)
			chmod(argv[2], 0600);

		if((fnew = creat(argv[2], (mode&~LOCKING))) < 0)
		{
			perror(argv[2]);
			return(1);
		}

		if((omode&LOCKING) == FULL_LOCK)
			readlock(fold);

		while((n = read(fold, buf, 512)) > 0)
			if(write(fnew, buf, n) != n)
			{
				perror(argv[2]);
				return(1);
			}

		if(n == -1)
		{
			perror(argv[1]);
			return(1);
		}
	}

	if(mode&LOCKING)
		chmod(argv[2], mode);

	return(0);
}
