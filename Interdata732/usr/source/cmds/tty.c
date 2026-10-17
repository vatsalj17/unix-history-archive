/*
 * tty -- print tty number
 */

char ttyname[]		{ "ttyx\n" };

main()
{
	register c;

	if ((c = ttyn(0)) == 'x')
		write(1, "Not a tty\n", 10);
	else {
		ttyname[3] = c;
		write(1, ttyname, 5);
	}
}
