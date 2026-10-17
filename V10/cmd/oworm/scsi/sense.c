#include	"scsi.h"

s_sense(drive, extended)
{
	int n;
	struct scsi_i input;
	struct scsi_o output;
	int i;
	extern char *exstab[];

	n = s_sensei(&input, drive, extended);
	if(scsiio(&input, 0, &output, n, "sense") < 0)
		return(1);
	prcheck(1, &output);
	Fprint(1, "\t");
	for(i = 0; i < 16; i++)
		Fprint(1, " #%x", output.data[i]);
	Fprint(1, "\n");
	return(0);
}

s_sensei(i, drive, extended)
	register struct scsi_i *i;
{
	i->bus_id = scsi_id;
	i->cmd[0] = 0x03;
	i->cmd[1] = drive<<5;
	i->cmd[2] = 0;
	i->cmd[3] = 0;
	i->cmd[4] = extended? 12:4;
	i->cmd[5] = 0;
	return(i->cmd[4]);
}

char *smsg[16] =
{
	"good", "check", "met/good", "reserved", "busy", "reserved",
	"reserved", "reserved", "intermediate good", "reserved", "intermediate good/met",
	"reserved", "reservation conflict", "reserved", "reserved", "reserved",
};

char *exstab[16] =
{
	"no sense",
	"recovered error",
	"not ready",
	"medium error",
	"hardware error",
	"illegal request",
	"unit attention",
	"data protect",
	"blank check",
	"???",
	"copy aborted",
	"???",
	"???",
	"???",
	"???",
	"???",
};
