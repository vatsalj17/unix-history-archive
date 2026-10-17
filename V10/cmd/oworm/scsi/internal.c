#include	"scsi.h"

static
internal(n, b1, nb, o)
	struct scsi_o *o;
{
	struct scsi_i input;

	input.bus_id = scsi_id;
	input.cmd[0] = 0x1D;
	input.cmd[1] = b1;
	input.cmd[2] = 0;
	input.cmd[3] = 0;
	input.cmd[4] = 10;
	input.cmd[5] = 0;
	input.data[0] = n;
	input.data[1] = b1>>8;
	input.data[2] = 0;
	input.data[3] = 0;
	input.data[4] = 0;
	input.data[5] = 0;
	input.data[6] = 0;
	input.data[7] = 0;
	input.data[8] = 0;
	input.data[9] = 0;
	if(scsiio(&input, 10, o, 0, "internal") < 0)
		return(1);
	if(s_diag(0, nb, o))
		return(1);
	if(CHECK(*o)){
		prcheck(1, o);
		return(1);
	}
	return(0);
}

static char *cmd[] = {
	"internal command table",
	"error information table",
	"arm controller diagnostics",
	"scsi control board diagnostics",
	"drive controller diagnostics",
	"jukebox status",
	0
};

static char *msg1[16] =
{
	"drive not connected or powered off",
	"drive connected but no disk",
	"diagnostic aborted: write-protect",
	"diagnostic aborted: write area full",
	"urk 4", "urk 5", "urk 6", "urk 7", "urk 8", "urk 9", "urk 10",
	"urk 11", "urk 12", "urk 13", "urk 14", "urk 15"
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

s_internal(n, drive)
{
	struct scsi_o output;
	struct scsi_i input;
	register unsigned char *d;
	int i;
	long t1, t2;
	int lower;
	extern char *cmesg[];
	extern char *i0com[], *i1err[], *scsicmd[], *busid[], *scsiident[];

	switch(n)
	{
	case -1:
		for(i = 0; cmd[i]; i++)
			Fprint(1, "internal %d: %s\n", i, cmd[i]);
		break;
	case 0:
		if(internal(0xe5, 0, 256, &output))
			return(1);
		Fprint(1, "%s:\n", cmd[n]);
		Fprint(1, "Diagnostic #E5: last 16 internal tasks (drive,shelf)\n");
		for(i = 0, d = output.data; i < 16; i++, d += 16){
			Fprint(1, "[%d] %s (%d,%d)\n",
				d[0], i0com[d[1]], d[2], d[3]);
		}
		break;
	case 1:
		if(internal(0xe4, 0, 256, &output))
			return(1);
		Fprint(1, "%s:\n", cmd[n]);
		Fprint(1, "Diagnostic #E4: last 16 errors; initiator[identify] error[sense] (cmd)\n");
		for(i = 0, d = output.data; i < 16; i++, d += 16){
			Fprint(1, "%s[%s]: %s[#%x] (%s)\n",
				busid[d[0]], scsiident[d[1]], i1err[d[14]], d[15], scsicmd[d[4]]);
		}
		break;
	case 2:
		Fprint(1, "%s:\n", cmd[n]);
		Fflush(1);
		time(&t1);
		if(internal(0x90, 0, 8, &output))
			return(1);
		time(&t2);
		d = output.data;
		if(d[0] == 0)
			Fprint(1, "\tended normally");
		else
			Fprint(1, "\tfailed, error codes=#%x, #%x, #%x",
				d[0], d[1], d[2]);
		Fprint(1, " (time: %lds)\n", t2-t1);
		break;
	case 3:
		Fprint(1, "%s:\n", cmd[n]);
		Fflush(1);
		time(&t1);
		if(internal(0xe0, 0, 8, &output))
			return(1);
		time(&t2);
		d = output.data;
		if(d[0] == 0)
			Fprint(1, "\tended normally");
		else
			Fprint(1, "\tfailed, error codes=#%x, #%x, #%x",
				d[0], d[1], d[2]);
		Fprint(1, " (time: %lds)\n", t2-t1);
		break;
	case 4:
		if(s_istatus(drive, &output))
			return(1);
		if((output.data[100]&0x80) && (drive == (output.data[100]&7)))
			lower = 0x100;
		else if((output.data[101]&0x80) && (drive == (output.data[101]&7)))
			lower = 0x200;
		else {
			fprint(2, "drive %d not occupied\n", drive);
			return(1);
		}
		Fprint(1, "drive %d[%ser]: %s\n", drive, (lower == 0x200)?"low":"upp", cmd[n]);
		Fflush(1);
		time(&t1);
		if(internal(0x18, lower|((drive&7)<<5)|1, 256, &output))
			return(1);
		time(&t2);
		d = output.data;
		if(d[1]&0x80){
			Fprint(1, "diagnostic result:");
			if((d[1]&0x70) == 0)
				Fprint(1, " no faults");
			else {
				if(d[1]&0x10)
					Fprint(1, " controller-fault");
				if(d[1]&0x20)
					Fprint(1, " drive-fault");
				if(d[1]&0x10)
					Fprint(1, " disk-fault");
				Fprint(1, " (last error code 0x%2.2ux)", d[4]);
			}
		} else
			Fprint(1, "diagnostic not performed: %s", msg1[d[1]&0xF]);
		Fprint(1, " (time: %lds)\n", t2-t1);
		for(i = 0; i < 10; i++)
			Fprint(1, "test %d[%s]: %s\n", i, testn[i], cmesg[d[i*8+drive+8]]);
		Fprint(1, "diagnostic count (drive:avail):");
		for(d += 104, i = 0; i < 8; i++, d += 2)
			Fprint(1, " %d:%d", i, d[0]+d[1]*256);
		Fprint(1, "\n");
		Fflush(1);
		break;
	case 5:
		input.bus_id = scsi_id;
		input.cmd[0] = 0xD3;
		input.cmd[1] = drive<<5;
		input.cmd[2] = 0;
		input.cmd[3] = 0;
		input.cmd[4] = 0;
		input.cmd[5] = 0;
		input.cmd[6] = 0;
		input.cmd[7] = 0;
		input.cmd[8] = 0;
		input.cmd[9] = 0;
		if(scsiio(&input, 0, &output, 20, "alternate 5") < 0)
			return(1);
		if(CHECK(output)){
			prcheck(1, &output);
			return(1);
		}
		Fprint(1, "%s: component(fatal err/err/cmds)\n", cmd[n]);
		d = output.data;
#define	ONE(str, x, sep)	Fprint(1, "%s(%d/%d/%d)%c", str, d[x+3], d[x+2], d[x+1]+256*d[x], sep)
	
		ONE("upper drive", 4, ' ');
		ONE("lower drive", 8, ' ');
		ONE("sys control", 12, ' ');
		Fprint(1, "backup mem(0/%d/%d)\n", d[19]+256*d[18], d[17]+256*d[16]);
		break;
	}
	return(0);
}

