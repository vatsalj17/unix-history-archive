#include	"scsi.h"

static char mtab[5][2] =
{
	'0', '0', '1', '1', '1', '2', '2', '2', '?', '?'
};
char *brdname[] = { "no doard", "T.D. Systems Viking", "U.S. Design 1158" };

s_config()
{
	struct scsi_i input;
	struct scsi_o output;
	int i;

	input.bus_id = scsi_id;
	input.cmd[0] = 0x12;
	input.cmd[1] = 0;
	input.cmd[2] = 0;
	input.cmd[3] = 0;
	input.cmd[4] = 44;
	input.cmd[5] = 0;
	if(scsiio(&input, 0, &output, 44, "config") < 0)
		return(1);
	i = min(output.data[37], 4);
	Fprint(1, "config(%d,%d): %s device, '%0.28s', %c controller%s, %c drive%s\n",
		scsi_id, 0, (output.data[0] == 0x4)? "WORM":"Unknown",
		&output.data[8], mtab[i][0], (mtab[i][0] == '1')?"":"s",
		mtab[i][1], (mtab[i][1] == '1')?"":"s");
	Fprint(1, "\tUnibus-SCSI controller=%s\n", brdname[output.type]);
	Fprint(1, "\tROMS:");
	if(output.data[38] != 0xFF)
		Fprint(1, " upper controller=x%x,", output.data[38]);
	if(output.data[40] != 0xFF)
		Fprint(1, " lower controller=x%x,", output.data[40]);
	Fprint(1, " IF-129=x%x, SY-46=x%x, SS-30=x%x\n", output.data[36],
		output.data[42], output.data[43]);
	return(0);
}

s_ndrives()
{
	struct scsi_i input;
	struct scsi_o output;
	int i;

	input.bus_id = scsi_id;
	input.cmd[0] = 0x12;
	input.cmd[1] = 0;
	input.cmd[2] = 0;
	input.cmd[3] = 0;
	input.cmd[4] = 44;
	input.cmd[5] = 0;
	if(scsiio(&input, 0, &output, 44, "ndrives") < 0)
		return(0);
	i = min(output.data[37], 4);
	return(mtab[i][1] - '0');
}
