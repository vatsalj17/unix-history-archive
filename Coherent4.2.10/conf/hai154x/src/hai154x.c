/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1994 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */
/***********************************************************************
 *  Module: hai154x.c
 *
 *  Host Adapter Interface routines for the Adaptec AHA154XX series
 *  of host adapters.
 *
 *  Much of the information used to create this driver was obtained
 *  from the AHA1540B/1542B Technical Reference Manual available from
 *  Adaptec by phone or snail mail at:
 *
 *	  Adaptec - Literature Department
 *	  691 South Milpitas Blvd.
 *	  Milpitas, CA 95035
 *	  (408) 945-8600
 *
 *  Copyright (c), 1993 Christopher Sean Hilton, All Rights Reserved.
 *
 *  Last Modified: Wed Aug 25 07:19:10 1993 CDT
 */

#define HA_MODULE	   	/* Host Adapter Module */

#include <stddef.h>
#include <sys/coherent.h>
#include <sys/con.h>
#include <sys/cmn_err.h>
#include <sys/devices.h>
#include <sys/types.h>
#include <sys/hdioctl.h>
#include <sys/haiscsi.h>
#include <sys/stat.h>

#define LOCAL	static
#if __GNUC__
#define Register
#else
#define Register register
#endif

/*
 * Configurable variables - see /etc/conf/hai/Space.c
 */

extern unsigned short	hai154x_base;
extern unsigned short	hai154x_intr;
extern unsigned short	hai154x_dma;
extern unsigned short	hai154x_haid;

#if 0
extern int		HAI_SD_HDS;
extern int		HAI_SD_SPT;
#endif

extern unsigned char hai154x_xferspeed;
extern unsigned char hai154x_busofftime;
extern unsigned char hai154x_busontime;


/* Actually, the following is just a forward declaration. */
extern haft_t	_154x_haft;

#define CTRLREG		(hai154x_base + 0)	/* Control Register (Write) */
#define	 HRST		bit(7)	/* Hard Reset */
#define	 SRST		bit(6)	/* Soft Reset */
#define	 IRST		bit(5)	/* Interrupt Reset */
#define	 SCRST   	bit(4)	/* SCSI Bus Reset */

#define STSREG	  	(hai154x_base + 0)	/* Status Register (Read) */
#define	 STST		bit(7)	/* Self Test in progress */
#define	 DIAGF   	bit(6)	/* Internal Diagnostic Failure */
#define	 INIT		bit(5)	/* Mailbox Initialization Required */
#define	 IDLE		bit(4)	/* SCSI Host Adapter Idle */
#define	 CDF	 	bit(3)	/* Command/Data Out Port Full */
#define	 DF	  	bit(2)	/* Data In Port Full */
#define	 INVDCMD 	bit(0)	/* Invalid HA Command */

#define CMDDATAOUT 	(hai154x_base + 1)   /* Command/Data Out (Write) */
#define	 NOP		0x00	/* No Operation (really?) */
#define	 MBINIT	  	0x01	/* Mail Box Initialization */
#define	 STARTSCSI   	0x02	/* Start a SCSI Command */
#define	 STARTBIOS   	0x03	/* Start a BIOS Command */
#define	 HAINQUIRY   	0x04	/* HA Inquiry */
#define	 ENBLMBOA	0x05	/* Enable Mailbox Out Available Interrupt */
#define	 SETSELTO	0x06	/* Set Selection Timeout */
#define	 SETBUSON	0x07	/* Set Bus on time */
#define	 SETBUSOFF   	0x08	/* Set Bus off time */
#define	 SETXFERSPD	0x09	/* Set transfer speed */
#define	 RETINSTDEV  	0x0a	/* Return Installed Devices */
#define	 RETCFGDATA  	0x0b	/* Return Configuration Data */
#define	 ENBLTRGTMD  	0x0c	/* Enable Target Mode */
#define	 RETSUDATA   	0x0d	/* Return Setup Data */

/***********************************************************************
 *  These commands are specific to the Adaptec AHA-154xC.
 */

#define RETEXTBIOS	0x28	/* Return Extended BIOS Information */
#define  EXTBIOSON	bit(3)

#define SETMBIENBL	0x29	/* Set Mailbox Interface Enable */
#define  MBIENABLED	bit(1)

#define DATAIN	  	(hai154x_base + 1)

#define INTRFLGS	(hai154x_base + 2)
#define	 ANYINTR 	bit(7)	/* Any Interrupt */
#define	 SCRD		bit(3)	/* SCSI Reset Detected */
#define	 HACC		bit(2)	/* HA Command Complete */
#define	 MBOA		bit(1)	/* MBO Empty */
#define	 MBIF		bit(0)	/* MBI Full */

#define MBOFREE		0x00	/* Mailbox out is free */
#define MBOSTART	0x01	/* Start CCB in this Mailbox */
#define MBOABORT	0x02	/* Abort CCB in this Mailbox */

#define MBIFREE		0x00	/* Mailbox in is free */
#define MBISUCCESS	0x01	/* Mailbox's CCB Completed Successfully */
#define MBIABORTED	0x02	/* Mailbox's CCB Aborted */
#define MBIABRTFLD	0x03	/* CCB to Abort not found */
#define MBIERROR	0x04	/* CCB Completed with error */

#define TIMEOUT		-1	/* Timeout from pollxxx() functions. */

#define ST_HAINIT   	0x0001	/* Host Adapter being initialized */
#define ST_HAIDLE   	0x0002	/* Host Adapter is idle */
#define ST_HASTART      0x0003  /* Host Adapter is in start state */

/***********************************************************************
 *  Types
 */

#pragma align 1

typedef union addr3_u {		 /* addr3_u for big/little endian conversions */
	unsigned long   value;
	unsigned char   byteval[sizeof(unsigned long)];
} addr3_t;

typedef union mbo_u *mbo_p;	 /* Out Box to host adapter */

typedef union mbo_u {
	unsigned char   cmd;
	paddr_t		ccbaddr;
} mbo_t;

typedef union mbi_u *mbi_p;	 /* In Box from host adapter */

typedef union mbi_u {
	unsigned char   sts;
	paddr_t		ccbaddr;
} mbi_t;

typedef struct mb_s {		/* Host adapter mailbox type */
	mbo_t	   o[MAXDEVS];	/* One out box for each device */
	mbi_t	   i[MAXDEVS];	/* One in box for each possible reply */
} mb_t;

typedef struct haccb_s *haccb_p;	/* Host Adapter Command/Control Block */

typedef struct haccb_s {
	unsigned char   opcode;
	unsigned char   addrctrl;
	unsigned char   cdblen;
	unsigned char   senselen;
	unsigned char   datalen[3];
	unsigned char   bufaddr[3];
	unsigned char   linkaddr[3];
	unsigned char   linkid;
	unsigned char   hoststs;
	unsigned char   trgtsts;
	unsigned char   pad[2];
	cdb_t		cdb;
} haccb_t;

typedef struct dsentry_s *dsentry_p;

typedef struct dsentry_s {
	unsigned char   size[3];
	unsigned char   addr[3];
} dsentry_t;

typedef struct dslist_s *dslist_p;

typedef struct dslist_s {
	dsentry_t	entries[16];
} dslist_t;

#pragma align

/***********************************************************************
 *  Variables
 */

static int	hastate;	/* Host Adapter State */
static volatile int hainit_stsreg = 0; /* Host Adapter status flags. */
static volatile int hainit_ccflag = 0; /* Host Adapter Command Complete flag. */
static mb_t	mb;		/* Mailboxes for host adapter */	
static haccb_t  ccb[MAXDEVS];   /* CCB's for mailboxes */
static paddr_t  ccbbase;	/* ccbbase address for quick checkmail */
static srb_p	actv[MAXDEVS];  /* Active srb's for each target */
static dslist_t ds[MAXDEVS];	/* Data segment lists one for each target */
static unsigned chkport = 0,	/* Port for chkset/ckhclr */
		chkstop = 0,	/* Target value for chkset/chkclr */
		chkval  = 0;	/* Value in port chkset/chkclr */

/***********************************************************************
 *  Support Functions
 */

#ifndef DEBUG
#define dbg_printf(lvl, msg)
#else
unsigned AHA_DBGLVL = 1;
#define dbg_printf(lvl, msg)	{ if (AHA_DBGLVL >= lvl) cmn_err(CE_CONT, msg); }
#endif

#define min(a, b)   (((a) <= (b)) ? (a) : (b))

/***********************************************************************
 *  chkclr()	--  Check port (chkport) for bits (chkstop) to be clear.
 *				  If clear return 1 else return 0.  Leave value of
 *				  port in chkval;
 */

#if __USE_PROTO__
LOCAL int chkclr(void)
#else
LOCAL int
chkclr()
#endif
{
	return ((chkval = inb(chkport)) & chkstop) == 0;
}   /* chkclr() */

/***********************************************************************
 *  chkset() --	Check port (chkport) for bits (chkstop) to be set.
 *		If all bits are set return 1 else return 0. Leave
 *		value of port in chkval.
 */

#if __USE_PROTO__
LOCAL int chkset(void)
#else
LOCAL int
chkset()
#endif
{
	return ((chkval = inb(chkport)) & chkstop) == chkstop;
}   /* chkset() */

/***********************************************************************
 *  pollclr()   --  Wait ticks clock ticks for bit(s) to clear in a
 *                  port.
 */
#if __USE_PROTO__
LOCAL int pollclr(register unsigned port,	/* port to watch */
		  register unsigned bits,	/* bits to watch for */
		  unsigned ticks)	   	/* time to wait.  */
#else
LOCAL int pollclr(port, bits, ticks)
register unsigned	port;	   /* port to watch */
register unsigned	bits;	   /* bits to watch for */
unsigned		ticks;	   /* number of milliseconds to wait for */
#endif
{
	register s;

	s = sphi();
	chkport = port;
	chkstop = bits;
	busyWait(chkclr, ticks);
	spl(s);
	
#if 1   /* DEBUG */
	if ((chkval & bits) == 0)
		return chkval;
	else {
		cmn_err(CE_NOTE,
			"!pollclr: <Timeout Reg: 0x%x mask 0x%x value 0x%x>",
			port,
			bits,
			chkval);
		return TIMEOUT;
	}
#else
	return ((chkval & bits) == 0) ? chkval : TIMEOUT;
#endif	
}   /* pollclr() */

/***********************************************************************
 *  pollset()   --  Wait ticks clocks for bit(s) to get set
 *                  in a port.
 */
#if __USE_PROTO__
LOCAL int pollset(register unsigned port,	/* port to watch */
		  register unsigned bits,	/* bits to watch for */
		  unsigned ticks)	   	/* time to wait */
#else
LOCAL int pollset(port, bits, ticks)
register unsigned	port;	   /* port to watch */
register unsigned	bits;	   /* bits to watch for */
unsigned		ticks;	   /* number of milliseconds to wait for */
#endif
{
	register s;

	s = sphi();
	chkport = port;
	chkstop = bits;
	busyWait(chkset, ticks);
	spl(s);

#if 1   /* DEBUG */
	if ((chkval & bits) == bits)
		return chkval;
	else {
		cmn_err(CE_NOTE,
			"!pollset: <Timeout Reg: 0x%x mask 0x%x value 0x%x>",
			port,
			bits,
			chkval);
		return TIMEOUT;
	}
#else
	return ((chkval & bits) == bits) ? chkval : TIMEOUT;
#endif	
}   /* pollset() */

/***********************************************************************
 *  hacc()
 *
 *  Host Adapter Command Completed - Returns 1 if last host adapter
 *  command completed without error.
 */

#if __USE_PROTO__
LOCAL int hacc(void)
#else
LOCAL int
hacc()
#endif
{
	register unsigned stsreg;

	if (pollset(INTRFLGS, HACC, 350) == TIMEOUT)
		return 0;

	stsreg = pollset(STSREG, IDLE, 350) & (IDLE | INIT | CDF | INVDCMD);

	if (stsreg != IDLE) {
		cmn_err(CE_WARN, "Aha154x stuck - STSREG: 0x%x", stsreg);
		return 0;
	}
	return 1;
}   /* hacc() */

/***********************************************************************
 *  haidle()
 *
 *  Returns 1 if the Idle Bit is on in the adapter status register
 */

#define haidle()	(pollset(STSREG, IDLE, 350) != TIMEOUT)

/*
 * hai_flag_set(port, flags)
 *
 * Returns non-zero if all specified flags are set in the port.
 */
#define hai_flag_set(port, flags) ((inb((port)) & (flags)) == (flags))

/***********************************************************************
 *  gethabyte()
 *
 *  Get a byte from the host adapter Data In register.
 */
#if __USE_PROTO__
LOCAL int gethabyte(void)
#else
LOCAL int
gethabyte()
#endif
{
	if (pollset(STSREG, DF, 350) == TIMEOUT) {
		cmn_err(CE_NOTE,
			"!hai154x: <gethabyte timeout (0x%x) 0x%x 0x%x>",
			STSREG,
			DF,
			chkval);
		return TIMEOUT;
	}
	else    /* Beware of sign extension */
		return ((int)inb(DATAIN) & 0x00ff);
}   /* gethabyte() */

/***********************************************************************
 *  puthabyte()
 *
 *  Write a byte to the host adapter Command/Data Out Register.
 */
#if __USE_PROTO__
LOCAL int puthabyte(unsigned char b)
#else
LOCAL int
puthabyte(b)
unsigned char b;
#endif
{
	if (pollclr(STSREG, CDF, 350) == TIMEOUT)
		return 0;
	else {
		outb(CMDDATAOUT, b);
		return 1;
	}
}   /* puthabyte() */



/*
 * We need to wait until either a HACC or DF occurs signifying
 * the end of a pending command.
 */
static int hainit_status()
{
  int status;

  /* Check for hacc and/or DF */
  if (hainit_ccflag == 0 && hai_flag_set(STSREG, DF) == 0)
    return 0;
  else  /* HACC or DF came through */
    return 1;
}

/***********************************************************************
 *  Function:   aha_icmd()
 *  
 *  Send a command to the host adapter. and do all the handshaking
 *  etc. This assumes interrupts are enabled.
 */
#if __USE_PROTO__
LOCAL int aha_icmd(unsigned char *cmd, size_t cmdlen, unsigned char *data,
		   size_t datalen, int stifle)
#else
LOCAL int 
aha_icmd(cmd, cmdlen, data, datalen, stifle)
unsigned char 	*cmd;
size_t		cmdlen;
unsigned char	*data;
size_t		datalen;
int stifle;
#endif
{
  int errorflag = 0;
  int ch;
  unsigned char	*p;
  int cnt;
  
  /* Wait for card to go idle */
  if (!haidle()) {
    cmn_err(CE_WARN, "<hai154x: Timeout waiting for host adapter idle>");
    errorflag = 1;
    goto bail_cmd;
  }
  
  /*******************************************************
   *  Send as many of the command bytes as possible to
   *  the host.
   */

  hainit_ccflag = 0;
  hainit_stsreg = 0;

  p = cmd;
  
  for (cnt = 0; cnt < cmdlen && !errorflag && !hainit_ccflag; cnt++)
    errorflag = !puthabyte(*p++);

  /*******************************************************
   *  If the handshake failed then message to that effect.
   */
  if (errorflag || (hainit_ccflag && (datalen != 0))) {
    if (hainit_stsreg & INVDCMD)
      cmn_err(CE_WARN, "<hai154x: Invalid command or parameter (send)>");
    else
      cmn_err(CE_WARN, "<hai154x: Command handshake failed>");
    
    goto bail_cmd;
  }

  /* Busy wait for HACC or DF to occur */
  if (busyWait(hainit_status, 350) == 0) {
    cmn_err(CE_WARN, "<hai154x: status timeout (send)>");
    goto bail_cmd;
  }

  /*
   * Need to double check in case interrupt occured after we sent
   * last byte.
   */
  if (hainit_ccflag && datalen != 0) {
    if (hai_flag_set(STSREG, INVDCMD) && !stifle)
      cmn_err(CE_WARN, "<hai154x: Invalid command or parameter (last)>");
    else if (!stifle)
      cmn_err(CE_WARN, "<hai154x: Abnormal command completion>");

    goto bail_cmd;
  }


  /* If return data is expected, receive it */
  if (data && datalen) {
    p = data;
    
    for (cnt = 0; cnt < datalen && !errorflag && !hainit_ccflag; cnt++) {
      errorflag = ((ch = gethabyte()) == TIMEOUT);
      if (errorflag == 0)
	*p++ = ch;
      else {
	cmn_err(CE_WARN, "<hai154x: Timeout on command data recv>");
	goto bail_cmd;
      }
    }

    /* Check for premature end of data */
    if (hainit_ccflag && (cnt < datalen)) {
      cmn_err(CE_WARN, "<hai154x: command data underrun>");
      goto bail_cmd;
    }

    /* Wait for final HACC to acknowledge end of data bytes */
    if (busyWait(hainit_status, 350) == 0) {
      cmn_err(CE_WARN, "<hai154x: status timeout (command complete)>");
      goto bail_cmd;
    }

    /*
     * Check for data overrun; if busyWait ended, but
     * hainit_ccflag isn't set, then it ended because of DF
     * and there is unknown data waiting for us.
     */
    if (hainit_ccflag == 0) {
      cmn_err(CE_WARN, "<hai154x: command data overrun>");
      goto bail_cmd;
    }
  }

  hainit_ccflag = 0;
  return 1;			/* Return success */

  
  /***************************************************************
   *  In case of any sort of error handshake bytes into the bit
   *  bucket.
   */
 bail_cmd:

  while (inb(STSREG) & DF) {
    ch = gethabyte();
    busyWait(NULL, 1);
  }
  
  hainit_ccflag = 0;
  return 0;			/* Return as an error */
}

/***********************************************************************
 *  Function:   aha_checkbios()
 *  
 *  On the AHA154xC/154xCF check to see if the user is using the BIOS
 *  extensions available on this card. If so turn them off. This also
 *  determines if the card is using 255 head translation.
 */

#if __USE_PROTO__
LOCAL int aha_checkbios(void)
#else
LOCAL int
aha_checkbios()
#endif
{
	unsigned char 	cmddata[3];
	int 		retval;	

	cmddata[0] = RETEXTBIOS;
	hainit_ccflag = 0;
	if (!aha_icmd(cmddata, 1, cmddata + 1, 2, 1))
		return 0;

	retval = (cmddata[1] & EXTBIOSON) ? 255 : 64;
	if (cmddata[2] == 0)
		return retval;

	cmddata[0] = SETMBIENBL;
	cmddata[1] = 0;
	hainit_ccflag = 0;
	if (!aha_icmd(cmddata, 3, NULL, 0, 0)) {
		cmn_err(CE_WARN,
			"<hai154x: Set mailbox interface enable failed>");
		return 0;
	}
	return retval;
}	/* aha_checkbios() */

/***********************************************************************
 *  Function:   aha_inquiry()
 *  
 *  Do an inquiry on the host adapter to find out what type it is.
 */
#if __USE_PROTO__
LOCAL int aha_inquiry(void)
#else
LOCAL int 
aha_inquiry()
#endif
{
  unsigned char buf[5];
  unsigned char *command = &buf[0];
  unsigned char *data = &buf[1];
  
  command[0] = HAINQUIRY;
  
  if (!aha_icmd(command, 1, data, 4, 0)) {
    cmn_err(CE_WARN, "<hai154x: Host adapter inquiry command failure>");
    return -1;
  }

  return data[0];
}



/***********************************************************************
 *  Function:   aha_tunescsibus()
 *  
 *  Set host adapter bus on/bus off times from the external constants
 *  patched into the kernel. These constants are currently selectable
 *  by patching them into the kernel.  They should be set up to work
 *  with idtune. [csh]
 */

#if __USE_PROTO__
LOCAL int aha_tunescsibus(void)
#else
LOCAL int 
aha_tunescsibus()
#endif
{
	unsigned char 	busonbuf[2],
			*busoncmd	= &busonbuf[0],
			*busondata	= &busonbuf[1],
			busoffbuf[2],
			*busoffcmd	= &busoffbuf[0],
			*busoffdata	= &busoffbuf[1],
			xferspdbuf[2],
			*xferspdcmd 	= &xferspdbuf[0],
			*xferspddata	= &xferspdbuf[1];

	/***************************************************************
         *  Set up command buffers for bus on/off times and xfer speed.
         */

	busoncmd[0] = SETBUSON;
	busondata[0] = hai154x_busontime;

	busoffcmd[0] = SETBUSOFF;
	busoffdata[0] = hai154x_busofftime;

	xferspdcmd[0] = SETXFERSPD;
	xferspddata[0] = hai154x_xferspeed;
	
        /*******************************************************
         *  Set the bus on time.
         */

        if (/* hai154x_busontime != DEFAULT  && */
	    !aha_icmd(busoncmd, 2, NULL, 0, 0)) {
                cmn_err(CE_WARN, "<hai154x: Set bus on time failed>");
		return 0;
	}

        /*******************************************************
         *  And finally the bus off time.
         */
        if (/* hai154x_busofftime != DEFAULT && */
	    !aha_icmd(busoffcmd, 2, NULL, 0, 0))  {
                cmn_err(CE_WARN, "<hai154x: Set bus off time failed>");
		return 0;		
	}

        if (/* hai154x_busofftime != DEFAULT && */
	    !aha_icmd(xferspdcmd, 2, NULL, 0, 0))  {
                cmn_err(CE_WARN, "<hai154x: Set bus off time failed>");
		return 0;		
	}

	return 1;
}	/* aha_tunescsibus() */

/***********************************************************************
 *  Function:   aha_hareset()
 *  
 *  Reset the host adapter. There are two types of reset. A hard reset
 *  resets both the host adapter and all the devices attached to the
 *  bus. A soft reset only resets the host adapter. During initialization
 *  a hard reset is called for. This seems to be the only thing that
 *  will work with the AHA-154xC adapters as well as the 174x adapters
 *  operating in standard mode. The Bus Logic controllers also seem
 *  to need the hard reset. During operation a soft reset is called
 *  for as this will allow operations on the devices attached to continue
 *  without problems. When calling this function as a part of error
 *  recovery try the soft reset first before going to the hard reset.
 */
#if __USE_PROTO__
LOCAL int aha_hareset(int hardreset)
#else 
LOCAL int aha_hareset(hardreset)
int	hardreset;
#endif
{
	paddr_t 	mbaddr;		/* Mail box array's paddr. */
	unsigned	stsreg;
	unsigned char	mbibuf[5],
			*mbicmd 	= &mbibuf[0],
			*mbidata	= &mbibuf[1];
	int		retries = 0;

	/***************************************************************
         *  Set up the mailbox command in the buffer.
         */

	mbaddr = vtop(&mb);
	mbicmd[0] = MBINIT;
	mbidata[0] = (sizeof(mb) / (sizeof(mbo_t) + sizeof(mbi_t)));
	mbidata[1] = (( unsigned char *) &mbaddr)[2];
	mbidata[2] = (( unsigned char *) &mbaddr)[1];
	mbidata[3] = (( unsigned char *) &mbaddr)[0];

	do {
		/*******************************************************
                 *  If desired try a soft reset first. The soft reset
                 *  doesn't halt operations in progress on devices on
                 *  the bus so if the soft reset will work then more
                 *  power to it. If the soft reset doesn't cut go back
                 *  to the hard reset.
                 */

		if (hardreset || retries > 0)
			outb(CTRLREG, HRST);
		else
			outb(CTRLREG, SRST);

		/*******************************************************
                 *  Wait for the self-test status to clear in the status
                 *  register. give it 75 clock ticks (750 millisec).
                 */

		stsreg = pollclr(STSREG, STST, 75);
		if (stsreg == TIMEOUT)
			cmn_err(CE_WARN,
				"<hai154x: Host adapter self-test timeout>");

		/*******************************************************
                 *  After the self test clears make sure the diagnostics
                 *  didn't fail.
                 */
		
		else if (stsreg & DIAGF)
			cmn_err(CE_WARN,
				"<hai: Host adapter diagnostic failed>");

		/*******************************************************
                 *  If the diagnostics passed the host adapter should
                 *  now be idle waiting for mailbox initilization.
                 */

		else if ((stsreg & (INIT | IDLE)) != (INIT | IDLE))
			cmn_err(CE_WARN,
				"<hai154x: Host adapter stuck (0x%x)>",
				stsreg);

		/*******************************************************
                 *  Initialize the mailboxes.
                 */

		else if (!aha_icmd(mbicmd, 5, NULL, 0, 0))
			cmn_err(CE_WARN,
				"<hai154x: Mailbox init failed>");

		/*******************************************************
                 *  Lastly, attempt to tune the SCSI Bus.
                 */

		else if (!aha_tunescsibus())
			cmn_err(CE_WARN,
				"<hai154x: Set SCSI bus parameters failed>");

      		/*******************************************************
                 *  Complete success, return.
                 */

		else
			return 1;
		

	} while (++retries < 1);
	return 0;
}	/* aha_hareset() */


#define hardreset()	aha_hareset(1)
#define softreset()	aha_hareset(0)


/***********************************************************************
 *  dmacascade()
 *
 *  Set the selected (hai154x_dma) dma channel to cascade mode.
 */

#if __USE_PROTO__
LOCAL void dmacascade(void)
#else
LOCAL void
dmacascade()
#endif
{
	int dmaporta, dmaportb, s;

	s = sphi();
	if (hai154x_dma == 0) {
		dmaporta = 0x0b;
		dmaportb = 0x0a;
	} else {
		dmaporta = 0xd6;
		dmaportb = 0xd4;
	}
	outb(dmaporta, 0xc0 | (hai154x_dma & 3));
	outb(dmaportb, (hai154x_dma & 3));
	spl(s);
}   /* dmacascade() */

/***********************************************************************
 *  checkmail()
 *
 *  Check the incoming mailboxes for messages.  Do this on any Mail
 *  Box In Full interrupt and before you fail a command for timeout.
 *  The code to determine which target device finished by tid is a
 *  bit tricky and relies on the following assumptions:
 *
 *	  1)  The host adapter is installed in a Intel (big-endian)
 *		  machine. Believe it or not there is (are) non-Intel CPU
 *		  ISA bus machines. and the one that I know of is a M68000
 *		  machine where this would not work.
 *
 *	  2)  The kernel's data space is physically contiguous and is
 *		  never swapped out.
 */

#if __USE_PROTO__
LOCAL int checkmail(void)
#else
LOCAL int
checkmail()
#endif
{
	static int startid = 0;
	int msgs = 0;
	int sts;
	int s;
	register int id;
	register srb_p r;
	register int i = startid;

	do {
		if (mb.i[i].sts != MBIFREE) {
			s = sphi();
			sts = mb.i[i].sts;
			flip(mb.i[i].ccbaddr);
			id = (unsigned) ((mb.i[i].ccbaddr & 0x00ffffffL) - ccbbase) / sizeof(haccb_t);
			if (actv[id]) {
				switch (sts) {
				case MBIABRTFLD:
					cmn_err(CE_WARN,
						"(%d, %d) Command 0x%x abort failed",
						major(actv[id]->dev),
						minor(actv[id]->dev),
						actv[id]->cdb.g0.opcode);
					actv[id]->status = ST_ABRTFAIL;
					break;
				case MBISUCCESS:
				case MBIERROR:
					actv[id]->status = ccb[id].trgtsts;
					break;
				case MBIABORTED:
					actv[id]->status = ST_DRVABRT;
					break;
				default:
					cmn_err(CE_WARN,
						"Host Adapter Mailbox In value corrupted");
					break;
				}

				r = actv[id];
				actv[id] = NULL;
				if (r->cleanup)
					(*(r->cleanup))(r);
				msgs |= bit(id);
			}
			mb.i[i].ccbaddr = 0;
			spl(s);
		}
		else if (msgs)
			break;
		i = (i + 1) & 7;
	} while (i != startid);
	startid = i;

	return msgs;
}   /* checkmail() */

/***********************************************************************
 *  resetdevice()
 *
 *  Reset a SCSI target.
 */

#if __USE_PROTO__
LOCAL void _154x_bdr(register int id)
#else
LOCAL void
_154x_bdr(id)
register int id;
#endif
{
	register haccb_p	c = &(ccb[hai154x_haid]);
	int			tries;

	c->opcode = 0x81;
	c->addrctrl = (id << 5);
	for (tries = 10; tries > 0; --tries) {
		if (!_154x_haft.hf_present)
			_154x_haft.hf_present = softreset();
		if (_154x_haft.hf_present) {
			mb.o[hai154x_haid].cmd = MBOSTART;
			if (puthabyte(STARTSCSI) && (inb(STSREG) & INVDCMD) == 0)
				break;
		}
	}
}   /* _154x_bdr() */

/***********************************************************************
 *  abortscsi()
 *
 *  Abort the SCSI Command at a target device on the bus.
 */

#if __USE_PROTO__
LOCAL void _154x_abort(register srb_p r)
#else
LOCAL void
_154x_abort(r)
register srb_p   r;
#endif
{
	int s,
	    tries;

	s = sphi();
	r->timeout = 0;
	for (tries = 10; _154x_haft.hf_present && tries > 0; --tries) {
		mb.o[r->target].cmd = MBOABORT;
		if (puthabyte(STARTSCSI) && (inb(STSREG) & INVDCMD) == 0)
			break;

		cmn_err(CE_WARN,
			"hai154x: _154x_abort() - Command start failed.");
		_154x_haft.hf_present = softreset();

		/*******************************************************
                 *  This is strange. The soft reset call may have to
                 *  do a hard reset. If it does then the command is
                 *  gone anyhow.  What should happen is the command
                 *  start should work and then the hardware should
                 *  report that It cannot find the command in question.
                 *  [csh]
                 */

	}

	if (tries <= 0)
		cmn_err(CE_WARN,
			"hai154x: _154x_abort() - Cannot reach host adapter.");
	else {
		busyWait(checkmail, 100);

		if (r->status == ST_PENDING) {
			cmn_err(CE_WARN,
				"hai154x: _154x_abort() failed: id %d",
				r->target);
			_154x_bdr(r->target);

			actv[r->target] = NULL;
			r->status = ST_DRVABRT;
			if (r->cleanup)
				(*r->cleanup)(r);
		}
	}

	spl(s);
}   /* _154x_abort() */

/***********************************************************************
 *  Driver Interface Functions
 */

/***********************************************************************
 *  hatimer()
 *
 *  Host adapter Timeout handler.
 */

#if __USE_PROTO__
LOCAL void _154x_timer(void)
#else
LOCAL void 
_154x_timer()
#endif
{
  register int	id;
  register srb_p  r;
  register int	active;
  int			 s;
  
  s = sphi();

  checkmail();		/* Cleanup any missed interrupts, etc. */
  active = 0;

  for (id = 0; id < MAXDEVS; ++id) {
    if (actv[id] != NULL && actv[id]->timeout != 0) {
      if (--actv[id]->timeout == 0) {
	cmn_err(CE_WARN, 
		"(%d, %d) <hai154x: timeout - id: %d lun: %d scmd: (0x%x)>",
		major(actv[id]->dev), minor(actv[id]->dev), actv[id]->target,
		actv[id]->lun, actv[id]->cdb.g0.opcode);

	_154x_abort(actv[id]);

	if (actv[id]) {
	  cmn_err(CE_WARN, "hai154x: No cleanup");
	  actv[id]->status = ST_DRVABRT;
	  r = actv[id];
	  actv[id] = NULL;
	  if (r->cleanup)
	    (*(r->cleanup))(r);
	}
      }
      else
	active = 1;
    }
  }

  drvl[SCSIMAJOR].d_time = active;
  spl(s);
}

/***********************************************************************
 *  haintr()
 *
 *  SCSI interrupt handler for host adapter.
 */

#if __USE_PROTO__
LOCAL void haintr(void)
#else
void
haintr()
#endif
{
  int intrflgs;
  int stsreg;

#if defined(DEBUG)
  int old_cc_flag;
#endif
  
  intrflgs = inb(INTRFLGS);
  
  
  switch (hastate) {
  case ST_HAINIT:		/* Init - Interrupts shouldn't be allowed! */
    if (intrflgs & ANYINTR)
      outb(CTRLREG, IRST);

#if defined(DEBUG)
    cmn_err(CE_WARN, "<hai_154x: unexpected interrupt during init: 0x%x>",
	    intrflgs);
#endif
    
    break;
    
    
  case ST_HASTART:		/* Start - Mailboxes shouldn't be up. */
#if defined(DEBUG)
    old_cc_flag = hainit_ccflag;
#endif

    hainit_stsreg = inb(STSREG);
    hainit_ccflag = ((intrflgs & HACC) != 0);

    if (intrflgs & ANYINTR)
      outb(CTRLREG, IRST);

#if defined(DEBUG)
    if (hainit_ccflag == 0)
      cmn_err(CE_WARN, "<hai154x: Unexpected interrupt during start: 0x%x>",
	      intrflgs);
    if (old_cc_flag && hainit_ccflag)
      cmn_err(CE_WARN, "<hai154x: ccflag before previous completion>");
#endif
    
    break;
    

  default:			/* Normal operation */
    /***************************************************************
     *  If the host adapter command complete flag is set get the
     *  status register (for invalid command flag [INVDCMD]).
     */
    
    if (intrflgs & HACC) {
      stsreg = inb(STSREG);
#if 0
      /*******************************************************
       *  If the state here is "just tried to start a SCSI
       *  command" then that command failed (invalid command)
       *  mark this condition and bail.
       */
#endif		
    }
    
    /***************************************************************
     *  If this is a real interrupt do an Interrupt reset to clear
     *  it.
     */
    
    if (intrflgs & ANYINTR)
      outb(CTRLREG, IRST);
    
    if (intrflgs & MBIF) {
      
      /*******************************************************
       *  During normal operation the only interrupt that
       *  we concern ourselves with is the MBIF (Mailbox
       *  in full) interrupt. This means that a SCSI command
       *  has finished.
       */
      
      checkmail();
    }
    break;
  }
}




#if __USE_PROTO__
static int _154x_init(void)
#else
static int
_154x_init()
#endif /* __USE_PROTO__ */
{
  /*
   * While we cannot assume that interrupts are enabled,
   * we set the interrupt vector just in case.  Under
   * DDI/DKI there is a clear difference between
   * xxinit() and xxstart(), but for the old type drivers there isn't.
   */
  hastate = ST_HAINIT;
  setivec(hai154x_intr, haintr);
  return 0;
}


#if __USE_PROTO__
static int _154x_start_driver(void)
#else
static int
_154x_start_driver()
#endif /* __USE_PROTO__ */
{
  register int i;

  hastate = ST_HASTART;

  /* Do an inquiry to find out which 154x it is (or isn't) */
  i = aha_inquiry();
  
  switch (i) {
  case '\0':
    cmn_err(CE_CONT, "AHA-1540, 16 head BIOS\n");
    break;
    
  case '0':
    cmn_err(CE_CONT, "AHA-1540, 64 head BIOS\n");
    break;
    
  case 'B':
    cmn_err(CE_CONT, "AHA-1640, 64 head BIOS\n");
    break;
    
  case 'C':
    cmn_err(CE_CONT, "AHA-1740A/1742A/1744, standard mode\n");
    break;
    
  case 'A':
    cmn_err(CE_CONT, "AHA-1540B/AHA-1542B\n");
    break;
    
  case 'D':
    cmn_err(CE_CONT, "AHA-1540C/AHA-1542C\n");
    break;
    
  case 'E':
    cmn_err(CE_CONT, "AHA-1540CF/AHA-1542CF\n");
    break;
    
  default:
    cmn_err(CE_NOTE, "Adaptec AHA-154x not found (inq = 0x%x).", i);
    break;
  }


  /* Check to see if the card has an extended BIOS enabled */
  if (aha_checkbios() == 255) {
    cmn_err(CE_CONT, "Extended BIOS detected.\n");
  }

  /* Hard reset the card so we start from a known state */
  if (!(_154x_haft.hf_present = hardreset())) {
    cmn_err(CE_WARN,
	    "hai154x: Initialization failed on AHA-154x at (0x%x)",
	    hai154x_base);
    return 1;
  }
  
  /* Set up the mailbox structures to the initial state */
  for (i = 0; i < MAXDEVS; ++i) {
    actv[i] = NULL;
    mb.o[i].ccbaddr = vtop(ccb + i);
    flip(mb.o[i].ccbaddr);
    mb.o[i].cmd = MBOFREE;
  }
  
  /* Set up cascading DMA for the card. */
  ccbbase = vtop(ccb);
  dmacascade();
  
  /*
   * This call doesn't belong here. This really should be done from 
   * the disk drive init routines. But this is in testing so...
   */
  
  {
    extern int at_drive_ct;
    extern int HAI_DISK;
    int biosnum = at_drive_ct;
    int id;
    
    for (id = 0 ; id < MAXDEVS ; ++id)
      if (HAI_DISK & bit(id))
	loadbiosparms(biosnum++, id);
  }
  
  return 0;
}

/***********************************************************************
 *  _154x_load()
 *
 *  Initialize the host adapter for operation.
 */
#if __USE_PROTO__
LOCAL int _154x_load(void)
#else
LOCAL int
_154x_load()
#endif
{
  /*
   * DDI/DKI specifies separate init and start routines.
   * While this is not a DDI/DKI driver, it should be followed in
   * the case that interrupts are disabled during the init routine.
   */

  /* Call init routine */
  if (_154x_init())
    return 0;

  /* Call start routine */
  if (_154x_start_driver())
    return 0;

  /* Set state to idle.  The load is complete. */
  hastate = ST_HAIDLE;

  return 1;
}


/*
 * _154x_unload() -- Unload routine. For this adapter it's just a stub.
 */

#if __USE_PROTO__
LOCAL void _154x_unload(void)
#else
LOCAL void
_154x_unload()
#endif
{
    return;
}

#if __USE_PROTO__
LOCAL int mkdslist(dsentry_p d, bufaddr_p b)
#else
LOCAL int mkdslist(d, b)
dsentry_p d;
bufaddr_p b;
#endif
{
    int s;
    size_t index;
    static haisgsegm_t seglist[16];
    haisgsegm_p seg = seglist;
    size_t segcount;

    s = sphi();
    switch (b->space & SPACE_MASK) {
    case KRNL_ADDR:
    case USER_ADDR:
	segcount = ukbuftosgl(b->ba_virt, 
			     b->size,
			     seglist,
			     sizeof(seglist) / sizeof(haisgsegm_t));
	break;
    case SYSGLBL_ADDR:
	segcount = sysgbuftosgl(b->ba_phys,
				b->size,
				seglist,
				sizeof(seglist) / sizeof(haisgsegm_t));
	break;
    case PHYS_ADDR:
	spl(s);
	return 0;
    case SGLIST_ADDR:
	seg = b->ba_sglist;
	segcount = b->size;
	break;
    } /* switch */

    for (index = 0; index < segcount; ++index) {
	d[index].size[2] = ((unsigned char *) &(seg[index].sgs_segsize))[0];
	d[index].size[1] = ((unsigned char *) &(seg[index].sgs_segsize))[1];
	d[index].size[0] = ((unsigned char *) &(seg[index].sgs_segsize))[2];
	d[index].addr[2] = ((unsigned char *) &(seg[index].sgs_segstart))[0];
	d[index].addr[1] = ((unsigned char *) &(seg[index].sgs_segstart))[1];
	d[index].addr[0] = ((unsigned char *) &(seg[index].sgs_segstart))[2];
    } /* for */

    spl(s);
    return segcount;
} /* mkdslist() */

/***********************************************************************
 *  startscsi()
 *
 *  Send a SCSI CDB to a target device on the bus.
 */

#if __USE_PROTO__
LOCAL int _154x_start(register srb_p r)
#else
LOCAL int 
_154x_start(r)
register srb_p   r;
#endif
{
	register haccb_p	c;
	paddr_t			 bufaddr;
	size_t			  datalen;
	register int		s;

	if (r->target >= MAXDEVS || r->lun >= MAXUNITS) {
		cmn_err(CE_WARN,
			"hai154x: Illegal device ID: %d LUN: %d",
			r->target,
			r->lun);
		return 0;
	}

	++r->tries;
	if (actv[r->target]) {
		cmn_err(CE_WARN, 
			"(%d, %d) hai154x: Device busy: old opcode (0x%x) new opcode (0x%x)",
			major(r->dev),
			minor(r->dev),
			ccb[r->target].cdb.g0.opcode,
			r->cdb.g0.opcode);
		return 0;
	}

	s = sphi();
	r->status = ST_PENDING;
	c = ccb + r->target;
	memset(c, 0, sizeof(haccb_t));
	c->opcode = 0;			  /* Start SCSI CDB */
	c->addrctrl = (r->target << 5) | (r->lun & 7);
	if (r->xferdir & DMAREAD)  c->addrctrl |= 0x08;
	if (r->xferdir & DMAWRITE) c->addrctrl |= 0x10;
	c->cdblen = cpycdb(&(c->cdb), &(r->cdb));
	c->senselen = 1;

/***********************************************************************
 *  Set up the CCB's Address here. This turned out to be a bit more
 *  complicated than I thought it would be.
 */

	if ((r->buf.space & SPACE_MASK) == PHYS_ADDR) {
		c->datalen[0] = ((unsigned char *) &(r->buf.size))[2];
		c->datalen[1] = ((unsigned char *) &(r->buf.size))[1];
		c->datalen[2] = ((unsigned char *) &(r->buf.size))[0];
		c->bufaddr[0] = ((unsigned char *) &(r->buf.addr.paddr))[2];
		c->bufaddr[1] = ((unsigned char *) &(r->buf.addr.paddr))[1];
		c->bufaddr[2] = ((unsigned char *) &(r->buf.addr.paddr))[0];
	}
	else {
		datalen = mkdslist(ds[r->target].entries, &(r->buf));
		if (datalen == 0) {
			cmn_err(CE_WARN,
				"SCSI ID %d - Bad Scatter/Gather list",
				r->target);
			spl(s);
			return 0;
		}
		else if (datalen == 1)
			memcpy(c->datalen, ds[r->target].entries, 6);
		else {
			c->opcode = 2;
			bufaddr = vtop(ds[r->target].entries);
			datalen *= 6;
			c->datalen[0] = ((unsigned char *) &datalen)[2];
			c->datalen[1] = ((unsigned char *) &datalen)[1];
			c->datalen[2] = ((unsigned char *) &datalen)[0];
			c->bufaddr[0] = ((unsigned char *) &bufaddr)[2];
			c->bufaddr[1] = ((unsigned char *) &bufaddr)[1];
			c->bufaddr[2] = ((unsigned char *) &bufaddr)[0];
		}
	}

	mb.o[r->target].cmd = MBOSTART;

	/***************************************************************
         *  CLEAN THIS UP.
         *  
         *  The _154x_start command will NOT generate an hacc interrupt
         *  if the command bytes and ccb are okay, It will just go
         *  on and do its thing. If however there is a problem _154x_start
         *  will generate a HACC interrupt with invalid command set
         *  high. WE SHOULD TRAP this condition.
         *  
         *  [csh]
         */

	if (puthabyte(STARTSCSI) && (inb(STSREG) & INVDCMD) == 0) {
		actv[r->target] = r;
		if (r->timeout)
			drvl[SCSIMAJOR].d_time = 1;
	}
	else {
		cmn_err(CE_WARN,
			"_154x_start() failed: Resetting host adapter.");
		mb.o[r->target].cmd = MBOFREE;
		actv[r->target] = NULL;
		r->status = ST_DRVABRT;
		if (r->cleanup)
			(*r->cleanup)(r);
		_154x_haft.hf_present = softreset();
		spl(s);
		return 0;
	}

	spl(s);
	return 1;
}   /* _154x_start() */

/*
 * Host adapter function table.
 *
 * Device modules will use this table to find the functions that they
 * need to access the host adapter.
 */

haft_t _154x_haft = {
	0,
	_154x_timer,
	_154x_load,
	_154x_unload,
	_154x_start,
	_154x_abort,
	_154x_bdr
};
