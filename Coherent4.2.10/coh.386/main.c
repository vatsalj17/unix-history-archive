/* $Header: /ker/coh.386/RCS/main.c,v 2.6 93/10/29 00:55:19 nigel Exp Locker: nigel $ */
/*
 * $Log:	main.c,v $
 * Revision 2.6  93/10/29  00:55:19  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.5  93/09/02  18:07:19  nigel
 * Nigel's r85, minor edits only
 * 
 * Revision 2.4  93/08/19  03:26:33  nigel
 * Nigel's r83 (Stylistic cleanup)
 */

#include <kernel/proc_lib.h>
#include <kernel/ddi_cpu.h>
#include <sys/stat.h>

#define	_KERNEL		1

#include <sys/cyrix.h>
#include <kernel/param.h>
#include <kernel/reg.h>
#include <kernel/typed.h>
#include <kernel/trace.h>
#include <sys/cmn_err.h>
#include <sys/uproc.h>
#include <sys/mmu.h>
#include <sys/devices.h>
#include <sys/fdisk.h>
#include <sys/proc.h>
#include <sys/seg.h>

#ifndef VERSION		/* This should be specified at compile time */
#define VERSION	"..."
#endif
#ifndef RELEASE
#define RELEASE "0"
#endif


#if	_USE_IDLE_PROCESS
extern	PROC	      *	iprocp;
#endif


int read_cmos ();

static void set_at_drive_ct ();
static void rpdev ();

/*
 * Configurable variables - see ker/conf/cohmain/Space.c
 *
 * at_drive_ct must be initialized at the number of "at" drives for PS1
 * and ValuePoint systems as the number of hard drives is not in CMOS.
 */

extern short at_drive_ct;

extern dev_t rootdev;
extern dev_t pipedev;
extern int ronflag;
extern int PHYS_MEM;
extern unsigned long _entry;		/* really the serial number */
extern unsigned long _bar;		/* really the serial number also */

/* End of configurable variables. */

char version [] = VERSION;
char release [] = RELEASE;
char copyright [] = "Copyright 1982, 1994 Mark Williams Company\n";


/*
 * set_at_drive_ct ()
 *
 * If "at_drive_ct" is already nonzero, don't change it.
 * Else, read CMOS and return 0, 1, or 2 as number of installed "at" drives.
 */

void
set_at_drive_ct ()
{
	int u;

        if (at_drive_ct <= 0) {
		/*
		 * Count nonzero drive types.
		 *
		 *	High nibble of CMOS 0x12 is drive 0's type.
		 *	Low  nibble of CMOS 0x12 is drive 1's type.
		 */
		u = read_cmos (0x12);
		if (u & 0x00F0)
			at_drive_ct ++;
		if (u & 0x000F)
			at_drive_ct ++;
	}
}


/*
 * rpdev ()
 *
 * If rootdev is zero, try to use data from tboot to set it.
 * If pipedev is zero, make it 0x883 if ronflag == 1, else make it rootdev.
 * Call rpdev () AFTER calling set_at_drive_ct ().
 */

static
void rpdev ()
{
	FIFO * ffp;
	typed_space * tp;
	extern typed_space boot_gift;
	unsigned root_ptn, root_drv, root_maj, root_min;

	if (rootdev == makedev (0, 0)) {
		int rc;

		if ((ffp = fifo_open (& boot_gift, 0)) == NULL)
			cmn_err (CE_PANIC,
				 "Cannot get rootdev from boot_gift ()");

		for (rc = 0; (tp = fifo_read (ffp)) != NULL ; rc ++) {
			BIOS_ROOTDEV * brp = (BIOS_ROOTDEV *) tp->ts_data;

			if (T_BIOS_ROOTDEV != tp->ts_type)
				continue;
				
			root_ptn = brp->rd_partition;

			/*
			 * root_drv = BIOS # of root drive
			 * root_ptn = partition # in range 0..3
			 * if root on second "at" device, add 4 to minor #
			 * if root on second scsi device, add 16 to minor #
			 */

			root_drv = root_ptn / NPARTN;
			root_ptn %= NPARTN;
			if (at_drive_ct > root_drv) {
				root_maj = AT_MAJOR;
				root_min = root_drv * NPARTN + root_ptn;
			} else { /* root on SCSI device */
				root_maj = SCSI_MAJOR;
				root_min = (root_drv - at_drive_ct) * 16 +
						root_ptn;
			}
			rootdev = makedev (root_maj, root_min);
			cmn_err (CE_CONT, "rootdev = (%d,%d)\n", root_maj,
				 root_min);
			break;
		}
		fifo_close (ffp);
	}

	if (pipedev == makedev (0, 0)) {
		if (ronflag)
			pipedev = makedev (RM_MAJOR, 0x83);
		else
			pipedev = rootdev;
		cmn_err (CE_CONT, "pipedev = (%d,%d)\n", (pipedev >> 8) & 0xff,
			 pipedev & 0xff);
	}
}


__EXTERN_C_BEGIN__

__NO_RETURN__	__idle		__PROTO ((void));

__EXTERN_C_END__


/* Special buffering for devices that need a lot of it. */
void	trAlloc		__PROTO ((void));
void	ftAlloc		__PROTO ((void));

void
main ()
{
	extern int BPFMAX;
	char * ndpTypeName ();
	extern int (* ndpEmFn)();
	extern short ndpType;

#if	0
	theSum = checkSum ();
#endif
	
	CHIRP ('a');
	while (& u + 1 < & u) {
		_CHIRP ('t', 8);
		_CHIRP ('k', 6);
		_CHIRP ('u', 4);
		_CHIRP ('h', 2);
		_CHIRP ('p', 0);
	}


	wrNdpUser (0);
	wrNdpSaved (0);
	u.u_bpfmax = BPFMAX;

	set_user_error (0);
	bufinit ();
	_CHIRP ('0', 156);
	cltinit ();
	_CHIRP ('1', 156);
	pcsinit ();
	_CHIRP ('2', 156);
	seginit ();
	_CHIRP ('3', 156);

	/* trAlloc () must be the first of the buffer allocators. */
	trAlloc ();
	ftAlloc ();

	set_at_drive_ct ();
	_CHIRP ('4', 156);
	rpdev ();
	_CHIRP ('5', 156);
	devinit ();
	_CHIRP ('6', 156);
	rlinit ();
	_CHIRP ('7', 156);

	putchar_init ();
	_CHIRP ('8', 156);

	cmn_err (CE_CONT,
		 "*** COHERENT Version %s - 386 Mode.  %uKB free memory. ***\n",
		 release, ctob (allocno ()) / 1024);

	/* Print default display type, based on BIOS int11h call. */
	if ((int11 () & 0x30) == 0x30)
		cmn_err (CE_CONT, "Monochrome.  ");
	else
		cmn_err (CE_CONT, "Color.  ");

	/* Display FP coprocessor/emulation setting. */
	senseNdp ();
	cmn_err (CE_CONT, ndpTypeName ());
	if (ndpType <= 1 && ndpEmFn)
		cmn_err (CE_CONT, "FP Emulation.  ");

	/* Display general system configuration parameters. */
	cmn_err (CE_CONT, "%u buffers.  %u buckets.  %u clists.\n",
		NBUF, NHASH, NCLIST);
	cmn_err (CE_CONT, "%uKB kalloc pool.  %u KB phys pool.\n",
		ALLSIZE / 1024, PHYS_MEM / 1024);

	/* Determine type of CPU and if cache should be enabled */

	if (CYRIX_CPU == 0)
		CYRIX_CPU = cyrix_detect();

	switch(CYRIX_CPU) {
		case CYRIX_NOT:
			cmn_err(CE_CONT, "Intel/AMD CPU Detected\n");
			break;

		case CYRIX_OEM:
			cmn_err(CE_CONT, "Cyrix OEM CPU Detected\n");
			break;

		default:
			cmn_err(CE_CONT, "Cyrix Upgrade CPU Detected -- ");
			if(CYRIX_CACHE) {
				cmn_err(CE_CONT, "Turning cache on.\n");
				cyrix_cache_on();
			}
			else
				cmn_err(CE_CONT, "Cache not turned on.\n");
			break;
	}

	cmn_err (CE_CONT, copyright);

	if (_entry)
		cmn_err (CE_CONT, "Serial Number %lu\n", _entry);

	/*
	 * Verify correct serial number
	 */

	if (_entry != _bar)
		cmn_err (CE_PANIC, "Verification error - call Mark Williams "
			 "Company at + 1-708-291-6700\n");

	iprocp = SELF;
	CHIRP ('b');

#if	_USE_IDLE_PROCESS
	if (pfork ()) {
		CHIRP ('i');
		__idle ();
	}
#endif

	fsminit ();
	CHIRP ('-');
	eprocp = SELF;
	eveinit ();
	CHIRP ('=');

#if	0
	checkTheSum ();
#endif
	CHIRP ('c');
}
