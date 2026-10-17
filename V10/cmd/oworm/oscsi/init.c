#include	"scsi.h"

#define	DEV	"/dev/scsi"

int scsi_fd;
int scsi_id;
int scsi_verbose;

scsiinit(complain)
{
	if((scsi_fd = open(DEV, 2)) < 0){
		if(complain)
			perror(DEV);
		return(1);
	}
	return(0);
}

scsiid(id)
{
	scsi_id = id;
}

scsiclro(o)
	register struct scsi_o *o;
{
	extern char *memset();
	memset((char *)o, 0xDD, sizeof(*o));
}

scsireg(fd, csr, per)
{
	char buf[256];
	extern char *strcat();

#define	add(x)	strcat(buf, x)

	buf[0] = 0;
	if(csr&CSR_DONE) add(", i/o done");
	if(csr&CSR_ERROR) add(", error");
	if(csr&CSR_R) add(", request active");
	if(csr&CSR_M) add(", message active");
	if(csr&CSR_I) add(", i/o active");
	if(csr&CSR_C) add(", c/d active");
	if(csr&CSR_S) add(", select active");
	if(csr&CSR_B) add(", busy active");
	if(buf[0] == 0)
		fprint(fd, "CSR: zero");
	else {
		fprint(fd, "CSR:%s", &buf[1]);
		if(csr&!CSR_BITS)
			fprint(fd, " BAD BITS SET 0x%ux", csr&!CSR_BITS);
	}
	fprint(fd, "; ");
	buf[0] = 0;
	if(per&PER_CHECK) add(", check status");
	if(per&PER_DRBUSY) add(", drive busy");
	if(per&PER_SBUSY) add(", busy timeout");
	if(per&PER_SELECT) add(", select timeout");
	if(buf[0] == 0)
		fprint(fd, "PER: zero\n");
	else
		fprint(fd, "PER:%s\n", &buf[1]);
}

static char *smsg[16] =
{
	"good", "check", "met/good", "reserved", "busy", "reserved",
	"reserved", "reserved", "intermediate good", "reserved", "intermediate good/met",
	"reserved", "reservation conflict", "reserved", "reserved", "reserved",
};

scsidump(o)
	register struct scsi_o *o;
{
	extern char *exstab[];

	Fprint(1, "csr=0x%uhx per=0x%uhx status=0x%uhx(%s) 0x%uhx 0x%uhx\n", o->csr,
		o->per, o->status[0], smsg[(o->status[0]>>1)&0xF], o->status[1],
		o->status[2]);
	Fprint(1, "data=0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", o->data[0],
		o->data[1], o->data[2], o->data[3], o->data[4], o->data[5],
		o->data[6], o->data[7], o->data[8], o->data[9], o->data[10],
		o->data[11]);
	if(o->data[0] == 0x70)
		Fprint(1, " (extended sense: %s)", exstab[o->data[2]&0xF]);
	Fprint(1, "\n");
}

scsiio(i, o, nn, str)
	struct scsi_i *i;
	struct scsi_o *o;
	char *str;
{
	int n;

	if((n = write(scsi_fd, (char *)i, N(nn))) != N(nn)){
		fprint(2, "%s: wrote %d, really wrote %d\n", str, N(nn), n);
		perror("write");
		return(1);
	}
	scsiclro(o);
	n = read(scsi_fd, (char *)o, sizeof *o);
	if(n < 0){
		perror(str);
		scsireg(2, o->csr, o->per);
		return(1);
	}
	return(n);
}
