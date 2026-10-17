#include	"scsi.h"

static
drdiag1(drive, lower, o)
	struct scsi_o *o;
{
	struct scsi_i input;
	int i, n;
	struct scsi_o output;

	input.bus_id = 1<<scsi_id;
	input.cmd[0] = 0x1D;
	input.cmd[1] = drive<<5;
	input.cmd[2] = 0;
	input.cmd[3] = 0;
	input.cmd[4] = 10;
	input.cmd[5] = 0;
	input.data[0] = 0x18;	/* drive diagnostic */
	input.data[1] = lower? 2:1;
	input.data[2] = 0;
	input.data[3] = 0;
	input.data[4] = 0;
	input.data[5] = 0;
	input.data[6] = 0;
	input.data[7] = 0;
	input.data[8] = 0;
	input.data[9] = 0;
	if(scsiio(&input, o, 10, "drdiag") < 0)
		return(1);
	for(i = 0; i < 15; i++){
		s_sense(drive, 1);
		sleep(2);
	}
	s_diag(drive, 256, o);
	if(CHECK(*o)){
		scsidump(o);
		return(1);
	}
	return(0);
}

static char *msg1[16] =
{
	"drive not connected or powered off",
	"drive connected but no disk",
	"diagnostic aborted: write-protect",
	"diagnostic aborted: write area full",
	"urk 4", "urk 5", "urk 6", "urk 7", "urk 8", "urk 9", "urk 10",
	"urk 11", "urk 12", "urk 13", "urk 14", "urk 15"
};
static char *cmesg[256] =
{
	0
};
static char *testn[10] =
{
	"drive on/off",
	"read disk id",
	"move",
	"seek",
	"blank sector search",
	"written sector search",
	"search writable area",
	"write",
	"ECC margin check",
	"read data compare"
};

s_drdiag(drive)
{
	struct scsi_o o;
	int i;
	register unsigned char *d;
	char buf[256];
	int lower;

	if(s_istatus(drive, &o))
		return(1);
	if((o.data[100]&0x80) && (drive == (o.data[100]&7)))
		lower = 0;
	else if((o.data[101]&0x80) && (drive == (o.data[101]&7)))
		lower = 1;
	else {
		fprint(2, "drive %d not occupied\n", drive);
		return(1);
	}
	Fprint(1, "drive %d[%ser]: drive diagnostic\n", drive, lower?"low":"upp");
	Fflush(1);
	if(drdiag1(drive, lower, &o))
		return(1);
	d = o.data;
	if(d[1]&0x80)
		Fprint(1, "diagnostic not performed: %s\n", msg1[d[1]&0xF]);
	else {
		Fprint(1, "diagnostic result:");
		if((d[1]&0x70) == 0)
			Fprint(1, "no faults");
		if(d[1]&0x10)
			Fprint(1, " controller-fault");
		if(d[1]&0x20)
			Fprint(1, " drive-fault");
		if(d[1]&0x10)
			Fprint(1, " disk-fault");
		Fprint(1, " (last error code 0x%2.2ux)\n", d[4]);
	}
	if(cmesg[0] == 0){
		for(i = 0; i < 256; i++){
			sprint(buf, "error code 0x%x", i);
			cmesg[i] = strdup(buf);
		}
		cmesg[0] = "good";
		cmesg[0xE0] = "test not done";
		cmesg[0xFE] = "drive not ready (no disk)";
		cmesg[0xFF] = "not connected or power off";
		cmesg[0xEE] = "diagnostic could not be done";
	}
	for(i = 0; i < 10; i++)
		Fprint(1, "test %d %s: %s\n", i, testn[i], cmesg[d[i*8+drive+8]]);
	Fprint(1, "diagnostic count:");
	for(i = 104; i < 120; i++)
		Fprint(1, " %2.2ux", d[i]);
	Fprint(1, "\n");
	Fflush(1);
	return(0);
}
