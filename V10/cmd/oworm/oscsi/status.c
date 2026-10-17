#include	"scsi.h"

static shelf();

s_status(drive, o)
	struct scsi_o *o;
{
	struct scsi_o output;
	register unsigned char *d;
	int i;

	if(o == 0)
		o = &output;
	if(s_istatus(drive, o))
		return(1);
	if(V){
		d = &o->data[16];
		for(i = 0; i < 8; i++, d += 4){
			Fprint(1, "drive %d: %sready,%sdisk in LUN,power %s,", i,
				(d[0]&1)?"":"not ", (d[0]&0x40)?"":"no ",
				(d[0]&0x80)?"off":"on");
			Fprint(1, "disk in %s %d",( d[1]&0x80)? "drive":"shelf", d[1]&0x7f);
			if(d[2]&0x80)
				Fprint(1, ", return shelf %d", d[2]&0x7F);
			Fprint(1, "\n");
		}
		for(i = 0; i < 50; i++, d++)
			if(*d){
				Fprint(1, "%d", i);
				shelf(*d);
			}
		Fprint(1, "I/O shelf");
		shelf(*d);
		d++;
		Fprint(1, "carrier: ");
		if(*d&0x80)
			Fprint(1, "disk shelf=%d\n", *d&0x7F);
		else
			Fprint(1, "no disk\n");
		d++;
		if(*d&0x80)
			Fprint(1, "upper drive: disk, LUN=%d\n", *d&7);
		else
			Fprint(1, "upper drive: no disk\n");
		d++;
		if(*d&0x80)
			Fprint(1, "lower drive: disk, LUN=%d\n", *d&7);
		else
			Fprint(1, "lower drive: no disk\n");
	}
	return(0);
}

s_istatus(drive, o)
	struct scsi_o *o;
{
	struct scsi_i input;

	input.bus_id = 1<<scsi_id;
	input.cmd[0] = 0x1D;
	input.cmd[1] = drive<<5;
	input.cmd[2] = 0;
	input.cmd[3] = 0;
	input.cmd[4] = 10;
	input.cmd[5] = 0;
	input.data[0] = 0xE2;	/* internal status */
	input.data[1] = 0;
	input.data[2] = 0;
	input.data[3] = 0;
	input.data[4] = 0;
	input.data[5] = 0;
	input.data[6] = 0;
	input.data[7] = 0;
	input.data[8] = 0;
	input.data[9] = 0;
	if(scsiio(&input, o, 10, "status") < 0)
		return(1);
	s_diag(drive, 128, o);
	if(CHECK(*o)){
		scsidump(o);
		return(1);
	}
	return(0);
}

static
shelf(i)
{
	Fprint(1, ": ", i&0xFF);
	if(i&0x80){
		Fprint(1, "%s,", (i&0x40)? "disk":"temporary");
		if(i&0x10) Fprint(1, "wait loading,");
		if(i&0x08) Fprint(1, "wait ejection,");
		if(i&0x20) Fprint(1, "use shelf instead of drive for LUN %d", i&7);
	} else
		Fprint(1, "no disk");
	Fprint(1, "\n");
}
