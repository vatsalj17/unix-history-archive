#
/*
 *	Remove symbol table and relocation bits from a.out files
 */ 

char tmpfil[] "/tmp/stmaXXXXX";

main(argc, argv)
char **argv;
{
	if(argc == 1)
		strip("a.out");
	else
		while(--argc > 0)
			strip(*++argv);
}

strip(s)
char *s;
{
	register int fi, fo, c;
	unsigned buf[256];
	long len;

	if( (fi = open(s, 0)) == -1 )
	{
		perror(s);
		return;
	}
	if( (fo = creat(mktemp(tmpfil), 0600)) == -1 )
	{
		perror(tmpfil);
		goto out;
	}
	read(fi, buf, 512);
	if( *buf > 0412 || *buf < 0407)
	{
		printf("Improper format: %s\n", s);
		goto out;
	}

	len = 16 + buf[1] + buf[2];	/* header + text + data */
	buf[4] = 0;			/* no symbol table */
	buf[7] =| 1;			/* no relocation */
	c = len > 512 ? 512 : len;	/* short file ?? */

	for(;;)
	{
		if( write(fo, buf, c) != c )
		{
			perror("Write error");
			goto out;
		}
		if( (len =- c) <= 0 )
			break;
		if( (c = read(fi, buf, 512)) <= 0 )
			break;
		if( c > len )
			c = len;
	}
	if( len )
	{
		printf("Header and files size disagree: %s\n", s);
		goto out;
	}

	close(fi);
	close(fo);

	if( (fi = open(tmpfil, 0)) == -1 )
	{
		perror(tmpfil);
		goto out;
	}
	if( (fo = creat(s, 0)) == -1 )
	{
		perror(s);
		goto out;
	}
	while((c = read(fi, buf, 512)) > 0)
		if( write(fo, buf, c) != c )
		{
			perror("Write error");
			goto out;
		}
out:
	close(fi);
	close(fo);
	unlink(tmpfil);
}
