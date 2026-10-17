#
/*
 * Everything in this file is a routine implementing a system call.
 */

#include "param.h"
#include "user.h"
#include "reg.h"
#include "inode.h"
#include "systm.h"
#include "proc.h"

getswit()
{

	u.u_ar0[R0] = SW->integ;
}

gtime()
{

	u.u_ar0[R0] = time[0];
	u.u_ar0[R1] = time[1];
}

stime()
{

	if(suser()) {
		time[0] = u.u_ar0[R0];
		time[1] = u.u_ar0[R1];
		wakeup(tout);
	}
}

setuid()
{
	register uid;

	uid = u.u_ar0[R0].lobyte;
	if(u.u_ruid == uid.lobyte || suser()) {
		u.u_uid = uid;
		u.u_procp->p_uid = uid;
		u.u_ruid = uid;
	}
}

getuid()
{

	u.u_ar0[R0].lobyte = u.u_ruid;
	u.u_ar0[R0].hibyte = u.u_uid;
}

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
	if(n < 0 && !suser())
		n = 0;
	u.u_procp->p_nice = n;
}

/*
 * Unlink system call.
 * panic: unlink -- "cannot happen"
 */
unlink()
{
	register *ip, *pp;

	pp = namei(2);
	if(pp == NULL)
		return;
	prele(pp);
	ip = iget(pp->i_dev, u.u_dent.u_ino);
	if(ip == NULL)
		goto out1;
	if((ip->i_mode&IFMT)==IFDIR && !suser())
		goto out;
	u.u_offset[1] =- DIRSIZ+2;
	u.u_base = &u.u_dent;
	u.u_count = DIRSIZ+2;
	u.u_dent.u_ino = 0;
	writei(pp);
	ip->i_nlink--;
	ip->i_flag =| IUPD;

out:
	iput(ip);
out1:
	iput(pp);
}

chdir()
{
	register *ip;

	ip = namei(0);
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
	iput(u.u_cdir);
	u.u_cdir = ip;
	prele(ip);
}

chmod()
{
	register *ip;

	if ((ip = owner()) == NULL)
		return;
	ip->i_mode =& ~07777;
	ip->i_mode =| u.u_arg[1]&07777;
	ip->i_flag =| IUPD;
	iput(ip);
}

chown()
{
	register *ip;

	if (!suser() || (ip = owner()) == NULL)
		return;
	ip->i_uid = u.u_arg[1].lobyte;
	ip->i_gid = u.u_arg[1].hibyte;
	ip->i_flag =| IUPD;
	iput(ip);
}

ssig()
{
	register a;

	a = u.u_arg[0];
	if(a<=0 || a>=NSIG || a==SIGKIL) {
		u.u_error = EINVAL;
		return;
	}
	u.u_ar0[R0] = u.u_signal[a];
	u.u_signal[a] = u.u_arg[1];
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
	for(p = &proc[0]; p < &proc[NPROC]; p++) {
		if(p == q)
			continue;
		if(a != 0 && p->p_pid != a)
			continue;
		if(a == 0 && (p->p_pgrp != q->p_pgrp || p <= &proc[1]))
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

	for(p = &u.u_utime; p  < &u.u_utime+6;) {
		suword(u.u_arg[0], *p++);
		u.u_arg[0] =+ 2;
	}
}

/*
 * alarm clock signal
 */
alarm()
{
	register c, *p;

	p = u.u_procp;
	c = p->p_clktim;
	p->p_clktim = u.u_ar0[R0];
	u.u_ar0[R0] = c;
}

/*
 * indefinite wait.
 * no one should wakeup(&u)
 */
pause()
{

	for(;;)
		sleep(&u, PSLEP);
}
