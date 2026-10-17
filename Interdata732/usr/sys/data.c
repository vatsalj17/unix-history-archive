#

		/*** unix system data areas ***/

#include "param.h"
#include "proc.h"
#include "buf.h"
#include "inode.h"
#include "file.h"
#include "text.h"

/*
 * mask & circular buffer for system trace routine
 */
extern	trmask;		/* now in low.s */
int	trbuff[66]	{ 64<<16 };


struct proc	proc[NPROC];

char	canonb[CANBSIZ];	/* buffer for erase and kill (#@) */
int	coremap[CMAPSIZ];	/* space for core allocation */
int	swapmap[SMAPSIZ];	/* space for swap allocation */
int	*rootdir;		/* pointer to inode of root directory */
int	cputype;		/* type of cpu =40, 45, or 70 */
int	execnt;			/* number of processes in exec */
int	lbolt;			/* time of day in HZ not in time */
int	time[2];		/* time in sec from 1970 */
int	tout[2];		/* time of day of next sleep */
int	csw;			/* copy of display switch value */
/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */
struct	mount
{
	int	m_dev;		/* device mounted */
	int	*m_bufp;	/* pointer to superblock */
	int	*m_inodp;	/* pointer to mounted on inode */
} mount[NMOUNT];
/*
 * The callout structure is for
 * a routine arranging
 * to be called by the clock interrupt
 * (clock.c) with a specified argument,
 * in a specified amount of time.
 * Used, for example, to time tab
 * delays on teletypes.
 */
struct	callo
{
	int	c_time;		/* incremental time */
	int	c_arg;		/* argument to routine */
	int	(*c_func)();	/* routine */
} callout[NCALL];

/*
 * in-core inode pool
 */
struct inode	inode[NINODE];

struct file	file[NFILE];

struct text	text[NTEXT];
int	mpid;			/* generic for unique process id's */
char	runin;			/* scheduling flag */
char	runout;			/* scheduling flag */
char	runrun;			/* scheduling flag */
int	curpri;			/*** more scheduling ***/
int	maxmem;			/* actual max memory per process */
int	updlock;		/* lock for sync */
int	rablock;		/* block to be read ahead */

struct buf	buf[NBUF];	/* buffer headers */
