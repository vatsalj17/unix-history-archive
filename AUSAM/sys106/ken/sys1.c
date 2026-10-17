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
#include "../buf.h"
#include "../reg.h"
#include "../inode.h"
#ifdef	ZOMBIE
#include "../pzomb.h"
#endif	ZOMBIE
#ifdef	SHARED_DATA
#include	"../text.h"
#endif	SHARED_DATA

/*
 * exec system call.
 * Because of the fact that an I/O buffer is used
 * to store the caller's arguments during exec,
 * and more buffers are needed to read in the text file,
 * deadly embraces waiting for free buffers are possible.
 * Therefore the number of processes simultaneously
 * running in exec has to be limited to NEXEC.
 */
#define EXPRI	-1

exec()
{
	int ap, na, nc, *bp;
	int ts, ds, sep;
	register c, *ip;
	register char *cp;
	extern uchar;

	/*
	 * pick up file names
	 * and check various modes
	 * for execute permission
	 */

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	while(execnt >= NEXEC)
		sleep(&execnt, EXPRI);
	execnt++;
	bp = getblk(NODEV);
	if(access(ip, IEXEC) || (ip->i_mode&IFMT)!=0)
		goto bad;

	/*
	 * pack up arguments into
	 * allocated disk buffer
	 */

#ifndef	MAPPED_BUFFERS
	cp = bp->b_addr;
#else	MAPPED_BUFFERS
	cp = &b;
	bswtch( bp );
#endif	MAPPED_BUFFERS
	na = 0;
	nc = 0;
	for(;;)
	{
		if((ap = fuword(u.u_arg[1])) == 0)
			break;
		if(u.u_error)				/* fix037 */
			goto bad;
		na++;
		u.u_arg[1] =+ 2;
		for(;;) {
			c = fubyte(ap++);
			if(c == -1)
				goto bad;
			*cp++ = c;
			nc++;
			if(nc > 510) {
				u.u_error = E2BIG;
				goto bad;
			}
			if(c == 0)
				break;
		}
	}
	if((nc&1) != 0) {
		*cp++ = 0;
		nc++;
	}

	/*
	 * read in first 8 bytes
	 * of file for segment
	 * sizes:
	 * w0 = 407/410/411 (410 implies RO text) (411 implies sep ID)
#ifdef	SHARED_DATA
	 *		(412 implies shared data)
#endif
	 * w1 = text size
	 * w2 = data size
	 * w3 = bss size
#ifdef	EP_ADDRESS
	 * w5 = entry point location
#endif
#ifdef	SHARED_DATA
	 * w6 = special id for shared data
#endif
	 */

	u.u_base = &u.u_arg[0];
#ifndef	EP_ADDRESS | SHARED_DATA
	u.u_count = 8;
#endif
#ifdef	EP_ADDRESS | SHARED_DATA
	u.u_count = 16;
#endif
	u.u_offset = 0; /* fix000 */
	u.u_segflg = SEG_KER;
	readi(ip);
#ifdef	SHARED_DATA
	/*
	 * must check permission to link to the shared data segment.
	 * Being super user is NOT a good criterion here, in fact
	 * this is one of the more important things to guard.
	 *				GGR Sep 79.
	 */
	if(u.u_arg[0] == 0412)	/* it is shared */
	{
		struct text *xp;
		int	*spid = u.u_arg[6]; /* MUST be int-> for no sensible reason */
		/* search for this 'data' segment existing */
		for(xp = text; xp < &text[NTEXT]; xp++)
			if(xp->x_iptr != NULL && xp->x_spid == spid)
			{
				sep = (ip->i_mode & ISUID) != 0;
#ifdef	AUSAM16
				if ((sep == 0 && guid(xp->x_iptr) == u.u_uid)
				   || (sep && (guid(ip) == guid(xp->x_iptr))))
#else	AUSAM16
				if ((sep == 0 && xp->x_iptr->i_uid == u.u_uid)
				   || (sep && (ip->i_uid == xp->x_iptr->i_uid)))
#endif	AUSAM16
					break;
				/* 'data' seg exists, and he can't touch it */
				u.u_error = EPERM;
				goto bad;
			}
	}
	u.u_flags =& ~USHRDATA;
#endif
	u.u_segflg = SEG_USD;
	if(u.u_error)
		goto bad;
	sep = 0;
	if(u.u_arg[0] == 0407) {
		u.u_arg[2] =+ u.u_arg[1];
		u.u_arg[1] = 0;
	} else
	if(u.u_arg[0] == 0411)
		sep++; else
#ifdef	SHARED_DATA
	if(u.u_arg[0] != 0412)
#endif
	if(u.u_arg[0] != 0410) {
		u.u_error = ENOEXEC;
		goto bad;
	}
	if(u.u_arg[1]!=0 && (ip->i_flag&ITEXT)==0 && ip->i_count!=1) {
		u.u_error = ETXTBSY;
		goto bad;
	}

	/*
	 * find text and data sizes
	 * try them out for possible
	 * exceed of max sizes
	 */

	ts = ((u.u_arg[1]+63)>>6) & 01777;
	ds = ((u.u_arg[2]+u.u_arg[3]+63)>>6) & 01777;
	if(estabur(ts, ds, SSIZE, sep))
		goto bad;

	/*
	 * allocate and clear core
	 * at this point, committed
	 * to the new image
	 */

#ifdef	SHARED_DATA
	if ( u.u_arg[0] == 0412 )
		u.u_flags =| USHRDATA;
#endif	SHARED_DATA
	u.u_prof[3] = 0;
	xfree();
	expand(USIZE);
#ifndef	SHARED_DATA
	xalloc(ip);
#endif
#ifdef	SHARED_DATA
	if (u.u_flags&USHRDATA)
		xalloc(ip , u.u_arg[6]);
	else
		xalloc(ip , ip);
#endif
	c = USIZE+ds+SSIZE;
	expand(c);
	while(--c >= USIZE)
		clearseg(u.u_procp->p_addr+c);

	/*
	 * read in data segment
	 */

	estabur(0, ds, 0, 0);
	u.u_base = 0;
	u.u_offset.loint = 020+u.u_arg[1]; /* fix000 */
	u.u_count = u.u_arg[2];
	readi(ip);

	/*
	 * initialize stack segment
	 */

	u.u_tsize = ts;
	u.u_dsize = ds;
	u.u_ssize = SSIZE;
	u.u_sep = sep;
	estabur(u.u_tsize, u.u_dsize, u.u_ssize, u.u_sep);
#ifndef	MAPPED_BUFFERS
	cp = bp->b_addr;
#else	MAPPED_BUFFERS
	cp = &b;
	bswtch( bp );
#endif	MAPPED_BUFFERS
	ap = -nc - na*2 - 4;
	u.u_ar0[R6] = ap;
	suword(ap, na);
	c = -nc;
	while(na--) {
		suword(ap=+2, c);
		do
			subyte(c++, *cp);
		while(*cp++);
	}
	suword(ap+2, -1);

	/*
	 * turn off tracing, if you don't own the new process	fix043
	 * set SUID/SGID protections, if no tracing
	 */

#ifdef	AUSAM16
	if (u.u_uid && guid(ip) != u.u_uid)	/* fix043 */
#else
	if (u.u_uid && ip->i_uid != u.u_uid)	/* fix043 */
#endif
		u.u_procp->p_flag =& ~STRC;	/* fix043 */

	if ((u.u_procp->p_flag&STRC)==0) {
		if(ip->i_mode&ISUID)
			if(u.u_uid != 0) {
#ifndef	AUSAM16
				u.u_uid = ip->i_uid;
				u.u_procp->p_uid = ip->i_uid;
#endif	AUSAM16
#ifdef	AUSAM16
				u.u_uid = u.u_procp->p_uid = guid(ip);
#endif	AUSAM16
			}
#ifdef	GROUP_ACCESS & !LOCKING
		if(ip->i_mode&ISGID)
			u.u_gid = ip->i_gid;
#endif
	}

	/*
	 * clear sigs, regs and return
	 */

	c = ip;
	for(ip = &u.u_signal[0]; ip < &u.u_signal[NSIG]; ip++)
		if((*ip & 1) == 0)
			*ip = 0;
#ifdef	IGNORE_SIGNALS
	u.u_procp->p_ignsig = 0;
#endif	IGNORE_SIGNALS
	for(cp = &regloc[0]; cp < &regloc[6];)
		u.u_ar0[*cp++] = 0;
#ifndef	EP_ADDRESS
	u.u_ar0[R7] = 0;
#endif
#ifdef	EP_ADDRESS
	u.u_ar0[R7] = u.u_arg[5] & 0177776;
#endif
#ifdef	FPU
	for(ip = &u.u_fsav[0]; ip < &u.u_fsav[25];)
		*ip++ = 0;
#endif
	ip = c;

bad:
	iput(ip);
	brelse(bp);
	if(execnt >= NEXEC)
		wakeup(&execnt);
	execnt--;
}

/*
 * exit system call:
 * pass back caller's r0
 */
rexit()
{

	u.u_arg[0] = u.u_ar0[R0] << 8;
	exit();
}

/*
 * Release resources.
 * Save u. area for parent to look at.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */
exit()
{
	register int *q, a;
	register struct proc *p;

#ifdef	BETTER_EXIT
	expand(USIZE);
#endif	BETTER_EXIT

#ifdef	TIME_LIMITS
	u.u_procp->p_rtl = 0;	/* disable any clock time interval left */
	u.u_cpusec = 0;
#endif	TIME_LIMITS

	u.u_procp->p_flag =& ~STRC;
	for(q = &u.u_signal[0]; q < &u.u_signal[NSIG];)
		*q++ = 1;
	for(q = &u.u_ofile[0]; q < &u.u_ofile[NOFILE]; q++)
		if(a = *q) {
			*q = NULL;
			closef(a);
		}
	plock(u.u_cdir);				/* fix025 */
	iput(u.u_cdir);
	xfree();
#ifndef	ZOMBIE
	a = malloc(swapmap, 1);
	if(a == NULL)
		panic("out of swap");
	p = getblk(swapdev, a);
	bcopy(&u, p->b_addr, 256);
	bwrite(p);
#endif
	q = u.u_procp;
	mfree(coremap, q->p_size, q->p_addr);
#ifndef	ZOMBIE
	q->p_addr = a;
#endif
	q->p_stat = SZOMB;
#ifdef	ZOMBIE
	q->pz_ur0 = u.u_arg[0];
	q->pz_utime = u.u_utime + u.u_cutime;
	q->pz_stime = u.u_stime + u.u_cstime;
#endif

#ifndef	INIT_FIX
loop:
#ifndef	MAX_PROC
	for(p = &proc[0]; p < &proc[NPROC]; p++)
#else
	for(p = &proc[0]; p <= max_proc; p++)
#endif	MAX_PROC
	if(q->p_ppid == p->p_pid) {
		wakeup(&proc[1]);
		wakeup(p);
#ifndef	MAX_PROC
		for(p = &proc[0]; p < &proc[NPROC]; p++)
#else
		for(p = &proc[0]; p <= max_proc; p++)
#endif	MAX_PROC
		if(q->p_pid == p->p_ppid) {
			p->p_ppid  = 1;
			if (p->p_stat == SSTOP)
				setrun(p);
		}
		swtch();
		/* no return */
	}
	q->p_ppid = 1;
	goto loop;
#endif

#ifdef	INIT_FIX
#define ORPHAN		0100000

	a = ORPHAN;
#ifndef	MAX_PROC
	for(p = &proc[0]; p < &proc[NPROC]; p++)
#else
	for(p = &proc[0]; p <= max_proc; p++)
#endif	MAX_PROC
	{
		if(q->p_pid == p->p_ppid)	/* child */
		{
			p->p_ppid = 1;
			if(p->p_stat == SZOMB)
				a++;
			else if(p->p_stat == SSTOP)
				setrun(p);
		}
		else if(q->p_ppid == p->p_pid)	/* parent */
		{
#ifdef	PROCESS_QUEUES
			if( p->p_stat == SSLEEP && p->p_wchan == p.integ )
#else
			if( p->p_stat == SWAIT && p->p_wchan == p.integ )
#endif	PROCESS_QUEUES
				setrun(p);	/* don't need wakeup */
			a =& ~ORPHAN;
		}
	}
	if(a)
	{
		if(a & ORPHAN)
			q->p_ppid = 1;
		p = &proc[1];
#ifdef	PROCESS_QUEUES
		if( p->p_stat == SSLEEP && p->p_wchan == p.integ )
#else
		if( p->p_stat == SWAIT && p->p_wchan == p.integ )
#endif	PROCESS_QUEUES
			setrun(p);
	}
	swtch();
#endif
}

/*
 * Wait system call.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * Look also for stopped (traced) children,
 * and pass back status from them.
 */
wait()
{
	register f;
#ifndef	ZOMBIE
	register *bp;
#endif
	register struct proc *p;
#ifdef	ZOMBIE
	register  pid;

	pid = u.u_procp->p_pid;
#endif

	f = 0;

loop:
#ifndef	MAX_PROC
	for(p = &proc[0]; p < &proc[NPROC]; p++)
#else
	for(p = &proc[0]; p <= max_proc; p++)
#endif	MAX_PROC
#ifdef	ZOMBIE
	if ( p->p_ppid == pid )  {
#endif
#ifndef	ZOMBIE
	if(p->p_ppid == u.u_procp->p_pid) {
#endif
		f++;
		if(p->p_stat == SZOMB) {
#ifdef	AUSAML
			if( u.u_procp->p_pid == 1)
			{
#ifdef	_1170 | _1145
				copyout(p, u.u_ar0[R0], sizeof proc[0], SEG_USD);
				if( p->p_lnode )
				 copyout( p->p_lnode , u.u_ar0[R0] + sizeof proc[0] , sizeof lnode[0] , SEG_USD );
#endif	_1170 | _1145
#ifndef	_1170 | _1145
				copyout(p, u.u_ar0[R0], sizeof proc[0]);
				if( p->p_lnode )
				 copyout( p->p_lnode , u.u_ar0[R0] + sizeof proc[0] , sizeof lnode[0] );
#endif	_1170 | _1145
			}
			if( p->p_lnode ) p->p_lnode->l_refcount--; /* one less user proc */
#endif	AUSAML
			u.u_ar0[R0] = p->p_pid;
#ifndef	ZOMBIE
			bp = bread(swapdev, f=p->p_addr);
			mfree(swapmap, 1, f);
#endif
			p->p_stat = NULL;
			p->p_pid = 0;
			p->p_ppid = 0;
			p->p_sig = 0;
			p->p_ttyp = 0;
			p->p_flag = 0;
#ifdef	MAX_PROC
			if( p >= max_proc ) {
				max_proc = p;
				while( (--max_proc)->p_stat == NULL );
			}
#endif	MAX_PROC
#ifndef	ZOMBIE
			p = bp->b_addr;
			u.u_cstime =+ p->u_cstime + p->u_stime ;  /* fix000 */
			u.u_cutime =+ p->u_cutime + p->u_utime ;  /* fix000 */
			u.u_ar0[R1] = p->u_arg[0];
			brelse(bp);
#endif
#ifdef	ZOMBIE
			u.u_cstime =+ p->pz_stime;
			u.u_cutime =+ p->pz_utime;
			u.u_ar0[R1] = p->pz_ur0;
#endif
			p->p_wchan = 0;			/* fix025 */
			return;
		}
		if(p->p_stat == SSTOP) {
			if((p->p_flag&SWTED) == 0) {
				p->p_flag =| SWTED;
				u.u_ar0[R0] = p->p_pid;
				u.u_ar0[R1] = (p->p_sig<<8) | 0177;
				return;
			}
			p->p_flag =& ~(STRC|SWTED);
			setrun(p);
		}
	}
	if(f) {
		sleep(u.u_procp, PWAIT);
		goto loop;
	}
	u.u_error = ECHILD;
}

/*
 * fork system call.
 */
fork()
{
#ifdef	LARGE_FILE_REFERENCES
	{
		register i;
		register struct file *fp;
		register count;
	
		for(i=0 ; i<NOFILE ; i++) {
			if ((fp = u.u_ofile[i]) != NULL) {
				count = fp->f_count&0377;
				if ( count > (0377-NOFILE) )  {
					u.u_error = ENREF;
					goto out;
				}
			}
		}
	}
#endif	LARGE_FILE_REFERENCES
#ifdef	SWAP_CHECK
	{
		register char	*a;
		register	largest = maxmem >> 3;

		if ( a = malloc( swapmap, largest ) )
			mfree( swapmap, largest, a );
		else
		{
			uprints("\nswap device full\n");
			u.u_error = EAGAIN;
			goto out;
		}
	}
#endif	SWAP_CHECK

	{
		register struct proc *p1, *p2;
	
		p1 = u.u_procp;
		for(p2 = &proc[0]; p2 < &proc[NPROC]; p2++)
			if(p2->p_stat == NULL)
				goto found;
		u.u_error = EAGAIN;
		goto out;
	
found:
#ifdef	AUSAML
	{
		register struct lnode *l;

		if( l = u.u_procp->p_lnode ) {
			/* limit structure exists */
			if( l->l_plimit && l->l_refcount >= l->l_plimit ) {
				/* too many processes */
				u.u_error = EPROCLIM;
				goto out;
			}
			l->l_refcount++;
		}
	}
#endif	AUSAML
		if(newproc()) {
			u.u_ar0[R0] = p1->p_pid;
			u.u_cstime = 0;	/* fix000 */
			u.u_stime = 0;	/* fix000 */
			u.u_cutime = 0;	/* fix000 */
			u.u_utime = 0;	/* fix000 */
#ifdef LOCKING
			/* ensure that children never inherit a lock on a file */
			u.u_locki = NULL;
#endif
			return;
		}
		u.u_ar0[R0] = p2->p_pid;
	
	}
out:
	u.u_ar0[R7] =+ 2;
}

/*
 * break system call.
 *  -- bad planning: "break" is a dirty word in C.
 */
sbreak()
{
	register a, n, d;
	int i;

	/*
	 * set n to new data size
	 * set d to new-old
	 * set n to new total size
	 */

	n = (((u.u_arg[0]+63)>>6) & 01777);
	if(!u.u_sep)
		n =- nseg(u.u_tsize) * 128;
	if(n < 0)
		n = 0;
	d = n - u.u_dsize;
	n =+ USIZE+u.u_ssize;
	if(estabur(u.u_tsize, u.u_dsize+d, u.u_ssize, u.u_sep))
		return;
	u.u_dsize =+ d;
	if(d > 0)
		goto bigger;
	a = u.u_procp->p_addr + n - u.u_ssize;
	i = n;
	n = u.u_ssize;
	while(n--) {
		copyseg(a-d, a);
		a++;
	}
	expand(i);
	return;

bigger:
#	ifdef	SWAP_CHECK
	i = maxmem >> 3;
	if ( a = malloc( swapmap, i ) )
		mfree( swapmap, i, a );
	else
	{
		u.u_error = ENOMEM;
		u.u_dsize =- d;
		return;
	}
#	endif	SWAP_CHECK
	expand(n);
	a = u.u_procp->p_addr + n;
	n = u.u_ssize;
	while(n--) {
		a--;
		copyseg(a-d, a);
	}
	while(d--)
		clearseg(--a);
}
#ifdef	GPROCS
/*
 *	gprocs - return process table to user
 */
gprocs()
{
	register p, i;

	if( (p = u.u_ar0[R0]) & 1 ){
		u.u_error = EINVAL;	/* odd address */
		return;
	}

#ifndef	MAX_PROC
	if(p)
#ifndef	_1170 | _1145
		copyout(&proc[0], p, sizeof proc );
#else
		copyout(&proc[0], p, sizeof proc, SEG_USD );
#endif	_1170 | _1145
	u.u_ar0[R0] = NPROC;

#else
	i = max_proc - &proc[0] + 1;
	if(p)
#ifndef	_1170 | _1145
		copyout(&proc[0], p, sizeof proc[0] * i);
#else
		copyout(&proc[0], p, sizeof proc[0] * i, SEG_USD);
#endif	_1170 | _1145
	u.u_ar0[R0] = i;
#endif	MAX_PROC
}
#endif
