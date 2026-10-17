#
/*
 */

#include "../defines.h"
#include "../param.h"
#ifdef	AUSAML
#include	"../lnode.h"
#endif	AUSAML
#include "../systm.h"
#include "../file.h"
#include "../user.h"
#include "../proc.h"
#include "../text.h"
#include "../inode.h"
#include "../buf.h"

/*
 * Swap out process p.
 * The ff flag causes its core to be freed--
 * it may be off when called to create an image for a
 * child process in newproc.
 * Os is the old size of the data area of the process,
 * and is supplied during core expansion swaps.
 *
 * panic: out of swap space
 * panic: swap error -- IO error
 */
xswap(p, ff, os)
int *p;
{
	register *rp, a;

	rp = p;
	if(os == 0)
		os = rp->p_size;
	a = malloc(swapmap, (rp->p_size+7) >> 3);
	if(a == NULL)
		panic("out of swap space");
#ifdef LOWER_TEXT_SWAPS
#ifdef	SHARED_DATA
	xccdec(rp, 0);
#endif
#ifndef	SHARED_DATA
	xccdec(rp->p_textp, 0);
#endif
#endif

#ifndef LOWER_TEXT_SWAPS
#ifdef	SHARED_DATA
	xccdec(rp);
#endif
#ifndef	SHARED_DATA
	xccdec(rp->p_textp);
#endif
#endif
	rp->p_flag =| SLOCK;
	swap(a, rp->p_addr, os, B_WRITE);
	if(ff)
		mfree(coremap, os, rp->p_addr);
	rp->p_addr = a;
	rp->p_flag =& ~(SLOAD|SLOCK);
	rp->p_time = 0;
	if(runout) {
		runout = 0;
		setrun( &proc[0] );	/* no need for wakeup */
	}
}

/*
 * relinquish use of the shared text segment
 * of a process.
 */
xfree()
{
	register *xp, *ip;

	if((xp=u.u_procp->p_textp) != NULL) {
#ifdef LOWER_TEXT_SWAPS | SHARED_DATA
		xccdec( u.u_procp , xp->x_count == 1 );
#else
		xccdec( xp );
#endif
		u.u_procp->p_textp = NULL;
		if(--xp->x_count == 0) {
			ip = xp->x_iptr;
			if((ip->i_mode&ISVTX) == 0) {
				xp->x_iptr = NULL;
#ifdef LOWER_TEXT_SWAPS
				if(xp->x_daddr)
#endif
					mfree(swapmap, (xp->x_size+7) >> 3, xp->x_daddr);
				ip->i_flag =& ~ITEXT;
				if(ip->i_flag&ILOCK)			/* fix025 */
					ip->i_count--;			/* fix025 */
				else					/* fix025 */
					iput(ip);			/* fix025 */
			}
		}
	}
}

#ifdef LOWER_TEXT_SWAPS
/*
 * Attach to a shared text segment.
 * If there is no shared text, just return.
 * If there is, hook up to it:
 * if it is not currently being used, it has to be read
 * in from the inode (ip).
 * If it is being used, but is not currently in core,
 * a swap has to be done to get it back.
 *
 * Modified and extensively rewritten to do away with
 * all the unnecessary swaps. Expands are called to fit
 * text, and the text mysteriously appears at the end of the proc.
 * The swap to establish the text on the disk is delayed
 * until it is required ( in xccdec) to remove overhead from
 * semi-lightly used texts....
 */
#ifdef	SHARED_DATA
xalloc(ip, spid)
int *ip;
int *spid;
#endif
#ifndef	SHARED_DATA
xalloc(ip)
int *ip;
#endif
{
	register struct text *xp;
	register *rp, ts;

	if(u.u_arg[1] == 0)
		return;
	rp = NULL;
	for(xp = &text[0]; xp < &text[NTEXT]; xp++)
		if(xp->x_iptr == NULL) {
			if(rp == NULL)
				rp = xp;
		} else
#ifdef	SHARED_DATA
			if( (xp->x_iptr == ip) || (xp->x_spid == spid) ) {
#endif
#ifndef	SHARED_DATA
			if(xp->x_iptr == ip) {
#endif
				xp->x_count++;
				rp = u.u_procp;
				xlock(xp);
				rp->p_textp = xp;
				if( xp->x_ccount != 0)
				{
					xp->x_ccount++;
					return;
				}
				ts = xp->x_size;
				if( (xp->x_caddr = malloc(coremap, ts)) == NULL)
				{
					savu(u.u_rsav);
					savu(u.u_ssav);
					xswap(rp, 1, 0);
					rp->p_flag =| SSWAP;
#ifdef	PROCESS_QUEUES
					qswtch();	/* no return */
#else
					swtch();	/* no return */
#endif	PROCESS_QUEUES
				}
				else
				{
					xp->x_flag =| TXTBUSY;
					rp->p_flag =| SLOCK;
					swap( xp->x_daddr, xp->x_caddr, ts, B_READ);
					rp->p_flag =& ~SLOCK;
					goto out;
				}
			}
	if((xp=rp) == NULL)
		panic("out of text");
	xp->x_count = 1;
	xp->x_ccount = 0;
	xp->x_iptr = ip;
#ifdef	SHARED_DATA
	xp->x_spid = spid;
#endif
	ts = ((u.u_arg[1]+63)>>6) & 01777;
	xp->x_size = ts;
	xp->x_daddr = 0;		/* text is not saved on disk yet */
	xp->x_flag =| TXTBUSY;
	expand(USIZE + ts);
	estabur(0, ts, 0, 0);
	u.u_count = u.u_arg[1];
	u.u_offset.loint = 020; /* fix000 */
	u.u_base = 0;
	rp = ip;
	readi(rp);
	rp->i_flag =| ITEXT;
	rp->i_count++;
	rp = u.u_procp;
	rp->p_textp = xp;
	rp->p_size = USIZE;		/* process and text now resident */
	xp->x_caddr = rp->p_addr + USIZE;
out:
	xrele(xp);
	xp->x_ccount = 1;
}
#endif




#ifndef LOWER_TEXT_SWAPS
/*
 * Attach to a shared text segment.
 * If there is no shared text, just return.
 * If there is, hook up to it:
 * if it is not currently being used, it has to be read
 * in from the inode (ip) and established in the swap space.
 * If it is being used, but is not currently in core,
 * a swap has to be done to get it back.
 * The full coroutine glory has to be invoked--
 * see slp.c-- because if the calling process
 * is misplaced in core the text image might not fit.
 * Quite possibly the code after "out:" could check to
 * see if the text does fit and simply swap it in.
 *
 * panic: out of swap space
 */
#ifdef	SHARED_DATA
xalloc(ip, spid)
int *ip;
int *spid;
#endif
#ifndef	SHARED_DATA
xalloc(ip)
int *ip;
#endif
{
	register struct text *xp;
	register *rp, ts;

	if(u.u_arg[1] == 0)
		return;
	rp = NULL;
	for(xp = &text[0]; xp < &text[NTEXT]; xp++)
		if(xp->x_iptr == NULL) {
			if(rp == NULL)
				rp = xp;
		} else
#ifdef	SHARED_DATA
			if( (xp->x_iptr == ip) || (xp->x_spid == spid) ) {
#endif
#ifndef	SHARED_DATA
			if(xp->x_iptr == ip) {
#endif
				xp->x_count++;
				u.u_procp->p_textp = xp;
#ifdef SHARED_DATA
				xlock(xp);
#endif
				goto out;
			}
	if((xp=rp) == NULL)
		panic("out of text");
	xp->x_count = 1;
	xp->x_ccount = 0;
	xp->x_iptr = ip;
#ifdef	SHARED_DATA
	xp->x_spid = spid;
	xp->x_flag =| TXTBUSY;
#endif
	ts = ((u.u_arg[1]+63)>>6) & 01777;
	xp->x_size = ts;
	if((xp->x_daddr = malloc(swapmap, (ts+7) >> 3)) == NULL)
		panic("out of swap space");
	expand(USIZE+ts);
	estabur(0, ts, 0, 0);
	u.u_count = u.u_arg[1];
	u.u_offset.loint = 020; /* fix000 */
	u.u_base = 0;
	readi(ip);
	rp = u.u_procp;
	rp->p_flag =| SLOCK;
	swap(xp->x_daddr, rp->p_addr+USIZE, ts, B_WRITE);
	rp->p_flag =& ~SLOCK;
	rp->p_textp = xp;
	rp = ip;
	rp->i_flag =| ITEXT;
	rp->i_count++;
	expand(USIZE);

#ifdef SHARED_DATA
	xrele(xp);
#endif
out:
	if(xp->x_ccount == 0) {
		savu(u.u_rsav);
		savu(u.u_ssav);
		xswap(u.u_procp, 1, 0);
		u.u_procp->p_flag =| SSWAP;
#ifdef	PROCESS_QUEUES
		qswtch();
#else
		swtch();
#endif	PROCESS_QUEUES
		/* no return */
	}
	xp->x_ccount++;
}
#endif

#ifndef	SHARED_DATA | LOWER_TEXT_SWAPS
/*
 * Decrement the in-core usage count of a shared text segment.
 * When it drops to zero, free the core space.
 */
xccdec(rp)
register *rp;  /* fix000 */
{
	if(rp!=NULL && rp->x_ccount!=0)
		if(--rp->x_ccount == 0)
			mfree(coremap, rp->x_size, rp->x_caddr);
}
#endif


#ifdef LOWER_TEXT_SWAPS | SHARED_DATA
/*
 * Decrement the in-core usage count of a shared text segment.
 * When it drops to zero, free the core space.
 * If the text is not established on the disk
 * and its not about to be thrown away then alloc space for
 * it and swap it out.
 * Shared datas cannot be abandoned in core and must be swapped also...
 * Synchronization is via the TXTBUSY flag.
 */
#ifdef LOWER_TEXT_SWAPS
xccdec(rp, aban)
#endif
#ifndef LOWER_TEXT_SWAPS
xccdec(rp)
#endif
register *rp;
{
	register *xp;



	if( (xp = rp->p_textp) != NULL && xp->x_ccount != 0)
	{
		xlock(xp);
		if( xp->x_ccount == 1)
		{		/* should we swap?? */
			if(
#ifdef LOWER_TEXT_SWAPS
			     (
				!aban ||		/* last usage */
				(xp->x_iptr->i_mode & ISVTX)	/* but not stick bit */
			     ) &&
			     (
#endif
#ifdef SHARED_DATA
				(xp->x_iptr != xp->x_spid)	/* shared data */
#endif
#ifdef SHARED_DATA & LOWER_TEXT_SWAPS
				||
#endif
#ifdef LOWER_TEXT_SWAPS
				(xp->x_daddr == 0)	/* not already saved */
			     )
#endif
			  )
			{
#ifdef LOWER_TEXT_SWAPS
				if( (xp->x_daddr == 0) &&
				    ((xp->x_daddr = malloc(swapmap, (xp->x_size + 7) >> 3)) == NULL) )
					panic("out of swap space");
#endif
				rp->p_flag =| SLOCK;
				xp->x_flag =| TXTBUSY;
				swap(xp->x_daddr, xp->x_caddr, xp->x_size, B_WRITE);
				rp->p_flag =& ~SLOCK;
				xrele(xp);
			}
		}
		if( --xp->x_ccount == 0)
			mfree( coremap, xp->x_size, xp->x_caddr);
	}

}


/*
 * xlock
 *
 * lock a text area to reserve it for private
 * use. texts must be locked whilst swapping etc.
 */
xlock(xp)
register struct text *xp;
{
	while( xp->x_flag & TXTBUSY)
	{
		xp->x_flag =| TXTWANT;
		sleep( xp, PSWP + 1);
	}
}

/*
 * xrele
 *
 * release exclusive use of a text area and
 * wakeup anyone who wants it
 */
xrele(xp)
register struct text *xp;
{
	if( xp->x_flag & TXTWANT)
		wakeup(xp);
	xp->x_flag =& ~( TXTWANT | TXTBUSY);
}
#endif
