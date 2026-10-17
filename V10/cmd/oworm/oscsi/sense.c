#include	"scsi.h"

s_sense(drive, extended)
{
	int n;
	struct scsi_i input;
	struct scsi_o output;

	n = s_sensei(&input, drive, extended);
	if(scsiio(&input, &output, n, "sense") < 0)
		return(1);
	scsidump(&output);
	return(0);
}

s_sensei(i, drive, extended)
	register struct scsi_i *i;
{
	i->bus_id = 1<<scsi_id;
	i->cmd[0] = 0x03;
	i->cmd[1] = drive<<5;
	i->cmd[2] = 0;
	i->cmd[3] = 0;
	i->cmd[4] = extended? ((extended<0)? 28:12):4;
	i->cmd[5] = 0;
	return(i->cmd[4]);
}

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
