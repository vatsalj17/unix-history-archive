/*
 *	strip file [...]
 */

int	buf[512/sizeof(int)];
int	fi, fo;
char	*tf;

struct {		/* buffer for fstat() call */
	int	dev;
	int	ino;
	int	mode;
	int	pad[15];
} stbuf;

main(argc, argv)
char	**argv;
{
	extern int	delexit();

	tf = mktemp("/tmp/stmXXXXX");
	signal(1, delexit);
	signal(2, delexit);
	signal(3, delexit);

	while (--argc) {
		fi = fo = -1;
		strip(*++argv);
		if (fi > 0)
			close(fi);
		if (fo > 0)
			close(fo);
	}
	delexit();
}

delexit()
{
	unlink(tf);
	exit();
}

#define ERR(msg)	return(printf(error, file, msg))
char	error[] = "%s: %s\n";

strip(file)
register char	*file;
{
	register int	size, len;

	if ((fi = open(file, 0)) < 0)
		ERR("can't open");
	if ((fo = creat(tf, 0600)) < 0)
		ERR("can't create temp file");
	if (read(fi, buf, 8*sizeof(int)) != 8*sizeof(int))
		ERR("bad format");
	switch (buf[0]) {		/* Check a.out magic word */
		default:
			ERR("bad format");

		case 0407:
		case 0410:
		case 0411:
			;
	}
	if (buf[7])		/* Already stripped? */
		return;
	size = buf[1] + buf[2];		/* text + data size */
	buf[4] = 0;			/* symtab size */
	buf[7] = 1;			/* no relocation bits */
	if (write(fo, buf, 8*sizeof(int)) != 8*sizeof(int))
		ERR("temp file write error");

	while (size > sizeof(buf)) {
		if (read(fi, buf, sizeof(buf)) != sizeof(buf))
			ERR("unexpected eof");
		if (write(fo, buf, sizeof(buf)) != sizeof(buf))
			ERR("temp file write error");
		size -= sizeof buf;
	}
	if (size) {
		if (read(fi, buf, size) != size)
			ERR("unexpected eof");
		if (write(fo, buf, size) != size)
			ERR("temp file write error");
	}
	fstat(fi, &stbuf);
	close(fo);
	if ((fo = creat(file, stbuf.mode)) < 0)
		ERR("can't rewrite");
	close(fi);
	if ((fi = open(tf, 0)) < 0)
		ERR("can't reopen temp file");
	while ((len = read(fi, buf, sizeof buf)) > 0)
		if (write(fo, buf, len) != len)
			ERR("temp file write error");
	if (len < 0)
		ERR("temp file read error");
}
