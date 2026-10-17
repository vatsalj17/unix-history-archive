#include	"scsi.h"

s_read(drive, block, nblock, o)
	struct scsi_o *o;
{
	struct scsi_i input;

	input.bus_id = 1<<scsi_id;
	input.cmd[0] = 0x28;
	input.cmd[1] = drive<<5;
	input.cmd[2] = block>>24;
	input.cmd[3] = block>>16;
	input.cmd[4] = block>>8;
	input.cmd[5] = block;
	input.cmd[6] = 0;
	input.cmd[7] = nblock>>8;
	input.cmd[8] = nblock;
	input.cmd[9] = 0;
	if(scsiio(&input, o, 1024, "read") < 0)
		return(1);
	if(CHECK(*o)){
		scsidump(o);
		return(1);
	}
	return(0);
}
