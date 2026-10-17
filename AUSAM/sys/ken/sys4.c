#
/*
 */

/*
 * Everything in this file is a routine implementing a system call.
 */

#include "../defines.h"
#include "../param.h"
#include "../file.h"
#include "../user.h"
#include "../reg.h"
#include "../inode.h"
#ifdef	AUSAML
#include	"../lnode.h"
#endif	AUSAML
#include "../systm.h"
#include "../proc.h"
#include "../text.h"

getswit()
{

	u.u_ar0[R0] = SW->integ;
}

gtime()
{

	u.u_ar0[R0] = time.hiint;	/* fix000 */
	u.u_ar0[R1] = time.loint;	/* fix000 */
}

stime()
{

	if ( suser() )  {
#ifdef	LRU_INODE
		/*
		 * On altering the time zap last inode reference times.
		 */
		register struct inode *ip;
		for(ip=inode;ip!= &inode[NINODE];ip++) ip->i_lrt=0;
#endif	LRU_INODE
		time.hiint = u.u_ar0[R0];	/* fix000 */
		time.loint = u.u_ar0[R1];	/* fix000 */
#ifndef	NEW_SLEEP
		wakeup( &tout );		/* fix000 */
#endif	NEW_SLEEP
	}
#ifdef	ALARM
#ifndef	MAX_PROC
	for(p = &proc[0]; p < &proc[NPROC]; p++)
#else
	for(p = &proc[0]; p <= max_proc; p++)
#endif	MAX_PROC
		if(p->p_stl)
		{
			spl1();
			diff = p->p_stl - newtime;
			if(newtime > 0)		/* leap fwd */
				if(diff <= 0)
					p->p_stl = 1;
				else
					p->p_stl = diff;
			else			/* leap back */
				if(diff > 0177777)
					p->p_stl = 0177777;
				else
					p->p_stl = diff;
			spl0();
		}
#endif	ALARM
}

setuid()
{
	register uid;

#ifndef	AUSAM16
	uid = u.u_ar0[R0].lobyte;
	if(u.u_ruid == uid.lobyte || suser()) {
		u.u_uid = uid;
		u.u_procp->p_uid = uid;
		u.u_ruid = uid;
	}
#endif	AUSAM16
#ifdef	AUSAM16
	uid = u.u_ar0[R0];
	if( u.u_ruid == uid || suser() )
		u.u_uid = u.u_ruid = u.u_procp->p_uid = uid;
#endif	AUSAM16
}

getuid()
{

#ifndef	AUSAM16
	u.u_ar0[R0].lobyte = u.u_ruid;
	u.u_ar0[R0].hibyte = u.u_uid;
#endif	AUSAM16
#ifdef	AUSAM16
	u.u_ar0[R0] = u.u_uid;
	u.u_ar0[R1] = u.u_ruid;
#endif	AUSAM16
}

#ifndef	AUSAM16
#ifdef	GROUP_ACCESS
setgid()
{
	register gid;

	gid = u.u_ar0[R0].lobyte;
	if(u.u_rgid == gid.lobyte || suser()) {
		u.u_gid = gid;
		u.u_rgid = gid;
	}
}

getgid()
{

	u.u_ar0[R0].lobyte = u.u_rgid;
	u.u_ar0[R0].hibyte = u.u_gid;
}
#endif
#endif	AUSAM16

getpid()
{
	u.u_ar0[R0] = u.u_procp->p_pid;
}

sync()
{

	update();
}

nice()
{
	register n;

	n = u.u_ar0[R0];
	if(n > 20)
		n = 20;
#ifndef	MORE_USER_PRIORITIES
	if(n < 0 && !suser())
#else
	if(n < 0)
	{
	    if( !suser())
#endif	MORE_USER_PRIORITIES
		n = 0;
#ifdef	MORE_USER_PRIORITIES
	}
	else
		n =<< 1;
#endif	MORE_USER_PRIORITIES
	u.u_procp->p_nice = n;
}

/*
 * Unlink system call.
 * Hard to avoid races here, especially		fix025
 * in unlinking directories.			fix025
 */
unlink()
{
	register *ip, *pp;
	extern uchar;

	pp = namei(&uchar, 2);
	if(pp == NULL)
		return;
	/*
	 * Check for unlink(".")		fix025
	 * to avoid hanging the iget		fix025
	 */
	if( pp->i_number != u.u_dent.u_ino )		/* fix025 */
		ip = iget(pp->i_dev,u.u_dent.u_ino);	/* fix025 */
	else {						/* fix025 */
		ip = pp;				/* fix025 */
		ip->i_count++;				/* fix025 */
	}						/* fix025 */
	if(ip == NULL)
		goto out1;				/* fix025 */
	if((ip->i_mode&IFMT)==IFDIR && !suser())
		goto out;
	/*
	 * Don't unlink a mounted file.			fix025
	 */
	if( ip->i_dev != pp->i_dev )			/* fix025 */
	{
		u.u_error = EBUSY;			/* fix025 */
		goto out;				/* fix025 */
	}
	if( ip->i_flag & ITEXT && ip->i_nlink == 1 )	/* fix025 */
	{
		u.u_error = ETXTBSY;			/* fix025 */
		goto out;				/* fix025 */
	}
	u.u_offset.loint =- DIRSIZ+2; /* fix000 */
	u.u_base = &u.u_dent;
	u.u_count = DIRSIZ+2;
	u.u_dent.u_ino = 0;
	writei(pp);
	ip->i_nlink--;
	ip->i_flag =| IUPD;

out:
	iput(ip);
out1:							/* fix025 */
	iput(pp);
}

chdir()
{
	register *ip;
	extern uchar;

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	if((ip->i_mode&IFMT) != IFDIR) {
		u.u_error = ENOTDIR;
	bad:
		iput(ip);
		return;
	}
	if(access(ip, IEXEC))
		goto bad;
	prele(ip);				/* fix025 */
	plock(u.u_cdir);			/* fix025 */
	iput(u.u_cdir);
	u.u_cdir = ip;
}

chmod()
{
	register *ip;

	if ((ip = owner()) == NULL)
		return;
	ip->i_mode =& ~07777;
	if (u.u_uid)
		u.u_arg[1] =& ~ISVTX;
#	ifdef	LOCKING
	if ( (ip->i_mode&IFMT) == IFDIR )
		u.u_arg[1] =& ~ILPROTOCOL;
#	endif	LOCKING
	ip->i_mode =| u.u_arg[1]&07777;
	ip->i_flag =| IACC;	/* fix029 */
	iput(ip);
}

chown()
{
	register *ip;

	if (!suser() || (ip = owner()) == NULL)
		return;
#ifndef	AUSAM16
	ip->i_uid = u.u_arg[1].lobyte;
	ip->i_gid = u.u_arg[1].hibyte;
#endif	AUSAM16
#ifdef	AUSAM16
	ip->i_uidl = u.u_arg[1].lobyte;
	ip->i_uidh = u.u_arg[1].hibyte;
#endif	AUSAM16
	ip->i_flag =| IACC;	/* fix029 */
	iput(ip);
}

#ifdef	SMDATE
/*
 * Change modified date of file:
 * Super users only may do this
 * time to r0-r1; sys smdate; file
 */

smdate()
{
	register struct inode *ip;
	long tbuf;	/* fix000 */

	if( !suser() ) return;
	if ((ip = owner()) == NULL)
		return;
	ip->i_flag =| IUPD;
	tbuf.loint = u.u_ar0[R1];	/* fix000 */
	tbuf.hiint = u.u_ar0[R0];	/* fix000 */
	iupdat(ip, tbuf);		/* fix000 */
	ip->i_flag =& ~IUPD;
	iput(ip);
}
#endif

ssig()
{
	register unsigned a;		/* fix042 */

	a = u.u_arg[0];
	if(a >= NSIG || a == SIGKIL)	/* fix042 */
	{
		u.u_error = EINVAL;
		return;
	}
	u.u_ar0[R0] = u.u_signal[a];
	u.u_signal[a] = u.u_arg[1];
#ifdef	IGNORE_SIGNALS
	if ( a < 16 )
	{
		register unsigned *p = &u.u_procp->p_ignsig;
		register b = 1<<a;

		if ( u.u_arg[1] & 1 )
			*p =| b;
		else
			*p =& ~b;
	}
#endif
	if(u.u_procp->p_sig == a)
		u.u_procp->p_sig = 0;
}

kill()
{
	register struct proc *p, *q;
	register a;
	int f;

	f = 0;
	a = u.u_ar0[R0];
	q = u.u_procp;
#ifndef	MAX_PROC
	for(p = &proc[0]; p < &proc[NPROC]; p++) {
#endif	MAX_PROC
#ifdef	MAX_PROC
	for(p = &proc[0]; p <= max_proc; p++) {
#endif	MAX_PROC
		if(p == q)
			continue;
		if(a != 0 && p->p_pid != a)
			continue;
#ifdef	ZOMBIE
		if( p->p_stat == SZOMB )
			continue;
#endif
		if(a == 0 && (p->p_ttyp != q->p_ttyp || p <= &proc[1]))
			continue;
		if(u.u_uid != 0 && u.u_uid != p->p_uid)
			continue;
		f++;
		psignal(p, u.u_arg[0]);
	}
	if(f == 0)
		u.u_error = ESRCH;
}

times()
{
	register *p;

	for(p = &u.u_utime; p  < &u.u_utime+4;) {	/* fix000 */
		suword(u.u_arg[0], *p++);
		u.u_arg[0] =+ 2;
	}
}

profil()
{

	u.u_prof[0] = u.u_arg[0] & ~1;	/* base of sample buf */
	u.u_prof[1] = u.u_arg[1];	/* size of same */
	u.u_prof[2] = u.u_arg[2];	/* pc offset */
	u.u_prof[3] = (u.u_arg[3]>>1) & 077777; /* pc scale */
}

#ifdef	SHARED_DATA
/*	.... and now for something completely different        */


enqueue()
{
	int register x,n;

	n = u.u_ar0[R0];
	while((x = fuword(n)) <= 0 && u.u_error == 0)	/* fix037 */
	{
		sleep(n|1, PPV);
	}
	suword(n, x-1);
}

dequeue()
{
	int register x,n;

	n = u.u_ar0[R0];
	x = fuword(n);
	if(u.u_error == 0)	/* fix037 */
	{
		suword(n, x+1);
		if(x == 0)
			wakeup(n|1);
	}
}
#endif

#ifdef	SETPSW
/*
 *	Return with ps set to supplied value
 *	old ps is pushed on stack.
 *	Only the super user may set certain bits
 */
setpsw()
{
	register m, s;

	m = 0140340;
	if(u.u_uid)
		m = 0177740;
	s = u.u_ar0[RPS];
	u.u_ar0[RPS] = (s&m) | (u.u_arg[0]&~m);
	grow(u.u_ar0[R6] =- 2);
	suword(u.u_ar0[R6], s);
}
#endif

#ifdef	TIME_LIMITS
tlimit()	/* cpu time limit */
{
	register t;

	spl6();
	u.u_tix = HZ;
	t = u.u_cpusec;
	u.u_cpusec = u.u_ar0[R0];
	spl0();
	u.u_ar0[R0] = t;	/* return remaining time */
}

clktim()	/* real time limit */
{
	register unsigned  x;

	spl1();
	x = u.u_procp->p_rtl;
	u.u_procp->p_rtl = u.u_ar0[R0].unsignd;
	spl0();
	u.u_ar0[R0].unsignd = x;
}
#endif

#ifdef	U_LOCK
slockon()	/* lock process in core */
{
	if ( suser() )
		u.u_procp->p_flag =| SLOCK;
}

slockoff()
{
	u.u_procp->p_flag =& ~SLOCK;
}
#endif

#ifdef	U_DELAY
sdelay()	/* user delay function */
{
  u.u_ar0[R0] = delay( u.u_ar0[R0] );
}
#endif

#ifdef	AUSAML
/*
 *	limits system call
 *
 *	sys limits; addr; function
 *
 *	limits( addr , function )
 *	struct lnode *addr;
 *	int function;
 *
 *	function = 0
 *		return own limit structure	(r0==1)
 *		ENOLIM => no limit structure
 *
 *	function = 1
 *		return limit structure associated with l_uid	(r0==1)
 *		ENOLIM => no limit structure
 *
 *	function = 2
 *		return all 'active' limit structures
 *		return r0 == number of active limit structures
 *
 *	function = 3
 *		super-users only
 *		initialize limit structure for l_uid
 *		no limit structure for super-users
 *		ETOOMANYU => no limit structures remaining for allocation.
 *
 *	function = OTHER
 *		EINVAL => illegal argument
 *
 */

limits()
{
	register i;
	register unsigned x;
	register struct lnode *l;
	struct lnode *addr;

	if( u.u_arg[0] & 1 ) {
		u.u_error = EINVAL;
		return;
	}
	addr = u.u_arg[0];	/* lnode buffer address */

	switch( u.u_arg[1] ) {	/* switch on desired function */
	default:
		u.u_error = EINVAL;
		return;
	case 0:
		l = u.u_procp->p_lnode;
		goto singo;
	case 1:
		x = fuword( & addr->l_uid );	/* get desired useruid */
		if( u.u_error )			/* fix037 */
			return;			/* fix037 */
		l = 0;
		for( i=0 ; i<MAXUSERS ; i++ )
			if( (lnode[i].l_refcount) && (lnode[i].l_uid==x) ) {
				l = &lnode[i];
				break;
			}
	singo:
		if( l == 0 ) {
			u.u_error = ENOLIM;
			return;
		}
#ifdef	_1170 | _1145
		copyout( l , addr , sizeof lnode[0] , SEG_USD );
#endif
#ifndef	_1170 | _1145
		copyout( l , addr , sizeof lnode[0] );
#endif
		u.u_ar0[R0] = 1;
		return;
	case 2:
		for( i=x=0 ; i<MAXUSERS ; i++ )
			if( lnode[i].l_refcount ) {
#ifdef	_1170 | _1145
				copyout( &lnode[i] , addr , sizeof lnode[0] , SEG_USD );
#endif
#ifndef	_1170 | _1145
				copyout( &lnode[i] , addr , sizeof lnode[0] );
#endif
				addr++;	/* step to next lnode */
				x++;	/* count how many */
			}
		u.u_ar0[R0] = x;
		return;
	case 3:
		if( !suser() ) return;	/* keep things reasonable */
		if( u.u_procp->p_lnode ) {
			u.u_error = ELIMEXIST;
			return;
		}
		x = fuword( & addr->l_uid );	/* get desired useruid */
		if( u.u_error )			/* fix037 */
			return;			/* fix037 */
		if( x == 0 ) {
			u.u_error = ENOROOTLIM;
			return;
		}
		l = 0;
		for( i=0 ; i<MAXUSERS ; i++ )
			if( lnode[i].l_refcount == 0 ) {
				if ( l == 0 ) l = &lnode[i];
			} else if( lnode[i].l_uid == x ) {
					l = &lnode[i];
					if( ++l->l_refcount >= l->l_plimit ) {
						--l->l_refcount;
						u.u_error = EPROCLIM;
						return;
					}
					goto found;
				}
		if( l == 0 ) {
			u.u_error = ETOOMANYU;
			return;
		}
#ifdef	_1170 | _1145
		copyin( addr , l , sizeof lnode[0] , SEG_USD );
#endif	_1170 | _1145
#ifndef	_1170 | _1145
		copyin( addr , l , sizeof lnode[0] );
#endif	_1170 | _1145
		l->l_refcount = 1;
	found:
		u.u_procp->p_lnode = l;
		u.u_ar0[R0] = 0;
		return;
	}
}
#endif	AUSAML

#ifdef	GETTAB

struct	tabdescr {
	char			*tb_base;	/* base of region */
	unsigned int		tb_size;	/* size of region */
	unsigned int		tb_items;	/* no. of elements in region */
	} tabdescr[] {
		{	proc,	sizeof proc,	NPROC	},
		{	text,	sizeof text,	NTEXT	},
		{	inode,	sizeof inode,	NINODE	},
		{	file,	sizeof file,	NFILE	},
	};

/*
 * Copy various system tables into user buffers
 */
gettable()
{
	register struct	tabdescr	*tbp;
	register char			*p;

	if (u.u_ar0[R0] >= (sizeof tabdescr / sizeof tabdescr[0]))
		goto bad;
	tbp = &tabdescr[u.u_ar0[R0]];
	p = u.u_arg[0];
	if (p != NULL) {
		if (p&01 || p>=(p + u.u_arg[1]) || u.u_arg[1]<tbp->tb_size)
			goto bad;
		if (copyout(tbp->tb_base, p, tbp->tb_size) < 0)
			goto bad;
	}
	u.u_ar0[R0] = tbp->tb_items;
	u.u_ar0[R1] = tbp->tb_base;
	return;

bad:
	u.u_error = EFAULT;
}
#endif	GETTAB
