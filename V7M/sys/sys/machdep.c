#include "../h/param.h"
#include "../h/systm.h"
#include "../h/acct.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/inode.h"
#include "../h/proc.h"
#include "../h/seg.h"
#include "../h/map.h"
#include "../h/reg.h"
#include "../h/buf.h"

/*
 * Icode is the octal bootstrap
 * program executed in user mode
 * to bring up the system.
 */
int	icode[] =
{
	0104413,	/* sys exec; init; initp */
	0000014,
	0000010,
	0000777,	/* br . */
	0000014,	/* initp: init; 0 */
	0000000,
	0062457,	/* init: </etc/init\0> */
	0061564,
	0064457,
	0064556,
	0000164,
};
int	szicode = sizeof(icode);

/*
 * Machine-dependent startup code
 */
/*** address of mmr3 to enable unibus map ***/
#define	MPCSR ((physadr)0172100)
#define UBMC ((physadr)0172516)
startup()
{
	register i;

	/*
	 * zero and free all of core
	 */

	i = ka6->r[0] + USIZE;
	UISD->r[0] = 077406;
	for(;;) {
		UISA->r[0] = i;
		if(fuibyte((caddr_t)0) < 0)
			break;
		clearseg(i);
		maxmem++;
		mfree(coremap, 1, i);
		i++;
	}
#ifdef	UBUSMAP
	if(ubmaps) {
	for(i=0; i<62; i+=2) {
		UBMAP->r[i] = i<<12;
		UBMAP->r[i+1] = 0;
		}
/* Enable the unibus map here instead of in the boot code.
 * This is done to allow a unibus disk to be the
 * system device. (rl02, rm02/3, rk06/7)
 */
	UBMC->r[0] =| 040;
	}
#endif	UBUSMAP
	printf("\n\nunix/v7m 2.1\n");
	printf("\nmem = %D\n", ctob((long)maxmem));
	if(MAXMEM < maxmem)
		maxmem = MAXMEM;
	mfree(swapmap, nswap, 1);
	swplo--;

	/*
	 * Set last user space address register to
	 * point to unibus I/O space, for use by the sui/fui
	 * calls in some of the following functions, such
	 * as clkstart().
	 */

	UISA->r[7] = ka6->r[1]; /* io segment */
	UISD->r[7] = 077406;
/*
 *	Clear/enable memory parity CSR's
 */
	for(i=0172100; i<0172140; i += 2)
		suiword((caddr_t)i, 1);
}

#ifdef	LCKPHYS
/*
 * set up a physical address
 * into users virtual address space.
 */
sysphys()
{
	register i, s, d;
	register struct a {
		int	segno;
		int	size;
		int	phys;
	} *uap;

	if(!suser())
		return;
	uap = (struct a *)u.u_ap;
	i = uap->segno;
	if(i < 0 || i >= 8)
		goto bad;
	s = uap->size;
	if(s < 0 || s > 128)
		goto bad;
	d = u.u_uisd[i+8];
	if(d != 0 && (d&ABS) == 0)
		goto bad;
	u.u_uisd[i+8] = 0;
	u.u_uisa[i+8] = 0;
	if(!u.u_sep) {
		u.u_uisd[i] = 0;
		u.u_uisa[i] = 0;
	}
	if(s) {
		u.u_uisd[i+8] = ((s-1)<<8) | RW|ABS;
		u.u_uisa[i+8] = uap->phys;
		if(!u.u_sep) {
			u.u_uisa[i] = u.u_uisa[i+8];
			u.u_uisd[i] = u.u_uisd[i+8];
		}
	}
	sureg();
	return;

bad:
	u.u_error = EINVAL;
}
#endif	LCKPHYS

/*
 * Determine which clock is attached, and start it.
 * panic: no clock
 * is printed if no clock can be found.
 *
 */
#define	CLOCK1	((physadr)0177546)
#define	CLOCK2	((physadr)0172540)
clkstart()
{
	lks = CLOCK1;
	if(fuiword((caddr_t)lks) == -1) {
		lks = CLOCK2;
		if(fuiword((caddr_t)lks) == -1)
			panic("no clock");
	}
	lks->r[0] = 0115;
}

/*
 * Let a process handle a signal by simulating an interrupt
 */
sendsig(p, signo)
caddr_t p;
{
	register unsigned n;

	n = u.u_ar0[R6] - 4;
	grow(n);
	suword((caddr_t)n+2, u.u_ar0[RPS]);
	suword((caddr_t)n, u.u_ar0[R7]);
	u.u_ar0[R6] = n;
	u.u_ar0[RPS] &= ~TBIT;
	u.u_ar0[R7] = (int)p;
}

#ifdef	UBUSMAP
/*
 * 11/70 routine to allocate the
 * UNIBUS map and initialize for
 * a unibus device.
 */
int	maplock;

mapalloc(bp)
register struct buf *bp;
{
	register i, a;

	if(!ubmaps)
		return;
	spl6();
	while(maplock&B_BUSY) {
		maplock |= B_WANTED;
		sleep((caddr_t)&maplock, PSWP+1);
	}
	maplock |= B_BUSY;
	spl0();
	bp->b_flags |= B_MAP;
	a = bp->b_xmem;
	for(i=16; i<32; i+=2)
		UBMAP->r[i+1] = a;
	for(a++; i<48; i+=2)
		UBMAP->r[i+1] = a;
	bp->b_xmem = 1;
}

mapfree(bp)
struct buf *bp;
{

	bp->b_flags &= ~B_MAP;
	if(maplock&B_WANTED)
		wakeup((caddr_t)&maplock);
	maplock = 0;
}
#endif	UBUSMAP
