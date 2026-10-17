/*
 * code related to mounting and unmounting filesystems
 */

#include "sys/param.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "sys/label.h"
#include "sys/log.h"
#include "sys/proc.h"


/*
 * sys fmount
 * call the filesystem-specific mount routine with
 *	the inode of the device to be mounted
 *	the inode of the mount point, still locked
 *	the flag argument
 * fill in the ceiling label for the fs.
 * (HACK: if fstype has 0200 bit on, 5th arg gives ceiling label)
 */

struct jlabel *labelin();

fmount()
{
	register struct a {
		int fstype;
		int fd;
		char *name;
		int flag;
		struct label *lab;
	} *uap = (struct a *)u.u_ap;
	register int type;
	register struct inode *ip0;
	struct inode *ip, *rip;
	struct jlabel **jpp;

	if(loggable(LX))
		seclog(LOG_FMOUNT, uap->fstype, "C");
	if(T_EXTERN & ~ jl_t(u.u_procp->p_lab)) {
		u.u_error = EPRIV;
		return;
	}
	type = 0177 & uap->fstype;
	if (uap->fstype < 0 || type >= nfstyp || fstypsw[type] == NULL) {
		u.u_error = EINVAL;
		return;
	}
	ip0 = geti(uap->fd, FREAD|FWRITE, (struct file **)NULL);
	if(ip0 == NULL)
		return;
	if ((ip = namei(uap->name, SEGUDATA, &nilargnamei, 1)) == NULL)
		return;
	if(loggable(LX))
		seclog(LOG_FMOUNT, uap->fstype, "II", ip, ip0);
	(*fstypsw[type]->t_mount)(ip0, ip, uap->flag, 1, type);
	if( u.u_error == 0 && (rip = ip->i_mroot) != 0) {
		if(uap->lab && (uap->fstype & 0200)) {
			jfree(rip->i_ceil);
			rip->i_ceil = labelin(uap->lab);
		}
		else if((jpp = fstypsw[type]->t_ceil) != 0) {
			jfree(rip->i_ceil);
			rip->i_ceil = labCPX(*jpp);
		}
		else if(rip->i_ceil == NULL)
			rip->i_ceil = labCP(labelyes);
	}
	iput(ip);
}

/*
 * sys funmount
 * call the fs-particular unmount routine with
 * the inode of the mount point
 */

funmount()
{
	struct a {
		char *name;
	} *uap = (struct a *)u.u_ap;
	struct inode *ip, *mip;

	if(loggable(LX))
		seclog(LOG_FMOUNT, -1, "C");
	if ((ip = namei(uap->name, SEGUDATA, &nilargnamei, 1)) == NULL)
		return;
	if ((mip = ip->i_mpoint) == NULL
	||  mip->i_mroot != ip) {	/* sanity, also catches root */
		iput(ip);
		u.u_error = EINVAL;
		return;
	}
	if(loggable(LX))
		seclog(LOG_FMOUNT, -1, "I", ip);
	iput(ip);	/* bag this? */
	(*fstypsw[ip->i_fstyp]->t_mount)((struct inode *)NULL, mip, 0, 0, ip->i_fstyp);
}
