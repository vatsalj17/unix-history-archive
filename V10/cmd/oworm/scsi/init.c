#include	"scsi.h"

#define	DEV	"/dev/scsi"

static char *regname[][2] =
{
	"??", "",
	"sa", "mscp",
	"csr", "per"
};

int scsi_fd;
int scsi_id;
int scsi_verbose;
int scsi_shush;

scsiinit(complain)
{
	scsi_shush = 0;
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

tdreg(fd, sa, mscp)
{
	char buf[256];
	extern char *strcat();

#define	add(x)	strcat(buf, x)

	buf[0] = 0;
	if(mscp&0x8000) add(", host buffer access");
	if(mscp&0x2000) add(", SCSI bus hung");
	if(mscp&0x1000) add(", resettable bad bus phase");
	if(mscp&0x0800) add(", select timeout");
	if(mscp&0x0200) add(", parity error");
	switch(mscp&0x3F)
	{
	case 011:	add(", dma error"); break;
	case 012:	add(", controller error"); break;
	default:	add(", bad mscp error code!!"); break;
	}
	if(buf[0] == 0)
		fprint(fd, "mscp: zero");
	else {
		fprint(fd, "mscp:%s", &buf[1]);
	}
	fprint(fd, "; ");
	fprint(fd, "sa: %hux\n", sa);
}

#define		CSR_DONE	0x8000
#define		CSR_ERROR	0x1000
#define		CSR_R		0x0020
#define		CSR_M		0x0010
#define		CSR_I		0x0008
#define		CSR_C		0x0004
#define		CSR_S		0x0002
#define		CSR_B		0x0001
#define		CSR_BITS	(CSR_DONE|CSR_ERROR|CSR_R|CSR_M|CSR_I|CSR_C|CSR_S|CSR_B)

#define		PER_CHECK	0x2000
#define		PER_DRBUSY	0x1000
#define		PER_SBUSY	0x0040
#define		PER_SELECT	0x0010

usreg(fd, csr, per)
{
	char buf[256];
	extern char *strcat();

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

scsidump(o)
	register struct scsi_o *o;
{
	extern char *exstab[], *smsg[];

	Fprint(1, "%s=0x%uhx %s=0x%uhx status=0x%uhx(%s) message=0x%uhx\n",
		regname[o->type][0], o->reg1, regname[o->type][1], o->reg2,
		o->scsistatus, smsg[(o->scsistatus>>1)&0xF], o->scsimesg);
	Fprint(1, "data=0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", o->data[0],
		o->data[1], o->data[2], o->data[3], o->data[4], o->data[5],
		o->data[6], o->data[7], o->data[8], o->data[9], o->data[10],
		o->data[11]);
	if(o->data[0] == 0x70)
		Fprint(1, " (extended sense: %s)", exstab[o->data[2]&0xF]);
	Fprint(1, "\n");
}

prcheck(fd, o)
	register struct scsi_o *o;
{
	int ot;
	extern char *exstab[], *smsg[];

	ot = o->type;
	if((ot < 0) || (ot > 2)) ot = 0;
	Fprint(fd, "%s=0x%uhx %s=0x%uhx status=0x%uhx(%s)",
		regname[ot][0], o->reg1, regname[ot][1], o->reg2,
		o->scsistatus, smsg[(o->scsistatus>>1)&0xF]);
	if(o->data[0] == 0x70)
		Fprint(fd, " (ext: %s)", exstab[o->data[2]&0xF]);
	Fputc(fd, '\n');
}

scsiio(i, icnt, o, ocnt, str)
	struct scsi_i *i;
	struct scsi_o *o;
	char *str;
{
	int n, loop;

	for(loop = 0; loop < 20; loop++){
		if((n = write(scsi_fd, (char *)i, INN(icnt))) != INN(icnt)){
			perror("write");
			fprint(2, "%s: wrote %d, really wrote %d\n", str, INN(icnt), n);
			return(-1);
		}
		scsiclro(o);
		if((n = read(scsi_fd, (char *)o, OUTN(ocnt))) != OUTN(ocnt)){
			if((n == 8) && (o->scsistatus == 8))
				continue;
			if(!scsi_shush){
				if(n < 0)
					perror("read");
				fprint(2, "%s: wanted %d, got %d\n", str, OUTN(ocnt), n);
				prcheck(2, o);
				Fflush(2);
			}
			if(n < 0)
				switch(o->type)
				{
				case TD_VIKING:	tdreg(2, o->reg1, o->reg2); break;
				case USD_1158:	usreg(2, o->reg1, o->reg2); break;
				default:	fprint(2, "BAD board type %d\n", o->type); break;
				}
			return(-1);
		}
		if(o->scsistatus == 0)
			break;
	}
	return(0);
}
