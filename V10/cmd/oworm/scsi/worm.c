#include	"scsi.h"
#include	"worm.h"

s_worm(drive, block)
{
	struct scsi_o o;
	register Superblock *s = (Superblock *)o.data;
	int i = 0;

	do {
		Fprint(1, "[%d]@%d: nfree=%d nextfree=%d\n", i++, block, s->nfree, s->nextffree);
		s_read(drive, block, 1, &o);
		block = s->nextsb;
	} while((block > 0) && (block < 1700000));
	Fprint(1, "last block = %d\n", block);
	Fflush(1);
	return(0);
}
