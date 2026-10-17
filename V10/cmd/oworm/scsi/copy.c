#include	"scsi.h"
#include	"worm.h"

s_copy(sdr, sbase, nblocks, ddr, dbase)
{
	struct scsi_o o;
	char buf[256];
	int wr, unwr;
	int n;
	long nb = nblocks;
	long t1, t2;
	long goo;
#define		TALK		10000
	extern char *ctime();

	fprint(1, "copying drive %d[%d-%d] to drive %d[%d-%d]\n", sdr,
		sbase, sbase+nblocks-1, ddr, dbase, dbase+nblocks-1);
	time(&t1);
	goo = sbase/TALK;
	while(nblocks > 0){
		if(sbase/TALK != goo){
			goo = sbase/TALK;
			time(&t2);
			print("doing block %ld at %s", goo*TALK, ctime(&t2));
		}
		n = min(256, nblocks);
		wr = s_wsearch(sdr, sbase, n);
		if(wr < 0)
			break;
		if(wr == n){
			sbase += n;
			dbase += n;
			nblocks -= n;
			continue;
		}
		sbase += wr;
		dbase += wr;
		n -= wr;	/* number of blocks */
		nblocks -= wr;
		unwr = s_bsearch(sdr, sbase, n);
		if(unwr < 0)
			break;
		/*print("writing %d-%d\n", sbase, sbase+unwr-1);/**/
		if(s_copy1(sdr, sbase, unwr, ddr, dbase))
			break;
		sbase += unwr;
		dbase += unwr;
		nblocks -= unwr;
	}
	time(&t2);
	t2 -= t1;
	if(t2 == 0) t2 = 1;
	fprint(1, "%ds: ", t2);
	if(nblocks){
		fprint(1, "copy buggered up: sbase=%d nblks=%d dbase=%d\n",
			sbase, nblocks, dbase);
		return(1);
	}
	fprint(1, "%d blocks at %.1fKB/s\n", nb, nb/(float)t2);
	return(0);
}

s_copy1(sd, sb, n, dd, db)
{
	struct scsi_i input;
	struct scsi_o o;

	input.bus_id = scsi_id;
	input.cmd[0] = 0x18;
	input.cmd[1] = sd<<5;
	input.cmd[2] = 0;
	input.cmd[3] = 0;
	input.cmd[4] = 20;
	input.cmd[5] = 0;
	input.data[0] = 0x10;	/* copy */
	input.data[1] = 0;
	input.data[2] = 0;
	input.data[3] = 0;
	input.data[4] = (scsi_id<<5)|sd;
	input.data[5] = (scsi_id<<5)|dd;
	input.data[6] = 0;
	input.data[7] = 0;
	input.data[8] = n>>24;
	input.data[9] = n>>16;
	input.data[10] = n>>8;
	input.data[11] = n;
	input.data[12] = sb>>24;
	input.data[13] = sb>>16;
	input.data[14] = sb>>8;
	input.data[15] = sb;
	input.data[16] = db>>24;
	input.data[17] = db>>16;
	input.data[18] = db>>8;
	input.data[19] = db;
	if(scsiio(&input, 20, &o, 0, "media") < 0)
		return(1);
	if(CHECK(o)){
		prcheck(1, &o);
		s_sense(sd, 1);
		return(1);
	}
	return(0);
}
