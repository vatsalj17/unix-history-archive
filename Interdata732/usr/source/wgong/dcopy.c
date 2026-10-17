#

char usage[]	"Usage: dcopy -v input-directory output-directory\n";
char source[]	"Input directory not found\n";
char target[]	"Output directory not found\n";


struct {
	int	dev;
	int	ino;
	int	flags;
	char	nlinks;
	char	uid;
	char	gid;
	char	size0;
	int	size1;
	int	addr[8];
	int	actime[2];
	int	mtime[2];
} stbuf;

char	*buff;			/* buffer allocated dynamically */
#define BUFFSIZE 1024*10

char	from[128], to[128];
char	*fend from;
char	*tend;
int	tdev, tino;		/* device & inode of target directory */
int	fdev;			/* device of source directory */

int	vflag;

main(argc, argv)
char **argv;
{
	register char *p, *q;

	if (--argc > 0 && **++argv == '-') {
		if (argv[0][1] == 'v')
			vflag++;
		argc--;
		argv++;
	}
	if (argc != 2) {
		write(1, usage, sizeof usage);
		exit(1);
	}
	p = argv[0];
	for (q = from; *q = *p++; *q++)
		;
	fend = q;
	if (stat(from, &stbuf) < 0 || (stbuf.flags&060000) != 040000) {
		write(1, source, sizeof source);
		exit(1);
	}
	fdev = stbuf.dev;
	p = argv[1];
	for (q = to; *q = *p++; *q++)
		;
	tend = q;
	if (stat(to, &stbuf) < 0 || (stbuf.flags&060000) != 040000) {
		write(1, target, sizeof target);
		exit(1);
	}
	tdev = stbuf.dev;
	tino = stbuf.ino;

	buff = sbrk(BUFFSIZE);

	close(0);
	close(2);
	dcopy(1);
}

dcopy(firsttime)
{
	register fi, fo, len;
	register t0, t1;
	int status;

	if (vflag) {
		puts(from);
		putchar('\n');
	}

	if (stat(from, &stbuf) < 0) {
		msg("can't find");
		return;
	}
	switch (stbuf.flags & 060000) {

	/* ordinary file */
	case 0000000:
		if ((fi = open(from, 0)) < 0)
			msg("can't open");
		else if ((fo = creat(to, stbuf.flags&07777)) < 0) {
			msg("can't create");
			close(fi);
		} else {
			while((len = read(fi, buff, BUFFSIZE)) > 0) {
				if (write(fo, buff, len) != len) {
					msg("write error");
					break;
				}
			}
			if (len < 0)
				msg("read error");
			close(fi);
			close(fo);
			chown(to, (stbuf.gid<<8) | stbuf.uid);
			smdate(to, stbuf.mtime);
		}
		break;

	/* special file */
	case 020000:
	case 060000:
		if (mknod(to, stbuf.flags, stbuf.addr[0]) < 0)
			msg("can't mknod");
		else {
			chown(to, (stbuf.gid<<8) | stbuf.uid);
			smdate(to, stbuf.mtime);
		}
		break;

	/* directory */
	case 040000:
		t0 = stbuf.mtime[0];
		t1 = stbuf.mtime[1];
		if (!firsttime) {
			if (stbuf.dev != fdev || 
			    (stbuf.dev == tdev && stbuf.ino == tino))
				break;
			if (!fork()) {
				execl("/bin/mkdir", "mkdir", to, 0);
				msg("can't exec mkdir");
				exit(1);
			}
			wait(&status);
			if (status)
				break;
			chmod(to, stbuf.flags&07777);
			chown(to, (stbuf.gid<<8) | stbuf.uid);
		}
		if ((fi = open(from, 0)) < 0)
			msg("can't open");
		else {
			while (read(fi, buff, 16) == 16)
				if ((buff[0] | buff[1]) != 0
				    && (buff[2] != '.'
					|| (buff[3] != '\0'
					&& (buff[3] != '.'
					    || buff[4] != '\0')))
				    && nxtpath(&buff[2])) {
					dcopy(0);
					lstpath();
				}
			close(fi);
		}
		if (!firsttime) {
			stbuf.mtime[0] = t0;
			stbuf.mtime[1] = t1;
			smdate(to, stbuf.mtime);
		}
		break;
	}
	return;
}

nxtpath(fn)
char *fn;
{
	register char *f, *t, *p;

	f = fend; t = tend;
	if (f[-1] != '/')
		*f++ = '/';
	if (t[-1] != '/')
		*t++ = '/';

	for (p = fn; p < &fn[14] && (*t = *f = *p++); ++t, ++f)
		if (t>=&to[sizeof(to)-1] || f>=&from[sizeof(from)-1]) {
			msg("name too long");
			return(0);
		}

	fend = f; tend = t;
	return(1);
}


lstpath()
{
	register char *f, *t;

	f = fend; t = tend;
	do
		--f;
	while (*--t != '/');
	*f = *t = '\0';
	fend = f; tend = t;
}

msg(s)
char *s;
{
	if (!vflag)
		puts(from);
	puts(" -- ");
	puts(s);
	putchar('\n');
}

puts(s)
char *s;
{
	register char *p;

	for (p = s; putchar(*p); p++)
		;
}
