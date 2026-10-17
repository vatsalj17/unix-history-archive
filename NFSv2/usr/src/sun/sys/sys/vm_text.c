/*	@(#)vm_text.c 1.1 85/05/30 SMI; from UCB 4.14 82/12/17	*/

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/buf.h"
#include "../h/seg.h"
#include "../h/vm.h"
#include "../h/cmap.h"
#include "../h/uio.h"
#include "../h/vfs.h"
#include "../h/vnode.h"

/*
 * relinquish use of the shared text segment
 * of a process.
 */
xfree()
{
	register struct text *xp;
	register struct vnode *vp;
	struct vnode *mvp;
	struct vattr vattr;

	if((xp=u.u_procp->p_textp) == NULL)
		return;
	xlock(xp);
	vp = xp->x_vptr;
	VOP_GETATTR(vp, &vattr, u.u_cred);
	if(--xp->x_count==0 && (vattr.va_mode & VSVTX)==0) {
		xunlink(u.u_procp);
		xp->x_rssize -= vmemfree(tptopte(u.u_procp, 0), u.u_tsize);
		if (xp->x_rssize != 0)
			panic("xfree rssize");
		vp->v_flag &= ~VTEXT;
		VN_RELE(vp);
		while (xp->x_poip)
			sleep((caddr_t)&xp->x_poip, PSWP+1);
		vsxfree(xp, (long)xp->x_size);
		xp->x_flag &= ~XLOCK;
		xp->x_vptr = NULL;
	} else {
		xp->x_flag &= ~XLOCK;
		xccdec(xp, u.u_procp);
	}
	u.u_procp->p_textp = NULL;
}

/*
 * Attach to a shared text segment.
 * If there is no shared text, just return.
 * If there is, hook up to it:
 * if it is not currently being used, it has to be read
 * in from the vnode (vp); the written bit is set to force it
 * to be written out as appropriate.
 * If it is being used, but is not currently in core,
 * a swap has to be done to get it back.
 */
xalloc(vp, pagi)
register struct vnode *vp;
{
	register struct text *xp;
	register size_t ts;
	register struct text *xp1;
	struct vattr vattr;

	if(u.u_exdata.ux_tsize == 0)
		return;
again:
	VOP_GETATTR(vp, &vattr, u.u_cred);
	xp1 = NULL;
	for (xp = text; xp < textNTEXT; xp++) {
		if(xp->x_vptr == NULL) {
			if(xp1 == NULL)
				xp1 = xp;
			continue;
		}
		if ((xp->x_count > 0 || (vattr.va_mode & VSVTX)) &&
		    xp->x_vptr == vp) {
			if (xp->x_flag&XLOCK) {
				xwait(xp);
				goto again;
			}
			xlock(xp);
			xp->x_count++;
			u.u_procp->p_textp = xp;
			xlink(u.u_procp);
			xunlock(xp);
			return;
		}
	}
	if((xp=xp1) == NULL) {
		tablefull("text");
		psignal(u.u_procp, SIGKILL);
		return;
	}
	if (xp->x_vptr) {
		goto again;
	}
	xp->x_vptr = vp;
	xp->x_flag = XLOAD|XLOCK;
	if (pagi)
		xp->x_flag |= XPAGV;
	ts = clrnd(btoc(u.u_exdata.ux_tsize));
	xp->x_size = ts;
	if (vsxalloc(xp) == NULL) {
		swkill(u.u_procp, "xalloc");
		return;
	}
	xp->x_count = 1;
	xp->x_ccount = 0;
	xp->x_rssize = 0;
	vp->v_flag |= VTEXT;
	VN_HOLD(vp);
	u.u_procp->p_textp = xp;
	xlink(u.u_procp);
	if (pagi == 0) {
		settprot(RW);
		u.u_procp->p_flag |= SKEEP;
		(void) vn_rdwr(UIO_READ, vp,
			(caddr_t)ctob(tptov(u.u_procp, 0)),
			(int)u.u_exdata.ux_tsize,
			(u.u_exdata.ux_mag == 0413?
			    CLBYTES:
			    sizeof(u.u_exdata)),
			UIOSEG_USER, IO_UNIT, (int *)0);
		u.u_procp->p_flag &= ~SKEEP;
	}
	settprot(RO);
	u.u_segflg = 0;
	xp->x_flag |= XWRIT;
	xp->x_flag &= ~XLOAD;
	xunlock(xp);
}

/*
 * Lock and unlock a text segment from swapping
 */
xlock(xp)
register struct text *xp;
{

	while(xp->x_flag&XLOCK) {
		xp->x_flag |= XWANT;
		sleep((caddr_t)xp, PSWP);
	}
	xp->x_flag |= XLOCK;
}

/*
 * Wait for xp to be unlocked if it is currently locked.
 */
xwait(xp)
register struct text *xp;
{

	xlock(xp);
	xunlock(xp);
}

xunlock(xp)
register struct text *xp;
{

	if (xp->x_flag&XWANT)
		wakeup((caddr_t)xp);
	xp->x_flag &= ~(XLOCK|XWANT);
}

/*
 * Decrement the in-core usage count of a shared text segment.
 * When it drops to zero, free the core space.
 */
xccdec(xp, p)
	register struct text *xp;
	register struct proc *p;
{

	if (xp==NULL || xp->x_ccount==0)
		return;
	xlock(xp);
	if (--xp->x_ccount == 0) {
		if (xp->x_flag & XWRIT) {
			vsswap(p, tptopte(p, 0), CTEXT, 0, xp->x_size, (struct dmap *)0);
			if (xp->x_flag & XPAGV)
				swap(p, xp->x_ptdaddr, (caddr_t)tptopte(p, 0),
				    xp->x_size * sizeof (struct pte),
				    B_WRITE, B_PAGET, swapdev_vp, 0);
			xp->x_flag &= ~XWRIT;
		} else
			xp->x_rssize -= vmemfree(tptopte(p, 0), xp->x_size);
		if (xp->x_rssize != 0)
			panic("text rssize");
	}
	xunlink(p);
	xunlock(xp);
}

/*
 * free the swap image of all unused saved-text text segments
 * which are from virtual filesystem vfsp(used by umount system call).
 */
xumount(vfsp)
	register struct vfs *vfsp;
{
	register struct text *xp;

	for (xp = text; xp < textNTEXT; xp++) 
		if ((xp->x_vptr != NULL) && (xp->x_vptr->v_vfsp == vfsp))
			xuntext(xp);
	mpurgevfs(vfsp);
}

/*
 * remove a shared text segment from the text table, if possible.
 */
xrele(vp)
	register struct vnode *vp;
{
	register struct text *xp;

	if ((vp->v_flag&VTEXT)==0)
		return;
	for (xp = text; xp < textNTEXT; xp++)
		if (vp==xp->x_vptr)
			xuntext(xp);
}

/*
 * remove text image from the text table.
 * the use count must be zero.
 */
xuntext(xp)
	register struct text *xp;
{
	register struct vnode *vp;

	xlock(xp);
	if (xp->x_count) {
		xunlock(xp);
		return;
	}
	vp = xp->x_vptr;
	xp->x_flag &= ~XLOCK;
	xp->x_vptr = NULL;
	vsxfree(xp, (long)xp->x_size);
	vp->v_flag &= ~VTEXT;
	VN_RELE(vp);
}

/*
 * Add a process to those sharing a text segment by
 * getting the page tables and then linking to x_caddr.
 */
xlink(p)
	register struct proc *p;
{
	register struct text *xp = p->p_textp;

	if (xp == 0)
		return;
	vinitpt(p);
	p->p_xlink = xp->x_caddr;
	xp->x_caddr = p;
	xp->x_ccount++;
}

xunlink(p)
	register struct proc *p;
{
	register struct text *xp = p->p_textp;
	register struct proc *q;

	if (xp == 0)
		return;
	if (xp->x_caddr == p) {
		xp->x_caddr = p->p_xlink;
		p->p_xlink = 0;
		return;
	}
	for (q = xp->x_caddr; q->p_xlink; q = q->p_xlink)
		if (q->p_xlink == p) {
			q->p_xlink = p->p_xlink;
			p->p_xlink = 0;
			return;
		}
	panic("lost text");
}

/*
 * Replace p by q in a text incore linked list.
 * Used by vfork(), internally.
 */
xrepl(p, q)
	struct proc *p, *q;
{
	register struct text *xp = q->p_textp;

	if (xp == 0)
		return;
	xunlink(p);
	q->p_xlink = xp->x_caddr;
	xp->x_caddr = q;
}

int xkillcnt = 0;

/*
 * Invalidate the text associated with vp.
 * Purge in core cache of pages associated with vp and kill all active
 * processes.
 */
xinval(vp)
	register struct vnode *vp;
{
	register struct text *xp;
	register struct proc *p;

	mpurge(vp);
	for (xp = text; xp < textNTEXT; xp++) {
		if ((xp->x_flag & XPAGV) && (xp->x_vptr == vp)) {
			for (p = xp->x_caddr; p; p = p->p_xlink) {
				/*
				 * swkill without uprintf
				 */
				printf(
				   "pid %d killed due to text modification\n",
				   p->p_pid);
				psignal(p, SIGKILL);
				p->p_flag |= SULOCK;
				xkillcnt++;
			}
			break;
		}
	}
}
