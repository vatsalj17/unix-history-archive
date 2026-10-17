/*
 *
 * lcopy.c:
 *	copies a locked file into
 *	into a another file of
 *	mode 0604
 *
 *	if argc == 2 then
 *	the locked file is written
 *	onto standard output
 *
 */
main(argc,argv)
int argc;
char ** argv;
{
	int buf[256];
	register a, b, c;
	if((argc > 3) || (argc < 2))
	{
		prints(2,"usage:\n	lcopy source [destination]\n");
		exit(-1);
	}
	b = open(argv[1],2);
	if( b == -1 )
	{
		perror(argv[1]);
		exit(-1);
	}
	fstat( b , buf );
	if( (buf[2]&02010) == 0 )
	{
		prints(2,"source not a locked file ??\n");
		exit(-1);
	}
	if( argc == 3 )
	{
		a = creat(argv[2],0600);
		if (a == -1) 
		{
			perror(argv[2]);
			exit(-1);
		}
	}
	else
	{
		a = 1;
	}
	writelock(b);
	while((c = read(b,buf,512)) > 0)
		write(a,buf,c);
	creat(argv[1],2610);	/* must exist, so mode has little relevance */
}
