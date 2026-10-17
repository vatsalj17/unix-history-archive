#include	"scsi.h"

static
table(drive, tab, data)
	unsigned char *data;
{
	int n, i;

	n = data[6];
	Fprint(1, "drive %d: alternate table %d (%d entries)\n", drive, tab, n);
	for(data += 0x18, i = 0; i < n; data += 4, i++)
		Fprint(1, "%lud%c", data[0]+256L*data[1]+256L*256*data[2],
			(i%10 == 9)? '\n':' ');
	if((i%10) && n)
		Fputc(1, '\n');
}

s_alternate(drive)
{
	struct scsi_i input;
	struct scsi_o output;
	int i;

	input.bus_id = scsi_id;
	input.cmd[0] = 0xC3;
	input.cmd[1] = drive<<5;
	input.cmd[2] = 0;
	input.cmd[3] = 0;
	input.cmd[4] = 0;
	input.cmd[5] = 0;
	if(scsiio(&input, 0, &output, 4096, "alternate") < 0)
		return(1);
	if(CHECK(output)){
		prcheck(1, &output);
		return(1);
	} else {
		for(i = 0; i < 4; i++)
			table(drive, i+1, &output.data[i*1024]);
		return(0);
	}
}
