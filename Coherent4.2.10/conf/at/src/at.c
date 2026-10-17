/*
 * $Header: /v/src/rcskrnl/conf/at/src/RCS/at.c,v 420.6 1993/12/01 22:10:37 srcadm Exp srcadm $
 */

/*********************************************************************
 *
 * Coherent, Mark Williams Company
 * RCS Header
 * This file contains proprietary information and is considered
 * a trade secret.  Unauthorized use is prohibited.
 *
 * $Id: at.c,v 420.6 1993/12/01 22:10:37 srcadm Exp srcadm $
 *
 * $Log: at.c,v $
 * Revision 420.6  1993/12/01  22:10:37  srcadm
 * Fixed 'break' bug on read failure in state machine and
 * added printf for version and revision tracking.
 *
 * Revision 420.4  1993/11/30  19:31:21  srcadm
 * Initial RCS submission.
 */

/* Embedded version constant */

static char rcsid[] = 	"#(@) $Id"
			"at.c: Last Modified: Sun Apr 24 22:46:56 1994 by [chris]";

/*
 * This is a driver for the
 * hard disk on the AT.
 *
 * Reads drive characteristics from ROM (thru interrupt vector 0x41 and 0x46).
 * Reads partition information from disk.
 *
 * Revision 2.5  93/08/20  10:49:54  nigel
 * Fix race work queue interlock problem
 * 
 * Revision 2.4  93/08/19  10:38:34  nigel
 * r83 ioctl (), corefile, new headers
 * 
 * Revision 2.3  93/08/19  04:02:13  nigel
 * Nigel's R83
 */

#include <common/_ccompat.h>
#include <sys/coherent.h>
 
#include <stdlib.h>
#include <kernel/typed.h>

#include <sys/buf.h>
#include <sys/cmn_err.h>
#include <sys/con.h>
#include <sys/devices.h>
#include <sys/errno.h>
#include <sys/fdisk.h>
#include <sys/hdioctl.h>
#include <sys/inline.h>
#include <sys/stat.h>
#include <sys/uproc.h>

#include <coh/defer.h>
#include <coh/fakedma.h>
#include <coh/misc.h>

#define	LOCAL

/*
 * Configurable parameters
 */

#define	HDBASE	0x01F0			/* Port base */
#define	HARDLIM	8			/* number of retries before fail */
#define	BADLIM	100			/* num to stop recov if flagged bad */

#define	BIT(n)		(1 << (n))

#define	CMOSA	0x70			/* write cmos address to this port */
#define	CMOSD	0x71			/* read cmos data through this port */

#if	_I386
# define	VERBOSE		1
#endif

/*
 * I/O Port Addresses
 */

#define	DATA_REG	(HDBASE + 0)	/* data (r/w) */
#define	AUX_REG		(HDBASE + 1)	/* error(r), write precomp cyl/4 (w) */
#define	NSEC_REG	(HDBASE + 2)	/* sector count (r/w) */
#define	SEC_REG		(HDBASE + 3)	/* sector number (r/w) */
#define	LCYL_REG	(HDBASE + 4)	/* low cylinder (r/w) */
#define	HCYL_REG	(HDBASE + 5)	/* high cylinder (r/w) */
#define	HDRV_REG	(HDBASE + 6)	/* drive/head (r/w) (D <<4)+(1 << H) */
#define CHS_MAGIC	0xa0		/* Magic cookie for CHS access */
#define	LBA0_REG	(HDBASE + 3)	/* Logical block address byte 0 */
#define	LBA1_REG	(HDBASE + 4)	/* Logical block address byte 1 */
#define	LBA2_REG	(HDBASE + 5)	/* Logical block address byte 2 */
#define	LBA3_REG	(HDBASE + 6)	/* Logical block address byte 3 */
#define LBA_MAGIC	0xe0		/* Magic cookie for LBA access */
#define	CSR_REG		(HDBASE + 7)	/* status (r), command (w) */
#define	HF_REG		(HDBASE + 0x206)	/* Usually 0x3F6 */
#define DISABLE_ATINTR	0x02

/*
 * Error from AUX_REG (r)
 */

#define	DAM_ERR		BIT(0)		/* data address mark not found */
#define	TR0_ERR		BIT(1)		/* track 000 not found */
#define	ABT_ERR		BIT(2)		/* aborted command */
#define	ID_ERR		BIT(4)		/* id not found */
#define	ECC_ERR		BIT(6)		/* data ecc error */
#define	BADBLK_ERR	BIT(7)		/* bad block detect */


/*
 * Status from CSR_REG (r)
 */

#define	ERR_ST		BIT(0)		/* error occurred */
#define	INDEX_ST	BIT(1)		/* index pulse */
#define	SOFT_ST		BIT(2)		/* soft (corrected) ECC error */
#define	DRQ_ST		BIT(3)		/* data request */
#define	SKC_ST		BIT(4)		/* seek complete */
#define	WFLT_ST		BIT(5)		/* improper drive operation */
#define	RDY_ST		BIT(6)		/* drive is ready */
#define	BSY_ST		BIT(7)		/* controller is busy */


/*
 * Commands to CSR_REG (w)
 */

#define	RESTORE(rate)	(0x10 +(rate))	/* X */
#define	SEEK(rate)	(0x70 +(rate))	/* X */
#define	READ_CMD	(0x20)		/* X */
#define	WRITE_CMD	(0x30)		/* X */
#define	FORMAT_CMD	(0x50)		/* X */
#define	VERIFY_CMD	(0x40)		/* X */
#define	DIAGNOSE_CMD	(0x90)		/* X */
#define	SETPARM_CMD	(0x91)		/* X */
#define IDDRIVE_CMD     (0xEC)

/*
 * Device States.
 */

typedef enum {
	SIDLE = 0,
	SRESET,
	SRETRY,
	SREAD,
	SWRITE
} at_state_t;

/*
 * Set up to report a timeout.
 */

static	int		report_scheduled;
static	int		report_drv;

/***********************************************************************
 *  Forward referenced local functions.
 */

LOCAL void	atreset		__PROTO((void));
LOCAL int	atdequeue 	__PROTO((void));
LOCAL void	atstart		__PROTO((void));
LOCAL int	aterror		__PROTO((void));
LOCAL void	atrecov		__PROTO((void));

#define	NOTBUSY()	((inb (ATSREG) & BSY_ST) == 0)
#define	ATBSYW(u)	(NOTBUSY () ? 1 : myatbsyw (u))

#define	DATAREQUESTED()	((inb (ATSREG) & DRQ_ST) != 0)
#define	ATDRQ()		(DATAREQUESTED () ? 1 : atdrq ())

/*** To Do *************************************************************
 *  We should change the way that we calculate the indexs in the
 *  structures for access data about individual partitions. That would
 *  make this a lot less confusing.
 */

#define partn(dev)	((minor(dev) % (N_ATDRV * NPARTN)) + \
			 ((minor(dev) & SDEV) ? (N_ATDRV * NPARTN) : 0))
#define partnbase(p)	(pparm[p]->p_base)
#define partnsize(p) 	(pparm[p]->p_size)

#define drv(dev)	((minor(dev) & SDEV) ? (minor(dev) % N_ATDRV) : \
					       (minor(dev) / NPARTN))
extern typed_space	boot_gift;
extern short		at_drive_ct;
extern int		at_nsecmax;
/*
 *	ATSECS is number of seconds to wait for an expected interrupt.
 *	ATSREG needs to be 3F6 for most new IDE drives;  needs to be
 *		1F7 for Perstor controllers and some old IDE drives.
 *		Either value works with most drives.
 *	atparm - drive parameters.  If initialized zero, try to use ROM values.
 */

extern	unsigned	ATSECS;
extern	unsigned	ATSREG;
extern	struct hdparm_s	atparm [];

/*
 * Partition Parameters - copied from disk.
 *
 *	There are N_ATDRV * NPARTN positions for the user partitions,
 *	plus N_ATDRV additional partitions to span each drive.
 *
 *	Aligning partitions on cylinder boundaries:
 *	Optimal partition size: 2 * 3 * 4 * 5 * 7 * 17 = 14280 blocks
 *	Acceptable partition size:  3 * 4 * 5 * 7 * 17 =  7140 blocks
 */

static struct fdisk_s pparm [N_ATDRV * NPARTN + N_ATDRV];


/*
 * Per disk controller data.
 * Only one controller; no more, no less.
 */

static struct at {
  buf_t	        *at_actf;	/* Link to first */
  buf_t	        *at_actl;	/* Link to last */
  daddr_t	at_bno;		/* Block # on disk */
  int		at_bufcnt;	/* Block count */
  unsigned	at_drv;
  unsigned	at_partn;
  unsigned char	at_dtype [N_ATDRV]; /* drive type, 0 if unused */
  unsigned char	at_tries;
  at_state_t	at_state;
  int           at_use_BIOS_parms[N_ATDRV]; /* Non-zero if using BIOS parms */
} at;


static char timeout_msg [] = "at%d: TO\n";
ide_info_t ide_drive_info[N_ATDRV]; /* info from drive itself */

#if __USE_PROTO__
static void
at_byte_order_copy(unsigned char *dest, unsigned char *src, int n)
#else
static void
at_byte_order_copy(dest, src, n)
unsigned char *dest;
unsigned char *src;
int n;
#endif /* __USE_PROTO__ */
{
  while (n) {
    *dest = *(src + 1);
    *(dest + 1) = *src;
    dest += 2;
    src += 2;
    n -= 2;
  }
  *dest = '\0';
}

#if __USE_PROTO__
LOCAL int notBusy (void)
#else
LOCAL int
notBusy ()
#endif

{
	return NOTBUSY ();
}

#if __USE_PROTO__
LOCAL void _report_timeout (void)
#else
LOCAL void
_report_timeout ()
#endif

{
	cmn_err(CE_CONT, timeout_msg, report_drv);
	report_scheduled = 0;
}

#if __USE_PROTO__
LOCAL void report_timeout (int unit) 
#else
LOCAL void
report_timeout (unit)
int		unit;
#endif

{
	short		s = sphi ();
	if (report_scheduled == 0) {
		report_scheduled = 1;
		spl (s);

		report_drv = unit;
		defer ((__DEFERRED_FN_PTR)_report_timeout, 0);
	} else
		spl (s);
}

/*
 * Wait while controller is busy.
 *
 * Return 0 if timeout, nonzero if not busy.
 */

#if __USE_PROTO__
LOCAL int myatbsyw (int unit) 
#else
LOCAL int
myatbsyw (unit)
int unit;
#endif

{
	if (busyWait (notBusy, ATSECS * HZ))
		return 1;
	report_timeout (unit);
	return 0;
}

#if __USE_PROTO__
int notReadyComp (void)
#else
int
notReadyComp ()
#endif
{
  return ((inb(ATSREG) & (BSY_ST | RDY_ST | SKC_ST)) == (RDY_ST | SKC_ST));
}

#if __USE_PROTO__
int wait_for_ready_comp(int a)
#else
int
wait_for_ready_comp (a)
int a;
#endif
{
  if ((inb(ATSREG) & (BSY_ST | RDY_ST | SKC_ST)) == (RDY_ST | SKC_ST))
    return 0;

  if (busyWait(notReadyComp, ATSECS * HZ))
    return 0;

  report_timeout(a);
  return -1;
}

#if __USE_PROTO__
LOCAL int dataRequested (void)
#else
LOCAL int
dataRequested ()
#endif

{
	return DATAREQUESTED ();
}

/*
 * Wait for controller to initiate request.
 *
 * Return 0 if timeout, 1 if data requested.
 */

#if __USE_PROTO__
LOCAL int atdrq (void)
#else
LOCAL int
atdrq ()
#endif

{
	if (busyWait (dataRequested, ATSECS * HZ))
		return 1;
	report_timeout (at.at_drv);
	return 0;
}

#if __USE_PROTO__
int at_get_drive_id(int dr)
#else
int
at_get_drive_id(dr)
int dr;
#endif
{
  int s;
  unsigned char status;
  ide_info_t *dr_info = &ide_drive_info[dr];

  s = sphi();

  /* Default to using BIOS params in case of failure */
  at.at_use_BIOS_parms[dr] = 1;

  /* Wait for drive to not be busy */
  ATBSYW(dr);

  /* Select the proper drive */
  outb(HDRV_REG, dr ? 0xb0 : 0xa0);
  
  /* Wait for drive to be selected */
  if (wait_for_ready_comp(dr)) {
    cmn_err(CE_WARN, "at%dx Failure selecting drive", dr);
    spl(s);
    atreset();
    return -1;
  }

  inb(CSR_REG);	/* Clear the interrupt this generated */

  outb(CSR_REG, IDDRIVE_CMD);	/* Send ID request command */

  ATBSYW(dr);
  status = inb(CSR_REG);

  if (status & ERR_ST) {
    spl(s);
    atreset();
    return -1;
  }

  if (ATDRQ() == 0) {
    cmn_err(CE_WARN, "at%dx Failure reading status", dr);
    spl(s);
    atreset();
    return -1;
  }

  repinsw(DATA_REG, (ushort_t *)dr_info, 256);

  /* Drive info request was successful, so we use these params instead */
  at.at_use_BIOS_parms[dr] = 0;
  spl(s);
  
  atreset();

  return 0;
}

/*
 * void
 * atload ()	- load routine.
 *
 *	Action:	The controller is reset and the interrupt vector is grabbed.
 *		The drive characteristics are set up at this time.
 */
#if __USE_PROTO__
LOCAL void atload(void)
#else
LOCAL void
atload ()
#endif
{
  size_t ptnsize;
  unsigned int u;
  struct hdparm_s * dp;
  struct { unsigned short off, seg; } p;
  
  if (at_drive_ct <= 0)
    return;
  
  /* Flag drives 0, 1 as present or not. */
  at.at_dtype [0] = 1;
  at.at_dtype [1] = at_drive_ct > 1 ? 1 : 0;
  
#if 0
  /* hex dump boot gift */
  {
    int bgi;
    unsigned char * bgp = (char *)& boot_gift;
    cmn_err(CE_CONT, "& boot_gift = %lx", & boot_gift);
    for (bgi = 0; bgi < 80; bgi ++) {
      cmn_err(CE_CONT, " %x", (* bgp ++));
    }
  }
#endif
  
  /*
   * Obtain Drive Characteristics.
   */
  for (u = 0, dp = atparm; u < at_drive_ct; ++ dp, ++ u) {
    struct hdparm_s int_dp;
    unsigned short ncyl = _CHAR2_TO_USHORT (dp->ncyl);
    
    if (ncyl == 0) {
      /*
       * Not patched.
       *
       * If tertiary boot sent us parameters,
       *   Use "fifo" routines to fetch them.
       *   This only gives us ncyl, nhead, and nspt.
       *   Make educated guesses for other parameters:
       *   Set landc to ncyl, wpcc to -1.
       *   Set ctrl to 0 or 8 depending on head count.
       *
       * Follow INT 0x41/46 to get drive static BIOS drive
       * parameters, if any.
       *
       * If there were no parameters from tertiary boot,
       * or if INT 0x4? nhead and nspt match tboot parms,
       *   use "INT" parameters (will give better match on
       *   wpcc, landc, and ctrl fields, which tboot can't
       *   give us).
       */
      
      FIFO * ffp;
      typed_space * tp;
      int found, parm_int;
      
      if (F_NULL != (ffp = fifo_open (& boot_gift, 0))) {
	for (found = 0; ! found && (tp = fifo_read (ffp)); ) {
	  BIOS_DISK * bdp = (BIOS_DISK *)tp->ts_data;
	  
	  if ((T_BIOS_DISK == tp->ts_type) && (u == bdp->dp_drive) ) {
	    found = 1;
	    _NUM_TO_CHAR2(dp->ncyl, bdp->dp_cylinders);
	    dp->nhead = bdp->dp_heads;
	    dp->nspt = bdp->dp_sectors;
	    _NUM_TO_CHAR2(dp->wpcc, 0xffff);
	    _NUM_TO_CHAR2(dp->landc, bdp->dp_cylinders);
	    
	    if (dp->nhead > 8)
	      dp->ctrl |= 8;
	  }
	}
	fifo_close (ffp);
      }
      
      if (u == 0)
	parm_int = 0x41;
      else /* (u == 1) */
	parm_int = 0x46;
      
      pxcopy ((paddr_t)(parm_int * 4),
	(__caddr_t)(& p), sizeof p, SEL_386_KD);
      pxcopy ((paddr_t)(p.seg <<4L) + p.off,
        (__caddr_t)(& int_dp), sizeof (int_dp), SEL_386_KD);
      
      if (! found || (dp->nhead == int_dp.nhead && dp->nspt == int_dp.nspt)) {
	* dp = int_dp;
	cmn_err(CE_CONT, "Using INT 0x%x", parm_int);
      } else
	cmn_err(CE_CONT, "Using INT 0x13(08)");
    } else {
      cmn_err(CE_CONT, "Using patched");
      
      /*
       * Avoid incomplete patching.
       */
      if (at.at_dtype [u] == 0)
	at.at_dtype [u] = 1;
      
      if (dp->nspt == 0)
	dp->nspt = 17;
      
#if FORCE_CTRL_8
      if (dp->nhead > 8)
	dp->ctrl |= 8;
#endif
    }
    
#if VERBOSE > 0
    cmn_err(CE_CONT, " drive %d parameters\n", u);
    
    cmn_err(CE_CONT,
	    "at%d: ncyl=%d nhead=%d wpcc=%d eccl=%d ctrl=%d landc=%d "
	    "nspt=%d\n", u, _CHAR2_TO_USHORT (dp->ncyl), dp->nhead,
	    _CHAR2_TO_USHORT (dp->wpcc), dp->eccl, dp->ctrl,
	    _CHAR2_TO_USHORT (dp->landc), dp->nspt);
#endif
  }
  
  /*
   * Initialize drive size and set access method if it hasn't 
   * been set already.
   */
  for (u = 0, dp = atparm; u < at_drive_ct; ++ dp, ++ u) {
    unsigned char tmp_model[41];
    unsigned char tmp_rev[9];
    unsigned char tmp_serno[21];
    ide_info_t *ti;
    
    if (at.at_dtype [u] == 0)
      continue;
    
    ptnsize = (long) _CHAR2_TO_USHORT(dp->ncyl) * dp->nhead * dp->nspt;
    pparm [N_ATDRV * NPARTN + u].p_size = ptnsize;
    
    /* Get the drive id info if possible */
    if (at_get_drive_id(u) == 0) {
      ti = &ide_drive_info[u];
      at_byte_order_copy(tmp_model, (unsigned char *)ti->ii_modelnum, 40);
      at_byte_order_copy(tmp_rev, (unsigned char *)ti->ii_firmrev, 8);
      at_byte_order_copy(tmp_serno, (unsigned char *)ti->ii_serialnum, 20);
      cmn_err(CE_CONT, "at%d: Model %s\n", u, tmp_model);
      cmn_err(CE_CONT, "at%d: Firmware Revision: %s   Ser. No.: %s\n",
	      u, tmp_rev, tmp_serno);

      /* Override BIOS parameters if IDE drive info available, when
         computing partition size. */
      ptnsize = (long) ti->ii_cyl * ti->ii_heads * ti->ii_spt;
      pparm [N_ATDRV * NPARTN + u].p_size = ptnsize;
    
    } else {
      cmn_err(CE_CONT, "at%d: MFM/RLL/ESDI or non-identifiable IDE", u);
    }
  }
  
  /*
   * Initialize Drive Controller.
   */
  atreset ();
}


/*
 * void
 * atunload ()	- unload routine.
 */

#if __USE_PROTO__
LOCAL void atunload(void) 
#else
LOCAL void
atunload ()
#endif

{
}

  
/*
 * void
 * atreset ()	-- reset hard disk controller, define drive characteristics.
 */

#if __USE_PROTO__
LOCAL void atreset(void)
#else
LOCAL void
atreset ()
#endif
{
  int 		u;
  struct hdparm_s *dp;
  int		s;
  
  s = sphi();
  at.at_state = SRESET;
  outb (HF_REG, 4);	/* Reset the Controller */
  busyWait2(NULL, 12);	/* Wait for minimum of 4.8 usec */
  outb (HF_REG, atparm [0].ctrl & 0x0F);
  
  ATBSYW (0);
  if (inb (AUX_REG) != 0x01) {
    /*
     * Some IDE drives always timeout on initial reset.
     * So don't report first timeout.
     */
    static one_bad;
    
    if (one_bad)
      cmn_err(CE_NOTE, "at: hd controller reset timeout\n");
    else
      one_bad = 1;
  }
  
  /*
   * Initialize drive parameters.
   */
  
  for (u = 0, dp = atparm; u < at_drive_ct; ++ dp, ++ u) {
    ide_info_t *tmp_ide_info;

    if (at.at_dtype [u] == 0)
      continue;
    
    ATBSYW (u);
    
    /*
     * Set drive characteristics.  The problem
     * is that with an EDPT drive we are supposed
     * to be using the IDE level CHS info and not
     * the bios info.  Whenever possible, we use
     * what the drive reports back to use over the BIOS info.
     */
    if (at.at_use_BIOS_parms[u] == 0) { /* Use IDE params from drive */
      tmp_ide_info = &ide_drive_info[u];
      outb (HF_REG, dp->ctrl);
      outb (HDRV_REG, 0xA0 + (u << 4) + tmp_ide_info->ii_heads - 1);
      
      outb (AUX_REG, _CHAR2_TO_USHORT (dp->wpcc) / 4);
      outb (NSEC_REG, tmp_ide_info->ii_spt);
      outb (SEC_REG, 0x01);
      outb (LCYL_REG, tmp_ide_info->ii_cyl & 0xFF);
      outb (HCYL_REG, tmp_ide_info->ii_cyl >> 8);
      outb (CSR_REG, SETPARM_CMD);
      ATBSYW (u);		/* Wait blindly */
      inb(CSR_REG);		/* Clear the interrupt this generated */
    } else {			/* Use the BIOS params */
      outb (HF_REG, dp->ctrl);
      outb (HDRV_REG, 0xA0 + (u << 4) + dp->nhead - 1);
      
      outb (AUX_REG, _CHAR2_TO_USHORT (dp->wpcc) / 4);
      outb (NSEC_REG, dp->nspt);
      outb (SEC_REG, 0x01);
      outb (LCYL_REG, dp->ncyl [0]);
      outb (HCYL_REG, dp->ncyl [1]);
      outb (CSR_REG, SETPARM_CMD);
      ATBSYW (u);		/* Wait blindly */
      inb(CSR_REG);		/* Clear the interrupt this generated */
    }
    
    /*
     * Restore heads.
     */
    
    outb (CSR_REG, RESTORE (0));
    ATBSYW (u);		/* Wait blindly */
    inb(CSR_REG);	/* Clear the interrupt this generated */
  }
  
  outb(HF_REG, dp->ctrl);
  at.at_state = SIDLE;
  spl(s);
}


/*
 * void
 * atopen (dev, mode)
 * dev_t dev;
 * int mode;
 *
 *	Input:	dev = disk device to be opened.
 *		mode = access mode [IPR, IPW, IPR + IPW].
 *
 *	Action:	Validate the minor device.
 *		Update the paritition table if necessary.
 */
 
#if __USE_PROTO__
LOCAL void atopen(dev_t dev /* , int mode */) 
#else
LOCAL void
atopen (dev /* , mode */)
dev_t	dev;
#endif
{
	int d;		/* drive */
	int p;		/* partition */

	p = minor (dev) % (N_ATDRV * NPARTN);

	if (minor (dev) & SDEV) {
		d = minor (dev) % N_ATDRV;
		p += N_ATDRV * NPARTN;
	} else
		d = minor (dev) / NPARTN;

	if (d >= N_ATDRV || at.at_dtype [d] == 0) {
		cmn_err(CE_WARN, "atopen: drive %d not present ", d);
		set_user_error (ENXIO);
		return;
	}

	if (minor (dev) & SDEV)
		return;

	/*
	 * If partition not defined read partition characteristics.
	 */
	if (pparm [p].p_size == 0)
		fdisk (makedev (major (dev), SDEV + d), & pparm [d * NPARTN]);

	/*
	 * Ensure partition lies within drive boundaries and is non-zero size.
	 */
	if (pparm [p].p_base + pparm [p].p_size >
	    pparm [d + N_ATDRV * NPARTN].p_size) {
		cmn_err(CE_WARN, "atopen: p_size too big ");
		set_user_error (EINVAL);
	} else if (pparm [p].p_size == 0) {
		cmn_err(CE_WARN, "atopen: p_size zero ");
		set_user_error (ENODEV);
	}
}


/*
 * void
 * atread (dev, iop)	- write a block to the raw disk
 * dev_t dev;
 * IO * iop;
 *
 *	Input:	dev = disk device to be written to.
 *		iop = pointer to source I/O structure.
 *
 *	Action:	Invoke the common raw I/O processing code.
 */
#if __USE_PROTO__
LOCAL void atread(dev_t dev, IO *iop) 
#else
LOCAL void
atread (dev, iop)
dev_t	dev;
IO	* iop;
#endif
{
	ioreq (NULL, iop, dev, BREAD, BFRAW | BFBLK | BFIOC);
}


/*
 * void
 * atwrite (dev, iop)	- write a block to the raw disk
 * dev_t dev;
 * IO * iop;
 *
 *	Input:	dev = disk device to be written to.
 *		iop = pointer to source I/O structure.
 *
 *	Action:	Invoke the common raw I/O processing code.
 */

#if __USE_PROTO__
LOCAL void atwrite(dev_t dev, IO *iop) 
#else
LOCAL void
atwrite (dev, iop)
dev_t	dev;
IO	* iop;
#endif

{
	ioreq (NULL, iop, dev, BWRITE, BFRAW | BFBLK | BFIOC);
}


/*
 * void
 * atioctl (dev, cmd, arg)
 * dev_t dev;
 * int cmd;
 * char * vec;
 *
 *	Input:	dev = disk device to be operated on.
 *		cmd = input / output request to be performed.
 *		vec = (pointer to) optional argument.
 *
 *	Action:	Validate the minor device.
 *		Update the paritition table if necessary.
 */

#if __USE_PROTO__
LOCAL void atioctl(dev_t dev, int cmd, char * vec)
#else
LOCAL void
atioctl (dev, cmd, vec)
dev_t dev;
int cmd;
char *vec;
#endif /* __USE_PROTO__ */
{
  int d;
  
  /*
   * Identify drive number.
   */
  if (minor (dev) & SDEV)
    d = minor (dev) % N_ATDRV;
  else
    d = minor (dev) / NPARTN;
  
  /*
   * Identify input / output request.
   */
  
  switch (cmd) {
    
  case HDGETA:
    /*
     * Get hard disk attributes.
     */
    kucopy (atparm + d, vec, sizeof (atparm [0]));
    break;
    
  case HDGETIDEINFO:
    /*
     * Get info from an IDE drive
     */
    if (at.at_use_BIOS_parms[d]) {
      set_user_error(ENXIO);
    }
    else
      kucopy(&ide_drive_info[d], vec, sizeof(ide_info_t));
    break;
    
  case HDSETA:
    /* Set hard disk attributes. */
    ukcopy (vec, atparm + d, sizeof (atparm [0]));
    at.at_dtype [d] = 1;		/* set drive type nonzero */
    pparm [N_ATDRV * NPARTN + d].p_size =
      (long) _CHAR2_TO_USHORT (atparm [d].ncyl) *
	atparm [d].nhead * atparm [d].nspt;
    atreset ();
    break;
    
  default:
    set_user_error (EINVAL);
    break;
  }
}

/*
 * void
 * atwatch ()		- guard against lost interrupt
 *
 *	Action:	If drvl [AT_MAJOR] is greater than zero, decrement it.
 *		If it decrements to zero, simulate a hardware interrupt.
 */

#if __USE_PROTO__
LOCAL void atwatch (void)
#else
LOCAL void
atwatch ()
#endif

{
	int s;

	s = sphi ();
	if (--drvl[AT_MAJOR].d_time <= 0) {
		cmn_err(CE_WARN, "at: <Watchdog timeout>");
		atreset();
		atrecov();
	}
	spl (s);
}


/*
 * void
 * atblock (bp)	- queue a block to the disk
 *
 *	Input:	bp = pointer to block to be queued.
 *
 *	Action:	Queue a block to the disk.
 *		Make sure that the transfer is within the disk partition.
 */

#if __USE_PROTO__
LOCAL void atblock (buf_t *bp) 
#else
LOCAL void
atblock (bp)
buf_t	* bp;
#endif

{
	struct fdisk_s * pp;
	int partn = minor (bp->b_dev) % (N_ATDRV * NPARTN);
	int		s;

	bp->b_resid = bp->b_count;

	if (minor (bp->b_dev) & SDEV)
		partn += N_ATDRV * NPARTN;

	pp = pparm + partn;

	/*
	 * Check for read at end of partition.
	 */

	if (bp->b_req == BREAD && bp->b_bno == pp->p_size) {
		bdone (bp);
		return;
	}

	/*
	 * Range check disk region.
	 */

	if (bp->b_bno + (bp->b_count / BSIZE) > pp->p_size ||
	    bp->b_count % BSIZE != 0 || bp->b_count == 0) {
		bp->b_flag |= BFERR;
		bdone (bp);
		return;
	}

	s = sphi ();
	bp->b_actf = NULL;
	if (at.at_actf == NULL)
		at.at_actf = bp;
	else
		at.at_actl->b_actf = bp;
	at.at_actl = bp;
	spl (s);

	if (at.at_state == SIDLE && atdequeue())
		atstart ();
}


/*
 * int
 * atdequeue ()		- obtain next disk read / write operation
 *
 *	Action:	See if there is work to do in the work queue. If so
 *		zero the retry counter.
 *
 *	Return:	0 = no work.
 *		1 = work to do.
 */

#if __USE_PROTO__
LOCAL int atdequeue (void)
#else
LOCAL int
atdequeue ()
#endif

{
	if (!at. at_actf)
		return 0;
	else {
		at.at_tries = 0;
		return 1;
	}
}	/* atdequeue() */

#if __USE_PROTO__
LOCAL int atsend(buf_t *bp) 
#else
LOCAL int
atsend(bp)
buf_t	*bp;
#endif

{
	if (!bp)
		cmn_err(CE_PANIC, "at: <atsend: no buffer>");

        if (ATDRQ () == 0) {
                cmn_err(CE_WARN, "at: <Failure starting write>");
                atreset();
                return 0;
        }
        else {
		repoutsw(DATA_REG,
			 (ushort_t *) (bp->b_vaddr + bp->b_count - bp->b_resid),
			 BSIZE / 2);
		return 1;	
	}
}

#if __USE_PROTO__
LOCAL int atrecv (buf_t *bp) 
#else
LOCAL int
atrecv (bp)
buf_t	*bp;
#endif /* __USE_PROTO__ */
{
	if (!bp)
		cmn_err(CE_PANIC, "at.c: <atrecv: with no buffer>");
		
	if (ATDRQ() == 0) {
		cmn_err(CE_WARN, "at: <Failure starting read>");
		atreset();
		return 0;
	}
	else {
		repinsw(DATA_REG,
			(ushort_t *) (bp->b_vaddr + bp->b_count - bp->b_resid),
			BSIZE / 2);
		return 1;
	}
}	/* atrecv() */



/*
 * void
 * atstart ()	- start or restart next disk read / write operation.
 *
 *	Action:	Initiate disk read / write operation.
 */

#if __USE_PROTO__
LOCAL void atstart (void)
#else
LOCAL void
atstart ()
#endif /* __USE_PROTO__ */
{
  buf_t *bp;			/* Utility buffer. */
  ldiv_t addr;			/* Utility address */
  struct hdparm_s *dp;		/* This drive's paramters */
  unsigned ncyl;		/* Number of cylinders */
  unsigned wpcc;		/* Write precomp cylinder */
  int s;
  
  /***************************************************************
   *  These parameters are calculated for this i/o request from
   *  the drive. Note that one i/o request can encompass many
   *  sectors.
   */
  
  unsigned	start_blkno;	/* Starting physical block no. */
  unsigned 	start_cyl;	/* Starting sector */
  unsigned 	start_head;	/* Starting head */
  unsigned 	start_sect;	/* Starting sector */
  unsigned 	nsec;		/* # sectors all requests so far */
  unsigned 	secs;		/* # sectors, this buffer */
  int		bufcnt;		/* Number of buffer, this request */
  

  /***************************************************************
   *  Sanity check - Is there any work to do?
   */
  
  if (!(bp = at. at_actf)) {
    cmn_err(CE_NOTE, "at: atstart called with empty work queue\n");
    drvl[AT_MAJOR]. d_time = 0;
    return;
  }
  
  
  /***************************************************************
   *  Now calculate the parameters for the controller for the
   *  current request.
   */
  
  at.at_partn = partn(bp->b_dev);
  at.at_drv = drv(bp->b_dev);
  start_blkno = bp->b_bno + ((bp->b_count - bp->b_resid) / BSIZE) +
    pparm[at.at_partn].p_base;
  
  /*
   * Added by Louis to trap divide by zero problem.
   * When the disk becomes full, for some reason one
   * of the ldiv's below causes a kernel panic with
   * a trap on divide by zero.  Unless the atparm
   * struct is being corrupted, at.at_drv may be wrong
   * if given a negative drive number, or one too large.
   * Assuming that N_ATDRV does not change, this means there
   * is a sign-extension problem of some type and/or
   * there is a plain-old-hosed inode on the system displaying
   * this behavior, since bp->b_dev in the system call
   * that paniced got the b_dev value from the inode
   * i_rdev field.  Also, sign extension is possible, since
   * minor(a) is just #def'ed as (a & 0xFF).....
   */
  
  ASSERT(at.at_drv >= 0 && at.at_drv <= 1);
  ASSERT(at.at_partn >= 0);
  
  /***************************************************************
   *  Generate a pointer to the disk drive geometry table.
   */
  dp = &(atparm[at.at_drv]);

  if (at.at_use_BIOS_parms[at.at_drv]) {
    addr = ldiv (start_blkno, dp->nspt);
    start_sect = addr.rem + 1;
    addr = ldiv (addr.quot, dp->nhead);
    start_cyl = addr.quot;
    start_head = addr.rem;
  } else {			/* Use IDE params */
    addr = ldiv(start_blkno, ide_drive_info[at.at_drv].ii_spt);
    start_sect = addr.rem + 1;
    addr = ldiv(addr.quot, ide_drive_info[at.at_drv].ii_heads);
    start_cyl = addr.quot;
    start_head = addr.rem;
  }

  /*
   * NIGEL: It is unclear why, but IDE writes appear to always blow a
   * revolution no matter what, even though reads appear to work quite
   * comfortably. It may be that this is a problem caused by IDE drives
   * trying to maintain the synchronous semantics of the write, and/or
   * because we are actually not making the read time either but the
   * slack is taken up by track-buffering.
   *
   * Either way, we gain a vast improvement in throughput for writes and
   * a modest gain for reads by looking ahead in the request chain and
   * coalescing separate requests to consecutive blocks into a single
   * multi-sector request (as far as the interface is concerned).
   */
  
  nsec = secs = bp->b_resid / BSIZE;
  bufcnt = 1;
  while (bp->b_actf != NULL && bp->b_actf->b_bno == bp->b_bno + secs &&
	 bp->b_actf->b_req == bp->b_req && bp->b_actf->b_dev == bp->b_dev) {
    bp = bp->b_actf;
    
    /*******************************************************
     *  The sector count register on the controller is
     *  one byte long. Make sure that the number of sectors
     *  to read will fit. Notice that a 0 in this register
     *  means read 256 sectors.
     */
    
    if (nsec + (secs = bp->b_resid / BSIZE) > at_nsecmax)
      break;

    nsec += secs;
    ++bufcnt;
  }

  at.at_bufcnt = bufcnt;
  
  s = sphi();
  
  ATBSYW(at.at_drv);
  
  dp = atparm + at.at_drv;
  wpcc = (unsigned) _CHAR2_TO_USHORT(dp->wpcc);
  ncyl = (unsigned) _CHAR2_TO_USHORT(dp->ncyl);
  outb (HF_REG, dp->ctrl);
  
  /*
   * If the write precompensation set for the drive is invalid
   * then don't set the write precompensation register. According
   * to "The Undocumented PC" as this function is no longer needed
   * by PC drives so many controllers have usurped this function.
   * Don't use it if we don't have to.  We use the BIOS params for
   * write precomp so the user has a way to force it off (by setting
   * it invalid).
   */
  
  if ((unsigned) wpcc < (unsigned) ncyl)
    outb (AUX_REG, wpcc / 4);
  
  outb (SEC_REG, start_sect);
  outb (LCYL_REG, start_cyl);
  outb (HCYL_REG, start_cyl >> 8);
  outb (HDRV_REG, (CHS_MAGIC |
		   (at.at_drv << 4) |
		   (start_head & 0x0f)));
  
  outb (NSEC_REG, nsec);
  
  if (at.at_actf->b_req == BREAD) {
    at.at_state = SREAD;
    outb (CSR_REG, READ_CMD);
  }
  else {
    at.at_state = SWRITE;
    outb (CSR_REG, WRITE_CMD);
    if (!atsend (at.at_actf)) 
      atrecov();
  }
  
  drvl [AT_MAJOR].d_time = ATSECS;
  spl(s);
}	/* atstart() */


/*
 * int
 * aterror ()
 *
 *	Action:	Check for drive error.
 *		If found, increment error count and report it.
 *
 *	Return: 0 = No error found.
 *		1 = Error occurred.
 */

#if __USE_PROTO__
LOCAL int aterror (void)
#else
LOCAL int
aterror ()
#endif

{
	int csr;
	int aux;
	char *errstr = NULL;
	char *retrystr = NULL;

	/***************************************************************
         *  The csr is not good for 400 ns after the busy status drops
         *  this routine should reflect that by waiting a little while
         *  before testing it.
         */

        busyWait2(NULL, 2);
	if ((csr = inb (ATSREG)) & (ERR_ST | WFLT_ST)) {
		aux = inb (AUX_REG);
		if (aux & BADBLK_ERR) {
			at.at_tries	= BADLIM;
		} 

		if ((csr & RDY_ST) == 0)	errstr = "Drive Not Ready";
		if (csr & WFLT_ST)		errstr = "Write Fault";
		if (aux & DAM_ERR)		errstr = "No Data Addr Mark";
		if (aux & TR0_ERR)		errstr = "Track 0 Not Found";
		if (aux & ID_ERR)		errstr = "ID Not Found";
		if (aux & ECC_ERR)		errstr = "Bad Data Checksum";
		if (aux & ABT_ERR)		errstr = "Command Aborted";
		if (aux & BADBLK_ERR)		errstr = "Block Flagged Bad";

		if (at.at_tries < HARDLIM)
			retrystr = "retrying...";
		else
		        retrystr = "I/O failed.";
	        cmn_err(CE_WARN, "at: <%s> %s\n", errstr, retrystr);
		return 1;
	}
	else
		return 0;
}	/* aterror() */

/*
 * void
 * atrecov ()
 *
 *	Action:	Attempt recovery.
 */

#if __USE_PROTO__
LOCAL void atrecov (void)
#else
LOCAL void
atrecov ()
#endif

{
	buf_t 		*bp = at.at_actf;
	struct hdparm_s *dp;		/* This drive's paramters */

	/***************************************************************
         *  These parameters are calculated for the recovery request.
         */

	unsigned	recov_cmd;	/* Recovery command. */
	unsigned	recov_blkno;	/* Recovery physical block no. */
	int		recov_cyl;	/* Recovery sector */
	int		recov_head;	/* Recovery head */
	int	 	recov_sect;	/* Recovery sector */


	/***************************************************************
         *  First, if we are here then an i/o operation failed. Bump
         *  the tries counter for this block.
         */
	
	++at.at_tries;

	/***************************************************************
         *  Next compute the block _we think_ failed for printing later.
         *  I still haven't eliminated the possiblity of phase errors
         *  in this driver.
         */

	dp = &(atparm[at.at_drv]);
	recov_blkno = bp->b_bno + ((bp->b_count - bp->b_resid) / BSIZE) + pparm[at.at_partn]. p_base;
	recov_cyl  = (recov_blkno / (dp->nspt * dp->nhead));
	recov_head = ((recov_blkno - (recov_cyl * dp->nspt * dp->nhead)) / (dp->nspt));
	recov_sect = (recov_blkno % (dp->nspt)) + 1;
        cmn_err(CE_WARN,
                "(%d,%d): <%s failed. Logical Block %u Cyl: %u Head: %lu Sect: %d >",
                major(bp->b_dev),
                minor(bp->b_dev),
                (bp->b_req == BREAD) ? "Read" : "Write",
                bp->b_bno,
                recov_cyl,
                recov_head,
                (unsigned) recov_sect);
	
	if (at.at_tries >= HARDLIM) {
		at.at_actf = bp->b_actf;
		bp->b_actf = bp->b_actl = NULL;
		bp->b_flag |= BFERR;
		bdone(bp);

		at.at_bufcnt = 0;
		drvl[AT_MAJOR].d_time = 0;
		at.at_state = SIDLE;
	}
	else {
                switch (at.at_tries) {
        
                /*******************************************************
                 *  At 1-2 tries seek in a cylinder.
                 */
        
                case 1:
                case 2:
                        if (--recov_cyl < 0) recov_cyl += 2;
                        recov_cmd = SEEK(0);
                        break;
        
                /*******************************************************
                 *  At 3-4 tries seek out a cylinder.
                 */
        
                case 3:
                case 4:
                        /*
                         * Move out 1 cylinder, then retry operation
                         */
                        if (++ recov_cyl >= _CHAR2_TO_USHORT (dp->ncyl)) recov_cyl -= 2;
                        recov_cmd = SEEK(0);
                        break;
        
                /*******************************************************
                 *  At 5-6 tries seek to cylinder 0.
                 */
         
                case 5:
                case 6:
                        recov_cyl = 0;
                        recov_cmd = SEEK(0);
                        break;
        
                /*******************************************************
                 *  Finally, restore the drive.
                 */
        
                default:
                        recov_cyl = 0;
                        recov_cmd = RESTORE (0);
                        break;
                }
        
                /*******************************************************
                 *  Take recovery action. When this completes the state
		 *  machine in atintr will restart the failed command
                 *  with current information.
                 */
                
                drvl[AT_MAJOR].d_time = (recov_cmd == RESTORE (0)) ? ATSECS * 2
                                       		                   : ATSECS;
                outb (LCYL_REG, recov_cyl);
                outb (HCYL_REG, recov_cyl >> 8);
                outb (HDRV_REG, (at.at_drv << 4) + 0xA0);
                outb (CSR_REG, recov_cmd);
                at.at_state = SRETRY;
	}	
}	/* atrecov() */

/*
 * void
 * atintr ()	- Interrupt routine.
 *
 *	Clear interrupt then defer actual processing.
 */

#if __USE_PROTO__
void atintr(void)
#else
void
atintr()
#endif

{
	buf_t *bp;

	if (!(bp = at.at_actf))
		at.at_state = SIDLE;
		
	(void) inb (CSR_REG);	/* clears controller interrupt */
	switch (at.at_state) {

	case SIDLE:
		cmn_err(CE_NOTE, "at: <Spurious interrupt>");
		return;
	
	case SRESET:
		return;
		
	case SRETRY:
		at.at_state = SIDLE;
		break;

	case SREAD:
		/*
		 * Check for I/O error before waiting for data.
		 */

		if (aterror ()) {
			atrecov ();
			break;
		}

		/*******************************************************
                 *  Try to get the sector into the buffer. If this
                 *  fails then try recovery steps.
                 */

		if (!atrecv(at.at_actf)) {
			atrecov();
			break;
		}

		/*******************************************************
                 *  If there is no error then assume a block got
                 *  read from the disk. If this finishes the block
                 *  then give it back to the kernel.
                 */

		bp->b_resid -= BSIZE;
		if (bp->b_resid == 0) {
			at. at_actf = bp->b_actf;
			bp->b_actf = bp->b_actl = NULL;
			bdone(bp);
			--at.at_bufcnt;
		}

		/*******************************************************
                 *  Now, set the driver state variables to appropriately
                 *  reflect what the driver expects to be doing.
                 */

                if (at.at_bufcnt > 0 && at.at_actf) {
			drvl[AT_MAJOR].d_time = ATSECS * 2;
			at.at_tries = 0;
		}
		else {
			drvl[AT_MAJOR].d_time = 0;
			at.at_state = SIDLE;
		}
		break;

	case SWRITE:
		/*
		 * Check for I/O error.
		 */

		if (aterror ()) {
			atrecov ();
			break;
		}

		/*******************************************************
                 *  If there is no error then assume the block got
                 *  written to the disk. If this finishes the block
                 *  then give it back to the kernel.
                 */

		bp->b_resid -= BSIZE;
		if (bp->b_resid == 0) {
			at. at_actf = bp->b_actf;
			bp->b_actf = bp->b_actl = NULL;
			bdone(bp);
			--at.at_bufcnt;
		}

		/*******************************************************
                 *  Now, set the driver state variables to appropriately
                 *  reflect what the driver expects to be doing.
                 */

                if (at.at_bufcnt > 0 && at.at_actf) {
			drvl[AT_MAJOR].d_time = ATSECS * 2;
			at.at_tries = 0;
			if (!atsend (at.at_actf)) {
				atrecov();
				at.at_state = SRETRY;				
			}
		}
		else {
			drvl[AT_MAJOR].d_time = 0;
			at.at_state = SIDLE;
		}
		break;
	}	/* switch */

	/***************************************************************
         *  If the driver is idle and there is work to be done then
         */

	if (at.at_state == SIDLE && atdequeue())
		atstart();
}	/* atintr() */



CON	atcon	= {
	DFBLK | DFCHR,			/* Flags */
	AT_MAJOR,			/* Major index */
	atopen,				/* Open */
	NULL,				/* Close */
	atblock,			/* Block */
	atread,				/* Read */
	atwrite,			/* Write */
	atioctl,			/* Ioctl */
	NULL,				/* Powerfail */
	atwatch,			/* Timeout */
	atload,				/* Load */
	atunload			/* Unload */
};

/* End of file */
