#include "sys/label.h"
#include "sys/param.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/filsys.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "sys/inode.h"
#include "sys/stream.h"
#include "sys/buf.h"
#include "sys/acct.h"
#include "sys/log.h"
#include "sys/pex.h"
#include "sys/filio.h"	/* for FIONPX */

/*
 * Convert a user supplied
 * file descriptor into a pointer
 * to a file structure.
 * Only task is to check range
 * of the descriptor.
 */

#ifndef getf	/* done inline */

struct file *
getf(f)
register int f;
{
 	register struct fildes *fr;
 
 	if ((unsigned)f >= NOFILE || (fr = u.u_ofile[f]) == NULL) {
		return (NULL);
	}
	return (fr->fr_file);
}

#endif
/*
 * return inode of a system call file descriptor arg
 * does u_labcheck security checking
 * (and some common sanity checking of mode of file descriptor, which
 * is confusing.  mask is FREAD|FWRITE, FREAD|FHUNGUP, or FWRITE|FHUNGUP)
 */
struct inode *
geti(d, mask, pf)
register int mask;
struct file **pf;
{
	register struct file *f;
	register struct inode *ip;

	if((f = getf(d)) != NULL && (ip = f->f_inode) != NULL) {

		switch( mask & f->f_flag ) {
		case 0:
			goto bad;
			break;
		case FHUNGUP:
			if((mask & FREAD) && (++u.u_nbadio > 64)) {
				if (u.u_nbadio > 128)
					psignal(u.u_procp, SIGKILL);
				else
					psignal(u.u_procp, SIGPIPE);
			} else if(mask & FWRITE) {
				u.u_error = EPIPE;
				psignal(u.u_procp, SIGPIPE);
			}
			return NULL;
		}
		if((u.u_labcheck & RCH) && !labRchk(ip))
			return NULL;
		if((u.u_labcheck & WCH) && !labWchk(ip))
			return NULL;
		if(pf != NULL) {
			*pf = f;
			u.u_offset = f->f_offset;
			u.u_checkfd = d;
			u.u_pofilep = &(u.u_ofile[d]->fr_pofile);
		}
		return ip;
	}
bad:
	u.u_error = EBADF;
	return NULL;
}

/*
 * Internal form of close.
 * Decrement reference count on
 * file structure.
 * Decrement reference count on the inode following
 * removal to the referencing file structure.
 */
closef(fp)
register struct file *fp;
{
	struct inode *ip;

	if(fp == NULL)
		return;
	if (fp->f_count > 1) {
		fp->f_count--;
		return;
	}
	ip = fp->f_inode;
	plock(ip);
	iput(ip);
	ffree(fp);
}

/*
 * close the file opened on ip
 * called from iput when last reference is removed;
 * ip is locked, and i_count == 1
 *
 * free files with no links;
 * call device driver for special files
 */

extern int rootfstyp;
extern dev_t rootdev;

closei(ip)
register struct inode *ip;
{
	register dev_t dev;
	register int (*cfunc)();

	if (ip->i_sptr)
		stclose(ip, 1);
	if ((ip->i_flag & IOPEN) == 0)
		return;
	ip->i_flag &=~ IOPEN;
	if(ip->i_fstyp)		/* temporary hack */
		return;
	dev = (dev_t)ip->i_un.i_rdev;
	switch(ip->i_mode & IFMT) {

	case IFCHR:
		cfunc = cdevsw[major(dev)]->d_close;
		break;

	case IFBLK:
		if (rootfstyp == 0 && rootdev == dev)	/* awful hack: root is not open */
			return;
		bflush(dev);
		binval(dev);
		cfunc = bdevsw[major(dev)]->d_close;
		break;

	default:
		return;
	}
	(*cfunc)(dev);
}

/*
 * the default open routine for the file system switch entry `t_open'
 */
struct inode *
nullopen(ip, rw)
	struct inode *ip;
{
	varused(rw);
	return(ip);
}

/*
 * openi called to allow handler
 * of special files to initialize and
 * validate before actual IO.
 * returns the inode,
 * NULL if error.
 * may return an alternate inode,
 * in which event the original has been put.
 *
 * Reeds's ERANGE goo for /dev/fd/x.
 * When called from open1(),
 * the device driver fd.c does f_dup(), openi() returns NULL, and
 * thus open1() does not do f_open().
 *
 * responsibilities of open routines (fs and stream):
 * if an inode is returned, it is unlocked,
 * and any other inode has been put.
 * if an error happens, NULL is returned and ip is put.
 */
struct inode *
openi(ip, rw)
register struct inode *ip;
{
	register dev_t dev;

	dev = (dev_t)ip->i_un.i_rdev;
	if (ip->i_sptr) 		/* stream is attached */
		ip = stopen(cdevsw[major(dev)]->qinfo, dev, rw, ip);
	else
		ip = (*fstypsw[ip->i_fstyp]->t_open)(ip, rw);
	if (ip)
		ip->i_flag |= IOPEN;
	else if (u.u_error == ERANGE) {
		u.u_error = 0;
		return NULL;
	}
	else if (u.u_error == 0)		/* temporary firewall */
		panic("openi");
	return (ip);
}

/*
 * Check mode permission on inode pointer.
 * Mode is READ, WRITE or EXEC.
 * In the case of WRITE, the
 * read-only status of the file
 * system is checked.
 * Also in WRITE, prototype text
 * segments cannot be written.
 * The mode is shifted to select
 * the owner/group/other fields.
 * The super user is granted all
 * permissions.
 */
access(ip, mode)
register struct inode *ip;
{
	register m;

	m = mode;
	if(m == IWRITE) {
		if(ip->i_fstyp==0 && ip->i_un.i_bufp->b_un.b_filsys->s_ronly != 0) {
			u.u_error = EROFS;
			return(1);
		}
		if (ip->i_flag&ITEXT)		/* try to free text */
			xrele(ip);
		if(ip->i_flag & ITEXT) {
			u.u_error = ETXTBSY;
			return(1);
		}
	}
	if(m != IWRITE && u.u_uid == 0)
		return(0);
	if(u.u_uid != ip->i_uid) {
		m >>= 3;
		if(u.u_gid == ip->i_gid)
			goto found;
		m >>= 3;
	}
found:
	if((ip->i_mode&m) != 0)
		return(0);

	u.u_error = EACCES;
	return(1);
}

/*
 * check inode ownership.
 * succeeds if the current user owns the file,
 * or is the super-user.
 */
accowner(ip)
register struct inode *ip;
{

	if(u.u_uid == ip->i_uid)
		return(1);
	if(suser(0))
		return(1);
	return(0);
}

/*
 *  Return 0 if the file isn't open for writing, 1 if it is
 */
writers(ip)
	register struct inode *ip;
{
	register struct file *fp;
 	register struct fildes *fr;
 	
	for (fr = ip->i_fr; fr; fr = fr->fr_ni) {
 		fp = fr->fr_file;
 		if (fp->f_count==0 || fp->f_inode != ip)
 			panic("writers");
 		if (fp->f_flag&FWRITE)
 			return(1);
 	}
	return(0);
}

/*
 *  Control concurrent access to a file.  The possibile types are:
 *  - 1 writer and n readers	(I1WNR)
 *  - 1 writer or n readers	(ILCKD)
 *  The file pointers are searched.  If an open with the given mode
 *  would result in a violation, errno is set to ECONC.
 */
concurrency(ip, mode)
	register struct inode *ip;
	int mode;
{
	switch(ip->i_mode&ICCTYP) {
	case ISYNC:
		if ((mode&FWRITE) && writers(ip))
			u.u_error = ECONC;
		break;
	case IEXCL:
		if ((mode&FWRITE) || writers(ip))
			u.u_error = ECONC;
		break;
	}
}

/*
 * Test if the current user is the  super user.
 * If mask not zero TU security check is obeyed.
 */
suser(mask)
{
	if(u.u_uid == 0) {
		if(u.u_labcheck & (mask? TU : 0)) {
			if(T_UAREA & ~ jl_t(u.u_procp->p_lab)) {
				if(loggable(LX))
					seclog(LOG_UAREA, 20, "C");
				u.u_error = EPRIV;
				return(0);
			}
			if(loggable(LX))
				seclog(LOG_UAREA, 21, "C");
		}
		u.u_acflag |= ASU;
		return(1);
	}
	u.u_error = EPERM;
	return(0);
}

/*
 * Allocate a user file descriptor.
 * New (secure unix) version.  Differences:
 *  1. f_count = 0 not 1.  Up to caller to increment count
 *  2. uses free list for allocation
 */
struct file *filefree;

struct file *
allocfile()
{
	register struct file *fp;

	if(filefree == NULL) {
		tablefull("file");
		return (NULL);
	}
	fp = filefree;
	filefree = (struct file *)fp->f_inode;
	fp->f_count = 0;
	fp->f_offset = ltoL(0);
	fp->f_inode = NULL;
	fp->f_fref = NULL;
	fp->f_lab = labCP(labelbot);
	return(fp);
}
ffree(fp)
struct file *fp;
{
	if(fp == NULL)
		panic("ffree\n");
	fp->f_lab = jfree(fp->f_lab);
	fp->f_count = 0;	/* redundant: thats how we got here */
	fp->f_flag = 0;		/* for pstat */
	fp->f_inode = (struct inode *)filefree;
	filefree = fp;
}

/*
 * Manage cross-reference between files and processes
 * Problem solved:  given an inode or file table entry,
 * what processes have it open?  Each open file in each
 * process has a fildes structure, pointed to by u_ofile[fd].
 * Each one is on 2 doubly linked lists: of other fildes
 * with same inode and others with same file table entry.
 */

static struct fildes	*frfr;
int frdebt;
int frhigh = 0;

frinit()
{
	register struct file *fp;
	register int i;

	frfr = NULL;
	for(i=0; i<fildescnt; i++)
		frfree(&fildes[i]);
	frdebt = 0;

	filefree = NULL;
	for(fp=file; fp<fileNFILE; fp++)
		ffree(fp);
}

frfree(t)
struct fildes *t;
{
	if(t == NULL)
		panic("frfree");
	frdebt--;
	t->fr_ni = frfr;
	t->fr_proc = 0;		/* for pstat */
	frfr = t;
}

struct fildes *
fralloc() {
	register struct fildes *t;
	
	if((t = frfr) == NULL) {
		u.u_error = ENFILE;
		printf("frdebt=%d\n", frdebt);
		panic("no more fildes");
	}
	frfr = frfr->fr_ni;

	if(++frdebt > frhigh) {
		frhigh = frdebt;
		if(frhigh % 100 == 0) 
			printf("frhigh=%d\n", frdebt);
	}
	t->fr_nf = NULL;
	t->fr_bf = NULL;
	t->fr_ni = NULL;
	t->fr_bi = NULL;
	t->fr_np = NULL;
	t->fr_bp = NULL;
	return t;
}

/*
 * Give inode ip to process p as file descriptor fd,
 * thereby creating file structure fp and fildes t.
 * Tell p, fp, and ip about t.
 */
long f_opx, f_opy;

f_open(fd, ip, mode)
register fd;
register struct inode *ip;
{
	register struct file *fp;
	register int d;

	if(u.u_procp == NULL)
		panic("f_open p");
	f_opx++;
	if(ip == NULL) {
		printf("f_open i cannot happen\n");
		u.u_error = EBADF; 	/* ??? */
		return NULL;
	}
	fp = allocfile();
	if(fp == NULL) {
		u.u_error = ENFILE;
		return NULL;
	}
	fp->f_inode = ip;
	fp->f_flag = mode;

	d = f_open1(fd, fp);
	if(d == -1)
		ffree(fp);
	else
		f_opy++;
	return d;
}

/*
 * duplicate descriptor, mask out some pofile bits
 * default T_NOCHK in pofile means "IF process is T_NOCHK, this fd is
 * immune"
 */
f_dup(fd, fr, pomask)
register fd;
register struct fildes *fr;
register pomask;
{
	register d;
	
	if(u.u_procp == NULL)
		panic("fdup p");

	if(fr == NULL)
		return NULL;
	else {
		d = f_open1(fd, fr->fr_file);
		if(d != -1) {
			u.u_ofile[d]->fr_pofile = 
				T_NOCHK | (pomask & fr->fr_pofile);
		}
	}
	return d;
}

/*
 * fd on input: prefered fd, or don't care
 * fd on output: one you got, or error
 */
f_open1(fd, fp)
register fd;
register struct file *fp;
{
	if(fp == NULL)
		return NULL;

	if(fd == -1) {
		register int i;

		for(i=0; i<NOFILE; i++) {
			if(u.u_ofile[i] == NULL) {
				fd = i;
				break;
			}
		}
		if(fd == -1) {
			u.u_error = EMFILE;
			return -1;
		}
	}
	if((unsigned)fd >= NOFILE) {
		u.u_error = EBADF;
		return -1;
	}
	if(u.u_ofile[fd] != NULL)
		f_close(fd);

	u.u_ofile[fd] = f_glue(u.u_procp, fp);

	if(loggable(LD))
		seclog(LOG_OPEN, fd, "2I2", u.u_procp->p_pid, 
			fp->f_inode, fp-file);
	return fd;
}

struct fildes *
f_glue(p, fp)
register struct file *fp;
register struct proc *p;
{
	register struct fildes *t;
	register struct inode *ip;

	if(fp == NULL)
		return NULL;

	t = fralloc();

	t->fr_proc = p;
	t->fr_file = fp;
	t->fr_pofile = T_NOCHK;

	if (t->fr_nf = fp->f_fref)
		t->fr_nf->fr_bf = t;
	fp->f_fref = t;
	fp->f_count++;

	ip = fp->f_inode;
	if (t->fr_ni = ip->i_fr)
		t->fr_ni->fr_bi = t;
	ip->i_fr = t;
	
	if (t->fr_np = p->p_ofile)
		t->fr_np->fr_bp = t;
	p->p_ofile = t;
	
	return t;
}


/*
 * Current process closes file desciptor fd.
 */

f_close(fd)
{
	register struct fildes *b, *n;
	register struct fildes *t;
	register struct file *fp;
	register struct inode *ip;

	t = u.u_ofile[fd];
	if(t == NULL)
		return;
	if(t->fr_proc != u.u_procp) 
		panic("f_close 1");

	u.u_ofile[fd] = NULL;
	if(loggable(LD))
		seclog(LOG_CLOSE, fd, "22", u.u_procp->p_pid, t->fr_pofile);

	/*
	 * adjust the file structure root
	 * and excise from the file structure chain
	 */
	if(fp = t->fr_file) {
		if (fp->f_fref == t)
			fp->f_fref = t->fr_nf;
		
		b = t->fr_bf; 			/* behind, back */
		n = t->fr_nf;			/* next */
		if (b) 
			b->fr_nf = n;
		if (n) 
			n->fr_bf = b;
	} else
		panic("f_close f");

	/*
	 * and the inode
	 */
	if(ip = fp->f_inode) {

		if (ip->i_fr == t)
			ip->i_fr = t->fr_ni;

		b = t->fr_bi;			/* behind , back */
		n = t->fr_ni;			/* next */
		if (b) 
			b->fr_ni = n;
		if (n) 
			n->fr_bi = b;
	} else
		panic("f_close i");

	/*
	 * and from proc chain
	 */
	if(u.u_procp->p_ofile == t)
		u.u_procp->p_ofile = t->fr_np;
	b = t->fr_bp;
	n = t->fr_np;
	if (b)
		b->fr_np = n;
	if (n)
		n->fr_bp = b;

	/*
	 * update access time if needed and if safe
	 * return t to free list
	 * relinquish any exclusive claim if this was p's last close of ip
	 * close file structure
	 */
	if( (t->fr_pofile&DIDREAD) && !(ip->i_flag&IACC) ) {
		if(labLE(u.u_procp->p_lab, ip->i_lab))
			ip->i_flag |= IACC;
	}
	frfree(t);
	if(ip->i_pex == u.u_procp) {
		for(t=ip->i_fr; t; t=t->fr_ni)
			if(t->fr_proc == u.u_procp)
				break;
		if(t == 0)
			pexioc(ip, FIONPX, (caddr_t)0, 0);
	}
	closef(fp);
}

/* compile time sanity check of bits in pofile */

#define PF0 (EXCLOSE|SAFETOREAD|SAFETOWRITE|DIDREAD|DIDWRITE|DIDRAISE)
#define PF1 (T_NOCHK|PF0)
#if PF0&T_NOCHK
T_NOCHK bit multiply booked!
#endif
#if PF1&~0xffff
pofile bits do not fit in short!
#endif
