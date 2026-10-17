#define	YES	0
#define	NO	1
#define	TIMEOUT	2

char	buf[100];


timedout()
{
	exit(TIMEOUT);
}

main(c,v)
char	**v;

{
	extern fout;

	fout = 2;
	v++;
	while(--c)
		printf("%s ",*v++);
	printf("? ");
	flush();
	signal(15,timedout);
	clktim(30);
	read(2,buf,sizeof buf);
	if(buf[0] != 'y') exit(NO);
	exit(YES);
}
