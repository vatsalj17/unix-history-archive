#include "sys/param.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/proc.h"
#include "sys/stream.h"
#include "sys/stat.h"
#include "sys/conf.h"
#include "sys/label.h"

static struct inode *mkpipend();
extern struct streamtab nilinfo;

#define	PIPFSTYP	6		/* until we do it right */

/*
 * The sys-pipe entry.
 * Allocate 2 open inodes, stream them, and splice them together
 */
pipe()
{
	struct inode *ip1, *ip2;

	if (makepipe(&ip1, &ip2)) {
		u.u_r.r_val1 = f_open(-1, ip1, FREAD|FWRITE);
		if(u.u_r.r_val1 == -1) {
			iput(ip1);
			iput(ip2);
			return;
		}
		u.u_r.r_val2 = f_open(-1, ip2, FREAD|FWRITE);
		if(u.u_r.r_val2 == -1) {
			f_close(u.u_r.r_val1);
			iput(ip2);
			return;
		}
	}
}

/*
 * also called by connld, unfortunately
 */
makepipe(ip1, ip2)
register struct inode **ip1, **ip2;
{
	register struct inode *i1, *i2;

	if (nfstyp <= PIPFSTYP || fstypsw[PIPFSTYP] == NULL) {	/* config error */
		u.u_error = ENODEV;
		return (0);
	}
	i1 = mkpipend();
	i2 = mkpipend();
	if (i1==NULL || i2==NULL) {
		if (i1)
			iput(i1);
		if (i2)
			iput(i2);
		return(0);
	}
	jfree(i2->i_lab);
	i2->i_lab = labDUP(i1->i_lab);

	qdetach(RD(i1->i_sptr->wrq->next), 1);
	i1->i_sptr->wrq->next = RD(i2->i_sptr->wrq);
	qdetach(RD(i2->i_sptr->wrq->next), 1);
	i2->i_sptr->wrq->next = RD(i1->i_sptr->wrq);
	*ip1 = i1;
	*ip2 = i2;
	return(1);
}

static struct inode *
mkpipend()
{
	register struct inode *ip;

	if ((ip = iuniq(PIPFSTYP)) == NULL)
		return(NULL);
	if ((ip = stopen(&nilinfo, NODEV, 0, ip)) == NULL)
		return(NULL);
	ip->i_flag |= IOPEN;		/* white lie */
	prele(ip);
	return(ip);
}

/*
 * pipe file system:
 * exists just so pipe creation is fast
 * inodes are in-core only
 * must be configured, but can't be mounted,
 * which is a little unfortunate
 * only stat exists
 */

static int pipstat();
struct fstypsw pipfs =
	fsinit(nulldev, nulldev, nodev, nodev, nodev, pipstat,
	nullnami, nodev, nodev, nullopen, nodev);

static
pipstat(ip, ub)
	register struct inode *ip;
	struct stat *ub;
{
	struct stat ds;

	ds.st_dev = ip->i_dev;
	ds.st_ino = ip->i_number;
	ds.st_mode = ip->i_mode;
	ds.st_nlink = 0;
	ds.st_uid = ip->i_uid;
	ds.st_gid = ip->i_gid;
	ds.st_size = 0;
	ds.st_atime = time;
	ds.st_mtime = time;
	ds.st_ctime = time;
	if (copyout((caddr_t)&ds, (caddr_t)ub, sizeof(ds)) < 0)
		u.u_error = EFAULT;
}

/*
 * if a pipe end, return other end
 */
struct inode *
otherend(ip)
register struct inode *ip;
{
	register struct queue *qp;
	register struct stdata *sp;

	if(ip == NULL || ip->i_fstyp != PIPFSTYP) return NULL;
	
	sp = ip->i_sptr;
	if(sp == NULL)
		return NULL;

	for(qp=sp->wrq; qp && qp->next; qp=qp->next)
		;
	if(qp == NULL) return NULL;
	sp = (struct stdata *)qp->ptr;
	if(sp == NULL) return NULL;
	return sp->inode;
}
