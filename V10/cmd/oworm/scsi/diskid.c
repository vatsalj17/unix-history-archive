#include	"scsi.h"

s_diskid(drive)
{
	struct scsi_i input;
	struct scsi_o output;
	long c, l;

	input.bus_id = scsi_id;
	input.cmd[0] = 0xC2;
	input.cmd[1] = drive<<5;
	input.cmd[2] = 0;
	input.cmd[3] = 0;
	input.cmd[4] = 0;
	input.cmd[5] = 0;
	if(scsiio(&input, 0, &output, 1024, "diskid") < 0)
		return(1);
	if(CHECK(output)){
		prcheck(1, &output);
		return(1);
	}
	Fprint(1, "disk id for drive %d:\n", drive);
	scsiodump(output.data, 1024);
}
