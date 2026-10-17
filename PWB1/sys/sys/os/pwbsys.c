#
#include "../hd/param.h"
#include "../hd/buf.h"
#include "../hd/filsys.h"
#include "../hd/inode.h"
#include "../hd/systm.h"
#include "../hd/user.h"
#include "../hd/reg.h"
#include "../hd/var.h"

pwbsys()
{
	extern uchar;
	register i;
	register *p;
	register struct buf *bp;

	switch(u.u_arg[0]) {

case 0:		/* uname */
	if (copyout(&pwbname, u.u_ar0[R0], 8))
		u.u_error = EFAULT;
	return;

case 1:		/* udata */
	if (u.u_ar0[R1]) {
		if(suser())
			i = copyin(u.u_ar0[R0],u.u_udata,32); else
			return;
	} else
		i = copyout(u.u_udata,u.u_ar0[R0],32);
	if (i)
		u.u_error = EFAULT;
	return;

case 2:		/* ustat */
	for(i=0; i<v.v_mount; i++) {
		p = &mount[i];
		if(p->m_bufp!=NULL && p->m_dev==u.u_ar0[R1]) {
			p = p->m_bufp->b_addr;
			if(copyout(&p->s_tfree, u.u_ar0[R0], 16))
				u.u_error = EFAULT;
			return;
		}
	}
	u.u_error = EINVAL;
	return;

case 3:		/* utime */
	if (!suser())
		return;
	u.u_dirp = u.u_ar0[R1];
	if ((p = namei(&uchar, 0))==NULL)
		return;
	if (getfs(p->i_dev)->s_ronly) {
		u.u_error = EROFS;
		return;
	}
	i = p->i_number+31;
	bp = bread(p->i_dev, ldiv(i,16));
	copyin (u.u_ar0[R0], bp->b_addr+32*lrem(i,16)+24, 8);
	p->i_flag =& ~(IACC|IUPD);
	bdwrite(bp);
	iput(p);
	return;

default:
	u.u_error = EFAULT;
	}
}
