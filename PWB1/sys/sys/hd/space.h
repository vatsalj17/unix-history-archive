#include "../hd/buf.h"
struct	buf	buf[NBUF];	/* buffer headers */
struct	buf	bfreelist;	/* head of the free list of buffers */
char	buffers[NBUF][514];	/* buffer pool */

#include "../hd/file.h"
struct	file	file[NFILE];	/* file table */

#include "../hd/inode.h"
struct	inode	inode[NINODE];	/* inode table */

#include "../hd/proc.h"
struct	proc	proc[NPROC];	/* process table */

#include "../hd/text.h"
struct	text text[NTEXT];	/* text table */

#include "../hd/systm.h"
struct map coremap[CMAPSIZ];
struct map swapmap[SMAPSIZ];
struct callo callout[NCALL];
struct mount mount[NMOUNT];

struct	cblock	cfree[NCLIST];

#include "../hd/var.h"
struct var v {
	NBUF,
	NCALL,
	NINODE,
	&inode[NINODE],
	NFILE,
	&file[NFILE],
	NMOUNT,
	&mount[NMOUNT],
	NPROC,
	&proc[1],
	NTEXT,
	&text[NTEXT],
	NCLIST,
};

char	pwbname[9] "pwbname";
