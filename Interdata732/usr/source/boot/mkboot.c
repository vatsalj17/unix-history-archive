#

/*
 *	mkboot [-addr] file dev
 *
 * Copy <file> as a bootstrappable program onto device <dev> at
 * block address <addr> (default 9600).
 * Set up sector 0 of the device to look like an OS/MT volume
 * descriptor for the LSU.
 *
 */

#include "/usr/sys/filsys.h"

struct {		/* pseudo OS/MT volume descriptor */
	char	volname[4];	/* name of volume */
	int	atrb;		/* attributes? */
	int	dirp;		/* beginning sector of directory */
	int	osaddr;		/* address of OS image # 0 */
	int	ossize;		/* size of OS image */
	int	bmap;		/* beginning sector of bit map */
} vol;

int	baddr	9600;		/* block address on dev for boot program */
char	buff[512];
extern	errno;

main(argc, argv)
char **argv;
{
	register dev, file, nblocks, len;
	register char *p;

	if (--argc && *(p = *++argv) == '-') {
		if ((baddr = atoi(++p)) <= 0)
			error("Illegal block number");
		argc--;
		argv++;
	}
	if (argc < 2)
		error("Usage: mkboot [ - baddr ] file dev");

	if ((file = open(*argv, 0)) < 0)
		error("Can't open %s", *argv);
	if ((dev = open(*++argv, 2)) < 0)
		error("Can't write on %s", *argv);

	/*
	 * Make sure we aren't overwriting file system
	 */
	seek(dev, 1, 3);
	if (read(dev, buff, 512) != 512)
		error("Superblock read error");
	if (baddr < buff[0].s_fsize)
		error("Are you trying to clobber the file system??");

	/*
	 * Skip a.out header
	 */
	seek(file, 32, 0);
	seek(dev, baddr, 3);

	/*
	 * Copy file & count blocks copied
	 */
	nblocks = 0;
	while ((len = read(file, buff, 512)) > 0) {
		if (write(dev, buff, len) != len)
			error("Write error %d", errno);
		nblocks++;
	}
	if (len < 0)
		error("Read error %d", errno);

	/*
	 * Set up a pseudo OS/MT volume descriptor for the LSU
	 */
	vol.osaddr = baddr*2;
	vol.ossize = nblocks*2;

	seek(dev, 0, 0);
	if (write(dev, &vol, sizeof vol) != sizeof vol)
		error("Volume label write error %d\n", errno);
}

error(s, x)
{
	printf(s, x);
	putchar('\n');
	exit(1);
}
