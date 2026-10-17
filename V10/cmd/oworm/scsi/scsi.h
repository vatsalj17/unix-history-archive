#include	<libc.h>

struct scsi_i
{
	short bus_id;		/* 0x8000 bit set means reset */
	char cmd[10];
	char data[4*1024];
};
#define		INN(n)		(12+n)
#define		OUTN(n)		(8+n)

#define		TD_VIKING	1
#define		USD_1158	2

struct scsi_o
{
	unsigned char type;
	unsigned char pad;
	unsigned char scsistatus;
	unsigned char scsimesg;
	unsigned short reg1;		/* td=sa, us=csr */
	unsigned short reg2;		/* td=mscp, us=per */
	unsigned char data[4*1024];
};

#define	CHECK(o)	(((o).scsistatus&0x1E) == 0x02)

extern int scsi_fd;
extern int scsi_id;
extern int scsi_verbose;
extern int scsi_shush;

#define		V		scsi_verbose

#define		A		0
#define		B		1

#define		ULONG(d)	((d[0]<<24)|(d[1]<<16)|(d[2]<<8)|d[3])
#define		LONG(cp)	ULONG(((unsigned char *)cp))

#define	UNLABELLED	"unlabelled"	/* unlabelled disks */
