#include	"scsi.h"

s_reset()
{
	struct scsi_i input;
	struct scsi_o output;
	register i, n;

	input.bus_id = -1;
	input.cmd[0] = 0x12;
	input.cmd[1] = 0<<5;
	input.cmd[2] = 0;
	input.cmd[3] = 0;
	input.cmd[4] = 36;
	input.cmd[5] = 0;
	if(scsiio(&input, &output, 36, "reset") < 0)
		return(1);
	sleep(5);
	for(i = 0; i < 60; i++){
		sleep(2);
		n = s_sensei(&input, 0, 0);
		if(scsiio(&input, &output, n, "reset sense") < 0)
			return(1);
		if(output.status[0] == 0){
			print("done\n");
s_sense(0, 0);
			return(0);
		}
		print(".");
	}
	return(0);
}
