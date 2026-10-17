/*
 * Convert old format archives to new format
 */

char noar[]	"No archive file";
char noup[]	"Archive file not updated";
char nocreat[]	"Can't create archive file";
char badform[]	"Bad archive format";
char readio[]	"Archive file read i/o error";
char inpio[]	"Input file i/o error";
char tmpio[]	"Temp file i/o error";

char tfname[]	"/tmp/arXXXXXXX";

int	magic	0177555;	/* magic word to identify archive format file */
int	nmagic	0177545;	/* magic word for new archive format */

struct {			/* old archive file header */
	char fname[8];
	int  mtime[2];
	char uid;
	char mode;
	int  size;
} dir;

struct {			/* new archive file header */
	char nfname[14];
	char nuid;
	char ngid;
	long ndate;
	long nsize;
	int  nmode;
} ndir;

struct {			/* buffer for stat() */
	int	s_dev;
	int	s_ino;
	int	s_flags;
	char	s_nlinks;
	char	s_uid;
	char	s_gid;
	char	s_size0;
	int	s_size1;
	int	s_addr[8];
	int	s_atime[2];
	int	s_mtime[2];
} stbuf;


char *afname;			/* name of archive file */
int  afmode;			/* protect mode of archive file */
int  afdev;			/* device on which archive file resides */
int  afi, tfi;			/* fd for archive & temp files */
int  buff[128];
int  nerr;			/* count of errors */
int  vflag;			/* 'verbose'*/
int  zero;			/* zero for padding */

char gid;		/* group id of user */

main(argc, argv)
char *argv[];
{
	extern dexit();


	gid = getgid()&0377;
	/* set up interrupt exit */
	if ((signal(2, 1)&01) == 0) {
		signal(1, &dexit);
		signal(2, &dexit);
	}
	if (argc < 2) {
		printf("Usage: arcv file ...\n");
		exit(1);
	}
	while (--argc) {
		afname = *++argv;
		convert();
	}
}

/*
 * Convert an archive
 */
convert()
{
	mktemp();

	if (!getaf())
		error(noar);

	while (getdir()) {
		msg('c');
		copyfl();
	}
	copyback();

}

/*
 * Open archive file & check format
 */
getaf()
{
	if (stat(afname, &stbuf) < 0) {
		afmode = 0666;
		afi = -1;
		return(0);
	}
	if ((afi = open(afname, 0)) < 0)
		error("Can't open old archive file");
	afmode = stbuf.s_flags & 0777;
	afdev = stbuf.s_dev;

	readaf(sizeof magic);
	if (buff[0] != magic)
		error(badform);
	return(1);
}

/*
 * Create temporary file
 */
mktemp()
{
	register pid;
	register char *p;

	/* use process id to create unique name in /tmp directory */
	for (p = tfname; *p; p++)
		;
	pid = getpid();
	while (*--p == 'X') {
		*p = (pid&7) + '0';
		pid =>> 3;
	}
	/*
	 * create file & reopen in input/output mode
	 */
	if ((tfi = creat(tfname, 0600)) < 0)
		error("Can't create temp file");
	close(tfi);
	if ((tfi = open(tfname, 2)) < 0)
		error("Can't open temp file???");
	/* write 'magic word' archive header */
	write(tfi, &nmagic, sizeof nmagic);
}

/*
 * Read from archive file, checking for i/o errors
 */
readaf(size)
{
	register len;

	if ((len = read(afi, buff, size)) == size)
		return;

	if (size < 0)			/* i/o error */
		error(readio);
	error(badform);			/* premature eof */
}


/*
 * Read next file header
 */
getdir()
{
	register len;

	if ((len = read(afi, &dir, sizeof dir)) == sizeof dir)
		return(1);
	if (len == 0)			/* end of file */
		return(0);
	if (len < 0)			/* i/o error */
		error(readio);
	error(badform);			/* premature eof */
}

/*
 * Copy a file from the old archive to the temporary file
 */
copyfl()
{
	register size;
	register i;

	/* convert the header to the new format */
	for (i=0; i<sizeof(dir.fname); i++)
		ndir.nfname[i] = dir.fname[i];
	for (; i<sizeof(ndir.nfname); i++)
		ndir.nfname[i] = '\0';
	ndir.nuid = dir.uid;
	ndir.ngid = gid;
	ndir.nsize = dir.size;
	ndir.nmode = dir.mode|0600;
	ndir.ndate = dir.mtime[1];
	write(tfi, &ndir, sizeof ndir);

	/* copy the file */
	for (size = dir.size; size >= 512; size =- 512) {
		readaf(512);
		write(tfi, buff, 512);
	}
	if (size) {
		readaf(size);
		write(tfi, buff, size);
	}
	/* pad to halfword if necessary */
	if (size & 01) {
		seek(afi, 1, 1);
		write(tfi, &zero, 1);
	}
}

/*
 * Verbose option -- running commentary
 */
msg(ch)
{
	if (vflag)
		printf("%c %.8s\n", ch, dir.fname);
}


/*
 * All finished -- replace archive with new temporary file
 */
copyback()
{
	register fd, len;

	/* if any errors have occured, don't update the archive */
	if (nerr)
		error(noup);

	/* create archive file if it didn't exist before */
	if (afi < 0) {
		if ((fd = creat(afname, afmode)) < 0)
			error(nocreat);
		fstat(fd, &stbuf);
		afdev = stbuf.s_dev;
		close(fd);
	}
	else
		close(afi);

	/* prevent interrupts while moving back */
	signal(1, 1);
	signal(2, 1);

	/* if temp & archive files on same device, just do a mv */
	stat(tfname, &stbuf);
	if (stbuf.s_dev == afdev) {
		unlink(afname);
		if (link(tfname, afname) < 0)
			error(nocreat);
		unlink(tfname);
		chmod(afname, afmode);
		return;
	}

	/* copy temp file to archive file */
	if ((fd = creat(afname, afmode)) < 0)
		error(nocreat);
	seek(tfi, 0, 0);
	while ((len = read(tfi, buff, 512)) > 0)
		write(fd, buff, len);
	if (len < 0)
		error(tmpio);
	unlink(tfname);
}

/*
 * Error exit
 */
error(s)
{
	printf("%s\n", s);
	dexit();
}

/*
 * Delete temp file & exit
 */
dexit()
{
	if (tfi)
		unlink(tfname);
	exit(1);
}
