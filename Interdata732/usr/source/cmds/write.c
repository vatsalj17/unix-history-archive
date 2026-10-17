/*
 *	write user [tty-no]
 */

char	usage[]		= "Usage: write user [ttyno]";
char	utmp[]		= "/etc/utmp";
char	name[10];

struct {
	char	name[8];
	char	tty;
	int	date[2];
	int	unused;
} ubuf;

struct {
	int	dev;
	int	inumber;
	int	flags;
	char	nlinks;
	char	uid;
	char	gid;
	char	size0;
	int	size1;
	int	addr[8];
	int	actime[2];
	int	modtime[2];
} stbuf;

struct {
	char	ttyn[8];
	char	ttyno;
	char	zero;
} otty	= {
	"/dev/tty",
	'\0',
	'\0'
};

char	ltty;
char	mtty;
int	ttyf;

extern	finish();

main(argc, argv)
char *argv[];
{
	register	ufil;
	register char	*p, *q;
	register	nlflag = 1;
	char		ch;

	if (argc < 2)
		error(usage);
	if (argc > 2)
		ltty = argv[2][0];

	if ((mtty = ttyn(0)) == 'x' && (mtty = ttyn(2)) == 'x')
		mtty = 0;
	if ((ufil = open(utmp, 0)) < 0)
		error("Can't open utmp");
	while (read(ufil, &ubuf, sizeof ubuf) == sizeof ubuf) {
		if (ubuf.tty == mtty)
			for (p = ubuf.name, q = name; p < &ubuf.name[8] &&
			    *p != ' '; *q++ = *p++)
				;
		if (ltty && (ubuf.tty != ltty))
			continue;
		for (p = argv[1], q = ubuf.name; *p == *q; p++, q++)
			;
		if (q < &ubuf.name[8] && (*q != ' ' || *p != '\0'))
			continue;
		otty.ttyno = ubuf.tty;
	}
	if (otty.ttyno == '\0')
		error("Not logged in.");
	if (name[0] == '\0')
		name[0] = name[1] = '?';

	if ((ttyf = open(&otty, 1)) < 0 ||
		fstat(ttyf, &stbuf) < 0 ||
		(stbuf.flags&02) == 0)
			error("Permission denied.");

	wrs(ttyf, "\007\007\007\n\nMessage from ");
	wrs(ttyf, name);
	wrs(ttyf, " ...\n");
	signal(2, finish);
	while (read(0, &ch, 1) == 1) {
		switch (ch) {
			case '\n':
				nlflag = 1;
				break;

			case '!':
				if (nlflag) {
					shell();
					wrs(2, "!\n");
					continue;
				}
				break;

			case '\004':
				finish();

			default:
				nlflag = 0;
		}
		write(ttyf, &ch, 1);
	}
	finish();
}

shell()
{
	int	status;

	signal(2, 1);
	if (fork() == 0) {
		signal(2, 0);
		execl("/bin/sh", "sh", "-t", 0);
		error("Can't exec shell.");
	}
	wait(&status);
	signal(2, finish);
}

finish()
{
	wrs(ttyf, "EOT\n");
	exit(0);
}

error(s)
char	*s;
{
	wrs(2, s);
	wrs(2, "\n");
	exit(1);
}

wrs(fd, s)
char	*s;
{
	register char	*p;
	register	n;

	n = 0;
	for (p = s; *p; p++)
		n++;
	write(fd, s, n);
}
