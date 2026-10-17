/***********************************************************************
 * haiss.c --   haiscsi compliant Host adapter routines for the Seagate
 *              ST01/ST02 and 8-bit Future Domain controllers.
 *
 *  These controllers use memory mapped I/O like the IBM professional
 *  Graphics array controler.
 *
 *  Copyright (c) 1993 Christopher Sean Hilton - All rights reserved.
 *  
 *  Last Modified: Wed Jul  6 15:04:35 1994 by [kroot]
 */

/***********************************************************************
 * $Id$
 *
 * $Log$
 */

static char rcsid[] = "@(#) haiss.c $Revision$";

#include <sys/coherent.h> 
#include <sys/con.h> 
#include <sys/stat.h>
#include <kernel/timeout.h> 
#include <sys/cmn_err.h> 
#include <sys/haiscsi.h>

/* A forward declaration masquerading as an external. */
extern haft_t	ss_haft;

#define LOCAL	 static  
#define Register register	/* For stupid compilers */

/***********************************************************************
 * The Seagate ST01/02 and Future Domain 8xx 8-bit host adapters use 
 * the same memory mapped layout and SCSI Bus controller chip so they 
 * program similarly.  The differences are: 
 * 
 * Controller            CSR(hex)  Data(hex)    SCSI ID 
 * -------------------- ---------- ---------- ---------- 
 * ST01/02                   1a00       1c00          7 
 * TMC-845 Type		     1c00       1e00          6 
 * TMC-840 Type              1a00       1c00          6 
 * 
 * These differences are taken care of in the ss_load() routine so 
 * the rest of the program need not concern itself with them.  
 */

#define SS_SRAMSTART	0x1800	/* Start of static RAM for test */
#define SS_SRAMEND	0x187f	/* End of Static Ram */ 
#define ST01_CSR	0x1a00	/* ST01 Control/Status Register */ 
#define ST01_DAT	0x1c00	/* ST01 Data Register */ 
#define FD845_CSR	0x1c00	/* Future Domain Control/Status Register */
#define FD845_DAT	0x1e00	/* Future Domain Data Register */
#define SS_RAMSIZE	0x2000	/* Total size of memory mapped area */

#define ST_ID		7 
#define FD_ID		6

/* Command bits within the CSR */

#define RST_OUT 	bit(0)	/* SCSI Reset Command */ 
#define SEL_OUT 	bit(1)	/* SCSI Select */ 
#define BUSY_OUT	bit(2)	/* SCSI Busy */ 
#define ATTN_OUT	bit(3)	/* SCSI Attention */ 
#define STARTARB	bit(4)	/* Start Arbitration */ 
#define ENPARITY	bit(5)	/* Enable Parity */ 
#define ENINTR		bit(6)	/* Enable Interrupt */ 
#define ENSCSI		bit(7)	/* Enable SCSI bus */

#define HAIDLE		(ENPARITY | ENINTR)

/* Status bits within the CSR */

#define BUSY_IN 	bit(0)	/* SCSI Busy signal */ 
#define MSG_IN		bit(1)	/* SCSI Message signal */ 
#define IO_IN		bit(2)	/* SCSI I/O signal */ 
#define CD_IN		bit(3)	/* SCSI Control/Data signal */ 
#define REQ_IN		bit(4)	/* SCSI Request signal */ 
#define SEL_IN		bit(5)	/* SCSI Select signal */ 
#define PARITYERR	bit(6)	/* Parity Error. */ 
#define ARBITCMP	bit(7)	/* Arbitration Complete */

#define RESELECT	(SEL_IN | IO_IN) /* SCSI reselect phase in progress */

/***********************************************************************
 * SCSI Bus IO phases: use with IOPHASE below to match what the bus 
 * is trying to do. 
 */

#define XP_DATAOUT	(0) 		/* Init -> Trgt Data */
#define XP_DATAIN	(IO_IN) 	/* Trgt -> Init Data */
#define XP_COMMAND	(CD_IN) 	/* Init -> Trgt CDB */ 
#define XP_STATUS	(CD_IN|IO_IN)	/* Trgt -> Init CDB Status */ 
#define XP_MSGOUT	(MSG_IN|CD_IN)	/* Init -> Trgt Message */ 
#define XP_MSGIN	(MSG_IN|CD_IN|IO_IN)/* Trgt -> Init Message */

/* SCSI Message Definitions */

#define MSG_CC		0x00	/* Command complete */ 
#define MSG_SAVEDPTR	0x02	/* Save data pointers */ 
#define MSG_RSTRDPTR	0x03	/* Restore data pointers */ 
#define MSG_DISCONNECT	0x04	/* Disconnect */ 
#define MSG_ABORT	0x06	/* Abort the current operation */ 
#define MSG_REJECT	0x07	/* Message reject (last message invalid) */
#define MSG_NOP 	0x08	/* No operation */ 
#define MSG_LCC		0x0a	/* Linked command complete */ 
#define MSG_LCCF	0x0b	/* Linked command complete (with flag) */ 
#define MSG_BUSDEVRST	0x0c	/* Bus device reset */ 
#define MSG_IDENTIFY	0x80	/* Identify message (Family) */ 
#define   ID_DISCONNECT	0x40	/* Disconnect Flag for Identify message */

/***********************************************************************
 * SCSI Bus Timings: 
 * 
 * These are the official SCSI-1 Bus Timings of the NBA, Get your 
 * gamecard now.  
 */

/*	Event 		Coh Time Event		       Actual Time	*/
/*	(Abrv.)		(usec) 	 Full Name		 (usec) */ 
/*      --------------	-------- -----------------	-----------	*/
#define	ARBIT_DLY	3	/* Arbitration Delay 	 2.200	*/
#define ASSERT_PER	1	/* Assertion period      0.090 	*/
#define BUSCLR_DLY	1	/* Bus Clear Delay	 0.800	*/
#define BUSFREE_DLY	1	/* Bus Free Delay	 0.800	*/
#define BUSSET_DLY	2	/* Bus Set Delay	 1.800	*/
#define BUSSETTLE_DLY	1	/* Bus Settle Delay	 0.400	*/
#define CABLESKEW_DLY	1	/* Cable Skew Delay	 0.010	*/
#define DATARLS_DLY	1	/* Data Release Delay	 0.400	*/
#define DESKEW_DLY	1	/* Deskew Delay		 0.045 */
#define HOLD_TM		1	/* Hold Time		 0.045 */
#define NEGATION_PER	1	/* Negation Period	 0.090	*/
#define RSTHOLD_TM	25	/* Reset Hold Time	 25.000	*/
#define SELABORT_TM	200	/* Selection Abort Time	 200.000 */ 
#define SELTO_DLY	250000	/* Sel. Timeout Delay	250,000.000	*/

#define RETRY_DELAY	3	/* Wait on a busy SCSI bus */ 
#define MAXBUSYCOUNT	120	/* Number of busy bus before reset */

#define REQ_BUSDEVICERST	0x0001	/* Request Bus Device Reset */
#define REQ_ABORTSCSICMD	0x0002	/* Request Abort SCSI command */ 
#define REQ_STARTSCSICMD	0x0004	/* Request Start SCSI */

#define CMD_PENDING		0x0001	/* Command pending */ 
#define CMD_DEVBUSY		0x0002	/* Device is busy */ 
#define CMD_OVERRUN		0x0004	/* Data overrun on command */ 
#define CMD_UNDERRUN		0x0008	/* Data underrun on command */ 
#define CMD_NEEDREQACK		0x0010	/* Command need req/ack handshake */

#define HAST_OK			0x00 
#define HAST_SELTO		0x11
#define HAST_OVERRUN		0x12 
#define HAST_UNDERRUN		0x12 
#define HAST_BUSFREE		0x13
#define HAST_BADBUSPH		0x14 
#define HAST_INVDCDB		0x20


#define MAXSEGCOUNT		16 
#define DATAREGSIZE		512

/***********************************************************************
 * Device states.  
 *
 * These states give an idea of what the device is doing in the context
 * of this host adapter. 
 */

typedef enum { 
    DEV_IDLE = 0,	/* Device is idle in this host's context */ 
    DEV_QUEUED,		/* Device has a command queued and waiting */ 
    DEV_SCSIPROT,	/* Device is in some stage of the SCSI protocol */ 
    DEV_WORKING,	/* Device is working on a request */
    DEV_FINISHED	/* Device is finished with SCSI request */ 
} devstate_t;

typedef struct ssccb_s *ssccb_p;

typedef struct ssccb_s { 
    devstate_t 		c_state; /* Current state of the device. */ 
    unsigned 		c_id;	 /* Command id number */ 
    unsigned		c_reqflags; /* Request flags */
    unsigned		c_cmdflags; /* Command flags */ 
    unsigned char	*c_cdb;	 /* Pointer to cdb to execute */ 
    unsigned 		c_cdblen; /* Length of cdb */ 
    unsigned		c_time;
    unsigned		c_target; 
    srb_p		c_srb;	 /* Current SCSI Request Block */ 
    ssccb_p		c_next;	 /* Next ccb in Request chain */ 
    size_t		c_segcount; /* Number of segments in the s/g list */ 
    size_t		c_dataofs; /* Offset into the current segment */ 
    haisgsegm_p		c_datain; /* pointer to s/g list for datain. */ 
    haisgsegm_p		c_dataout; /* pointer to s/g list for dataout. */ 
    haisgsegm_t		c_sgs[MAXSEGCOUNT]; /* Scatter gather list for xfer */
    unsigned short 	c_scsistatus; /* Status of the SCSI command */ 
} ssccb_t;

static ssccb_p ssdevs[MAXDEVS] = { 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL,
    NULL, 
    NULL 
};

static ssccb_t dummy; 
static srb_t dummysrb; 
static ssccb_p normal_queue = NULL,
	       priority_queue = NULL; 		
static unsigned char test_unit_ready[] = { 0, 0, 0, 0, 0, 0 };

/***********************************************************************
* Configurable variables - specified in Space.c */ 

enum {
    ST01=0,			/* ST01/02 controller */
    FD845=1,			/* TMC-845/850/860/875/885 */
    FD840=2			/* TMC-840/841/880/881 */ 
};

extern int haiss_type;		/* From the above enum */ 
extern int haiss_intr;		/* IRQ number */ 
extern int haiss_base;		/* Paragraph addr for BIOS */ 
extern int haiss_slowdev;	/* Bitmap of devices needing slow handshake */

extern int HAI_DISK;		/* Bit mapped values where disks are */ 
extern int HAI_TAPE;		/* Bit mapped values for where tapes are */ 
extern int HAI_CDROM;		/* Bit mapped values for where CD-ROMS are */

static int ss_devices;		/* Bitwise-or of HAI device bitmaps. */

static ssccb_p rawccb = NULL;
static unsigned char *ss_vbase = NULL; /* Controller Base selector */ 
static unsigned char *ss_csr = NULL, /* Command/Status register */
                     *ss_dat = NULL; /* Data Register */
static unsigned short hostid;	/* host adapter SCSI Bus address */ 
static unsigned short swap_status_bits = 0; 
static char ss_xfrbuf[DATAREGSIZE]; /* transfer buffer for slow devices */

static TIM sstim;

#define CSR 		(*(volatile unsigned char *) ss_csr)
#define DAT 		(*(volatile unsigned char *) ss_dat)

#define setcsr(v)	(CSR = (v)) 
#define setdat(v)	(DAT = (v))

#define min(a, b)	((a) < (b) ? (a) : (b))

#define reselect()	((CSR & (SEL_IN | IO_IN)) == (SEL_IN | IO_IN))

#define set_host_status(r, newstat)	((r)->hastat = (newstat))

#define DEBUG_TRACE	0 
#define DEBUG_QTRACE	0

/***********************************************************************
 * A squawk function for printing stuff out while debugging.  
 */

#if	DEBUG_TRACE 
#define trace(str)	cmn_err(CE_CONT, "!" (str))
#else 
#define trace(str) 
#endif
  
/***********************************************************************
 *  a quicker squawk function for more difficult bugs.
 */

#if	DEBUG_QTRACE
static char spinnerstr[] = "-/|\\";

#define qtrace(x, y, c)	_chirp((c), 2 * (80 * (y) + (x)))
#define spinner(x, y, i) \
{ if (++(i) >= sizeof(spinnerstr)) i = 0; qtrace((x), (y), spinnerstr[(i)]); }
#else
#define	qtrace(x, y, c)
#endif

/***********************************************************************
 *  Forward declarations for things which must be defined out of order.
 */

#if __USE_PROTO__
LOCAL void	ssintr(void);
#else
LOCAL void	ssintr();
#endif

/***********************************************************************
 *  Utility routines for doing timing functions. All times here are
 *  measured in micro seconds and converted to Coherent Clock ticks.
 *  
 *  One nasty side effect of these timers is that you cannot have two
 *  contexts trying to access them at the same time. They use static
 *  values for masks. This shouldn't be a problem with this design
 *  as only one routine accesses the SCSI bus, and thus needs the timers
 *  and masks, anyway.
 */

#define usec2t0(usec)	(((usec * 11932) + 5000) / 10000)

/*
 * Forward declarations for kernel functions so gcc doesn't complain
 */

int busyWait	__PROTO((int (*)(void), unsigned));
int busyWait2	__PROTO((int (*)(void), unsigned));
vaddr_t map_pv	__PROTO((paddr_t, size_t));
void unmap_pv	__PROTO((vaddr_t));
unsigned kucopy	__PROTO((void *, void *, unsigned));
unsigned ukcopy	__PROTO((void *, void *, unsigned));
void setivec	__PROTO((int, void (*)(void)));
void clrivec	__PROTO((int));
void pxcopy	__PROTO((paddr_t, vaddr_t, size_t, int));
void xpcopy	__PROTO((vaddr_t, paddr_t, size_t, int));
void timeout	__PROTO((TIM *, int, void (*)(), ...));

/***********************************************************************
 *  eventwait() --   Wait for an event (really?)
 *
 *  Wait up to ticks milliseconds for an event to happen. This is better
 *  than using a for loop.
 */

#if __USE_PROTO__
LOCAL int eventwait(int (*event)(), Register unsigned timeout_val)
#else
LOCAL int
eventwait(event, timeout_val)
int 			(*event)();
Register unsigned 	timeout_val;
#endif
{
    if (timeout_val == 0) {
	if (event)
	    return (*event)();
	else
	    return 0;
    }
    else if (timeout_val < 10000)
	return busyWait2(event, usec2t0(timeout_val));
    else
	return busyWait(event, (timeout_val + 5000) / 10000);
}	/* eventwait() */

static unsigned chkmask,	/* shared check value for allbitswait */
		chkevent;	/* shared check value for allbitswait */

#if __USE_PROTO__
LOCAL int allbchk(void)
#else
LOCAL int
allbchk()
#endif
{
	return ((CSR & chkmask) == chkevent);
}	/* allbchk() */

#if __USE_PROTO__
LOCAL int allbitswait(unsigned char mask, unsigned char event, unsigned ticks)
#else
LOCAL int
allbitswait(mask, event, ticks)
unsigned char	mask,
		event;
unsigned	ticks;
#endif
{
	chkmask = mask;
	chkevent = event;

	return (eventwait(&allbchk, ticks));
}	/* allbitswait() */

#if __USE_PROTO__
LOCAL void hareset(unsigned char newstate)
#else
LOCAL void
hareset(newstate)
unsigned char newstate;
#endif
{
    /*******************************************************
     *  Reset the Host adapter. This resets all of the
     *  devices on the bus also. The SCSI spec stats that
     *  the reset hold time is 25 usec or longer. Wait
     *  through two reset hold times here.
     */
    
    setcsr(ENPARITY | ENSCSI | RST_OUT);
    eventwait(NULL, 2 * RSTHOLD_TM);
    setcsr(newstate);
    
    /*******************************************************
     *  Allow 5 seconds for all devices on the bus to ready
     *  themselves after this reset. In practice only the
     *  disk drives will be ready after 5 sec but nobody
     *  should be trying to use the tape drive this soon
     *  anyway.
     */
    
    eventwait(NULL, 5000000U);
}

/***********************************************************************
 *  hainit() -- Reset the host adapter and set it up to accept
 *              commands.
 *
 *  This should look very familiar to you Hal.
 */

#if __USE_PROTO__
LOCAL int ss_load(void)
#else
LOCAL int
ss_load()
#endif
{
    paddr_t 	pbase;	/* Physical base of controller ram. */
    ssccb_p 	ctrl;	/* Pointer to a device control structure */
    unsigned	i, id,
    		numdevs;/* number of devices configured on the bus. */
    int 	erf;
    
    cmn_err(CE_CONT,
	    "Haiss: Seagate ST0x, Future Domain 8xx Host Adapter module.\n");
    
    /* Map the HA's memory. */
    
    pbase = ((paddr_t) (haiss_base << 4));
    ss_vbase = (unsigned char *) map_pv(pbase, SS_RAMSIZE);
    
    /*******************************************************************
     *  Check for the presence of the card by writing to the host
     *  adapter's Static Ram and then read back to make sure that the
     *  ram is there.  If it is assume that the card is present.
     */
    
    for (i = SS_SRAMSTART; i <= SS_SRAMEND; ++i)
	ss_vbase[i] = (i & 0xff);
    
    for (i = SS_SRAMSTART, erf = 0; !erf && i < SS_SRAMEND; ++i)
	erf |= (ss_vbase[i] != (i & 0xff));
    
    if (erf) {
	cmn_err(CE_CONT, "\tStatic ram test failed.\n");
	return (ss_haft.hf_present = 0);
    }
    
    /*******************************************************************
     * Now setup the parameters used to work with the HA.  The differences
     * between the Seagates and the Future Domains are resolved here.
     * Aside from the addressing differences earlier Future Domain
     * controllers swapped the SCSI C/D and MESSAGE bits off the chip.
     * This is compensated for with the flag swap_status_bits and with
     * function busphase().
     */
    
    swap_status_bits = 0;
    switch (haiss_type) {
    case ST01:
	cmn_err(CE_CONT, "\tConfigured for Seagate ST01/ST02\n");
	ss_csr = ss_vbase + ST01_CSR;
	ss_dat = ss_vbase + ST01_DAT;
	hostid = ST_ID;
	break;
    case FD845:
	cmn_err(CE_CONT, "\tConfigured for Future Domain TMC-845/850/860/875/885\n");
	ss_csr = ss_vbase + FD845_CSR;
	ss_dat = ss_vbase + FD845_DAT;
	hostid = FD_ID;
	break;
    case FD840:
	cmn_err(CE_CONT, "\tConfigured for Future Domain TMC-840/841/880/881\n");
	ss_csr = ss_vbase + ST01_CSR;
	ss_dat = ss_vbase + ST01_DAT;
	hostid = FD_ID;
	swap_status_bits = 1;
	break;
    default:
	cmn_err(CE_CONT, "\tbad configuration.\n");
	return (ss_haft.hf_present = 0);
    }
    
#if 	defined(INIT_RST)
    hareset(ENPARITY);
#endif	
    /***************************************************************
     *  Lastly search through the installed devices mask and find
     *  out how many control structures we need to for this
     *  configuration.
     */
    
    ss_devices = ( HAI_DISK | HAI_TAPE | HAI_CDROM );
    numdevs = 0;
    for (i = 0; i < MAXDEVS; ++i) {
        if (ss_devices & bit(i))
            ++numdevs;
    }
    ctrl = rawccb = (ssccb_p) kalloc(numdevs * sizeof(ssccb_t));
    if (!rawccb) {
	cmn_err(CE_PANIC,
		"haiss: Cannot allocate Host adapter control structures.\n");
	return (ss_haft.hf_present = 0);
    }
    
    /***************************************************************
     *  Clear out the control structures and put them into the reference
     *  table for use when the driver is working.
     */
    
    memset(rawccb, 0, numdevs * sizeof(ssccb_t));
    for (id = 0; id < MAXDEVS; ++id)
	if (ss_devices & bit(id)) {
	    ssdevs[id] = ctrl++;
	    ssdevs[id]->c_target = id;
	}
    
    /*******************************************************************
     *  All done here for now.  If I were only controlling DASD devices
     *  I could now go and initialize all the devices on the bus.  That
     *  is now the job of the individual device routines.  This will be
     *  a problem because at some point in time I will have to read and
     *  interpret the partition table of the disk...
     */
    
    /* Grab the Interrupt vector */
    
    setivec(haiss_intr, ssintr);
    cmn_err(CE_CONT,
	    "\tBase Memory:\t\t0x%x\n"
	    "\tInterrupt vector:\t%d\n",
	    haiss_base,
	    haiss_intr);


    /*
     * This call doesn't belong here. This really should be done from 
     * the disk drive init routines. But this is in testing so...
     */

    {
	extern int at_drive_ct;
	int biosnum = at_drive_ct;

	for (id = 0 ; id < MAXDEVS ; ++id)
	    if (HAI_DISK & bit(id))
		loadbiosparms(biosnum++, id);
    }
    return (ss_haft.hf_present = 1) /* 1 */;
}	/* hainit() */

/***********************************************************************
 *  ssunload()  --  Return memory, IRQ, and Selector back to Kernel
 *                  when done.
 */

#if __USE_PROTO__
LOCAL void ss_unload(void)
#else
LOCAL void
ss_unload()
#endif
{
	/* Return memory occupied by control structures. */
	if (rawccb)
		kfree(rawccb);

	/* Return the selector */
	unmap_pv((vaddr_t) ss_vbase);

	/* Return the IRQ */
	clrivec(haiss_intr);
}	/* ssunload() */

/***********************************************************************
 *  busphase() --       Get the I/O phase from the bus.
 *
 *  Read the bus I/O phase signals.  Swap the MSG_IN and CD_IN signals
 *  if this is an old Future Domain card which swaps them for us.
 */

#if __USE_PROTO__
LOCAL unsigned char busphase(void)
#else
LOCAL unsigned char
busphase()
#endif
{
    unsigned rawphase = CSR & (MSG_IN | IO_IN | CD_IN);
    
    if (swap_status_bits) {
	unsigned ret = rawphase & ~(MSG_IN | CD_IN);
	
	if (rawphase & MSG_IN) 
	    ret |= CD_IN;
	if (rawphase & CD_IN)  
	    ret |= MSG_IN;
	return (unsigned char) ret;
    }
    else
	return (unsigned char) rawphase;
}	/* busphase() */

/***********************************************************************
 *  macro busfree() --  Wait for a bus free phase.
 */

#define busfree()	((CSR & (BUSY_IN | SEL_IN)) == 0)

/***********************************************************************
 *  set_priority_queue() -- Set the priority queue pointer for the
 *                          sswork loop.
 *  
 *  Reselect phases occur on the SCSI bus to finish up commands that
 *  allow disconnect. These will be seen as reselect phases from the
 *  interrupt routine or when trying to arbitrate for the bus. The
 *  work loop is designed to process these requests first. To do this
 *  it needs to know which device is trying to reselect the host. 
 *  set_priority_queue puts the CCB for this device into the variable:
 *  priority_queue. This will be picked up by sswork and the device
 *  will be serviced.
 */


#if __USE_PROTO__
LOCAL ssccb_p set_priority_queue(void)
#else
LOCAL ssccb_p 
set_priority_queue()
#endif
{
    Register unsigned	scsibus;
    Register unsigned	target;
    Register unsigned	hostmask = bit(hostid);
    
    /*******************************************************
     *  Read the SCSI bus to see if we are needed.
     */
    
    scsibus = DAT;
    if ((scsibus & hostmask) == hostmask) {
	/***********************************************
	 *  Strip the host's tid bit from the SCSI
	 *  Bus and make sure that what's left is a
	 *  power of two. This will be the tid bit of 
	 *  the target that's attempting to reconnect
	 *  to the host adapter.
	 */
	
	scsibus &= ~hostmask;
	if (((scsibus - 1) & (scsibus)) == 0) {
	    for (target = 0 ; scsibus > 1 ; ++target, scsibus >>= 1)
		;
	    while (scsibus > 1) {
		scsibus >>= 1;
		++target;
	    }       /* while */
	    
	    if (!ssdevs[target] || ssdevs[target]->c_state != DEV_WORKING) {
		dummy.c_scsistatus = dummysrb.status = ST_PENDING;
		dummysrb.target = hostid;
		dummysrb.lun = 0;
		dummy.c_datain = dummy.c_dataout = NULL;
		dummy.c_srb = &dummysrb;
		priority_queue = &dummy;
		cmn_err(CE_WARN,
			"haiss: Invalid reselect at target %d,"
			" device state: %x",
			target,
			ssdevs[target]->c_state);
	    }       /* if */
	    else
		priority_queue = ssdevs[target];
	}       /* if */
	else
	    cmn_err(CE_NOTE, "haiss: Invalid SCSI reselect.");
    }       /* if */
    
    return priority_queue;
}	/* set_priority_queue() */

/***********************************************************************
 *  SCSI bus phase handlers. 
 *  
 *  These functions handle the various SCSI bus phase that we expect
 *  to see in normal operation.
 */

/***********************************************************************
 *  scsi_arbitrate() --     Arbitrate for the SCSI Bus.
 */

#if __USE_PROTO__
LOCAL int scsi_arbitrate(int host)
#else
LOCAL int
scsi_arbitrate(host)
int	host;
#endif
{
    if (busfree() && busfree()) {
	setcsr(HAIDLE);
	setdat(bit(host));  	/* set up our address on the bus */
	setcsr(ENINTR | ENPARITY | ENSCSI | STARTARB);
	if (allbitswait(ARBITCMP, ARBITCMP, 100) != 0)
	    return 1;
	else {
	    setcsr(HAIDLE);
	    return 0;
	}
    }
    else 
	return 0;
}	/* scsi_arbitrate() */

#if __USE_PROTO__
LOCAL int msgoutstart(void)
#else
LOCAL int
msgoutstart()
#endif
{
	return ((CSR & REQ_IN) != 0 && (busphase() == XP_MSGOUT));
}

/***********************************************************************
 *  msgoutack() --  Return (1) if we have a message out phase.
 */
#if __USE_PROTO__
LOCAL int msgoutack(void)
#else
LOCAL int
msgoutack()
#endif
{
	return (busphase() != XP_MSGOUT);
}	/* msgoutack() */

/***********************************************************************
 *  scsi_connect() --       Connect to a target on the SCSI bus.
 *  
 *  After arbitration this establishes a connection to a target on
 *  the SCSI bus. If needed a message out phase is generated and a
 *  message will be passed to the target device.
 */

#if __USE_PROTO__
LOCAL int scsi_connect(int host, int target, unsigned char message)
#else
LOCAL int 
scsi_connect(host, target, message)
int		host;
int		target;
unsigned char	message;
#endif
{
    int attn_req = (message != MSG_NOP);
    int i;
    
    /***************************************************************
     *  We won arbitration. Now begin the selection phase by 
     *  asserting the sel signal on the SCSI bus. Keep busy high
     *  too since we are dropping the arbitration signal. Then
     *  wait a Bus Clear Delay and a Bus Settle Delay.
     */
    
    setcsr(ENSCSI | ENINTR | ENPARITY | SEL_OUT | BUSY_OUT);
    busyWait2(NULL, 3);		/* Wait 2.6 us */
    
    /***************************************************************
     *  Drive the requested device's id onto the bus, wait 2 deskew
     *  delays.
     */
    
    setdat(bit(host) | bit(target));
    busyWait2(NULL, 2);
    
    setcsr((attn_req) ? (ENSCSI | ENPARITY | ENINTR | SEL_OUT | ATTN_OUT)
	   : (ENSCSI | ENPARITY | ENINTR | SEL_OUT));
    
    if (!allbitswait(BUSY_IN, BUSY_IN, SELTO_DLY)) {
	setdat(0);
	setcsr(ENSCSI | ENPARITY | ENINTR | SEL_OUT);
	if (!allbitswait(BUSY_IN, BUSY_IN, SELABORT_TM + (2 * DESKEW_DLY))) {
	    setcsr(HAIDLE);
	    cmn_err(CE_NOTE, "haiss: Select Timed Out on ID%d", target);
	    return 0;
	}		
    }
    
    busyWait2(NULL, 2);
    if (attn_req) {
	setcsr(ENSCSI | ENPARITY | ENINTR | ATTN_OUT);
	if (!busyWait(&msgoutstart, 500)) {
	    cmn_err(CE_WARN, "Cannot start message out phase!\n");
	    return 0;
	}
        
	setcsr(ENSCSI | ENPARITY | ENINTR);      /* Drop the ATTN Condition */
	setdat(message);                    /* Send the message */
	setcsr(ENSCSI | ENPARITY | ENINTR);
	for (i = 0 ; i < 3; ++i) {
	    if (busyWait(&msgoutack, 5)) {
		return 1;
	    }
	    else {
		cmn_err(CE_NOTE,
			"haiss: Resending message: (0x%x)\n", message);
		setdat(message);
		setcsr(ENSCSI | ENPARITY | ENPARITY);
		message = MSG_NOP;
	    }
	}
	return 0;
    }
    
    setcsr(ENSCSI | ENPARITY | ENINTR);
    return 1;
}	/* scsi_connect() */

/***********************************************************************
 *  scsi_reconnect() --     Reconnect to a device on the bus after
 *                          disconnect.
 *  
 *  Connect is Host Adapter Driven, reconnect is target driver. A target
 *  has grabbed the bus and selected the host. This function handshakes
 *  the bus into the ioxfer phase.
 */

#if __USE_PROTO__
LOCAL int scsi_reconnect(void)
#else
LOCAL int scsi_reconnect()
#endif
{
	setcsr(ENPARITY | ENSCSI | ENINTR | BUSY_OUT);
	if (allbitswait(SEL_IN, 0, SELTO_DLY)) {
		setcsr(ENPARITY | ENSCSI | ENINTR);
		return 1;
	}
	else {
		setcsr(HAIDLE);
		cmn_err(CE_NOTE, "haiss: SCSI Reselect failed.");
	}
	return 0;
}	/* scsi_reconnect() */

static int	bsyhigh;

#if __USE_PROTO__
LOCAL int reqcheck(void)
#else
LOCAL int 
reqcheck()
#endif
{
    unsigned char csr = CSR;
    
    if (csr & REQ_IN)
	return 1;
    
    if ((csr & BUSY_IN) == 0) {
	bsyhigh = 0;
	return 1;
    }
    
    return 0;
}	/* reqcheck() */

/***********************************************************************
 *  reqwait() --    Wait for the request and busy signals from the
 *                  target.
 *
 *  During the I/O phases the target should assert REQ_IN and BUSY_IN
 *  while it wants to do I/O.  Poll for those values in a loop which
 *  will timeout eventually. Like all other loops in this driver this
 *  one will have to be replaced with the new busyWait function when
 *  I get documentation and the function has been standardized (not
 *  in that order).
 *
 *  Input:  Pointer to caller's timeout flag.  This will be set to
 *      one if the function if the function times out, zero otherwise.
 */

#if __USE_PROTO__
LOCAL int reqwait(Register int *toptr)
#else
LOCAL int
reqwait(toptr)
Register int *toptr;
#endif
{
#if 0
    if (toptr)
	*toptr = 0;

    bsyhigh = 1;
    if (eventwait(&reqcheck, (4 * SELTO_DLY)) && bsyhigh)
	return 1;
    else if (bsyhigh && toptr) {
	cmn_err(CE_WARN, "haiss: SCSI REQ Timeout");
	if (ss_msgin_cc)
	    cmn_err(CE_WARN, "haiss: MSGIN:CC");
	*toptr = 1;
    }

    return 0;
#else
    int counter;
    unsigned char csr;

    if (toptr)
	*toptr = 0;

    counter = 10000000L;
    do {
	csr = (CSR & (BUSY_IN | REQ_IN));
	if ((csr & REQ_IN) == REQ_IN)
	    return 1;
	if ((csr & BUSY_IN) == 0)
	    return 0;
    } while (--counter > 0);

    cmn_err(CE_WARN, "haiss: SCSI REQ Timeout (CSR: 0x%x)", CSR);
    if (toptr)
	*toptr = 1;
    
    return 0;
#endif
}	/* reqwait() */

/***********************************************************************
 *  sswrite() -- byte by byte copy with handshake.
 *  
 *  Copy byte by byte from kernel memory to kernel memory.
 *  
 *  Assumes that one of the address is the data port from the Host
 *  Adapter.
 */

#if __USE_PROTO__
LOCAL size_t sswrite(void *source,
		     void *destination,
		     size_t size,
		     unsigned char phase)
#else
LOCAL size_t 
sswrite(source, destination, size, phase)
char 		*source;	/* Source Address */
char 		*destination;	/* Destination Address */
size_t		size;		/* Size of copy */
unsigned char	phase;		/* required busphase */
#endif
{
    Register char *src = source;
    Register char *dst = destination;
    Register int index = 0;

#if 0
    static char capbuf[64];
    
    while (index < sizeof(capbuf) && index < size && busphase() == phase) {
	
	/*******************************************************
	 * For kernel reads and writes syncronize each byte 
	 * going back and forth over the host. This is closer
	 * to the behaviour that the old ss driver used to use.
	 */
	
	if (reqwait(NULL)) {
	    capbuf[index] = dst[index] = src[index];
	    ++index;
	}
	else
	    break;
    }
#endif
    while (index < size && busphase() == phase) {
	
	/*******************************************************
	 * For kernel reads and writes syncronize each byte 
	 * going back and forth over the host. This is closer
	 * to the behaviour that the old ss driver used to use.
	 */
	
	if (reqwait(NULL)) {
	    dst[index] = src[index];
	    ++index;
	}
	else
	    break;
    }
    
#if 0
    dumpmem("\nsswrite capture buffer:", capbuf, min(index, sizeof(capbuf)));
#endif
    
    return index;
}	/* sswrite() */

/***********************************************************************
 *  dataout() --    Handle the Data out phase of the bus.  This gets
 *                  special treatment because of the way Coherent's
 *                  memory management works.  Up for review when the
 *                  memory management system changes.
 */

#if __USE_PROTO__
LOCAL void dataout(ssccb_p c, int *toflag)
#else
LOCAL void
dataout(c, toflag)
ssccb_p		c;
unsigned	*toflag;
#endif
{
    haisgsegm_p dataend;
    haisgsegm_p dataseg;
    size_t dataofs;
    char *dst = (c->c_cmdflags & CMD_NEEDREQACK) ? ss_xfrbuf : (char *) ss_dat;
    size_t xfrsz;
    int s;
    
    s = sphi();
    dataend = (c->c_sgs + c->c_segcount);
    dataseg = c->c_dataout;
    dataofs = c->c_dataofs;
    while (dataseg && busphase() == XP_DATAOUT) {
	xfrsz = dataseg->sgs_segsize - dataofs;
	if (xfrsz > DATAREGSIZE)
	    xfrsz = DATAREGSIZE;
	
	pxcopy(dataseg->sgs_segstart + dataofs, dst, xfrsz, SEL_386_KD);
	if (dst != (char *) ss_dat)
	    sswrite(dst, ss_dat, xfrsz, XP_DATAOUT);
	
        /*
         * The fast copy routine doesn't modify xfrsz so we could have
         * problems here but the errors should be obvious in any case
         * the slow transfer case can count bytes so it does. This is 
         */
	dataofs += xfrsz;
	if (dataofs >= dataseg->sgs_segsize) {
	    dataofs = 0;
	    if (++dataseg >= dataend)
		dataseg = NULL;
	} /* if */

	if (!reqwait(toflag))
	    break;
    } /* while */
    c->c_dataofs = dataofs;
    c->c_dataout = dataseg;
    spl(s);
    
    if (reqwait(toflag) && busphase() == XP_DATAOUT) {
	while (reqwait(toflag) && busphase() == XP_DATAOUT) 
	    DAT = 0x00;
	
	c->c_cmdflags |= CMD_UNDERRUN;
    }	/* if */
    
    if (!c->c_dataout)
	c->c_cmdflags &= ~CMD_OVERRUN;
}	/* dataout() */

/***********************************************************************
 *  datain() --     Handle the Data out phase of the bus. This gets
 *                  special treatment because of the way Coherent's
 *                  memory management works.  Up for review when the
 *                  memory management system changes.
 */

#if __USE_PROTO__
LOCAL void datain(Register ssccb_p c, int *toflag)
#else
LOCAL void
datain(c, toflag)
ssccb_p		c;
int		*toflag;
#endif
{
    haisgsegm_p dataend;
    haisgsegm_p dataseg;
    size_t dataofs;
    size_t xfrsz;
    char *src = (c->c_cmdflags & CMD_NEEDREQACK) ? ss_xfrbuf : (char *) ss_dat;
    unsigned char bb;
    int s;

    s = sphi();
    dataend = (c->c_sgs + c->c_segcount);
    dataseg = c->c_datain;
    dataofs = c->c_dataofs;
    while (dataseg && busphase() == XP_DATAIN) {
	xfrsz = dataseg->sgs_segsize - dataofs;
	if (xfrsz > DATAREGSIZE)
	    xfrsz = DATAREGSIZE;
	
	if (src != (char *) ss_dat)
	    sswrite(ss_dat, src, xfrsz, XP_DATAIN);
        xpcopy(src, dataseg->sgs_segstart + dataofs, xfrsz, SEL_386_KD);
	
	dataofs += xfrsz;
	if (dataofs >= dataseg->sgs_segsize) {
	    dataofs = 0;
	    if (++dataseg >= dataend)
		dataseg = NULL;
	} /* if */
	
	if (!reqwait(toflag))
	    break;
    } /* while */
    c->c_dataofs = dataofs;
    c->c_datain = dataseg;
    spl(s);
    
    if (reqwait(toflag) && busphase() == XP_DATAIN) {
	while (reqwait(toflag) && busphase() == XP_DATAIN)
	    bb = DAT;
	c->c_cmdflags |= CMD_OVERRUN;	
    }
    
    if (!c->c_datain) 
	c->c_cmdflags &= ~CMD_UNDERRUN;
}	/* datain() */

/***********************************************************************
 *  scsi_ioxfer() --        Handle SCSI I/O transfer phases.
 *  
 *  SCSI I/O phases are driven by the target. This function follows
 *  the target until the I/O phases are complete.
 */

#if __USE_PROTO__
LOCAL int scsi_ioxfer(Register ssccb_p c)
#else
LOCAL int 
scsi_ioxfer(c)
Register ssccb_p	c;
#endif
{
    Register srb_p	r = c->c_srb;
    unsigned		msgin;
    unsigned		bb;
    int			toflag = 0;
    int 		errors = 0;
    int			done = 0;

    /***************************************************************
     *  For as long as the target is requesting information send some
     *  even if it comes from the bit bucket. This may cause an infinite
     *  loop but only if the target is broken or incredibly hungry.
     *  This may need to be changed in the future, no problem.
     */

    if (c == NULL)
	cmn_err(CE_PANIC, "haiss: Null buffer passed to scsi_ioxfer");

    while (!done && reqwait(&toflag)) {
	if (errors) {
	    cmn_err(CE_WARN, "haiss: Error in SCSI I/O phases.");
	    c->c_datain = c->c_dataout = NULL;
	    c->c_cdb = NULL;
	    c->c_cdblen = 0;
	    setcsr(ENSCSI | ENPARITY | ENINTR | ATTN_OUT);
	}
	
	switch (busphase()) {
	    /*******************************************************
	     *  COMMAND: Transfer a SCSI CDB from the initiator to the
	     *  target.  Ex: Send the read command to the Hard disk.
	     */
	    
	case XP_COMMAND:
	    if (c->c_cdb) {
		while (c->c_cdblen > 0 && reqwait(&toflag)) {
		    setdat(*c->c_cdb++);
		    if (--c->c_cdblen == 0) {
			c->c_cdb = NULL;
			c->c_cmdflags |= CMD_PENDING;
		    }
		}
	    }
	    else {
		++errors;
		sswrite(test_unit_ready, ss_dat, sizeof(test_unit_ready), XP_COMMAND);
	    }
	    break;
	    
	case XP_DATAOUT:
	    dataout(c, &toflag);
	    break;
	    
	case XP_DATAIN:
	    datain(c, &toflag);
	    break;
	    
	    /*******************************************************
	     *  STATUS: Transfer the status byte from the last CDB
	     *  to the Initiator. This allows us to figure out if
	     *  the transfer went okay. An unexpected status byte
	     *  is not treated as an error.
	     */
	    
	case XP_STATUS:
	    if (errors) {
		bb = DAT;
		++errors;				
	    }
	    else
		c->c_scsistatus = DAT; 
	    break;
	    
	    /*******************************************************
	     *  MSG_OUT: This is the last phase of an error 
	     *  condition. It should happen for perhaps a bus
	     *  cyle or two before the target acknowledges the
	     *  Abort message and drops the SCSI bus.
	     */
	    
	case XP_MSGOUT:
	    if (errors < 3)
		DAT = MSG_ABORT;
	    else
		DAT = MSG_BUSDEVRST;
	    break;
	    
	    /*******************************************************
	     *  MSGIN:  Get a message in from the device. There
	     *  are actually only a handful that we care about
	     *  but I'm sure that some will have to be added because
	     *  there are some that require a response which will
	     *  add some logic to the driver.
	     */
	    
	case XP_MSGIN:
	    if (errors) {
		bb = DAT;
		++errors;
		break;
	    }
	    
	    msgin = DAT;
	    switch (msgin) {
		/***********************************************
		 *  Command Complete Message.
		 */
	    case MSG_CC:
		c->c_state = DEV_FINISHED;
		c->c_cmdflags &= ~CMD_PENDING;
		if (c->c_scsistatus == ST_BUSY)
		    c->c_cmdflags |= CMD_DEVBUSY;
		if (c->c_scsistatus == ST_PENDING)
		    c->c_scsistatus = ST_GOOD;
		done = 1;
		break;
		
		/***********************************************
		 *  Disconnect Message.
		 */
	    case MSG_DISCONNECT:
		c->c_state = DEV_WORKING;
		done = 1;
		break;
		
	    case MSG_NOP:
	    case MSG_SAVEDPTR:
	    case MSG_RSTRDPTR:
		break;
		
		/***********************************************
		 *  Whole host of messages that we could care
		 *  less about.
		 */
	    default:
		if (!(msgin & MSG_IDENTIFY))
		    cmn_err(CE_NOTE, "haiss: Bad message 0x%x\n", msgin);
		break;
	    }       /* switch */
	    break;

	default:
	    cmn_err(CE_WARN, "haiss: Invalid SCSI Bus Phase.");
	    ++errors;
	    break;
	}	/* switch */
    }	/* while */
    setcsr(HAIDLE);

    if (errors != 0) {
	c->c_state = DEV_FINISHED;
	c->c_scsistatus = ST_ABRT;
	r->hastat = HAST_BADBUSPH;
    }

    return (errors == 0);
}	/* scsi_ioxfer() */

/***********************************************************************
 *  ssdone()
 */

#if __USE_PROTO__
LOCAL void ssdone(Register ssccb_p c)
#else
LOCAL void
ssdone(c)
Register ssccb_p	c;
#endif
{
    Register srb_p r = c->c_srb;

    drvl[SCSIMAJOR].d_time &= ~(bit(c->c_target));
    c->c_cmdflags = 0;
    c->c_cdblen = 0;
    c->c_cdb = NULL;
    c->c_datain = c->c_dataout = NULL;
    c->c_srb = NULL;
    c->c_state = DEV_IDLE;
    if (r) {
        r->status = c->c_scsistatus;
        if (r->cleanup) 
            (*(r->cleanup))(r);
    }
}	/* ssdone() */

/***********************************************************************
 *  sswork() --         Process work requests which require the SCSI
 *                      Bus.
 *  
 *  This loop handles all work requeusts which require the SCSI Bus.
 *  This is needed because the SCSI bus may be busy when a device tries
 *  to start a command.
 *
 *
 *  Returns 1 if a new context was generated and 0 if not.
 */

#if __USE_PROTO__
LOCAL int sswork(void)
#else
LOCAL int
sswork()
#endif

{
    Register ssccb_p	q;
    Register srb_p	r;
    static int 		buserrors = 0;	/* Start buserrors at 0 */
    static volatile int	locked = 0;	/* Activity flag */
    int 		alldone = 0;	/* Assume not! */
    int			s;
    
    /***************************************************************
     *  It could get confusing if more than one context of this
     *  routine was loaded at any one time.I also cannot figure
     *  out a locking mechanism to for each call so I chose to
     *  do this here at the work loop.
     */
    
    s = sphi();
    if (locked) {
	spl(s);
	return 0;
    }
    else
	++locked;
    spl(s);
    
    /***************************************************************
     *  Check here to see if the bus has been busy too long. If
     *  it has then do a SCSI Bus Reset. This will cancel all 
     *  commands on all devices and return their states to normal.
     *  
     *  This removes two failure conditions: 
     *      1.  When the bus is truly hung and needs to be reset.
     *  
     *      2.  When a third party SCSI command goes awry or 
     *          takes to long and causes 1.
     */
    
    if (buserrors > MAXBUSYCOUNT) {
	cmn_err(CE_WARN, "haiss: SCSI bus hung, resetting");
	hareset(HAIDLE);
	buserrors = 0;
    }
    
    /***************************************************************
     *  Loop in here until there isn't any work to do or we can't
     *  get the bus. The order in here is for interrupt service
     *  when most of the time we will be processing normal 
     *  requests.
     */
    
    for ( ; ; ) {
	
	/*******************************************************
	 *  Come here to either start work anew or finish up
	 *  some previous request. The busy could be in any
	 *  phase here but we shouldn't be driving it at all.
	 */
	
	if ((q = priority_queue) != NULL) {
	    priority_queue = NULL;
	    r = q->c_srb;
	    /***********************************************
	     *  Try the reselect. If it fails just drop
	     *  the task. The device is the only thing
	     *  that can initiate closure here.
	     */
	    
	    if (!scsi_reconnect()) {
		set_host_status(r, HAST_SELTO);
		break;
	    }

	    scsi_ioxfer(q);
	    s = sphi();
	    if (q->c_state == DEV_FINISHED)
		ssdone(q);
	    buserrors = 0;
	    spl(s);
	}	/* if */
	
	/*******************************************************
	 *  Next start any normal work that needs to get done.
	 *  This may be a mistake but so far all work requires
	 *  the SCSI Bus. The arbitration code for the bus
	 *  goes here.
	 */
	
	else if ((q = normal_queue) != NULL) {
	    r = q->c_srb;
	    /***********************************************
	     *  So far all requests require arbitration
	     *  so do it here. This also allows us to check
	     *  for a reselect/arbitration colision here.
	     */
	    
	    if (!scsi_arbitrate(hostid)) {
		if (reselect() && set_priority_queue())
		    continue;
		else
		    break;
	    }	/* if */
	    
	    /***********************************************
	     *  Only do one of the next three actions.
	     *  Bus Device Reset is highest priority followed
	     *  by Abort Command, followed by Start Command.
	     */
	    
	    if (q->c_reqflags & REQ_BUSDEVICERST) {
		if (!scsi_connect(hostid, q->c_target, MSG_BUSDEVRST)) {
		    set_host_status(r, HAST_SELTO);
		    cmn_err(CE_WARN,
			    "haiss: SCSI Reset at ID %d failed (SELTO)",
			    q->c_target);
		    break;
		}
		q->c_state = DEV_FINISHED;
	    }	/* else if */
	    else if (q->c_reqflags & REQ_ABORTSCSICMD) {
		if (!scsi_connect(hostid, r->target, MSG_ABORT)) {
		    set_host_status(r, HAST_SELTO);
		    cmn_err(CE_WARN,
			    "haiss: Abort SCSI command failed (SELTO)");
		    break;
		}
		q->c_scsistatus = ST_DRVABRT;
		q->c_state = DEV_FINISHED;
		cmn_err(CE_NOTE,
			"haiss: Command 0x%x at ID %d LUN %d aborted.",
			r->cdb.g0.opcode,
			r->target,
			r->lun);
	    }	/* else if */
	    else if (q->c_reqflags & REQ_STARTSCSICMD) {
		if (!scsi_connect(hostid,
				  r->target,
				  MSG_IDENTIFY | ID_DISCONNECT | r->lun)) {
		    set_host_status(r, HAST_SELTO);
		    break;
		}
		scsi_ioxfer(q);
	    }
	    s = sphi();
            q->c_reqflags = 0;
	    normal_queue = q->c_next;
	    q->c_next = NULL;
	    if (q->c_state == DEV_FINISHED)
		ssdone(q);
	    buserrors = 0;
	    spl(s);
	}	/* else ... if */
	
	/*******************************************************
	 *  Finally if we get down here then there isn't any
	 *  work to do so mark the alldone flag and exit.
	 */
	
	else {
	    alldone = 1;
	    break;
	}	/* else */
	
	/*******************************************************
	 *  At this point the host should be Idle. Wait like
	 *  that for a bus settle delay or so to clear out
	 *  any pending commands that the devices have finished.
	 */
	
	eventwait(NULL, 4 * BUSSETTLE_DLY);
    }	/* for */
    
    /***************************************************************
     *  If this doesn't finish up all the work the bus is probably
     *  busy, do something else for a couple of cycles and come
     *  back here.
     */
    
    if (!alldone) {
	timeout(&sstim, 2, (void (*)()) &sswork, 0);
	++buserrors;
    }
    
    locked = 0;
    return 1;
}	/* sswork() */

/***********************************************************************
 *  ssintr() --         Handle hardware interrupts.
 *  
 *  In the Implementations that I see of this card the SCSI Select line
 *  is tied to the interrupt generation hardware. External assertion
 *  of this line causes an interrupt to be generated. This will normally
 *  be caused by a SCSI reselect phase on the bus. The proper action
 *  here is to determine if a reconnect is happening and if so whether
 *  or not the host is the reselected device.
 */
#if __USE_PROTO__
LOCAL void ssintr(void)
#else
LOCAL void
ssintr()
#endif

{
    /***************************************************************
     *  Interrupt check for a reselect the expected reselect phase.
     */
    while (reselect()) {
	
	/*******************************************************
	 *  Try and set up the priority queue from the reselect
	 *  info and then run with it.
	 */
	
	if (set_priority_queue()) {
	    if (!sswork())
		break;
	}
    }	/* while */
}	/* ssintr() */

/***********************************************************************
 *  ssinsqueue() --     Insert a request into the driver's work queue.
 *  
 *  Insert a request into the driver's work queue as long as one's
 *  not there already.
 */

#if __USE_PROTO__
LOCAL void ssinsqueue(Register ssccb_p c, int newreqflags)
#else
LOCAL void
ssinsqueue(c, newreqflags)
Register ssccb_p	c;
int			newreqflags;
#endif
{
    Register ssccb_p 	q;
    int			s;
    
    s = sphi();
    c->c_reqflags |= newreqflags;
    if (!normal_queue) {
	normal_queue = c;
    }
    else {
	q = normal_queue;
	
	for ( ; ; ) {
	    if (q == c) {
		q = NULL;
		break;
	    }
	    if (!q->c_next)
		break;
	    else
		q = q->c_next;
	}
	
	if (q) {
	    q->c_next = c;
	    c->c_next = NULL;			
	}
    }
    spl(s);
}	/* ssinsqueue() */

/***********************************************************************
 *  ss_start() --      Start a scsi command. 
 *  
 *  For this host adapter make sure the request is valid, set the
 *  flags and start the work queue.
 */

#if __USE_PROTO__
LOCAL int ss_start(Register srb_p r)
#else
LOCAL int
ss_start(r)
Register srb_p	r;
#endif
{
    Register ssccb_p	c;
    static int		srbid = 0;	/* Number of srb */
    int s;

    s = sphi();
    ++r->tries;
    if (r->target >= MAXDEVS ||
	r->lun >= MAXUNITS ||
	!(c = ssdevs[r->target])) {
	spl(s);
	cmn_err(CE_WARN,
		"haiss: Illegal device ID%d LUN%d\n",
		r->target,
		r->lun);
	return 0;
    }
    
    if (c->c_state != DEV_IDLE) {
	spl(s);
	cmn_err(CE_WARN, "haiss: ID%d busy.", r->target);
	return 0;
    }
    r->status = c->c_scsistatus = ST_INVDSRB;
    spl(s);
    
    c->c_srb = r;
    c->c_cdb = &(r->cdb.g0.opcode);
    switch (c->c_cdb[0] & GROUPMASK) {
    case GROUP0:
	c->c_cdblen = 6;
	break;
    case GROUP1:
    case GROUP2:
	c->c_cdblen = 10;
	break;
    case GROUP5:
	c->c_cdblen = 12;
	break;
    default:
	c->c_state = DEV_FINISHED;
	c->c_cdb = NULL;
	c->c_cdblen = 0;
	r->hastat = HAST_INVDCDB;
	return 0;
    }
    
    /***************************************************************
     *  Setup the data area.
     */
    
    c->c_id = 0x10000000 | (r->target << 24) | (srbid++ & 0xffffff);
    c->c_time = r->timeout;
    c->c_cmdflags = 0;

    if ((r->buf.space & WATCH_REQACK) != 0 || 
        (haiss_slowdev & bit(c->c_target)) != 0)
        c->c_cmdflags |= CMD_NEEDREQACK;
    
    if (r->buf.ba_phys) {
	switch (r->buf.space & SPACE_MASK) {
	case PHYS_ADDR:		/* Physical Address - (who knows) */
	    c->c_segcount = 1;
	    c->c_sgs[0].sgs_segstart = r->buf.ba_phys;
	    c->c_sgs[0].sgs_segsize = r->buf.size;
	    break;
	case KRNL_ADDR:		/* Kernel Address */
	case USER_ADDR:		/* User Address */
	    c->c_segcount = ukbuftosgl(r->buf.ba_virt,
				       r->buf.size,
				       &(c->c_sgs[0]),
				       MAXSEGCOUNT);
	    if (c->c_segcount == 0) {
		cmn_err(CE_WARN, "haiss: Cannot create scatter/gather list.");
		return 0;
	    }
	    break;
	case SYSGLBL_ADDR:		/* System Global address (yeah) */
	    c->c_segcount = sysgbuftosgl(r->buf.ba_phys,
					 r->buf.size,
					 &(c->c_sgs[0]),
					 MAXSEGCOUNT);
	    if (c->c_segcount == 0) {
		cmn_err(CE_WARN, "haiss: Cannot create scatter/gather list.");
		return 0;
	    }
	    break;
	case SGLIST_ADDR:		/* Scatter gather list */
	    if (r->buf.size > MAXSEGCOUNT) {
		cmn_err(CE_WARN, "haiss: Cannot handle scatter/gather list.");
		return 0;
	    }
	    else {
		c->c_segcount = r->buf.size;
		memcpy(&(c->c_sgs[0]),
		       r->buf.ba_sglist,
		       r->buf.size * sizeof(haisgsegm_t));
	    }
	    break;
	} /* switch */
    } /* if */
    c->c_datain = c->c_dataout  = NULL;
    c->c_dataofs = 0;
    if (r->xferdir & DMAWRITE) {
	c->c_dataout = &(c->c_sgs[0]);
	c->c_cmdflags |= CMD_OVERRUN;
    }	/* if */
    else if (r->xferdir & DMAREAD) {
	c->c_datain = &(c->c_sgs[0]);
	c->c_cmdflags |= CMD_UNDERRUN;
    }	/* else - if */

    r->status = c->c_scsistatus = ST_PENDING;
    drvl[SCSIMAJOR].d_time |= bit(r->target);
    ssinsqueue(c, REQ_STARTSCSICMD);
    sswork();
    return 1;
}	/* ss_start() */

/***********************************************************************
 *  ss_abort() --      Abort a SCSI command in progress.
 *  
 *  Stop a scsi command on the bus or just drop one that's in a long
 *  retry loop.
 */

#if __USE_PROTO__
LOCAL void ss_abort(Register srb_p r)
#else
void
LOCAL ss_abort(r)
Register srb_p	r;
#endif
{
    Register ssccb_p	c;
    Register int	s;
    
    s = sphi();
    if (!r || r->target >= MAXDEVS || r->lun >= MAXUNITS)
	return;

    if (!(c = ssdevs[r->target]) || c->c_state != DEV_WORKING || c->c_srb != r)
	return;

    cmn_err(CE_NOTE,
	    "haiss: Abort SCSI command ID: %d LUN: %d opcode 0x%x",
	    r->target,
	    r->lun,
	    r->cdb.g0.opcode);
    if (!(c->c_cmdflags & CMD_PENDING))
	return;
    
    ssinsqueue(c, REQ_ABORTSCSICMD);
    sswork();
    spl(s);
}	/* ss_abort() */

/***********************************************************************
 *  ss_bdr() --    Reset a SCSI device.
 *  
 *  Send a SCSI device the Bus Device Reset message. The should cause
 *  the device to go through it's power on reset sequence of operations
 *  and then come back to a known state.
 */

#if __USE_PROTO__
LOCAL void ss_bdr(Register int target)
#else
LOCAL void
ss_bdr(target)
Register int    target;
#endif
{
    Register ssccb_p	c;
    int s;
    
    s = sphi();
    if (target >= MAXDEVS || !(c = ssdevs[target]))
	return;
    
    ssinsqueue(c, REQ_BUSDEVICERST);
    sswork();
    spl(s);
    
    return;
}	/* ss_bdr() */

#if __USE_PROTO__
LOCAL void ss_timer(void)
#else
LOCAL void
ss_timer()
#endif
{
    Register ssccb_p c;
    Register int id;
    int s;

    for (id = 0 ; id < MAXDEVS ; ++id) {
	if ((c = ssdevs[id]) != NULL) {
	    s = sphi();
	    if ((drvl[SCSIMAJOR].d_time & bit(id)) &&
		c->c_time != 0 &&
		--(c->c_time) == 0) {
		spl(s);
		if (c->c_srb) {
		    cmn_err(CE_WARN, "haiss: timeout at ID %d", c->c_target);
		    ss_abort(c->c_srb);
		}
	    }
	    else
		spl(s);
	}
    }
    return;
}	/* hatimer() */

/*
 * The host adapter function table.
 */

haft_t ss_haft = {
	0,
	ss_timer,		/* Timeout handler */
	ss_load,		/* Initialization routine */
	ss_unload,		/* shutdown routine */
	ss_start,		/* Start a SCSI command from an srb */
	ss_abort,		/* Abort a SCSI command from an srb */
	ss_bdr			/* Reset a scsi device */
};
