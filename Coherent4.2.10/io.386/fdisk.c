/* $Header: /ker/io.386/RCS/fdisk.c,v 2.4 93/10/29 00:58:31 nigel Exp Locker: nigel $ */
/*
 * Fixed disk configuration.
 *
 * $Log:	fdisk.c,v $
 * Revision 2.4  93/10/29  00:58:31  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.3  93/08/19  04:02:21  nigel
 * Nigel's R83
 * 
 */

#include <sys/coherent.h>
#include <sys/uproc.h>
#include <sys/errno.h>
#include <sys/fdisk.h>
#include <sys/buf.h>
#include <sys/con.h>
#include <sys/file.h>

/* fdisk(dev, fp)	--	Fixed Disk Configuration
 * dev_t dev;
 * struct fdisk_s *fp;
 *
 *	Input:	dev = special device to read partition information from
 * 		fp  = pointer to memory-resident partition info (to update)
 *
 *	Action:	Open special device for reading.
 *		Read first block from the device.
 *		If valid signature present on block,
 *			copy partition information to memory
 *
 *	Return:	1 = partition information successfully updated
 *		0 = failure (could not read block, or bad signature)
 */

int
fdisk (dev, fp)
dev_t dev;
register struct fdisk_s *fp;
{
	register struct hdisk_s *hp;
	BUF *bp;
	int s, i;
	int ret = 0;

	s = sphi();
	(void) dopen (dev, IPR, DFBLK, NULL);

	if (get_user_error () == 0) {		/* special device now open */

		if ((bp = bread (dev, (daddr_t) 0, BUF_SYNC)) != NULL) {
			/* data read */
			/* buffer cache is in kernel data space */
			hp = bp->b_vaddr;

			if (hp->hd_sig == HDSIG) {
				for (i = 0 ; i < NPARTN ; i ++)
					* fp ++ = hp->hd_partn [i];
				ret = 1;
			}
			brelease (bp);
		}
		dclose (dev, IPR, DFBLK, NULL);
	} else
		printf ("fdisk : driver failed open = %d\n", get_user_error ());

	spl (s);
	return ret;
}
