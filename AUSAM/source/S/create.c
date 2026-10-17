/*
 *	create - create specified files
 */ 
main(argc, argv)
register char **argv;
{
	register i;
	register fd;

	if(argc < 2)
	{
		prints(2, "Usage: create file ...\n");
		exit(1);
	}

	for(i = 1; i < argc; i++)
		if((fd = creat(argv[i], 0600)) < 0)
		{
			perror(argv[i]);
		}
		else
		{
			close(fd);
		}
	return 0;
}
