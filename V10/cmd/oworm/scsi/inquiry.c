#include	"scsi.h"

s_inquiry(id, drive)
{
	struct scsi_i input;
	struct scsi_o output;

	input.bus_id = id;
	input.cmd[0] = 0x12;
	input.cmd[1] = drive<<5;
	input.cmd[2] = 0;
	input.cmd[3] = 0;
	input.cmd[4] = 6;
	input.cmd[5] = 0;
	if(scsiio(&input, 0, &output, 6, "inquiry") < 0)
		return(1);
	Fprint(1, "drive %d,%d: ", id, drive);
	if(output.data[5]&0x80)
		Fprint(1, "power off (0x%x)\n", output.data[5]&0xFF);
	else
		Fprint(1, "%s,%s,%s,%s,%s (0x%x)\n",
			(output.data[5]&0x40)?"no disk":"disk",
			(output.data[5]&0x08)?"write protect":"writable",
			(output.data[5]&0x04)?"no alternate":"",
			(output.data[5]&0x02)?"drive error":"",
			(output.data[5]&0x01)?"ready":"not ready",
			output.data[5]&0xFF);
	return(0);
}
