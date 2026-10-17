/*
 * mesg -- set current tty to accept or forbid writes
 *
 *	mesg [y] [n]
 *		y allow messages
 *		n forbid messages
 */

int	sbuf[40];

main(argc, argv)
char *argv[];
{
	register char *tty;

	tty = "/dev/ttyx";
	tty[8] = ttyn(1);
	if(stat(tty, sbuf) == -1)
	{
		perror(tty);
		return 1;
	}
	if(argc < 2)
	{
		if(sbuf[2] & 02)
			goto no;
		else
			goto yes;
	}
	if(*argv[1] == 'y')
		goto yes;

no:
	if(chmod(tty, 0600) == -1)
		goto bad;
	goto was;

yes:
	if(chmod(tty, 0602) == -1)
		goto bad;

was:
	if(sbuf[2] & 02)
		write(2, "was y\n", 6);
	else
		write(2, "was n\n", 6);
	return 0;

bad:
	perror("cannot change");
	return 1;
}
