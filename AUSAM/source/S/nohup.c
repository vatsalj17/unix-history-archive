main(argc, argv)
register int   argc;
register char *argv[];
{
	if(argc < 2)
	{
		prints(2, "Arg count\n");
	}
	else
	{
		argv++;
		argv[--argc] = 0;
		signal( 1, 1);
		signal( 2, 1);
		signal( 3, 1);
		execc( argv[0], argv);
		perror(argv[0]);
	}
	return 1;
}
