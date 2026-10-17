/*	@(#)param.c 1.1 85/05/30 SMI; from UCB 6.1 83/07/29	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/socket.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/vnode.h"
#include "../h/file.h"
#include "../h/callout.h"
#include "../h/clist.h"
#include "../h/cmap.h"
#include "../h/mbuf.h"
#include "../h/quota.h"
#include "../h/kernel.h"
#include "../ufs/inode.h"
/*
 * System parameter formulae.
 *
 * This file is copied into each directory where we compile
 * the kernel; it should be modified there to suit local taste
 * if necessary.
 *
 * Compiled with -DTIMEZONE=x -DDST=x -DMAXUSERS=xx
 */

#define	HZ 50
int	hz = HZ;
int	tick = 1000000 / HZ;
struct	timezone tz = { TIMEZONE, DST };
#ifdef sun
#define	NPROC (10 + 16 * MAXUSERS)
#undef MAXUPRC
#define	MAXUPRC	(NPROC - 5)
#else
#define	NPROC (10 + 8 * MAXUSERS)
#endif
int	nproc = NPROC;
int	maxuprc = MAXUPRC;
int	ntext = 24 + MAXUSERS;
int	ninode = (NPROC + 16 + MAXUSERS) + 64;
int	nfile = 16 * (NPROC + 16 + MAXUSERS) / 10 + 64;
int	ncallout = 16 + NPROC;
int	nclist = 100 + 16 * MAXUSERS;
int	nport = NPROC / 2;
#ifdef QUOTA
int	nquota = (MAXUSERS * 9)/7 + 3;
int	ndquot = (MAXUSERS*NMOUNT)/4 + NPROC;
#endif

/*
 * These are initialized at bootstrap time
 * to values dependent on memory size
 */
int	nbuf, nswbuf;

/*
 * These have to be allocated somewhere; allocating
 * them here forces loader errors if this file is omitted.
 */
struct	proc *proc, *procNPROC;
struct	text *text, *textNTEXT;
struct	inode *inode, *inodeNINODE;
struct	file *file, *fileNFILE;
struct 	callout *callout;
struct	cblock *cfree;
struct	buf *buf, *swbuf;
short	*swsize;
int	*swpf;
char	*buffers;
struct	cmap *cmap, *ecmap;
#ifdef QUOTA
struct	quota *quota, *quotaNQUOTA;
struct	dquot *dquot, *dquotNDQUOT;
#endif
