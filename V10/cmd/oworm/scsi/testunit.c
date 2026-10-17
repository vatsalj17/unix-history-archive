#include	"scsi.h"

s_testunit(id, drive)
{
	struct scsi_i input;
	struct scsi_o output;

	input.bus_id = id;
	input.cmd[0] = 0x0;
	input.cmd[1] = drive<<5;
	input.cmd[2] = 0;
	input.cmd[3] = 0;
	input.cmd[4] = 0;
	input.cmd[5] = 0;
	if(scsiio(&input, 0, &output, 0, "testunit") < 0)
		return(1);
	if(CHECK(output)){
		prcheck(1, &output);
		return(1);
	} else
		Fprint(1, "okay\n");
	return(0);
}
