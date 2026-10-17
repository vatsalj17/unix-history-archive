/*
 * MacIntosh RAM Disk Driver
 * (c) Apple Computer 1983
 * 
 * This is the unofficial Mac ramdisk driver.  Most useful on 512K Macs
 * Written in Dec '83 my MDB
 * 
 * Routines:
 * 
 *  Open   --  Zero the ram and create a  directory
 * 
 *  Prime  -- read and write calls.  Get correct info from read or write param
 *            block, and get or put the bytes.  All calls are synchronous, since
 *            ram doesn't have a lot of latency.
 * 
 *  Close  -- Not closable at present 
 */

/*
 * history
 * 12/xx/83	MDB	Created.
 * 07/21/84	Croft	translated to C.
 */

#include "mac/quickdraw.h"
#include "mac/osintf.h"
#include "mac/device.h"

/*
 * for small mac, disk is kept in heap, so use bootconfig 
 * to up sysheap from 16K to 24K;  sigh, but then MacPaint/MacTerm are too big.
 */
#define	myramSize 0x2000	/* 8K */

#define	asize	512		/* allocation block size */
#define	dblocks	5		/* 0,1=boot;2,3=mdb;4=fdir */
#define	ablocks	myramSize/asize - dblocks

struct DR {
	char	drSigWord[2];		/* signature */
	long	drCrDate;		/* creation date/time*/
	long	drLsBkUp;		/* last backup */
	short	drAtrb;			/* attributes */
	short	drNmFls;		/* number of files in dir */
	short	drDirSt;		/* 1st file directory block */
	short	drBlLn;			/* length of file dir in blocks */
	short	drNmAlBlks;		/* number of allocation blocks */
	long	drAlBlkSiz;		/* alloc block size */
	long	drClpSiz;		/* bytes to try to alloc as clump */
	short	drAlBlSt;		/* 1st data block */
	long	drNxtFNum;		/* next free file number */
	short	drFreeBks;		/* number free blocks */
	char	drVN[28];		/* volume name */
};

struct DR myDir = {
	0xd2, 0xd7,			/* signature */
	0,
	0,
	0,
	0,
	4,				/* 1st file dir block */
	1,				/* # dir blocks */
	ablocks,			/* # alloc blocks */
	asize,				/* alloc block size */
	asize,				/* clump size */
	5,				/* 1st data block */
	1,				/* next file # */
	ablocks,			/* # free blocks */
	"\07RamDisk"
};

Ptr	myramBase, NewPtrSys();



/*
 * driver Open.
 */
drvrOpen(pb,dce)
	IOParam *pb;
	DCtlEntry *dce;
{
	if ((myramBase = NewPtrSys(myramSize)) == 0)
		return (mFulErr);
	bzero(myramBase,myramSize);
	bcopy(&myDir,myramBase+1024,sizeof myDir);
	return (ioRTS);
}


/*
 * driver Close.
 */
drvrClose(pb,dce)
	IOParam *pb;
	DCtlEntry *dce;
{
	return (ioRTS);
}


/*
 * driver Prime.
 */
drvrPrime(pb,dce)
	register IOParam *pb;
	register DCtlEntry *dce;
{
	register char *cp;
	register count;

	cp = myramBase + (dce->dCtlPosition & (~0x1ff)); /* truncate offset */
	count = pb->ioActCount = pb->ioReqCount;
	count = (count + 0x1ff) & (~0x1ff);	/* round to 512 mult */
	switch (pb->ioTrap & 0xff) {

	case aWrCmd:
		bcopy(pb->ioBuffer, cp, count);
		break;

	case aRdCmd:
		bcopy(cp, pb->ioBuffer, count);
		break;
	}
	return (ioDone);
}


/*
 * driver Control.
 */
drvrCtl(pb,dce)
	register CntrlParam *pb;
	register DCtlEntry *dce;
{
	switch (pb->csCode) {
	case killCode:
		return (ioKill);

	case ejectCode:
	default:
		return (controlErr);
	}
}


/*
 * driver Status.
 */
drvrStatus(pb,dce)
	register CntrlParam *pb;
	register DCtlEntry *dce;
{
	return (controlErr);
}
