#include	"scsi.h"

static
start(drive, st)
{
	struct scsi_i input;
	struct scsi_o output;

	input.bus_id = 1<<scsi_id;
	input.cmd[0] = 0x1B;
	input.cmd[1] = drive<<5;
	input.cmd[2] = 0;
	input.cmd[3] = 0;
	input.cmd[4] = st;
	input.cmd[5] = 0;
	if(scsiio(&input, &output, 0, "start") < 0)
		return(1);
	if(CHECK(output)){
		scsidump(&output);
		return(1);
	}
	return(0);
}

s_start(drive)
{
	return(start(drive, 1));
}

s_stop(drive)
{
	return(start(drive, 0));
}
