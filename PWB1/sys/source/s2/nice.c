/* nice */
int	nicarg	4;

main(argc, argv)
char **argv;
{

	if(argc > 1 && argv[1][0] == '-') {
		nicarg = atoi(&argv[1][1]);
		argc--;
		argv++;
	}
	if(argc < 2) {
		prints("usage: nice [ -n ] command\n");
		exit(1);
	}

	argv[argc] = 0;
	if(argv[argc-1] == -1)  /* don't ask why */
		argv[argc-1] = -2;
	nice(nicarg);
	pexec(argv[1], &argv[1]);
}
prints(s)
char *s;
{
	while(*s)
	write(2, s++, 1);
}
