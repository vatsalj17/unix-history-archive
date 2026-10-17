#define	HTTC	0172472
#define	HTCS1	0172440

char rderr[] "read";
char skerr[] "seek";
char wrerr[] "write";
char t[] "/dev/rmt0";

int slave 0;
int rewoff 3;	/* rewind offline */ 

main(argc, argv)
int argc;
char *argv[];
{
	register int kmem, tape;

	nice(-64);	/* get some service */ 
	if(argc > 1)
		if((argv[1][0] >= '0') && (argv[1][0] <= '7'))
		{
			t[8] = argv[1][0];
			slave = t[8]-'0';
		}
		else
		{
			printf("illegal slave number '%c'\n", argv[1][0]);
			exit(1);
		}

	if((tape = open(t, 0)) < 0)
	{
		perror(t);
		exit(1);
	}
	if((kmem = open("/dev/kmem", 2)) < 0)
	{
		perror("/dev/kmem");
		exit(1);
	}
	close(tape);

	if(seek(kmem, HTTC, 0) == -1)
		perror(skerr);
	if(write(kmem, &slave, 2) != 2)
		perror(wrerr);

	if(seek(kmem, HTCS1, 0) == -1)
		perror(skerr);
	if(write(kmem, &rewoff, 2) != 2)
		perror(wrerr);
}
