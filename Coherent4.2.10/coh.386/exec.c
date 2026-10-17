/* $Header: /ker/coh.386/RCS/exec.c,v 2.7 93/10/29 00:55:04 nigel Exp Locker: nigel $ */
/*
 * Processing of the exec () system call.
 *
 * $Log:	exec.c,v $
 * Revision 2.7  93/10/29  00:55:04  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.6  93/09/02  18:04:39  nigel
 * Use new flag stuff, fix interrupt fubar
 * 
 * Revision 2.4  93/08/19  03:26:24  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Revision 2.2  93/07/26  15:22:36  nigel
 * Nigel's R80
 */

#include <common/_gregset.h>
#include <common/_tricks.h>
#include <kernel/proc_lib.h>
#include <kernel/cred_lib.h>
#include <kernel/sig_lib.h>
#include <sys/cmn_err.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/cred.h>
#include <signal.h>
#include <fcntl.h>
#include <stddef.h>

#define	_KERNEL		1

#include <kernel/reg.h>
#include <sys/uproc.h>
#include <sys/mmu.h>
#include <sys/acct.h>
#include <sys/buf.h>
#include <canon.h>
#include <sys/con.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <a.out.h>
#include <l.out.h>
#include <sys/proc.h>
#include <sys/sched.h>
#include <sys/seg.h>
#include <sys/fd.h>
#include <sys/types.h>

/*
 * Round section and segment start address to nearest lower click boundary.
 */

static void
xecrnd(xsp)
struct xecseg * xsp;
{
	int diff;

	diff = xsp->fbase & (NBPC - 1);
	xsp->mbase -= diff;
	xsp->fbase -= diff;
	xsp->size += diff;
}


static SEG *
exsread (sp, ip, xsp, shrdSz)
SEG *sp;
struct inode *ip;
struct xecseg *xsp;
int shrdSz;
{
	int sa, so;
	IO		io;

	sa = xsp->fbase;
	so = xsp->mbase & (NBPC - 1);

	io.io_seg = IOPHY;
	io.io_seek = sa;
	io.io.pbase = MAPIO (sp->s_vmem, so);
	io.io_flag = 0;

	if (shrdSz) {	/* shared l.out? */

		/* Load SHRD. */
		io.io_ioc = shrdSz;
		sp->s_lrefc ++;
		iread (ip, & io);
		sp->s_lrefc --;

		if ((io.io_ioc = xsp->size - shrdSz) != 0) {

			/* Advance file and RAM offsets past SHRD. */
			sa += shrdSz;
			so += shrdSz;

			/* Advance RAM offset to next 16-byte boundary. */
			so = (so + 15) & ~ 15; /* round up */

			/* Load PRVD. */
			io.io_seg = IOPHY;
			io.io_seek = sa;
			io.io.pbase = MAPIO (sp->s_vmem, so);
			io.io_flag = 0;

			sp->s_lrefc ++;
			iread (ip, & io);
			sp->s_lrefc --;
		}
	} else {	/* NOT shared l.out */
		io.io_ioc = xsp->size;

		sp->s_lrefc ++;
		iread (ip, & io);
		sp->s_lrefc --;
	}

	/*
	 * NIGEL: This perturbs me. This check seems to really belong
	 * somewhere at the top-level, and/or from testing the return values
	 * from the read calls. Why isn't the residual from the read tested?
	 */

	if (curr_signal_pending ())
		set_user_error (EINTR);

	return get_user_error () == 0 ? sp : NULL;
}


struct adata {		/* Storage for arg and env data */
	int	np;		/* Number of pointers in vector */
	int	nc;		/* Number of characters in strings */
};


/*
 * Given a pointer to a list of arguments, a pointer to an argument count
 * and a pointer to a byte count, count the #characters/#strings
 * in the arguments
 */

static int
excount(usrvp, adp, wdin)
caddr_t usrvp;
struct adata *adp;
int	wdin;
{
	caddr_t	usrcp;
	int c;
	unsigned nb;
	unsigned na;
	int	wdmask;

	wdmask = -1;
	if (wdin == sizeof (short))
		wdmask = (unsigned short) wdmask;
	na = nb = 0;
	if (usrvp != NULL) {
		for (;;) {
			usrcp = (caddr_t) (getupd (usrvp) & wdmask);
			usrvp += wdin;
			if (get_user_error ())
				return 0;
			if (usrcp == NULL)
				break;
			na ++;
			for (;;) {
				c = getubd (usrcp ++);
				if (get_user_error ())
					return 0;
				nb ++;
				if (c == '\0')
					break;
			}
		}
	}
	adp->np = na;
	adp->nc = nb;
	return 1;
}


static int
exarg (out, in)
caddr_t	in, out;
{
	char 	c;
	caddr_t	init_in;

	init_in = in;
	do {
		c = getubd (in ++);
		dmaout (sizeof (char), out ++, & c);
	} while (c);
	return in - init_in;
}


/*
 * Given a pointer to a list of arguments and a pointer to a list of
 * environments, return a stack with the arguments and environments on it.
 */

static SEG *
exstack(xhp, argp, envp, wdin)
struct xechdr *xhp;
caddr_t	argp, envp;
int wdin;
{
	register SEG *sp;		/* Stack segment pointer */
	struct sdata {		/* To keep segment pointers */
		caddr_t	vp;		/* Argv[i], envp[i] pointer */
		caddr_t	cp;		/* Argv[i][j], envp[i][j] pointer */
	} stk;
	struct adata arg, env;
	int	chrsz, vecsz, stksz, wdmask, wdout, stkoff, stktop;
	int	stkenvp;
	int i;

	/* Validate and evaluate size of args and envs */
	if (! excount (argp, & arg, wdin) || ! excount (envp, & env, wdin))
		return NULL;

	/* Calculate stack size and allocate it */
	chrsz = __ROUND_UP_TO_MULTIPLE (arg.nc + env.nc, sizeof (int));
	vecsz = (arg.np + 1 + env.np + 1) * sizeof (long);
	stksz = __ROUND_UP_TO_MULTIPLE (vecsz + chrsz + ISTSIZE, NBPC);

	if (stksz > MADSIZE || (sp = salloc (stksz, SFDOWN)) == NULL) {
		set_user_error (E2BIG);
		return NULL;
	}

	/* Set up target stack */
	stktop = xhp->segs [SISTACK].mbase;
	stk.cp = (caddr_t) stktop - chrsz;
	stk.vp = (caddr_t) stktop - chrsz - vecsz;
	stkoff = MAPIO (sp->s_vmem, stksz - stktop);
	u.u_argc = arg.np;
	u.u_argp = stk.vp;
	wdmask = -1;
	if (wdin == sizeof (short))
		wdmask = (unsigned short) wdmask;

	switch (stktop) {

	case ISP_386:
		wdout = sizeof (long);
		xhp->initsp = (unsigned long) stk.vp - sizeof (long);
		dmaout (sizeof (long), xhp->initsp + stkoff, & arg.np);
		break;

	case ISP_286:
		wdout = sizeof (short);
		xhp->initsp = (unsigned long) stk.vp - 3 * sizeof (short);
		stkenvp = (unsigned long) stk.vp + (arg.np + 1) * sizeof (short);
		dmaout (sizeof (short), xhp->initsp + stkoff, & arg.np);
		dmaout (sizeof (short), xhp->initsp + sizeof (short) + stkoff,
			& stk.vp);
		dmaout (sizeof (short), xhp->initsp + 2 * sizeof (short) +
					stkoff,	& stkenvp);
		break;

	default:
		ASSERT ("impossible switch selector" == NULL);
		return NULL;
	}

	/* Arguments */
	for (i = 0 ; i < arg.np ; i ++, argp += wdin, stk.vp += wdout) {
		dmaout (wdout, stk.vp + stkoff, & stk.cp);
		stk.cp += exarg (stk.cp + stkoff, getupd (argp) & wdmask);
	}

	/* skip null word after arguments */
	stk.vp += wdout;

	/* Environments */
	for (i = 0; i < env.np ; i ++, envp += wdin, stk.vp += wdout) {
		dmaout (wdout, stk.vp + stkoff, & stk.cp);
		stk.cp += exarg (stk.cp + stkoff, getupd (envp) & wdmask);
	}

	return sp;
}




/*
 * Set up the first process, a small program which will exec
 * the init program.
 */

extern char	aicode [];	/* actual init data */
extern char	aicode_end [];

void
eveinit ()
{
	SEG * sp;
	size_t		size = aicode_end - aicode;

/*	static struct xechdr xecinit[NUSEG+1] = { {0},{0},{0},{ISP_386}}; */ 

	/*
	 * Allocate, record, initialize code segment, make it executable.
	 */

	if ((sp = salloc (__ROUND_UP_TO_MULTIPLE (size, NBPC), 0)) == NULL)
		cmn_err (CE_PANIC, "eveinit ()");

	SELF->p_segl [SIPDATA].sr_segp = sp;

	/*
	 * Start process.
	 */

	u.u_argp = 0;

	if (sproto (0) == 0)
		cmn_err (CE_PANIC, "eveinit ()");

	segload ();
	setspace (SEL_386_UD);
	kucopy (aicode, 0, size);
}


/*
 * Open a file, make sure it is l.out, coff, or v86 as well as
 * executable.
 *
 * "xhp" points to a cleared xechdr supplied by the caller.
 * "np" is the file name.
 * "shrds" points to an int that will be written by exlopen().
 *   *shrds is set nonzero only for shared l.out.
 *
 * If file is COFF, there may be multiple text (or data?) sections.
 * Use "xlist" linked structure to keep track of variably many sections
 * after the first text and data sections.
 *
 * return NULL if failure, else return inode pointer for the file.
 */

struct inode *
exlopen (xhp, np, shrds, xlist, dirent) 
struct xechdr *xhp;
char *np;
int *shrds;
struct xecnode ** xlist;
struct direct *	dirent;
{
	int	i, nscn, hdrsize;
	buf_t	      *	bp;
	unsigned short magic;
	struct ldheader head;
	struct filehdr fhead;
	struct aouthdr ahead;	
	struct scnhdr scnhdr;

	/*
	 * Make sure the file is executable and read the header. Note that
	 * this is about the only case of ftoi () with mode 'r' that actually
	 * uses the resulting filename information.
	 */

	if (ftoi (np, 'r', IOUSR, NULL, dirent, SELF->p_credp))
		return NULL;

	if ((u.u_cdiri->i_mode & IFMT) != IFREG ||
	    ! iaccess (u.u_cdiri, IPE, SELF->p_credp)) {
		idetach (u.u_cdiri);
		return NULL;
	}

#if	0
	/*
	 * The check here for IPE is redundant, so the IFMT check was moved
	 * up.
	 */
	if ((ip->i_mode & (IPE | (IPE << 3) | (IPE << 6))) == 0 ||
	    (ip->i_mode & IFMT) != IFREG) {
		set_user_error (EACCES);
		idetach (ip);
		return NULL;
	}
#endif

	if ((bp = vread (u.u_cdiri, (daddr_t) 0)) == NULL) {
		set_user_error (ENOEXEC);
		idetach (u.u_cdiri);
		return NULL;
	}


	/*
	 * Copy everything we need from the l.out header and check magic
	 * number and machine type.
	 */

	* shrds = 0;
	magic = * (unsigned short *) bp->b_vaddr;
	canint (magic);

	switch (magic) {
	case L_MAGIC:		/* Coherent 286 format */
		memcpy (& head, bp->b_vaddr, sizeof (struct ldheader));

		canint (head.l_machine);
		if (head.l_machine != M_8086)
			goto bad;

		for (i = 0 ; i < NXSEG ; i ++)
			cansize (head.l_ssize [i]);

		canint (head.l_flag);
		canvaddr (head.l_entry);

		/*
		 * If a shared and separated image
	 	 * has stuff in segments that makes it impossible
		 * to share, give an error immediately so that we don't
		 * lose the parent.
		 */
		head.l_flag &= LF_SHR | LF_SEP | LF_KER;

		if ((head.l_flag & LF_SEP) == 0 ||
		    (head.l_flag & LF_KER) != 0 ||
		    head.l_ssize [L_PRVI] || head.l_ssize [L_BSSI])
			goto bad;

		xhp->magic = XMAGIC (I286MAGIC,I_MAGIC);
		xhp->entry = head.l_entry;

		xhp->segs [SISTEXT].fbase = sizeof (struct ldheader);
		xhp->segs [SISTEXT].mbase = NBPS;
		xhp->segs [SISTEXT].size = head.l_ssize [L_SHRI];

		xhp->segs [SIPDATA].fbase = sizeof (struct ldheader) +
						xhp->segs [SISTEXT].size;
		xhp->segs [SIPDATA].mbase = 0;
		xhp->segs [SIPDATA].size = head.l_ssize [L_SHRD] +
						head.l_ssize [L_PRVD];
		if (head.l_flag & LF_SHR)
			* shrds = head.l_ssize [L_SHRD];

		xhp->segs [SIBSS].fbase = 0;
		xhp->segs [SIBSS].mbase = xhp->segs [SIPDATA].size;
		xhp->segs [SIBSS].size = head.l_ssize [L_BSSD];

		xhp->segs [SISTACK].mbase = ISP_286;	/* size 0, fbase 0 */
		brelease (bp);
		return u.u_cdiri;

	case I386MAGIC:		/* ... COFF */
		memcpy (& fhead, bp->b_vaddr, sizeof (struct filehdr));
		hdrsize = sizeof (ahead) + sizeof (fhead);

		if (fhead.f_opthdr != sizeof (ahead) ||
		    (fhead.f_flags & F_EXEC) == 0 ||
		    fhead.f_nscns * sizeof (scnhdr) > BSIZE)
			goto bad;

		memcpy (& ahead, bp->b_vaddr + sizeof (fhead),
			sizeof (ahead));
		if (ahead.magic != Z_MAGIC)
			goto bad;

		xhp->magic = XMAGIC (I386MAGIC, ahead.magic);
		xhp->entry = ahead.entry;

		for (i = 0 ; i < fhead.f_nscns ; i ++) {
			memcpy (& scnhdr,
				bp->b_vaddr + hdrsize + sizeof (scnhdr) * i,
				sizeof (scnhdr));

			switch ((int) (scnhdr.s_flags)) {
			case STYP_INFO:
				continue;

			case STYP_BSS:
				nscn = SIBSS;
				break;

			case STYP_TEXT:
				nscn = SISTEXT;
				break;

			case STYP_DATA:
				nscn = SIPDATA;
				break;

			default:
				goto bad;
			}

			/* Text/data shouldn't collide with stack. */
			if ((unsigned) scnhdr.s_vaddr >= ISP_386)
				goto bad;

			/* Have we already seen a segment of this type? */
			if (xhp->segs [nscn].size) {
				struct xecnode * tmp;

				if (nscn != SISTEXT)
					goto bad;

				/* insert new node at head of "xlist" */
				tmp = (struct xecnode *)
					  kalloc (sizeof (struct xecnode));

				if (tmp == NULL) {
					cmn_err (CE_WARN,
						 "can't kalloc(xecnode)");
					goto bad;
				}

				tmp->xn = * xlist;
				* xlist = tmp;
				tmp->segtype = nscn;
				tmp->xseg.mbase = scnhdr.s_vaddr;
				tmp->xseg.fbase = scnhdr.s_scnptr;
				tmp->xseg.size = scnhdr.s_size;
			} else {
				xhp->segs [nscn].mbase = scnhdr.s_vaddr;
				xhp->segs [nscn].fbase = scnhdr.s_scnptr;
				xhp->segs [nscn].size = scnhdr.s_size;
			}
		}

		/* Text and data segments must both be nonempty. */
		if (! xhp->segs [SISTEXT].size || ! xhp->segs [SIPDATA].size)
			goto bad;

		xhp->entry = ahead.entry;

		xhp->segs [SISTACK].mbase = ISP_386;	/* size 0, fbase 0 */
		xhp->magic = XMAGIC (I386MAGIC, ahead.magic);
		brelease (bp);	
		return u.u_cdiri;

	default:
	bad:		
		brelease (bp);
		set_user_error (ENOEXEC);
		idetach (u.u_cdiri);
		return NULL;
	}
}


/*
 * Pass control to an image in a file.
 * Make sure the format is acceptable. Release
 * the old segments. Read in the new ones. Some special
 * care is taken so that shared and (more important) shared
 * and separated images can be run on the 8086.
 */

int
pexece (np, argp, envp, regsetp)
char	      *	np;
char	      *	argp [];
char	      *	envp [];
gregset_t     *	regsetp;
{
	struct xechdr	head;
	struct inode	*ip;			/* Load file INODE */
 	SEG	      *	textseg;
 	SEG	      *	dataseg;
	SEG	      *	ssegp;
	int	i;			/* For looping over segments*/
	int roundup;
	int shrdsize;
	struct xecnode * xlist = NULL;		/* list head */
	struct xecnode * xp;
	struct xecseg tempseg;
	unsigned int textSize;
	struct direct	dir;

	memset (& head, 0, sizeof (head)); 
	if ((ip = exlopen (& head, np, & shrdsize, & xlist, & dir)) == NULL)
		goto done;

	roundup = shrdsize & 0xf;
	ssegp = exstack (& head, argp, envp,
			 __xmode_286 (regsetp) ? sizeof (short) :
						 sizeof (int));

	if (! ssegp) {
		idetach(ip);
		goto done;
	}

	/* Release shared memory. */
	shmAllDt ();

	/*
	 * At this point the file has been validated as an object module, and
	 * the argument list has been built. Release all of the original
	 * segments. At this point we have committed to the new image. A "sys
	 * exec" that gets an I/O error is doomed.
	 *
 	 * NOTE: User-area segment is NOT released.
 	 *	 Segment pointer in proc is erased BEFORE invoking sfree().
	 */

	for (i = 1 ; i < NUSEG ; ++ i) {
		SR	      *	segp;

 		if ((segp = SELF->p_segl [i].sr_segp) != NULL) {
			SELF->p_segl [i].sr_segp = NULL;
			sfree (segp);
		}
	}

	/*
	 * Read in the loadable segments.
	 */

	switch (head.magic) {
	case XMAGIC (I286MAGIC, I_MAGIC):
		if ((textseg = ssalloc (ip, SFTEXT,
					head.segs [SISTEXT].size)) == NULL)
			goto out; 

		if (! exsread (textseg, ip, & head.segs [SISTEXT], 0))
			goto out;

		if ((dataseg = ssalloc (ip, 0, roundup +
					       head.segs [SIPDATA].size +
					       head.segs [SIBSS].size)) == NULL)
			goto out;

		if (! exsread (dataseg, ip, & head.segs [SIPDATA], shrdsize))
			goto out;

		head.segs [SIPDATA].size += roundup;
		break;

	case XMAGIC (I386MAGIC, Z_MAGIC):
		/*
		 * Round segment address down to nearest click boundary.
		 * Ciaran did this.  I'm not sure why, but will preserve
		 * it for now. -hws-
		 */

		tempseg = head.segs [SISTEXT];	/* save pre-rounding value */
		xecrnd (head.segs + SISTEXT);
		xecrnd (head.segs + SIPDATA);

		/*
		 * Compute text segment size by taking highest address
		 * seen in any text section.
		 */

		textSize = head.segs [SISTEXT].size +
				head.segs [SISTEXT].mbase;
		for (xp = xlist ; xp ; xp = xp->xn) {
			unsigned int tmpSize;
			if (xp->segtype != SISTEXT)
				continue;
			tmpSize = xp->xseg.size + xp->xseg.mbase;
			if (tmpSize > textSize)
				textSize = tmpSize;
		}

		/* Entry point must be within text segment. */
		if (head.entry >= textSize)
			goto out;

		if ((textseg = ssalloc (ip, SFTEXT | SFSHRX,
					textSize)) == NULL)
			goto out;

		if (textseg->s_ip == 0) {
			if (! exsread (textseg, ip, & tempseg, 0))
				goto out;

			/* load additional text sections, if any */
			for (xp = xlist ; xp ; xp = xp->xn) {
				if (xp->segtype != SISTEXT)
					continue;
				if (! exsread (textseg, ip, & xlist->xseg, 0))
					goto out;
			}

			textseg->s_ip = ip;
			ip->i_refc ++;
		}

		if ((dataseg = ssalloc (ip, 0, head.segs [SIPDATA].size +
					head.segs[SIBSS].size)) == NULL)
			goto out;

		if (dataseg->s_ip == 0 &&
		    ! exsread (dataseg, ip, & head.segs [SIPDATA], 0))
			goto out;

		/* Deallocate nodes hooked into xlist by exlopen. */
		while (xlist != NULL) {
			struct xecnode * tmp = xlist->xn;
			kfree (xlist);
			xlist = tmp;
		}
		break;

	default:
		ASSERT ("Impossible switch selector" == NULL);
		goto out;
	}

	SELF->p_segl [SISTACK].sr_segp = ssegp;
	SELF->p_segl [SISTEXT].sr_segp = textseg;
	SELF->p_segl [SIPDATA].sr_segp = dataseg;

	if (sproto (& head) == 0)
		goto out;

	/*
	 * At this point, and no earlier, we can modify the user register
	 * image for the new process because now we are committed to executing
	 * the new image.
	 *
	 * As a general security thing, we begin by zeroing out the user-level
	 * register image /except/ for the flags, where we just clear the
	 * user-settable status bits.
	 */

	{
		__flag_reg_t	flags = __FLAG_REG (regsetp);
		memset (regsetp, 0, sizeof (* regsetp));
		__FLAG_REG (regsetp) = __FLAG_CLEAR_STATUS (flags);
	}

	switch (head.magic) {
	case XMAGIC (I286MAGIC, I_MAGIC):
		__SET_SELECTOR (regsetp->_i286._cs, SEL_286_UII);
		__SET_SELECTOR (regsetp->_i286._ds, SEL_286_UD);
		regsetp->_i286._ss = regsetp->_i286._es = regsetp->_i286._ds;
		regsetp->_i286._ip = head.entry;
		regsetp->_i286._usp = head.initsp;
		break;

	case XMAGIC (I386MAGIC, Z_MAGIC):
		__SET_SELECTOR (regsetp->_i386._cs, SEL_386_UI);
		__SET_SELECTOR (regsetp->_i386._ds, SEL_386_UD);
		regsetp->_i386._ss = regsetp->_i386._es = regsetp->_i386._ds;
		regsetp->_i386._eip = head.entry;
		regsetp->_i386._uesp = head.initsp;
		break;
	}


	/*
	 * The new image is read in
	 * and mapped. Perform the final grunge
	 * (set-uid stuff, accounting, loading up
	 * registers, etc).
	 */

	u.u_flag &= ~AFORK;

	memcpy (SELF->p_comm, dir.d_name, sizeof (SELF->p_comm));

	if (iaccess (ip, IPR, SELF->p_credp) == 0) {
		/* Can't read ? no dump or trace */
		SELF->p_flags |= PFNDMP;
		SELF->p_flags &= ~PFTRAC;
	}

	/*
	 * Record file access time.
	 */

	iaccessed (ip);


	{
		n_uid_t		uid = SELF->p_credp->cr_uid;
		n_gid_t		gid = SELF->p_credp->cr_gid;

		if ((ip->i_mode & ISUID) != 0) {
			/* Set user id ? no trace */
			uid = ip->i_uid;
			SELF->p_flags &= ~PFTRAC;
		}

		if ((ip->i_mode & ISGID) != 0) {
			/* Set group id ? no trace */
			gid = ip->i_gid;
			SELF->p_flags &= ~PFTRAC;
		}

		if ((SELF->p_credp = cred_execid (SELF->p_credp, uid,
						  gid)) == NULL)
			goto out;
	}

	for (i = 0 ; i < NOFILE; i ++) {
		int		j = fd_get_flags (i);
		if (j != -1 && (j & FD_CLOEXEC) != 0)
			fd_close (i);	/* close fd on exec bit set */
	}


	/*
	 * Default every signal that is not ignored.
	 */

	for (i = 1 ; i <= _SIGNAL_MAX ; ++ i) {
		__sigaction_t	act;

		curr_signal_action (i, NULL, & act);
		if (act.sa_handler != SIG_IGN) {
			act.sa_handler = SIG_DFL;
			act.sa_flags = 0;
			___SIGSET_SET (act.sa_mask, 0);
			curr_signal_action (i, & act, NULL);
		}
	}

	/*
	 * We have successfully completed an exec (), which means that the
	 * setpgid () function can no longer be used to change our process
	 * group from the parent process.
	 */

	SELF->p_flags |= PFEXEC;

	if (SELF->p_flags & PFTRAC)	/* Being traced */
		sendsig (SIGTRAP, SELF);
	idetach (ip);

	/* initialize u area ndp fields */
	ndpNewProc ();

	segload ();

	/* 
	 * Also, set up the new scheduling priority for the process.
	 * We cannot assume this process will have the same properties
	 * as the one we exec'ed from.
	 * Starting priority should be the same as in
	 * ~/coh.386/lib/proc_init.c
	 */
	SELF->p_schedPri = NCRTICK * (100 / 2);
	SELF->p_foodstamp = 0;
	goto done;


	/*
	 * Alas, exec() has failed..
	 */
out:
	/* Deallocate nodes hooked into xlist by exlopen. */
	while (xlist != NULL) {
		struct xecnode * tmp = xlist->xn;
		kfree (xlist);
		xlist = tmp;
	}

	/* Release the inode for the load file. */
	idetach (ip);

	/* If we allocated a text segment, let it go. */
	if ((textseg = SELF->p_segl [SISTEXT].sr_segp) != NULL) {
		SELF->p_segl [SISTEXT].sr_segp = NULL;
		sfree (textseg);
	}

	/* If we allocated a data segment, let it go. */
	if ((dataseg = SELF->p_segl [SIPDATA].sr_segp) != NULL) {
		SELF->p_segl [SIPDATA].sr_segp = NULL;
		sfree (dataseg);
	}

	/*
	 * Return through the "sys exit" code with a "SIGSYS", or with the
	 * signal actually received if we are aborting due to interrupted exec.
	 */

 	pexit (get_user_error () == EINTR ? curr_signal_pending () : SIGSYS);

done:
	return 0;	
}

