/*
 *	This command enables a user to break a connection that
 *	that has been made by 'log'.
 *	Also could be used to stop 'watchers' !!
 *	The argument to this command is merely the name of either
 *	special file that was involved in the redirection.
 *	This command like log must also be set-uid to root so that
 *	the connect system call will work.
 *
 *					ian j    AGSM    aug 1979
 */

char lname[] "/dev/xxxxxxxxxxxxxxx";

main(argc,argv)
int argc;
char **argv;
{
	register char *p, *q;
	register int fd;

	if( argc != 2 )
	{
		prints(2,"useage:  disconnect char-special-file\n");
		exit(1);
	}
	p = &lname[5];
	q = argv[1];
	while( *p++ = *q++ );	/* make correct file name */
	if( (access(lname,6) == -1 ) ||
	    ((fd=open(lname,2)) == -1) ||
	    (disconnect(fd) == -1)  )
	{
		perror(argv[1]);
		exit(1);
	}
	prints(2,"offending process may now be killed\n");
}
