/*
 *		export dev filename [recl]
 *
 * Copy a file to an OS-formatted disc.
 * Output file will be indexed, lrecl=recl, blksize 1/1.
 *
 * Note: no check is made to see whether the OS file already exists --
 *	this may cause duplicate filenames.
 */

/* OS directory structure */

struct dir {
	char	fnm[12];
	int	flba;
	int	llba;
	int	keyl;
	int	date[2];
	int	counts;
	char	atrb;
	char	dbsz;
	char	ibsz;
	char	flro;
	int	csec;
	int	filler;
};

struct dirblk {
	int nextdir;
	struct dir dirent[5];
};

/* OS disc information */

int disc;		/* fd for OS disc	*/
int bitmapsector;	/* bit map		*/
int bitmap[19584/32];
int dirsector;		/* directory block	*/
struct dirblk dirbuff;
struct dir *dirp;	/* &current dir entry	*/
int indexsector;	/* index block		*/
int indexbuf[64];
int *indexp;		/* &next free index	*/
int offset;		/* no. of chars in file */
int recl;		/* logical record length of file */

char buffer[256];


main(argc, argv)
char *argv[];
{
	register len, i;

	if (argc < 3)
		error("Usage: export dev filename [recl]");
	if ((disc = open(argv[1], 2)) < 0)
		error("Can't open %s", argv[1]);
	close(0);
	if (open(argv[2], 0) != 0)
		error("Can't find %s", argv[2]);

	if (argc < 4 || (recl = atoi(argv[3])) <= 0)
		recl = 256;

	getmap();
	oscreat(argv[2]);

	while ((len = read(0, buffer, 256)) > 0) {
		offset =+ len;
		while (len < 256)
			buffer[len++] = '\0';	/* pad last block with zeroes */
		oswrite(buffer);
	}
	if (len < 0)
		error("Input file read error");

	/* because of the peculiar way in which os/32 functions, if the
	 * logical file size is an exact multiple of 256 bytes, an extra
	 * block must be allocated. I don't know whether this is a bug
	 * or a feature.
	 */
	if (recl == 256)
		offset = (offset+255)&~0377;
	else
		offset = (offset/recl)*recl;
	if ((offset&0377) == 0)
		oswrite(buffer);

	osflush();
	putmap();
}

/*
 * Error exit
 */
error(s, x)
{
	printf(s, x);
	putchar('\n');
	exit(1);
}

/*
 * Initialize directory entry and first index block for new OS file
 */
oscreat(name)
{
	register struct dir *dp;

	dirp = dp = makdir();
	zero(dp, sizeof *dp);
	packfd(name, dp->fnm);
	dp->atrb = 0120;		/* indexed file */
	dp->ibsz = dp->dbsz = 1;	/* blksize 1/1 */
	dp->keyl = recl;		/* record length */

	indexsector = alloc();
	dp->flba = indexsector;
	indexbuf[0] = 0;
	indexp = &indexbuf[2];
}

/*
 * Zero a block of memory
 */
zero(addr, len)
char *addr;
{
	register char *p;
	register n;

	p = addr;
	for (n=len; n--; *p++ = '\0')
		;
}

/*
 * Pack a file name into OS directory format, converting lowercase
 * letters to uppercase.
 */
packfd(name, dest)
char *name, *dest;
{
	register char *p;
	register c, i;

	i = 0;
	for (p=name; (c = *p) != '\0' && c != '.'; p++)
		if (i < 8) {
			if (c >= 'a' && c <= 'z')
				c =+ 'A'-'a';
			dest[i++] = c;
		}
	while(i < 8)
		dest[i++] = ' ';
	if (c = '.')
		for (p++; (c = *p) != '\0'; p++)
			if (i < 11) {
				if (c>='a' && c<='z')
					c =+ 'A'-'a';
				dest[i++] = c;
			}
	while (i < 11)
		dest[i++] = ' ';
}


/*
 * Get an unused directory entry.  If none found, allocate a new
 * directory block. Return a pointer to the dir entry.
 */
makdir()
{
	register struct dir *dp;
	int sector;

	seek(disc, 8, 0);
	if (read(disc, &sector, sizeof sector) <= 0)
		error("Volume descriptor read error");
	if (sector == 0)
		error("No directory entries");

	while(sector) {
		dirsector = sector;
		xseek(disc, dirsector);
		if (xread(disc, &dirbuff, sizeof dirbuff))
			error("Directory block read error");
		for (dp = &dirbuff.dirent[0]; dp < &dirbuff.dirent[5]; dp++)
			if ((dp->atrb&020) == 0)
				return(dp);
		sector = dirbuff.nextdir;
	}

	xseek(disc, dirsector);
	dirsector = alloc();
	dirbuff.nextdir = dirsector;
	xwrite(disc, &dirbuff, sizeof dirbuff);
	zero(&dirbuff, sizeof dirbuff);
	return(&dirbuff.dirent[0]);
}

/*
 * Write one sector of the OS file
 */
oswrite(buff)
{
	register sector;

	sector = alloc();
	if (indexp >= &indexbuf[64])
		nextblk();
	*indexp++ = sector;
	xseek(disc, sector);
	xwrite(disc, buff, 256);
}

/*
 * Allocate the next index block
 */
nextblk()
{
	register sector;

	sector = alloc();
	indexbuf[1] = sector;
	xseek(disc, indexsector);
	xwrite(disc, indexbuf, sizeof indexbuf);
	indexbuf[0] = indexsector;
	indexsector = sector;
	indexp = &indexbuf[2];
}

/*
 * Update directory information and flush the last index block
 */
osflush()
{
	while (indexp < indexbuf[64])
		*indexp++ = 0;
	indexbuf[1] = 0;
	xseek(disc, indexsector);
	xwrite(disc, indexbuf, sizeof indexbuf);

	dirp->csec = offset/recl;
	dirp->llba = indexsector;
	xseek(disc, dirsector);
	xwrite(disc, &dirbuff, sizeof dirbuff);
}

/*
 * Read bit map
 */
getmap()
{
	seek(disc, 20, 0);
	if (read(disc, &bitmapsector, sizeof bitmapsector) <= 0)
		error("Volume descriptor read error");
	xseek(disc, bitmapsector);
	if (xread(disc, bitmap, sizeof bitmap))
		error("Bit map read error");
}

/*
 * Rewrite bit map
 */
putmap()
{
	xseek(disc, bitmapsector);
	xwrite(disc, bitmap, sizeof bitmap);
}

/*
 * Allocate one free sector from the bit map -- return sector number.
 */
alloc()
{
	register d, i, b;

	for (d=0; d<sizeof bitmap; d++)
		if (bitmap[d] != -1) {
			for (i=31; bitmap[d]&(b = 1<<i); i--)
				;
			bitmap[d] =| b;
			return(32*d + 31-i);
		}
	error("Disc full");
}

/*
 * OS disc i/o routines
 */
xseek(fd, sect)
{
	seek(fd, sect*256, 0);
}

xread(fd, addr, len)
{
	return(read(fd, addr, len) <= 0);
}
xwrite(fd, addr, len)
{
	if (write(fd, addr, len) != len)
		error("Write error");
}
