	/*	mkdir dirname [dirname...]	*/

int	uid;
int	gid;
struct {
	int	dev;
	int	inode;
	int	modes;
	char	nlinks;
	char	uid;
	char	gid;
	char	size0;
	int	size;
	int	addr[8];
	int	actime[2];
	int	modtime[2];
} statbuff;


main(argc, argv)
char **argv;
{
	uid = getuid() & 0377;
	gid = getgid() & 0377;

	while (--argc)
		make(*++argv);
}

make(name)
char *name;
{
	char buff[100];
	register char *pdir, *p, *q;
	register m;

	/*
	 * Copy filename to buffer -- leave p pointing at last char
	 */
	q = name;
	for (p=buff; *p = *q++; p++)
		if (p >= &buff[sizeof buff - 4]) {
			printf("%.64s... : name too long\n", name);
			return;
		}

	/*
	 * Check for write permission in parent directory
	 */
	pdir = parent(name);
	if (stat(pdir, &statbuff) < 0) {
		printf("Can't find %s\n", pdir);
		return;
	}
	if (uid) {
		m = statbuff.modes;
		if (uid == statbuff.uid)
			m =>> 6;
		else if (gid == statbuff.gid)
			m =>> 3;
		if ((m & 02) == 0) {
			printf("No write permission in %s\n", pdir);
			return;
		}
	}

	/*
	 * Make directory node, and change owner to caller's userid
	 */
	if (mknod(name, 0140777, 0) < 0) {
		perror(name);
		return;
	}
	if (chown(name, (gid<<8) + uid) < 0)
		perror(name);

	/*
	 * Link standard entries '.' and '..'
	 */
	p[0] = '/';
	p[2] = p[1] = '.';
	p[3] = '\0';
	if (link(pdir, buff) < 0) {
		printf("Can't make entry '..'\n");
		return;
	}
	p[2] = '\0';
	if (link(name, buff) < 0) {
		printf("Can't make entry '.'\n");
		return;
	}
}
/*
 * Given a filename, construct the name of its parent directory
 */
parent(fname)
char *fname;
{
	static char pbuff[100];
	register char *f, *p, *slash;
	register c;

	/*
	 * Copy filename to buffer, remembering position of last '/'
	 */
	slash = 0;
	f = fname;
	for (p = pbuff; c = *f++; p++) {
		if (c == '/')
			slash = p;
		*p = c;
	}

	/*
	 * no slash found - parent is "."
	 * otherwise - parent is "xxx/."
	 */
	if (slash == 0)
		return(".");
	slash[1] = '.';
	slash[2] = '\0';
	return(pbuff);
}
