
/*
 * Logging pseudo devices /dev/log
 */

#include "sys/param.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/proc.h"
#include "sys/conf.h"
#include "sys/log.h"

daddr_t devlogdev = makedev(9, 0);
extern short nilpofile;
struct logbuf abuf;
struct inode alog;			/* exclusive use of abuf */
long logtrans;
int lgwrite();
struct cdevsw lgcdev = cdinit(nulldev, nulldev, nodev, lgwrite, nodev);

lgwrite(dev)
{
	register unsigned k;
	unsigned n = u.u_count;
	register struct inode *ip;

	if(major(dev) != major(devlogdev))
		panic("devlogdev");
	k = minor(dev);
	if(k >= lgcnt) {
		u.u_error = ENXIO;
		return;
	}
	if(n>LOGLEN)
		n = LOGLEN;
	copyin(u.u_base, (caddr_t)abuf.body, n);
	abuf.code = LOG_USER;
	abuf.mode = minor(dev);

	u.u_base += u.u_count;
	u.u_offset = Lladd(u.u_offset, u.u_count);
	u.u_count = 0;
	if((ip = lg[k]) != NULL) {
		plock(&alog);
		aflush(ip, n + (&abuf.body[0] - (char *)&abuf));
		prele(&alog);
	}
}



/*
 * write a log record, borrowing user's uarea parameters.
 */
aflush(ip, len)
register struct inode *ip;
{
	int *xp, x[7];
	off_t siz;

	if(ip == NULL)
		return;
	u.u_labcheck |= LABAUDIT;

	abuf.len = len;
	abuf.colon = ':';
	abuf.slug = logtrans++;
	abuf.pid = u.u_procp->p_pid;

	siz = ip->i_size;

	xp = x;
	*xp++ = u.u_offset.lo;
	*xp++ = u.u_offset.hi;
	*xp++ = (int)u.u_base;
	*xp++ = u.u_count;
	*xp++ = u.u_segflg;
	*xp++ = u.u_error;
	*xp = (int)u.u_pofilep;

	u.u_offset = ltoL(siz);
	if(nilpofile != -1){
		printf("nilpofile=%x\n", 0xffff&nilpofile);
		nilpofile = -1;
	}

	u.u_pofilep = &nilpofile;
	u.u_base = (caddr_t)&abuf;
	u.u_count = abuf.len;
	u.u_segflg = SEGSYS;
	u.u_error = 0;

	writei(ip);
	if(u.u_error)
		ip->i_size = siz;

	xp = x;
	u.u_offset.lo = *xp++;
	u.u_offset.hi = *xp++;
	u.u_base = (caddr_t)*xp++;
	u.u_count = *xp++;
	u.u_segflg = *xp++;
	u.u_error = *xp++;
	u.u_pofilep = (short *)*xp;
	u.u_labcheck &= ~LABAUDIT;
}
