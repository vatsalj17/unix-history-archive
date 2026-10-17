#include	"scsi.h"

static int cnts[256];
static char *cmsg[256];

s_media(drive, lbn, count, verbose)
{
	struct scsi_o o;
	int bn, c;
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
	Fprint(1, "drive %d: %smedia check for %d blocks [%d-%d], %s drive\n",
		drive, verbose?"detailed ":"", count, lbn, lbn+count-1,
		lower? "lower" : "upper");
	if(cmsg[0] == 0){
		for(bn = 0; bn < 256; bn++){
			sprint(buf, "rare error 0x%x", bn);
			cmsg[bn] = strdup(buf);
		}
		cmsg[0] = "good";
		cmsg[0x45] = "unwritten";
		cmsg[0x81] = "<50% burst";
		cmsg[0x82] = "50-96% burst";
		cmsg[0x83] = ">96% burst";
		cmsg[0x84] = "uncorrectable";
	}
	for(bn = 0; bn < 256; bn++)
		cnts[bn] = 0;
	for(bn = lbn, c = count; c >= 256; c -= 256, bn += 256){
		if(s_media1(drive, bn, lower, &o))
			return(1);
		for(d = o.data; d < &o.data[256];){
			if(verbose && *d)
				Fprint(1, "lbn %d: %s\n", bn+(d-o.data), cmsg[*d]);
			cnts[*d++]++;
		}
	}
	if(c){
		if(s_media1(drive, bn, lower, &o))
			return(1);
		for(d = o.data; c; c--){
			if(verbose && *d)
				Fprint(1, "lbn %d: %s\n", bn+(d-o.data), cmsg[*d]);
			cnts[*d++]++;
		}
	}
	for(c = 0; c < 256; c++)
		if(cnts[c])
			Fprint(1, "%d %s, ", cnts[c], cmsg[c]);
	Fprint(1, "\n");
	return(0);
}

s_mediaf(drive, lbn, count, file)
	char *file;
{
	struct scsi_o o;
	int bn, c;
	char buf[256];
	int lower;
	int fd;

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
	if((fd = creat(file, 0666)) < 0){
		perror(file);
		return(1);
	}
	Finit(fd, (char *)0);
	fprint(1, "drive %d: media check for %d blocks [%d-%d], %s drive on file %s\n",
		drive, count, lbn, lbn+count-1,
		lower? "lower" : "upper", file);
	for(bn = lbn, c = count; c >= 256; c -= 256, bn += 256){
		if(s_media1(drive, bn, lower, &o))
			return(1);
		Fwrite(fd, o.data, 256);
	}
	if(c){
		if(s_media1(drive, bn, lower, &o))
			return(1);
		Fwrite(fd, o.data, c);
	}
	Fflush(fd);
	close(fd);
	fprint(1, "done\n");
	return(0);
}

s_media1(drive, lbn, lower, o)
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
	input.data[0] = 0x0A;	/* error margin check */
	input.data[1] = lower? 2:1;
	input.data[2] = 0;
	input.data[3] = 0;
	input.data[4] = drive;
	input.data[5] = lbn;
	input.data[6] = lbn>>8;
	input.data[7] = lbn>>16;
	input.data[8] = 0;
	input.data[9] = 0;
	if(scsiio(&input, o, 10, "media") < 0)
		return(1);
	s_diag(drive, 256, o);
	if(CHECK(*o)){
		scsidump(o);
		return(1);
	}
	return(0);
}
