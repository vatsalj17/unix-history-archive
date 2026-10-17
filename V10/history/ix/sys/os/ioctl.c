#include "sys/param.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "sys/filio.h"
#include "sys/pex.h"

/*
 * ioctl system call
 * Check legality, execute common code, and switch out to individual
 * device routine.
 */
ioctl()
{
	struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		int	cmd;
		caddr_t	cmarg;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = geti(uap->fdes, FREAD|FWRITE, &fp);
	if(ip == NULL)
		return;
	if(ip->i_pex != 0  && ip->i_pex != u.u_procp) {
		u.u_error = ECONC;
		return;
	}
	if(!labIOCTL(uap->cmd, ip))
		return;

	switch (uap->cmd) {

	case FIOCLEX:			/* close on exec */
		u.u_ofile[uap->fdes]->fr_pofile |= EXCLOSE;
		return;

	case FIONCLEX:			/* no close on exec */
		u.u_ofile[uap->fdes]->fr_pofile &= ~EXCLOSE;
		return;

	case FIOPX:			/* only this proc may use it */
	case FIONPX:			/* any proc may use it */
	case FIOQX:			/* query exclusivity */
	case FIOAPX:			/* this file accepts PX ioctls */
	case FIOANPX:			/* this file rejects PX ioctls */
		pexioc(ip, uap->cmd, uap->cmarg, 1);
		return;

	}
	if( ip->i_pexflag&PEX_IMPURE) {
		u.u_error = ECONC;
		return;
	}

	if (ip->i_sptr) {
		stioctl(ip, uap->cmd, uap->cmarg);
		return;
	}
	(*fstypsw[ip->i_fstyp]->t_ioctl)(ip, uap->cmd, uap->cmarg, fp->f_flag);
}
