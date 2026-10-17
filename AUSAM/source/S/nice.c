/* nice */
#ifndef unsw_orig
int	nicarg	4;
#endif
#ifdef unsw_orig
int	nicarg	17;
#endif

main(argc, argv)
register int argc;
register char *argv[];
{

	if(argc > 1 && argv[1][0] == '-') {
		nicarg = atoi(&argv[1][1]);
		argc--;
		argv++;
	}
	if(argc < 2) {
		prints(2, "usage: nice [ -n ] command\n");
		exit(1);
	}
	argc--;
	argv++;
	argv[argc] = 0;
	nice(nicarg);
	execc( argv[0], argv);
	prints(2, argv[0]);
	prints(2, ": Not found\n");
	exit(1);
}
