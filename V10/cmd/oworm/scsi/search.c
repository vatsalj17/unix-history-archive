#include	"scsi.h"

static
srch(drive, blk, nblk, blank)
{
	struct scsi_i input;
	struct scsi_o output;
	int addr;

	input.bus_id = scsi_id;
	input.cmd[0] = blank? 0x2C:0x2D;
	input.cmd[1] = drive<<5;
	input.cmd[2] = blk>>24;
	input.cmd[3] = blk>>16;
	input.cmd[4] = blk>>8;
	input.cmd[5] = blk;
	input.cmd[6] = 0;
	input.cmd[7] = 0;
	input.cmd[8] = 0;
	input.cmd[9] = 0;
	if(scsiio(&input, 0, &output, 6, "?search") < 0)
		return(-1);
	if(CHECK(output)){
		prcheck(1, &output);
		return(-1);
	}
	if(output.data[0] != 0x80){
		fprint(2, "%csearch error: ad %sval, class=0x%x code=0x%x\n",
			blank?'b':'w', (output.data[0]&0x80)?"":"in",
			(output.data[0]>>4)&7, output.data[0]&0xF);
		return(-1);
	}
	addr = ULONG((&output.data[2]))-blk;
	if(output.data[1]&0x01)
		if(addr < nblk)
			return(addr);
	return(nblk);
}

s_bsearch(drive, base, nblk)
{
	return(srch(drive, base, nblk, 1));
}

s_wsearch(drive, base, nblk)
{
	return(srch(drive, base, nblk, 0));
}
