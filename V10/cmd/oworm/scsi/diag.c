#include	"scsi.h"

s_diag(drive, n, o)
	struct scsi_o *o;
{
	struct scsi_i input;

	input.bus_id = scsi_id;
	input.cmd[0] = 0x1C;
	input.cmd[1] = drive<<5;
	input.cmd[2] = 0;
	input.cmd[3] = n>>8;
	input.cmd[4] = n;
	input.cmd[5] = 0;
	if(scsiio(&input, 0, o, n, "diag") < 0)
		return(1);
	return(0);
}
