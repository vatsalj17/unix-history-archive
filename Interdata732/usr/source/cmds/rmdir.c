/*
 * rmdir dirname [dirname...]
 */

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
	if (argc<2) {
		printf("Usage: rmdir dirname [dirname...]\n");
		exit(1);
	}

	gid = getgid() & 0377;
	uid = getuid() & 0377;

	while (--argc)
		rmdir(*++argv);
}

rmdir(pathname)
char *pathname;
{
	char buff[100];
	char dirbuff[16];
	register char *p, *q, *pdir;
	register fd, m;

	/*
	 * Copy filename to buffer -- leave p pointing at last char
	 */
	q = pathname;
	for (p=buff; *p = *q++; p++)
		if (p >= &buff[sizeof buff - 4]) {
			printf("%.64s... : name too long\n", pathname);
			return;
		}

	/*
	 * Check for write permission in parent directory
	 */
	pdir = parent(pathname);
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
	 * make sure it's empty first
	 */
	if ((fd = open(pathname, 0)) < 0) {
		printf("%s: can't open\n", pathname);
		return;
	}
	while ( (m = read(fd, dirbuff, sizeof dirbuff)) == sizeof dirbuff )
		if ( (dirbuff[0] | dirbuff[1]) && !dotname(&dirbuff[2]) ) {
			printf("%s: not empty\n", pathname);
			close(fd);
			return;
		}
	close(fd);
	if (m != 0) {
		perror(pathname);
		return;
	}

	/*
	 * remove standard entries . and ..
	 */
	p[0] = '/';
	p[1] = '.';
	p[2] = '\0';

	if (unlink(buff) < 0) {
		perror(buff);
		return;
	}

	p[2] = '.';
	p[3] = '\0';

	if (unlink(buff) < 0) {
		perror(buff);
		return;
	}

	/*
	 * now remove the directory itself
	 */
	if (unlink(pathname) < 0)
		perror(pathname);
}

/*
 * Check for standard names '.' or '..'
 */
dotname(name)
char *name;
{
	register char *p;

	p = name;
	return (p[0] == '.' &&
		(p[1] == '\0' ||
			(p[1] == '.' && p[2] == '\0')));
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
