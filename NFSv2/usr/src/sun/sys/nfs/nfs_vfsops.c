/*	@(#)nfs_vfsops.c 1.1 85/05/30 SMI	*/

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/vfs.h"
#include "../h/vnode.h"
#include "../h/pathname.h"
#include "../h/uio.h"
#include "../h/socket.h"
#include "../netinet/in.h"
#include "../rpc/types.h"
#include "../rpc/xdr.h"
#include "../rpc/auth.h"
#include "../rpc/clnt.h"
#include "../nfs/nfs.h"
#include "../nfs/nfs_clnt.h"
#include "../nfs/rnode.h"


#ifdef NFSDEBUG
extern int nfsdebug;
#endif

struct vnode *makenfsnode();
int nfsmntno;

/*
 * nfs vfs operations.
 */
int nfs_unmount();
int nfs_root();
int nfs_statfs();
int nfs_sync();

struct vfsops nfs_vfsops = {
	nfs_unmount,
	nfs_root,
	nfs_statfs,
	nfs_sync,
};


/*
 * nfs_mount system call
 * This guy is a bit tricky.  Here is what we do:
 * 1) lookup the directory to be covered (uap->freg).
 * 2) create an rpc conection to the server at uap->addr.
 * 3) get the root vnode of the server and make it the current dir.
 * 4) look up the vnode of the served directory (uap->fspec).
 * 5) restore the original current directory.
 * 6) add a vfs struct to the mounted file system list.
 * 6) release the server root vnode
 *
 * The mess about looking up the served directory starting at the
 * current directory (which is the server's root) allows the client
 * to mount a sub-tree of the server's filesystem.
 */
nfs_mount(uap)
	register struct a {
		struct sockaddr_in *addr;/* file server address */
		fhandle_t *fh;		/* File handle to be mounted */
		char	*freg;		/* File to mount on */
		int	mflag;		/* flags */
		int	hard;		/* hard or soft mount */
	} *uap;
{
	struct vnode *rootvp = NULL;	/* the server's root */
	struct vnode *hidvp = NULL;	/* local vnode to be hidden */
	struct vfs *nfsvfs = NULL;	/* nfs vfs handle */
	struct mntinfo *mi = NULL;	/* mount info, pointed at by vfs */
	struct vattr va;		/* root vnode attributes */
	struct nfsfattr na;		/* root vnode attributes in nfs form */
	struct pathname local;		/* local path (name to hide) */
	struct statfs sb;		/* server's file system stats */
	fhandle_t fh;			/* root fhandle */

	/*
	 * Must be super user
	 */
	if (!suser()) {
		return;
	}

	/*
	 * Get vnode to be covered
	 */
	u.u_error = pn_get(uap->freg, UIOSEG_USER, &local);
	if (u.u_error) {
		return;
	}
	u.u_error = lookuppn(&local, FOLLOW_LINK, (struct vnode **)0, &hidvp);
	if (u.u_error) {
		goto error;
	}
	if (hidvp->v_type != VDIR) {
		u.u_error = ENOTDIR;
		goto error;
	}

	/*
	 * create a mount record
	 */
	mi = (struct mntinfo *)kmem_alloc((u_int)sizeof(*mi));
	mi->mi_refct = 0;
	mi->mi_hard = uap->hard;
	mi->mi_mntno = nfsmntno++;
	u.u_error = copyin((caddr_t)uap->addr, (caddr_t)&mi->mi_addr,
	    sizeof(mi->mi_addr));
	if (u.u_error) {
		goto error;
	}
	mi->mi_addr.sin_family = AF_INET;
	mi->mi_addr.sin_port = htons(NFS_PORT);

	/*
	 * Make a vfs struct for nfs.  We do this here instead of below
	 * because rootvp needs a vfs before we can do a getattr on it.
	 */
	nfsvfs = (struct vfs *)kmem_alloc((u_int)sizeof(*nfsvfs));
	VFS_INIT(nfsvfs, &nfs_vfsops, (caddr_t)mi);

	/*
	 * Make the root vnode
	 */
	u.u_error = copyin((caddr_t)uap->fh, (caddr_t)&fh, sizeof(fh));
	if (u.u_error) {
		goto error;
	}
	rootvp = makenfsnode(&fh, (struct nfsfattr *) 0, nfsvfs);
	if (rootvp->v_flag & VROOT) {
		u.u_error = EBUSY;
		goto error;
	}
	/*
	 * get attributes of the root vnode then remake it to include 
	 * the attributes.
	 */
	u.u_error = VOP_GETATTR(rootvp, &va, u.u_cred);
	if (u.u_error) {
		goto error;
	}
	VN_RELE(rootvp);
	vattr_to_nattr(&va, &na);
	rootvp = makenfsnode(&fh, &na, nfsvfs);
	rootvp->v_flag |= VROOT;
	mi->mi_rootvp = rootvp;

	/*
	 * link vfs structure for nfs into mounted
	 * file systems list.
	 */
	u.u_error = vfs_add(hidvp, nfsvfs, uap->mflag);
	if (u.u_error) {
		goto error;
	}
	vfs_unlock(nfsvfs);
	u.u_error = VFS_STATFS(nfsvfs, &sb);
	if (u.u_error) {
                /*
                 *  remove nfsvfs from mount list before freeing it in
                 *  error: below.  make sure it's locked first
                 *  since vfs_remove will panic if not locked.
                 */
                vfs_lock(nfsvfs);
                vfs_remove(nfsvfs);
		goto error;
	}

	/*
	 * Set filesystem beock size to the smallest of: MAXDATA
	 * (changed to 4096 to fix problems with 8k filesystems),
	 * server's transfer size, server's filesystem block size.
	 * If block size is less than CLBYTES make it bigger.
	 */
	mi->mi_bsize = MIN(va.va_blocksize, 4096);
	if (mi->mi_tsize < CLBYTES) {
		mi->mi_bsize = MIN(mi->mi_bsize, 4096);
	} else {
		mi->mi_bsize = MIN(mi->mi_bsize, mi->mi_tsize);
	}
	nfsvfs->vfs_bsize = mi->mi_bsize;

#ifdef NFSDEBUG
        dprint(nfsdebug, 10, "vfs %x: vnodecov = %x, data = %x\n",
            nfsvfs, nfsvfs->vfs_vnodecovered, nfsvfs->vfs_data);
        dprint(nfsdebug, 10, "hidvp %x: vfsmountedhere %x vfs %x\n",
            hidvp, hidvp->v_vfsmountedhere, hidvp->v_vfsp);
        dprint(nfsdebug, 10, "rootvp %x: vfsmountedhere %x vfs %x\n",
            rootvp, rootvp->v_vfsmountedhere, rootvp->v_vfsp);
#endif

error:
	if (u.u_error) {
		if (mi) {
			kmem_free((caddr_t)mi, (u_int)sizeof(*mi));
		}
		if (nfsvfs) {
			kmem_free((caddr_t)nfsvfs, (u_int)sizeof(*nfsvfs));
		}
		if (hidvp) {
			VN_RELE(hidvp);
		}
		if (rootvp) {
			VN_RELE(rootvp);
		}
	}
	pn_free(&local);
}

#ifdef notneeded
/*
 * Called by vfs_mountroot when nfs is going to be mounted as root
 */
nfs_mountroot()
{

	return(EOPNOTSUPP);
}
#endif

/*
 * vfs operations
 */

nfs_unmount(vfsp)
	struct vfs *vfsp;
{
	struct mntinfo *mi = (struct mntinfo *)vfsp->vfs_data;

#ifdef NFSDEBUG
        dprint(nfsdebug, 4, "nfs_unmount(%x) mi = %x\n", vfsp, mi);
#endif
	xumount(vfsp);	/* remove unused sticky files from text table */
	dnlc_purge();	/* remove dnlc entries for this file sys */
	rflush(vfsp);

	/*
	 * free vnodes held in buffer cache
	 */
	if (mi->mi_refct != 1) {
		rinval(vfsp);
	}
	if (mi->mi_refct != 1 || mi->mi_rootvp->v_count != 1) {
		return (EBUSY);
	}
	VN_RELE(mi->mi_rootvp);
	kmem_free((caddr_t)mi, (u_int)sizeof(*mi));
	vfs_remove(vfsp);
	kmem_free((caddr_t)vfsp, (u_int)sizeof(*vfsp));
	return(0);
}

/*
 * find root of nfs
 */
int
nfs_root(vfsp, vpp)
	struct vfs *vfsp;
	struct vnode **vpp;
{

	*vpp = (struct vnode *)((struct mntinfo *)vfsp->vfs_data)->mi_rootvp;
	(*vpp)->v_count++;
#ifdef NFSDEBUG
        dprint(nfsdebug, 4, "nfs_root(0x%x) = %x\n", vfsp, *vpp);
#endif
	return(0);
}

/*
 * Get file system statistics.
 */
int
nfs_statfs(vfsp, sbp)
register struct vfs *vfsp;
struct statfs *sbp;
{
	struct nfsstatfs fs;
	struct mntinfo *mi;
	fhandle_t *fh;
	int error = 0;

	mi = vftomi(vfsp);
	fh = vtofh(mi->mi_rootvp);
#ifdef NFSDEBUG
        dprint(nfsdebug, 4, "nfs_statfs fh %o %d\n", fh->fh_fsid, fh->fh_fno);
#endif
	error = rfscall(mi, RFS_STATFS, xdr_fhandle,
	    (caddr_t)fh, xdr_statfs, (caddr_t)&fs, u.u_cred);
	if (!error) {
		error = geterrno(fs.fs_status);
	}
	if (!error) {
		mi->mi_tsize = fs.fs_tsize;
		sbp->f_bsize = fs.fs_bsize;
		sbp->f_blocks = fs.fs_blocks;
		sbp->f_bfree = fs.fs_bfree;
		sbp->f_bavail = fs.fs_bavail;
		/*
		 * XXX This is wrong - should be a real fsid
		 */
		bcopy((caddr_t)&fh->fh_fsid, (caddr_t)sbp->f_fsid,
		    sizeof(fsid_t));
	}
#ifdef NFSDEBUG
        dprint(nfsdebug, 5, "nfs_statfs returning %d\n", error);
#endif
	return (error);
}

/*
 * Flush any pending I/O.
 */
int
nfs_sync(vfsp)
	struct vfs * vfsp;
{

#ifdef NFSDEBUG
        dprint(nfsdebug, 5, "nfs_sync %x\n", vfsp);
#endif
	rflush(vfsp);
	return(0);
}
