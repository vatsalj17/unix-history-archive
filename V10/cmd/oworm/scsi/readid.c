#include	"scsi.h"

static
myread(drive, block, o)
	long block;
	struct scsi_o *o;
{
	struct scsi_i input;

	input.bus_id = scsi_id;
	input.cmd[0] = 0x28;
	input.cmd[1] = drive<<5;
	input.cmd[2] = block>>24;
	input.cmd[3] = block>>16;
	input.cmd[4] = block>>8;
	input.cmd[5] = block;
	input.cmd[6] = 0;
	input.cmd[7] = 0;
	input.cmd[8] = 1;
	input.cmd[9] = 0;
	if(scsiio(&input, 0, o, 1024, "readid") < 0)
		return(1);
	if(CHECK(*o))
		return(1);
	return(0);
}

/*
	return label, UNLABELLED or "" (error)
*/

s_readid(drive, buf)
	char *buf;
{
	struct scsi_i input;
	struct scsi_o output;
	long b, lastb;

	buf[0] = 0;
	scsi_shush = 1;
	if(myread(drive, 0L, &output) == 0)
		goto done;
	for(b = 1, lastb = -1;;){
		if(myread(drive, b, &output))
			break;
		lastb = b;
		b = ((long *)output.data)[9];
	}
	scsi_shush = 0;
	if(lastb < 0){
		s_sense(drive, 1);
		strcpy(buf, UNLABELLED);
		return;
	}
	myread(drive, lastb, &output);
done:
	strncpy(buf, (char *)&output.data[42], 128);
	buf[127] = 0;
}
