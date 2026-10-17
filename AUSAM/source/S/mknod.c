main(argc, argv)
int argc;
char **argv;
{
	register int m, a, b;

	if(argc != 5)
	{
		printf("arg count\n");
		goto usage;
	}
	if(*argv[2] == 'b')
		m = 0160666;
	else if(*argv[2] == 'c')
		m = 0120666;
	else
		goto usage;
	a = number(argv[3]);
	b = number(argv[4]);
	if( a == -1 || b == -1 )
		goto usage;
	if(mknod(argv[1], m, (a<<8)|b) < 0)
	{
		perror("mknod");
		return 1;
	}
	return 0;

usage:
	printf("usage: mknod name b/c major minor\n");
	return 1;
}

number(s)
register char *s;
{
	register int n = 0;
	register char c;

	while(c = *s++)
	{
		if( c<'0' || c>'9' )
			return -1;
		n = n*10 + c-'0';
		if( n < 0 )
			return -1;
	}
	return n;
}
