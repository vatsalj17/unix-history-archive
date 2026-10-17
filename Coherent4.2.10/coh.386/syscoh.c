/* $Header: /ker/coh.386/RCS/syscoh.c,v 2.7 93/10/29 00:55:59 nigel Exp Locker: nigel $ */
/*
 * Functions for the COHERENT-specific system call.
 *
 * $Log:	syscoh.c,v $
 * Revision 2.7  93/10/29  00:55:59  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.6  93/09/02  18:08:41  nigel
 * Nigel's r85, minor edits only
 * 
 * Revision 2.5  93/08/19  03:27:00  nigel
 * Nigel's r83 (Stylistic cleanup)
 */

#include <kernel/proc_lib.h>
#include <sys/errno.h>

#define	_KERNEL		1

#include <kernel/trace.h>
#include <sys/uproc.h>
#include <sys/con.h>
#include <sys/types.h>
#include <sys/cyrix.h>


/*
 * Allow user to write to his own text segment.
 */

static int
cohWtext (dest,src,numBytes)
caddr_t dest;
caddr_t src;
size_t numBytes;
{
	if ((accdata(src, numBytes) || accstack(src, numBytes) ||
	     acctext(src, numBytes) || accShm(src, numBytes)) &&
	     acctext (dest, numBytes)) {
		memcpy (dest, src, numBytes);
		return 0;
	}
	set_user_error (EINVAL);
	return -1;
}


/*
 * Test of shared memory support.
 */

static int
coh_shm(x1, x2, x3, x4)
int x1, x2, x3, x4;
{
	int index, base;

	switch (x1) {
	case 0:
		return shmAlloc(x2);
		break;

	case 1:
		return shmFree(x2);
		break;

	case 2:
		/* Since we are out of args, will use interface
		 * cohcall(COH_SHM, 2, numBytes, base+index, segp)
		 * to call shmAttach, using low bits of base to
		 * carry the index into p_shmsr. */

		base =  x3 & 0xFFFFF000;
		index = x3 & 0x00000FFF;
		if (index >= 0 && index < NSHMSEG) {
			return shmAttach(index, x2, base, x4);
		} else
			SET_U_ERROR(EINVAL, "bad COH shm index");
		break;

	case 3:
		return shmDetach(x2);
		break;

	default:
		SET_U_ERROR(EINVAL, "bad COH shm function");
		break;
	}
	return -1;
}

/*
 * Test of video io map support.
 */

static int
vio(x1, x2, x3, x4)
int x1, x2, x3, x4;
{
	switch (x1) {
	case 0:
		return iomapOr(x2, x3);
		break;

	case 1:
		return iomapAnd(x2, x3);
		break;

	case 2:
		return kiopriv(x2, x3);
		break;

	case 3:
		return mapPhysUser(x2, x3, x4);
		break;

	default:
		SET_U_ERROR(EINVAL, "bad COH vio function");
		break;
	}
	return -1;
}


unsigned int DR0,DR1,DR2,DR3,DR7;

/*
 * Set a kernel breakpoint.
 */

static int
setbp(bp_num, addr, type, len)
unsigned int bp_num, addr, type, len;
{
	/* Range check arguments.
	 * Update RAM images of writeable debug registers.
	 * Call routine (while in RING 1) which will cause GP fault.
	 * GP Fault handler (in RING 0) will copy RAM images to DR's.
	 */
	if (bp_num >= 4 || type >= 4 || len >= 4 || type == 2 || len == 2) {
		SET_U_ERROR(EINVAL, "bad bp setting");
		return -1;
	}

	switch(bp_num) {
	case 0:
		DR0 = addr;
		write_dr0 (DR0);
		DR7 |= (type << 16) | (len << 18) | 0x303;
		break;

	case 1:
		DR1 = addr;
		write_dr1 (DR1);
		DR7 |= (type << 20) | (len << 22) | 0x30C;
		break;

	case 2:
		DR2 = addr;
		write_dr2 (DR2);
		DR7 |= (type << 24) | (len << 26) | 0x330;
		break;

	case 3:
		DR3 = addr;
		write_dr3 (DR3);
		DR7 |= (type << 28) | (len << 30) | 0x3C0;
		break;
	}
	write_dr7 (DR7);
	return 0;
}


/*
 * Clear a kernel breakpoint.
 */

static int
clrbp(bp_num)
unsigned int bp_num;
{
	/* Range check arguments.
	 * Update RAM images of writeable debug registers.
	 * Call routine (while in RING 1) which will cause GP fault.
	 * GP Fault handler (in RING 0) will copy RAM images to DR's.
	 */

	if (bp_num >= 4) {
		SET_U_ERROR(EINVAL, "bad bp # to clear");
		return -1;
	}

	switch(bp_num) {
	case 0:
		DR7 &= ~ 0x3;
		break;

	case 1:
		DR7 &= ~ 0xC;
		break;

	case 2:
		DR7 &= ~ 0x30;
		break;

	case 3:
		DR7 &= ~ 0xC0;
		break;
	}
	if ((DR7 & 0xFF) == 0)
		DR7 &= ~0x300;

	write_dr7 (DR7);
	return 0;
}


/*
 * Only allow this if running as superuser.
 *
 * a1		call type
 * ----------	----------
 * COH_PRINTF	kernel printf
 * COH_DEVLOAD	call load() routine for device with major number a2
 * COH_SETBP	a2=bp#,a3=addr,a4=type,a5=len;  set kernel breakpoint
 * COH_CLRBP	a2=bp#;  clear kernel breakpoint
 * COH_REBOOT	reboot
 * COH_GETINT11 returns hardware equipment word saved at boot time
 * COH_GETCYRIX returns Cyrix CPU id (see cyrix.c for details)
 */

int
ucohcall(a1,a2,a3,a4,a5)
int a1, a2, a3, a4, a5;
{
	int ret = 0;

	if (! super()) {
		SET_U_ERROR (EPERM, "cohcall, must be root");
		return -1;
	}

	switch(a1) {
	case COH_PRINTF:
		printf (a2);
		break;

#if	0
	case COH_DEVLOAD:
		ret = devload (a2);
		break;
#endif
	case COH_SETBP:
		ret = setbp (a2, a3, a4, a5);
		break;

	case COH_CLRBP:
		ret = clrbp (a2);
		break;

	case COH_REBOOT:
		ret = boot ();
		break;

	case COH_VIO:
		ret = vio (a2, a3, a4, a5);
		break;

	case COH_SHM:
		ret = coh_shm (a2, a3, a4, a5);
		break;

	case COH_WTEXT:
		ret = cohWtext (a2, a3, a4);
		break;

	case COH_GETINT11:
		ret = (int11 () & 0x0000FFFF);
		break;

	case COH_GETCYRIX:
		ret = CYRIX_CPU;
		break;

	default:
		SET_U_ERROR (EINVAL, "bad COH function");
	}
	return ret;
}

#if	0
/*
 * Initialize a device.
 */
int
devload(maj_num)
int maj_num;
{
	int ret = -1;
	int mask = 1<<maj_num;

	if (dev_loaded & mask) {
		SET_U_ERROR (EIO, "already loaded");
		return -1;
	}

	if (drvl [maj_num].d_conp == 0) {
		SET_U_ERROR (EIO, "no driver");
		return -1;
	}

	if (drvl [maj_num].d_conp->c_load) {
		(* drvl [maj_num].d_conp->c_load) ();
		dev_loaded |= mask;
		ret = 0;
	}

	return ret;
}

#endif

