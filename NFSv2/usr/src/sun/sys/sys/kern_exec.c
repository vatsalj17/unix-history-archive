/*	@(#)kern_exec.c 1.1 85/05/30 SMI; from UCB 4.5 83/07/01	*/

#include "../machine/reg.h"
#include "../machine/pte.h"
#include "../machine/psl.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/user.h"
#include "../h/kernel.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/socketvar.h"
#include "../h/vnode.h"
#include "../h/pathname.h"
#include "../h/seg.h"
#include "../h/vm.h"
#include "../h/text.h"
#include "../h/file.h"
#include "../h/uio.h"
#include "../h/acct.h"

#ifdef vax
#include "../vax/mtpr.h"
#endif

/*
 * texts below this size will be read in (if there is enough free memory)
 * even though the file is 413.
 */
size_t pgthresh = clrnd(btoc(PGTHRESH));

/*
 * exec system call, with and without environments.
 */
struct execa {
	char	*fname;
	char	**argp;
	char	**envp;
};

execv()
{
	((struct execa *)u.u_ap)->envp = NULL;
	execve();
}

execve()
{
	register nc;
	register char *cp;
	register struct buf *bp;
	register struct execa *uap;
	int na, ne, ucp, ap, c;
	int indir, uid, gid;
	char *sharg;
	char *execnamep;
	struct vnode *vp;
	struct vattr vattr;
	daddr_t bno;
	struct pathname pn;
	char cfarg[SHSIZE];
	int resid;

	uap = (struct execa *)u.u_ap;
	u.u_error = pn_get(uap->fname, UIOSEG_USER, &pn);
	if (u.u_error)
		return;
	u.u_error = lookuppn(&pn, FOLLOW_LINK, (struct vnode **)0, &vp);
	if (u.u_error) {
		pn_free(&pn);
		return;
	}
	bno = 0;
	bp = 0;
	indir = 0;
	uid = u.u_uid;
	gid = u.u_gid;
	if (u.u_error = VOP_GETATTR(vp, &vattr, u.u_cred))
		goto bad;
	if (vattr.va_mode & VSUID)
		uid = vattr.va_uid;
	if (vattr.va_mode & VSGID)
		gid = vattr.va_gid;

  again:
	/*
	 * XXX should change VOP_ACCESS to not let super user always have it
	 * for exec permission on regular files.
	 */
	if (u.u_error = VOP_ACCESS(vp, VEXEC, u.u_cred))
		goto bad;
	if ((u.u_procp->p_flag&STRC)
	    && (u.u_error = VOP_ACCESS(vp, VREAD, u.u_cred)))
		goto bad;
	if (vp->v_type != VREG ||
	   (vattr.va_mode & (VEXEC|(VEXEC>>3)|(VEXEC>>6))) == 0) {
		u.u_error = EACCES;
		goto bad;
	}

	/*
	 * Read in first few bytes of file for segment sizes, ux_mag:
	 *	407 = plain executable
	 *	410 = RO text
	 *	413 = demand paged RO text
	 * Also an ASCII line beginning with #! is
	 * the file name of a ``shell'' and arguments may be prepended
	 * to the argument list if given here.
	 *
	 * SHELL NAMES ARE LIMITED IN LENGTH.
	 *
	 * ONLY ONE ARGUMENT MAY BE PASSED TO THE SHELL FROM
	 * THE ASCII LINE.
	 */
	u.u_exdata.ux_shell[0] = 0;	/* for zero length files */
	u.u_error =
	    vn_rdwr(UIO_READ, vp, (caddr_t)&u.u_exdata, sizeof (u.u_exdata),
		0, UIOSEG_KERNEL, IO_UNIT, &resid);
	if (u.u_error)
		goto bad;
	u.u_count = resid;
#ifndef lint
	if (u.u_count > sizeof(u.u_exdata) - sizeof(u.u_exdata.Ux_A) &&
	    u.u_exdata.ux_shell[0] != '#') {
		u.u_error = ENOEXEC;
		goto bad;
	}
#endif
	switch (u.u_exdata.ux_mag) {

	case 0407:
		u.u_exdata.ux_dsize += u.u_exdata.ux_tsize;
		u.u_exdata.ux_tsize = 0;
		break;

	case 0413:
	case 0410:
		if (u.u_exdata.ux_tsize == 0) {
			u.u_error = ENOEXEC;
			goto bad;
		}
		break;

	default:
		if (u.u_exdata.ux_shell[0] != '#' ||
		    u.u_exdata.ux_shell[1] != '!' ||
		    indir) {
			u.u_error = ENOEXEC;
			goto bad;
		}
		cp = &u.u_exdata.ux_shell[2];		/* skip "#!" */
		while (cp < &u.u_exdata.ux_shell[SHSIZE]) {
			if (*cp == '\t')
				*cp = ' ';
			else if (*cp == '\n') {
				*cp = '\0';
				break;
			}
			cp++;
		}
		if (*cp != '\0') {
			u.u_error = ENOEXEC;
			goto bad;
		}
		cp = &u.u_exdata.ux_shell[2];
		while (*cp == ' ')
			cp++;
		execnamep = cp;
		while (*cp && *cp != ' ')
			cp++;
		sharg = NULL;
		if (*cp) {
			*cp++ = '\0';
			while (*cp == ' ')
				cp++;
			if (*cp) {
				bcopy((caddr_t)cp, (caddr_t)cfarg, SHSIZE);
				sharg = cfarg;
			}
		}
		indir = 1;
		VN_RELE(vp);
		u.u_error =
		    lookupname(execnamep, UIOSEG_KERNEL, FOLLOW_LINK,
			(struct vnode **)0, &vp);
		if (u.u_error) {
			vp = (struct vnode *)0;
			goto bad;
		}
		if (u.u_error = VOP_GETATTR(vp, &vattr, u.u_cred))
			goto bad;
		goto again;
	}

	/*
	 * Collect arguments on "file" in swap space.
	 */
	na = 0;
	ne = 0;
	nc = 0;
	if ((bno = (daddr_t)rmalloc(argmap, (long)ctod(clrnd((int)btoc(NCARGS))))) == 0) {
		swkill(u.u_procp, "exece");
		goto bad;
	}
	if (bno % CLSIZE)
		panic("execa rmalloc");
	if (uap->argp) for (;;) {
		ap = NULL;
		if (indir && (na == 1 || na == 2 && sharg))
			ap = (int)uap->fname;
		else if (uap->argp) {
			ap = fuword((caddr_t)uap->argp);
			uap->argp++;
		}
		if (ap==NULL && uap->envp) {
			uap->argp = NULL;
			if ((ap = fuword((caddr_t)uap->envp)) == NULL)
				break;
			uap->envp++;
			ne++;
		}
		if (ap == NULL)
			break;
		na++;
		if (ap == -1)
			u.u_error = EFAULT;
		do {
			if (nc >= NCARGS-1)
				u.u_error = E2BIG;
			if (indir && na == 2 && sharg != NULL)
				c = *sharg++ & 0377;
			else if ((c = fubyte((caddr_t)ap++)) < 0)
				u.u_error = EFAULT;
			if (u.u_error) {
				if (bp)
					brelse(bp);
				bp = 0;
				goto badarg;
			}
			if (nc % (CLSIZE*NBPG) == 0) {
				if (bp)
					bdwrite(bp);
				bp = getblk(argdev_vp, bno + ctod(nc / NBPG),
				    CLSIZE*NBPG);
				cp = bp->b_un.b_addr;
			}
			nc++;
			*cp++ = c;
		} while (c > 0);
	}
	if (bp)
		bdwrite(bp);
	bp = 0;
	nc = (nc + NBPW-1) & ~(NBPW-1);
	getxfile(vp, nc + (na+4)*NBPW, uid, gid);
	if (u.u_error) {
badarg:
		for (c = 0; c < nc; c += CLSIZE*NBPG) {
			bp = baddr(argdev_vp, bno + ctod(c / NBPG),
				CLSIZE*NBPG);
			if (bp) {
				bp->b_flags |= B_AGE;		/* throw away */
				bp->b_flags &= ~B_DELWRI;	/* cancel io */
				brelse(bp);
				bp = 0;
			}
		}
		goto bad;
	}

	/*
	 * copy back arglist
	 */
	ucp = USRSTACK - nc - NBPW;
	ap = ucp - na*NBPW - 3*NBPW;
	u.u_ar0[SP] = ap;
	(void) suword((caddr_t)ap, na-ne);
	nc = 0;
	for (;;) {
		ap += NBPW;
		if (na==ne) {
			(void) suword((caddr_t)ap, 0);
			ap += NBPW;
		}
		if (--na < 0)
			break;
		(void) suword((caddr_t)ap, ucp);
		do {
			if (nc % (CLSIZE*NBPG) == 0) {
				if (bp)
					brelse(bp);
				bp = bread(argdev_vp, bno + ctod(nc / NBPG),
				    CLSIZE*NBPG);
				bp->b_flags |= B_AGE;		/* throw away */
				bp->b_flags &= ~B_DELWRI;	/* cancel io */
				cp = bp->b_un.b_addr;
			}
			(void) subyte((caddr_t)ucp++, (c = *cp++));
			nc++;
		} while(c&0377);
	}
	(void) suword((caddr_t)ap, 0);
	setregs();

	/*
	 * Remember file name for accounting.
	 */
	u.u_acflag &= ~AFORK;
	if (pn.pn_pathlen > MAXCOMLEN)
		pn.pn_pathlen = MAXCOMLEN;
	bcopy((caddr_t)pn.pn_buf, (caddr_t)u.u_comm,
	    (unsigned)(pn.pn_pathlen + 1));
bad:
	pn_free(&pn);
	if (bp)
		brelse(bp);
	if (bno)
		rmfree(argmap, (long)ctod(clrnd((int)btoc(NCARGS))), (long)bno);
	if (vp)
		VN_RELE(vp);
}

/*
 * Read in and set up memory for executed file.
 */
getxfile(vp, nargc, uid, gid)
	register struct vnode *vp;
	int nargc, uid, gid;
{
	register size_t ts, ds, ss;
	int pagi;

	if ((u.u_exdata.ux_tsize != 0) && ((vp->v_flag & VTEXT) == 0) &&
	    (vp->v_count != 1)) {
		register struct file *fp;

		for (fp = file; fp < fileNFILE; fp++) {
			if (fp->f_type == DTYPE_VNODE &&
			    fp->f_count > 0 &&
			    (struct vnode *)fp->f_data == vp &&
			    (fp->f_flag & FWRITE)) {
				u.u_error = ETXTBSY;
				goto bad;
			}
		}
	}

	/*
	 * Compute text and data sizes and make sure not too large.
	 */
	ts = clrnd(btoc(u.u_exdata.ux_tsize));
	ds = clrnd(btoc((u.u_exdata.ux_dsize+u.u_exdata.ux_bsize)));
	ss = clrnd(SSIZE + btoc(nargc));
	if (chksize((unsigned)ts, (unsigned)ds, (unsigned)ss))
		goto bad;

	if ((u.u_exdata.ux_mag == 0413) &&
	    ((ts + clrnd(btoc(u.u_exdata.ux_dsize))) > MIN(freemem, pgthresh)))
		pagi = SPAGI;
	else
		pagi = 0;
	/*
	 * Make sure there is enough space to start process.
	 */
	u.u_cdmap = zdmap;
	u.u_csmap = zdmap;
	if (swpexpand(ds, ss, &u.u_cdmap, &u.u_csmap) == NULL)
		goto bad;

	/*
	 * At this point, we are committed to the new image!
	 * Release virtual memory resources of old process, and
	 * initialize the virtual memory of the new process.
	 * If we resulted from vfork(), instead wakeup our
	 * parent who will set SVFDONE when he has taken back
	 * our resources.
	 */
	if ((u.u_procp->p_flag & SVFORK) == 0)
		vrelvm();
	else {
		u.u_procp->p_flag &= ~SVFORK;
		u.u_procp->p_flag |= SKEEP;
		wakeup((caddr_t)u.u_procp);
		while ((u.u_procp->p_flag & SVFDONE) == 0)
			sleep((caddr_t)u.u_procp, PZERO - 1);
		u.u_procp->p_flag &= ~(SVFDONE|SKEEP);
	}
	u.u_procp->p_flag &= ~(SPAGI|SSEQL|SUANOM|SOUSIG);
	u.u_procp->p_flag |= pagi;
	u.u_dmap = u.u_cdmap;
	u.u_smap = u.u_csmap;
#ifdef sun
	u.u_hole.uh_first = u.u_hole.uh_last = 0;
#endif
	vgetvm(ts, ds, ss);

	if (pagi == 0) {
		u.u_error =
		    vn_rdwr(UIO_READ, vp,
			(char *)ctob(dptov(u.u_procp, 0)),
			(int)u.u_exdata.ux_dsize,
			(int)((u.u_exdata.ux_mag == 0413?
			    CLBYTES:
			    sizeof(u.u_exdata))+u.u_exdata.ux_tsize),
			UIOSEG_USER, IO_UNIT, (int *)0);
	}
	xalloc(vp, pagi);
	if (pagi && u.u_procp->p_textp)
		vinifod((struct fpte *)dptopte(u.u_procp, 0),
		    PG_FTEXT, u.u_procp->p_textp->x_vptr,
		    (long)(1 + ts/CLSIZE), (int)btoc(u.u_exdata.ux_dsize));

#ifdef vax
	/* THIS SHOULD BE DONE AT A LOWER LEVEL, IF AT ALL */
	mtpr(TBIA, 0);
#endif

	if (u.u_error)
		swkill(u.u_procp, "getxfile: i/o error mapping pages");
	/*
	 * set SUID/SGID protections, if no tracing
	 */
	if ((u.u_procp->p_flag&STRC)==0) {
		if (uid != u.u_uid || gid != u.u_gid)
			u.u_cred = crcopy(u.u_cred);
		u.u_uid = uid;
		u.u_procp->p_uid = uid;
		u.u_gid = gid;
	} else
		psignal(u.u_procp, SIGTRAP);
	u.u_tsize = ts;
	u.u_dsize = ds;
	u.u_ssize = ss;
bad:
	return;
}

/*
 * Clear registers on exec
 */
setregs()
{
	register int i;
	register struct proc *p = u.u_procp;

	/*
	 * Reset caught signals.  Held signals
	 * remain held through p_sigmask.
	 */
	while (p->p_sigcatch) {
		(void) spl6();
		i = ffs((long)p->p_sigcatch);
		p->p_sigcatch &= ~(1 << (i - 1));
		u.u_signal[i] = SIG_DFL;
		(void) spl0();
	}
#ifdef vax
#ifdef notdef
	/* should pass args to init on the stack */
	for (rp = &u.u_ar0[0]; rp < &u.u_ar0[16];)
		*rp++ = 0;
#endif
	u.u_ar0[PC] = u.u_exdata.ux_entloc+2;
#endif
#ifdef mc68000
	{ register struct regs *r = (struct regs *)u.u_ar0;
	  for (i = 0; i < 8; i++) {
		r->r_dreg[i] = 0;
		if (&r->r_areg[i] != &r->r_sp)
			r->r_areg[i] = 0;
	  }
	  r->r_ps = PSL_USERSET;
	  r->r_pc = u.u_exdata.ux_entloc;
	}
	u.u_eosys = REALLYRETURN;
#endif
	for (i=0; i<NOFILE; i++) {
		if (u.u_pofile[i]&UF_EXCLOSE) {
			closef(u.u_ofile[i]);
			u.u_ofile[i] = NULL;
			u.u_pofile[i] = 0;
		}
		u.u_pofile[i] &= ~UF_MAPPED;
	}
}
