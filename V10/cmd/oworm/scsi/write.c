#include	"scsi.h"

static
swrite(drive, block, nblock, input)
	struct scsi_i *input;
{
	struct scsi_o output;

	input->bus_id = scsi_id;
	input->cmd[0] = 0x2A;
	input->cmd[1] = drive<<5;
	input->cmd[2] = block>>24;
	input->cmd[3] = block>>16;
	input->cmd[4] = block>>8;
	input->cmd[5] = block;
	input->cmd[6] = 0;
	input->cmd[7] = nblock>>8;
	input->cmd[8] = nblock;
	input->cmd[9] = 0;
	if(scsiio(input, nblock*1024, &output, 0, "write") < 0)
		return(1);
	if(CHECK(output)){
		prcheck(1, &output);
		return(1);
	}
	return(0);
}

s_write(drive, block, nblock)
{
	struct scsi_o output;
	struct scsi_i input;
	register long *j, i;
	int nb = 3;

	if((drive < 0) || (drive > 7)){
		fprint(2, "bad drive number %d\n", drive);
		return(1);
	}
	fprint(1, "writing %d blocks [%d-%d] on drive %d: ", nblock, block, block+nblock-1, drive);
	while(nblock > 0){
		if(nblock < nb)
			nb = nblock;
		for(j = (long *)input.data, i = nb*256-1; i >= 0; i--)
			*j++ = block;
		if(swrite(drive, block, nb, &input))
			return(1);
		nblock -= nb;
		block += nb;
	}
	fprint(1, "done\n");
	return(0);
}
