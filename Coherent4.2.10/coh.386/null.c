/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 2.6
 *	Copyright (c) 1982, 1994.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * Null and memory driver.
 *  Minor device 0 is /dev/null
 *  Minor device 1 is /dev/mem, physical memory
 *  Minor device 2 is /dev/kmem, kernel data
 *  Minor device 3 is /dev/cmos
 *  Minor device 4 is /dev/boot_gift
 *  Minor device 5 is /dev/clock
 *  Minor device 6 is /dev/ps
 *  Minor device 7 is /dev/kmemhi, virtual memory 0x8000_0000-0xFFFF_FFFF
 *  Minor device 11 is /dev/idle
 *  Minor device 12 is /dev/freemem
 *
 * Revision 2.7  94/03/08  20:35:15  udo
 * Cleaned up and /dev/freemem added
 * 
 * $Log:	null.c,v $
 * Revision 2.6  93/10/29  00:55:25  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.5  93/09/13  07:58:47  nigel
 * Updated to reflect the fact that most driver entry points are 'void' again.
 * 
 * Revision 2.4  93/08/19  03:26:39  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Revision 2.2  93/07/26  14:28:57  nigel
 * Nigel's R80
 * 
 * Revision 1.7  93/04/14  10:06:37  root
 * r75
 * 
 * Revision 1.10  93/03/02  08:16:25  bin
 * kernel 73 update
 * 
 * Revision 1.6  92/11/09  17:10:54  root
 * Just before adding vio segs.
 * 
 * Revision 1.2  92/01/06  11:59:49  hal
 * Compile with cc.mwc.
 * 
 * Revision 1.1	88/03/24  16:14:04	src
 * Initial revision
 */

#include <kernel/ddi_cpu.h>
#include <kernel/proc_lib.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/cred.h>
#include <sys/types.h>
#include <stddef.h>

#define	_KERNEL		1

#include <kernel/typed.h>
#include <sys/con.h>
#include <sys/inode.h>
#include <sys/seg.h>
#include <sys/coh_ps.h>
#include <sys/io.h>
#include <sys/proc.h>
#include <sys/uproc.h>
#include <sys/mmu.h>

#include <sys/null.h>

/*
 * The symbol "DANGEROUS" should be undefined for a production system.
 */
#ifdef TRACER
#define KMEM_IOCTL	/* Allow ioctl()s for /dev/kmem.  */
#define DANGEROUS	/* Allow dangerous ioctl()s for /dev/null.  */
#endif

unsigned char 	read_cmos	__PROTO ((unsigned offset));
void	 	write_cmos 	__PROTO ((unsigned offset,
					  unsigned char value));

/* These are minor numbers. */
#define DEV_NULL	0	/* /dev/null	*/
#define DEV_MEM		1	/* /dev/mem	*/
#define DEV_KMEM	2	/* /dev/kmem	*/
#define DEV_CMOS	3	/* /dev/cmos	*/
#define DEV_BOOTGIFT	4	/* /dev/bootgift*/
#define DEV_CLOCK	5	/* /dev/clock	*/
#define DEV_PS		6	/* /dev/ps	*/
#define DEV_KMEMHI	7	/* /dev/kmemhi	*/
#define DEV_IDLE	11	/* /dev/idle	*/
#define DEV_FMEM	12	/* /dev/freemem	*/

#define KMEMHI_BASE	0x80000000
#define PXCOPY_LIM	4096

/*
 * CMOS devices are limited by an 8 bit address.
 */
#define MAX_CMOS	255
#define CMOS_LEN	256

/*
 * The first 14 bytes of the CMOS are the clock.
 */
#define MAX_CLOCK	13
#define CLOCK_LEN	14

/*
 * These are definitions for mucking with the CMOS clock.
 */
#define SRA	10	/* Status Register A */
#define SRB	11	/* Status Register B */
#define SRC	12	/* Status Register C */
#define SRD	13	/* Status Register D */

#define UIP	0x80	/* Update In Progress bit of SRA.	*/
#define NO_UPD	0x80	/* No Update bit of SRB.		*/


/*
 * int lock_clock() -- Stop the update cycle on the CMOS RT clock and
 * wait for it to settle.  Returns 0 if the clock would not settle
 * in time.
 */
static int
lock_clock()
{
	register int i;

	/*
	 * Wait for the clock to settle.  If it does not settle in
	 * a reasonable amount of time, give up.
	 */

	i = 65536;	/* Loop for a longish time.  */
	while (-- i > 0) {
		if (0 == (UIP & read_cmos (SRA))) {
			break;	/* Break if there is no update in progress.  */
		}
	}
	
	if (0 == i) {
		/* The clock would not settle.  */
		return 0;
	}

	/*
	 * There is a tiny race here--an interrupt could conceivably
	 * come here, thus allowing enough delay for another update to
	 * begin.  But if we take interrupts that take a full second
	 * to process, other things are going to break horribly.
	 */
	
	/*
	 * Lock out updates.
	 * We set the No Updates bit in Clock Status Register B.
	 */
	write_cmos (SRB, read_cmos (SRB) | NO_UPD);
	return 1;
}


/*
 * void unlock_clock() -- Restart the update cycle on the CMOS RT clock.
 */ 
static void
unlock_clock()
{
	/*
	 * We clear the No Updates bit in Clock Status Register B.
	 */
	write_cmos (SRB, read_cmos(SRB) & ~ NO_UPD);
}


/*
 * Null/memory open routine.
 */
static void
nlopen(dev, mode)
dev_t dev;
int mode;
{
	switch (minor(dev)) {
	case DEV_PS:
		/* /dev/ps is read only */
		if (IPR != (IPR & mode)) 
			set_user_error (EACCES);
		break;

	default:
		/*
		 * For other minor devices there is
		 * usually no action for open().
		 */
		break;
	}
	return;
}


/*
 * Null/memory close routine.
 */
static void
nlclose(dev, mode)
dev_t dev;
int mode;
{
}


/*
 * Null/memory read routine.
 */
static void
nlread (dev, iop)
dev_t dev;
IO *iop;
{
	register unsigned 	bytesRead;
	register PROC 		*pp1;		/* */
	char			psBuf[ARGSZ];	/* buffer for command line
						 * arguments for ps. */
	stMonitor		psData;		/* All process data for */
	unsigned int 		seek;
	extern typed_space 	boot_gift;

	switch (minor (dev)) {
	case DEV_NULL:
		/*
		 * Read nothing.
		 * Do NOT update iop->io_ioc.
		 * This way, caller knows 0 bytes were read.
		 */
		break;

	case DEV_MEM: {
		int src = iop->io_seek;
		int dest = iop->io.pbase;

		while (iop->io_ioc) {
			int numBytes = PXCOPY_LIM;
			if (numBytes > iop->io_ioc)
				numBytes = iop->io_ioc;

			bytesRead = pxcopy (src, dest, numBytes, SEL_386_UD);
			src += bytesRead;
			dest += bytesRead;
			iop->io_ioc -= bytesRead;
			if (get_user_error () == EFAULT) {
				set_user_error (0);
				break;
			}
		}
		break;
	}

	case DEV_KMEM:
		iowrite (iop, (caddr_t) iop->io_seek, iop->io_ioc);
		if (get_user_error () == EFAULT)
			set_user_error (0);
		break;

	case DEV_CLOCK:
		/*
		 * Don't go past the end of the CLOCK.
		 */
		if (iop->io_seek >= CLOCK_LEN)
			break;

		/*
		 * Lock the clock before any reading.
		 */

		if (lock_clock () == 0) {
			set_user_error (EIO);
			break;
		}

		/*
		 * Read the requested data out of the CMOS.
		 */
		for (seek = iop->io_seek; seek < CLOCK_LEN ; seek++) {
			if (ioputc (read_cmos (seek), iop) == -1)
				break;
		}

		/*
		 * Now that we are done reading the CMOS, let
		 * the clock loose.
		 */
		unlock_clock ();
		break;

	case DEV_CMOS:
		/*
		 * Don't go past the end of the CMOS.
		 */
		if (iop->io_seek >= CMOS_LEN)
			break;

		/*
		 * Read the requested data out of the CMOS.
		 */
		for (seek = iop->io_seek; seek < CMOS_LEN; seek ++) {
			if (ioputc (read_cmos (seek), iop) == -1)
				break;
		}
		break;

	case DEV_BOOTGIFT:
		/*
		 * Reads all from the data structure boot_gift.
		 */
		if (iop->io_seek < BG_LEN) {
			bytesRead = iop->io_ioc;

			/*
			 * Copy no more than to the end of boot_gift.
			 */
			if (iop->io_seek + bytesRead > BG_LEN)
				bytesRead = BG_LEN - iop->io_seek;

			iowrite (iop, (char *) & boot_gift + iop->io_seek,
				 bytesRead);
		}
		break;

	case DEV_PS:
		__GLOBAL_LOCK_PROCESS_TABLE ("nlread ()");

		/* Main driver loop. Go through all processes. Fill struct PS
		 * and send put to user buffer.
		 */
		for (pp1 = & procq; (pp1 = pp1->p_nforw) != & procq; ) {
			int		i;	/* loop index */
			unsigned	uLen; 	/* Process size */
			unsigned	uLenR;	/* Real process size */

			/* Check if driver can send next proc data */ 
			if (iop->io_ioc < sizeof (stMonitor)) 
				break;
				
			/* Calculate the size of process. */
			uLen = uLenR = 0;
			for (i = 0 ; i < NUSEG ; i++) {
				SEG	      *	sp;

				if ((sp = pp1->p_segl [i].sr_segp) == NULL)
					continue;
				uLenR += sp->s_size;
				if (i == SIUSERP /* || i == SIAUXIL */)
					continue;
				uLen += sp->s_size;
			} 

			memcpy (psData.u_comm, pp1->p_comm, ARGSZ);
			if (pp1->p_sleep == NULL)
				psData.u_sleep [0] = 0;
			else
				strncpy (psData.u_sleep, pp1->p_sleep,
					 sizeof (psData.u_sleep));

			/* fill up stMonitor */
			psData.p_pid = pp1->p_pid;
			psData.p_ppid = pp1->p_ppid;
			psData.p_uid = pp1->p_credp->cr_uid;
			psData.p_ruid = pp1->p_credp->cr_ruid;
#if 1
			psData.p_rgid = pp1->p_group;
#else
			psData.p_rgid = pp1->p_credp->cr_rgid;
#endif
			psData.p_state = pp1->p_state;
			psData.p_flags = pp1->p_flags;
			psData.rrun = (char *) pp1 != pp1->p_event;
			psData.p_event = pp1->p_event;
			psData.p_ttdev = pp1->p_ttdev;
			psData.p_nice = pp1->p_nice;
			psData.size = (short) (uLen >> 10);
			psData.rsize = (short) (uLenR >> 10);
			psData.p_schedPri = pp1->p_schedPri;
			psData.p_utime = pp1->p_utime;
			psData.p_stime = pp1->p_stime;

			memcpy (psData.pr_argv, psBuf, ARGSZ);

			/* send data to user */
			iowrite (iop, (char *) & psData, sizeof (stMonitor));
		}
		__GLOBAL_UNLOCK_PROCESS_TABLE ();
		break;

	case DEV_KMEMHI:
		iowrite (iop, (caddr_t) iop->io_seek - KMEMHI_BASE,
			 iop->io_ioc);
		if (get_user_error () == EFAULT)
			set_user_error (0);
		break;

	default:
		set_user_error (ENXIO);
	}
}


/*
 * Null/memory write routine.
 */
static void
nlwrite(dev, iop)
dev_t dev;
IO *iop;
{
	unsigned bytesWrit;
	unsigned seek;
	int	ch;

	switch (minor (dev)) {
	case DEV_NULL:
		/*
		 * Tell caller all bytes were written.
		 */
		iop->io_ioc = 0;
		break;

	case DEV_MEM:
		while (iop->io_ioc) {
			int src = iop->io.pbase;
			int dest = iop->io_seek;
			int numBytes = PXCOPY_LIM;
			if (numBytes > iop->io_ioc)
				numBytes = iop->io_ioc;

			bytesWrit = xpcopy (src, dest, numBytes, SEL_386_UD);
			src += bytesWrit;
			dest += bytesWrit;
			iop->io_ioc -= bytesWrit;
			if (get_user_error () == EFAULT) {
				set_user_error (0);
				break;
			}
		}
		break;

	case DEV_KMEM:
		ioread (iop, (caddr_t) iop->io_seek, iop->io_ioc);
		break;

	case DEV_CLOCK:
		/*
		 * Don't go past the end of the CLOCK.
		 */
		if (iop->io_seek >= CLOCK_LEN)
			break;

		/*
		 * Lock the clock before any writing.
		 */
		if (lock_clock () == 0) {
			set_user_error (EIO);
			break;
		}

		/*
		 * Write the requested data into the CMOS.
		 */

		for (seek = iop->io_seek ; seek < CLOCK_LEN ; seek ++) {
			if ((ch = iogetc (iop)) == -1)
				break;
			write_cmos (seek, ch);
		}

		/*
		 * Now that we are done writing the CMOS, let
		 * the clock loose.
		 */
		unlock_clock ();
		break;

	case DEV_CMOS:
		/*
		 * Don't go past the end of the CMOS.
		 */
		if (iop->io_seek >= CMOS_LEN)
			break;

		/*
		 * Write the requested data into the CMOS.
		 */
		for (seek = iop->io_seek ; seek < CMOS_LEN ; seek ++) {
			if ((ch = iogetc (iop)) == -1)
				break;
			write_cmos (seek, ch);
		}
		break;

	case DEV_KMEMHI:
		ioread (iop, (caddr_t) iop->io_seek - KMEMHI_BASE, iop->io_ioc);
		break;

	default:
		set_user_error (ENXIO);
	}
}

#ifdef DANGEROUS /* Includes all of docall().  */
/*
 * MASSIVE SECURITY HOLE!  This should NOT be included in a distribution
 * system.  Among other problems, it becomes possible to do "setuid(0)".
 *
 * Call a function with arguments.
 *
 * Takes an array of unsigned ints.  The first element is the length of
 * the whole array, the second element is a pointer to the function to
 * call, all other elements are arguments.  At most 5 arguments may be
 * passed.
 *
 * Returns the return value of the called fuction in uvec[0].
 */
static int
docall(uvec)
unsigned uvec [];
{
	int (* func)();
	unsigned kvec[7];
	int retval;

	printf("NLCALL security hole.\n");

	/* Fetch the first element of vec.  */
	ukcopy (uvec, kvec, sizeof (unsigned));

	if (kvec [0] < 2 || kvec[0] > 7) {
		/* Invalid number of elements in uvec.  */
		set_user_error (EINVAL);
		return -1;
	}
	
	/* Fetch the whole vector.  */
	ukcopy (uvec, kvec, kvec [0] * sizeof (unsigned));

	/* Extract the function.  */
	func = (int (*)()) kvec [1];

	/* Call the function with all arguments.  */
	retval = (* func) (kvec [2], kvec [3], kvec [4], kvec [5], kvec [6]);

	kucopy (& retval, uvec, sizeof (unsigned));
	return retval;
}
#endif /* DANGEROUS */


/*
 * Do an ioctl call for /dev/null.
 */
static void
nlioctl(dev, cmd, vec, mode, credp, rvalp)
dev_t	  dev;
int	  cmd;
char	* vec;
int	  mode;
cred_t	* credp;
int	* rvalp;
{
	switch (minor (dev)) {
#ifdef KMEM_IOCTL
	case DEV_KMEM:
		switch (cmd) {
#ifdef DANGEROUS
		case NLCALL:	/* Call a function.  */
			* rvalp = docall (vec);
			break;
#endif /* DANGEROUS */
		default:
			set_user_error (EINVAL);
			break;
		}
#endif /* KMEM_IOCTL */

	case DEV_IDLE: {
		/*
		 * Write into a two-word user struct at "vec" -
		 *
		 * Low address gets number of ticks for which system was
		 * idle, since boot.
		 *
		 * High address gets total number of ticks since boot.
		 *
		 * By looking at time differences of these, we can estimate
		 * CPU load factor - this is how xload does it.
		 */

		IDLESYS *mem = (IDLESYS *) vec;

		if (cmd != NLIDLE) { 
			set_user_error (EINVAL);
			break;
		}

		putuwd ((char *)&mem->idle_ticks, ddi_cpu_data()->dc_idle_ticks);
		putuwd ((char *)&mem->total_ticks, lbolt);
		break;
	}

	case DEV_FMEM: {
		/*
		 * Compute amount of memory left for user processes
		 * and amount of free memory and return them in a
		 * two-word structure. Both values are kbytes.
		 */

		FREEMEM *mem = (FREEMEM *) vec;

		if (cmd != NLFREE) { 
			set_user_error (EINVAL);
			break;
		}

		putuwd ((char *)&mem->avail_mem, (sysmem.efree - sysmem.tfree) << 2);
		putuwd ((char *)&mem->free_mem, (sysmem.pfree - sysmem.tfree) << 2);
		break;
	}

	default:
		set_user_error (EINVAL);
		break;
	}

}


/*
 * Configuration table.
 */
CON nlcon ={
	DFCHR,				/* Flags */
	0,				/* Major index */
	nlopen,				/* Open */
	nlclose,			/* Close */
	NULL,				/* Block */
	nlread,				/* Read */
	nlwrite,			/* Write */
	nlioctl,			/* Ioctl */
	NULL,				/* Powerfail */
	NULL,				/* Timeout */
	NULL,				/* Load */
	NULL				/* Unload */
};
