main(argc, argv)
char **argv;
{

/*	close(0); what dummy put this here? (commented out by KFH) */

	argv[argc] = 0;

	execv(argv[1], &argv[2]);
	perror( argv[1] );
	exit(1);
}
