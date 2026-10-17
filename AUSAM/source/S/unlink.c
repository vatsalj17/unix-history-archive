/*
 *	unlink	unlinks anything including directories
 *
 *		unlink file ...
 */ 

main(argc, argv)
char **argv;
{
	int i;

	if(argc < 2)
	{
		write(1, "arg count\n", 10);
		exit(1);
	}

	for(i = 1; i < argc; i++)
		if(unlink(argv[i]))
			perror( argv[i] );
}
