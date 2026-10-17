/*
 *  import dev filename [newname]
 *
 *	- read file <filename> from OS-formatted disc <dev> and
 *	  copy it to file <newname> (default - standard output)
 */

struct osfile {		/* information about os file */
	int	iscontig;	/* contiguous file */
	int	size;		/* size of file (in bytes) */
	int	sector;		/* current sector */
	int	offset;		/* current offset in bytes */
	int	nleft;		/* sectors left in current block */
	int	*indexbuf;	/* buffer for index block */
	int	*indexp;	/* &current index */
	int	ibsize;		/* sectors per index block */
	int	dbsize;		/* sectors per data block */
} osfile;

struct dir {			/* OS file directory */
	char	fnm[12];	/* filename */
	int	flba;		/* sector no. of first block */
	int	llba;		/* sector no. of last block */
	int	keyl;		/* protect keys / lrecl */
	int	date[2];
	int	counts;
	char	atrb;		/* filetype */
	char	dbsz;		/* sectors per data block */
	char	ibsz;		/* sectors per index block */
	char	flro;
	int	csec;		/* no. of records in file */
	int	filler;
};

struct dirblk {		/* directory block */
	int nextdir;
	struct dir dirent[5];
};

int	disc;
char	buffer[512];

main(argc, argv)
char *argv[];
{
	register fd, len;

	if (argc < 3)
		error("Arg count");
	if ((disc = open(argv[1], 0)) < 0)
		error("Can't open %s", argv[1]);
	if ((osopen(&osfile, argv[2])) < 0)
		error("Can't find %s", argv[2]);
	fd = 1;
	if (argc > 3 && (fd = creat(argv[3], 0666)) < 0)
		error("Can't create %s", argv[3]);

	while ((len = osread(&osfile, buffer)) > 0)
		write(fd, buffer, len);
}

error(s, x)
{
	printf(s, x);
	putchar('\n');
	exit(1);
}

/*
 * Find the OS file and initialize information
 */
osopen(afp, name)
struct osfile *afp;
{
	register struct osfile *fp;
	register struct dir *dp;
	extern int *sbrk();

	fp = afp;
	if ((dp = getdir(name)) == 0)
		return(-1);

	switch((dp->atrb)>>5) {
	case 00:		/* contiguous file */
		fp->iscontig = 1;
		fp->size = (dp->llba - dp->flba + 1)*256;
		fp->sector = dp->flba;
		break;
	case 02:		/* indexed file */
		fp->ibsize = dp->ibsz;
		fp->dbsize = dp->dbsz;
		fp->size = dp->csec * (dp->keyl & 077777);
		fp->indexbuf = sbrk(fp->ibsize * 256);
		fp->indexbuf[1] = dp->flba;
		fp->indexp = &fp->indexbuf[fp->ibsize * 256/4];
		break;
	default:
		error("Illegal os file type %o", dp->atrb);

	}
}

/*
 * Find directory entry for given file
 */
getdir(name)
{
	char namebuff[11];
	struct dirblk dirbuff;
	int sect;
	register struct dir *dp;
	register i;
	register char *p, c;

	/* convert filename to form in which it appears in directory */
	i = 0;
	for (p=name; (c = *p) != '\0' && c != '.'; p++)
		if (i < 8) {
			if (c >= 'a' && c <= 'z') c =+ 'A'-'a';
			namebuff[i++] = c;
		}
	while (i < 8)
		namebuff[i++] = ' ';
	if (c = '.')
		for (p++; (c = *p) != '\0'; p++)
			if (i < 11) {
				if (c>='a' && c<='z') c =+ 'A'-'a';
				namebuff[i++] = c;
			}
	while (i < 11)
		namebuff[i++] = ' ';

	seek(disc, 8, 0);		/* pointer to first directory block */
	xread(disc, &sect, sizeof sect);

	while (sect) {
		xseek(disc, sect);
		if (xread(disc, &dirbuff, sizeof dirbuff))
			error("Directory block read error");
		for (dp = &dirbuff.dirent[0]; dp < &dirbuff.dirent[5]; dp++)
			if ((dp->atrb&020) && match(dp, namebuff))
				return(dp);
		sect = dirbuff.nextdir;
	}
	return(0);
}

/*
 * String comparison
 */
match(dirname, name)
char *dirname, *name;
{
	register char *p, *q;
	register count;

	p = dirname; q = name;
	for (count = 0; count < 11; count++)
		if (*p++ != *q++)
			return(0);
	return(1);
}

/*
 * Read a sector from OS file
 */
osread(afp, buff)
struct osfile *afp;
{
	register struct osfile *fp;
	register len;

	fp = afp;
	len = fp->size - fp->offset;
	if (len > 256)
		len = 256;
	if (len <= 0)
		return(len);
	if (!fp->iscontig && --fp->nleft < 0)
		nextblk(fp);

	xseek(disc, fp->sector++);
	if (xread(disc, buff, len))
		printf("Read error - offset %o", fp->offset);
	fp->offset =+ len;
	return(len);
}

/*
 * Find next block of indexed file
 */
nextblk(afp)
struct osfile *afp;
{
	register struct osfile *fp;

	fp = afp;
	if (fp->indexp >= &fp->indexbuf[fp->ibsize * 256/4]) {
		xseek(disc, fp->indexbuf[1]);
		if (xread(disc, fp->indexbuf, fp->ibsize * 256))
			error("Index block read error");
		fp->indexp = &fp->indexbuf[2];
	}
	fp->sector = *fp->indexp++;
	fp->nleft = fp->dbsize - 1;
}

xseek(fd, sect)
{
	seek(fd, sect*256, 0);
}

xread(fd, addr, len)
int *addr;
{
	extern int errno;
	register io;

	if ((io = read(fd, addr, len)) < 0)
		return(1);
	return(0);
}
