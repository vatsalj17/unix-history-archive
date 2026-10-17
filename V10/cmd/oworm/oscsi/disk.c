#include	"scsi.h"

s_diskset(shelf, side, drive, complain)
{
	struct scsi_i input;
	struct scsi_o output;

	input.bus_id = 1<<scsi_id;
	input.cmd[0] = 0xD6;
	input.cmd[1] = drive<<5;
	input.cmd[2] = 0;
	input.cmd[3] = (shelf<<1)|side;
	input.cmd[4] = 0;
	input.cmd[5] = 0;
	if(scsiio(&input, &output, 0, "set") < 0)
		return(1);
	if(CHECK(output)){
		if(complain)
			scsidump(&output);
		return(1);
	}
	return(0);
}

s_diskrelease(shelf, side, drive, complain)
{
	struct scsi_i input;
	struct scsi_o output;

	input.bus_id = 1<<scsi_id;
	input.cmd[0] = 0xD7;
	input.cmd[1] = (drive<<5)|(shelf >= 0);
	input.cmd[2] = 0;
	input.cmd[3] = (shelf<<1)|side;
	input.cmd[4] = 0;
	input.cmd[5] = 0;
	if(scsiio(&input, &output, 0, "release") < 0)
		return(1);
	if(CHECK(output)){
		if(complain) scsidump(&output);
		return(1);
	}
	return(0);
}

s_diskeject(drive)
{
	struct scsi_i input;
	struct scsi_o output;

	input.bus_id = 1<<scsi_id;
	input.cmd[0] = 0xC0;
	input.cmd[1] = drive<<5;
	input.cmd[2] = 0;
	input.cmd[3] = 0;
	input.cmd[4] = 0;
	input.cmd[5] = 0;
	if(scsiio(&input, &output, 0, "eject") < 0)
		return(1);
	if(CHECK(output)){
		scsidump(&output);
		return(1);
	}
	return(0);
}
