#include	<libc.h>

struct scsi_i
{
	short bus_id;
	char cmd[10];
	char data[4*1024];
};
#define		N(n)	(12+n)

/*
	STATUS:
	+0:	scsi status phase byte
	+1:	message phase command
	+2:	controller status?
*/

struct scsi_o
{
	unsigned char status[4];
	unsigned short csr;
	unsigned short per;
	unsigned char data[4*1024];
};

#define	CHECK(o)	((((o).status[0]&0xFF) != 0xEE) && ((o).status[0]&2))

extern int scsi_fd;
extern int scsi_id;
extern int scsi_verbose;

#define		V		scsi_verbose

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

#define		A		0
#define		B		1

#define		ULONG(d)	((d[0]<<24)|(d[1]<<16)|(d[2]<<8)|d[3])
#define		LONG(cp)	ULONG(((unsigned char *)cp))
