#include	"scsi.h"

s_capacity(drive, nb, bl)
	long *nb, *bl;
{
	struct scsi_i input;
	struct scsi_o output;
	long c, l;

	input.bus_id = 1<<scsi_id;
	input.cmd[0] = 0x25;
	input.cmd[1] = drive<<5;
	input.cmd[2] = 0;
	input.cmd[3] = 0;
	input.cmd[4] = 0;
	input.cmd[5] = 0;
	input.cmd[6] = 0;
	input.cmd[7] = 0;
	input.cmd[8] = 0;
	input.cmd[9] = 0;
	if(scsiio(&input, &output, 8, "capacity") < 0)
		return(1);
	if(CHECK(output)){
		scsidump(&output);
		return(1);
	} else {
		c = ULONG((&output.data[0]));
		l = ULONG((&output.data[4]));
		if(V)
			Fprint(1, "drive %d: capacity %dx%d (%uxx%ux)\n", drive,
				c, l, c, l);
		if(bl) *bl = l;
		if(nb) *nb = c;
		return(0);
	}
}
