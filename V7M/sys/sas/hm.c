/*
 * RM02/3 & RP04/5/6 disk driver
 * Disk on second RH controller (167300)
 * Fred Canter 8/17/80
 */

#include "../h/param.h"
#include "../h/inode.h"
#include "saio.h"

struct	device
{
	union {
		int	w;
		char	c[2];
	} hmcs1;		/* Control and Status register 1 */
	int	hmwc;		/* Word count register */
	caddr_t	hmba;		/* UNIBUS address register */
	int	hmda;		/* Desired address register */
	union {
		int	w;
		char	c[2];
	} hmcs2;		/* Control and Status register 2*/
	int	hmds;		/* Drive Status */
	int	hmer1;		/* Error register 1 */
	int	hmas;		/* Attention Summary */
	int	hmla;		/* Look ahead */
	int	hmdb;		/* Data buffer */
	int	hmmr;		/* Maintenance register */
	int	hmdt;		/* Drive type */
	int	hmsn;		/* Serial number */
	int	hmof;		/* Offset register */
	int	hmdc;		/* Desired Cylinder address register*/
	int	hmcc;		/* Current Cylinder */
	int	hmer2;		/* Error register 2 */
	int	hmer3;		/* Error register 3 */
	int	hmec1;		/* Burst error bit position */
	int	hmec2;		/* Burst error bit pattern */
	int	hmbae;		/* 11/70 bus extension */
	int	hmcs3;
};

#define	HMADDR	((struct device *)0176300)
#define RM	024
#define NRPSEC	22
#define NRPTRK	19
#define NRMSEC	32
#define NRMTRK	5

#define	P400	020
#define	M400	0220
#define	P800	040
#define	M800	0240
#define	P1200	060
#define	M1200	0260

#define	GO	01
#define	PRESET	020
#define	RTC	016
#define	OFFSET	014
#define	SEARCH	030
#define	RECAL	06
#define DCLR	010
#define	WCOM	060
#define	RCOM	070

#define	IE	0100
#define	PIP	020000
#define	DRY	0200
#define	ERR	040000
#define	TRE	040000
#define	DCK	0100000
#define	WLE	04000
#define	ECH	0100
#define VV	0100
#define FMT22	010000

hmstrategy(io, func)
register struct iob *io;
{
	register unit;
	register i;
	daddr_t bn;
	int sn, cn, tn;
	int nsect, ntrac;

	if (((unit = io->i_unit) & 04) == 0)
		bn = io->i_bn;
	else {
		unit &= 03;
		bn = io->i_bn;
		bn -= io->i_boff;
		i = unit + 1;
		unit = bn%i;
		bn /= i;
		bn += io->i_boff;
	}

	HMADDR->hmcs2.w = unit;

	if((HMADDR->hmdt & 077) < RM) {	/* drive type */
		nsect = NRPSEC;
		ntrac = NRPTRK;
	} else {
		nsect = NRMSEC;
		ntrac = NRMTRK;
		}

	if((HMADDR->hmds & VV) == 0) {
		HMADDR->hmcs1.c[0] = PRESET|GO;
		HMADDR->hmof = FMT22;
	}
	cn = bn/(nsect*ntrac);
	sn = bn%(nsect*ntrac);
	tn = sn/nsect;
	sn = sn%nsect;

	HMADDR->hmdc = cn;
	HMADDR->hmda = (tn << 8) + sn;
	HMADDR->hmba = io->i_ma;
	HMADDR->hmwc = -(io->i_cc>>1);
	unit = (segflag << 8) | GO;
	if (func == READ)
		unit |= RCOM;
	else if (func == WRITE)
		unit |= WCOM;
	HMADDR->hmcs1.w = unit;
	while ((HMADDR->hmcs1.w&DRY) == 0)
			;
	if (HMADDR->hmcs1.w & TRE) {
		printf("disk error: cyl=%d track=%d sect=%d cs2=%o, er1=%o\n",
		    cn, tn, sn, HMADDR->hmcs2, HMADDR->hmer1);
		return(-1);
	}
	return(io->i_cc);
}
