/*
 *	size -- determine object size
 */ 

main(argc, argv)
register int argc;
register char **argv;
{
	register f;
	unsigned buf[010];
	long sum;

	if( argc == 1 )
	{
		*argv = "a.out";
		argc++;
		--argv;
	}
	while(--argc)
	{
		++argv;
		if((f = open(*argv, 0)) == -1)
		{
			perror(*argv);
			continue;
		}
		if(read(f, buf, 020) != 020)
		{
			perror(*argv);
			continue;
		}
		if(buf[0] > 0412 || buf[0] < 0407)
		{
			printf("Bad format: %s\n", *argv);
			close(f);
			continue;
		}
		printf("%s: ", *argv);
		printf("%u+%u+%u = ", buf[1], buf[2], buf[3]);
		sum =  buf[1];
		sum =+ buf[2];
		sum =+ buf[3];
		printf("%ld = 0%lo\n", sum, sum);
		close(f);
	}
}
