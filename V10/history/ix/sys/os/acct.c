/*	acct.c	4.4	81/03/08	*/

#include "sys/param.h"
#include "sys/systm.h"
#include "sys/acct.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/proc.h"	/* for p_lab, etc */
#include "sys/label.h"
#include "sys/log.h"

struct	inode *acctp;

/*
 * Perform process accounting functions.
 */
sysacct()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
	} *uap;

	if(T_LOG & ~ jl_t(u.u_procp->p_lab))
		u.u_error = EPRIV;
	if(loggable(LL))
		seclog(LOG_LOG, 130, "C");
	if(u.u_error)
		return;
	uap = (struct a *)u.u_ap;
	if (suser(0) == 0)
		return;
	if (uap->fname==NULL) {
		if (ip = acctp) {
			plock(ip);
			iput(ip);
			acctp = NULL;
		}
		return;
	}
	if (acctp) {
		u.u_error = EBUSY;
		return;
	}
	ip = namei(uap->fname, SEGUDATA, &nilargnamei, 1);
	if(ip == NULL)
		return;
	if(loggable(LL))
		seclog(LOG_LOG, 130, "CI", ip);
	if((ip->i_mode & IFMT) != IFREG) {
		u.u_error = EACCES;
		iput(ip);
		return;
	}
	if (access(ip, IWRITE)) {
		iput(ip);
		return;
	}
	acctp = ip;
	prele(ip);
}

extern short nilpofile;
struct	acct acctbuf;
/*
 * On exit, write a record on the accounting file.
 */
acct()
{
	register i;
	register struct inode *ip;
	off_t siz;
	register struct acct *ap = &acctbuf;

	if ((ip=acctp)==NULL)
		return;
	plock(ip);
	for (i=0; i<sizeof(ap->ac_comm); i++)
		ap->ac_comm[i] = u.u_comm[i];
	ap->ac_utime = compress((long)u.u_vm.vm_utime);
	ap->ac_stime = compress((long)u.u_vm.vm_stime);
	ap->ac_etime = compress((long)(time - u.u_start));
	ap->ac_btime = u.u_start;
	ap->ac_uid = u.u_ruid;
	ap->ac_gid = u.u_rgid;
	ap->ac_mem = 0;
	if (i = u.u_vm.vm_utime + u.u_vm.vm_stime)
		ap->ac_mem = (u.u_vm.vm_ixrss + u.u_vm.vm_idsrss) / i;
	ap->ac_io = compress((long)(u.u_vm.vm_inblk + u.u_vm.vm_oublk));
	ap->ac_tty = u.u_ttyino;
	ap->ac_flag = u.u_acflag;
	siz = ip->i_size;
	u.u_offset = ltoL(siz);
	u.u_base = (caddr_t)ap;
	u.u_count = sizeof(acctbuf);
	u.u_segflg = SEGSYS;
	u.u_error = 0;
	u.u_pofilep = &nilpofile;
	writei(ip);
	if(u.u_error)
		ip->i_size = siz;
	prele(ip);
}

/*
 * Produce a pseudo-floating point representation
 * with 3 bits base-8 exponent, 13 bits fraction.
 */
compress(t)
register long t;
{
	register exp = 0, round = 0;

	while (t >= 8192) {
		exp++;
		round = t&04;
		t >>= 3;
	}
	if (round) {
		t++;
		if (t >= 8192) {
			t >>= 3;
			exp++;
		}
	}
	return((exp<<13) + t);
}
