/*  Global Defines  */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

/*  This data relates to the working directory and the portion of that
    directory currently displayed on the screen.
*/

#define mfiles	200		/* Max number of files vsh can handel */
extern int tfiles;		/* Number of files in this directory */

/*  The files of a directory are partitioned into pages.  Each page
    represents a display on the terminal screen.  */

#define	nfpp	20		/* Number of files per page */
#define	mpages	(mfiles/nfpp)	/* Max number of pages */

extern int tpages,		/* Total pages this directory */
cpage,				/* Current page this display */
pageend;			/* Index of last file on this page */

/*  File information about working dir */

extern struct stat wd_stb;		/* its inode */

/* Path name of working directory */
#define MPLEN	STRMAX - 1	/* Max path len (chars) */
#define LPLEN	STRMAX - 20	/* Limit path len */

extern int wdfile;		/* its file number */
extern char wdname [STRMAX];	/* its full path name */

extern struct direct dirbuf [mfiles + 1];	/* its contents */
#define	max_dir_size	(sizeof dirbuf - sizeof dirbuf [0])

extern char *d_namep [mfiles];		/* pointers to all the file names */

/* filename returns a pointer to the file assoc. with arg on cpage */
#define	filename(arg)	(d_namep [arg + cpage * nfpp - nfpp])

extern int dir_status;			/* its logical status */
#define	loaded		0
#define	unloaded	1
#define	toobig		2
#define	protected	3

/*  scratch */
extern struct stat scr_stb;

/* Useful macros */

#define	DOT	"."
#define DOTDOT	".."
#define	SLASH	"/"
#define dirsize sizeof dirbuf[0]
#define	dnamesize	DIRSIZ

#define	ISROOT(arg)	(arg [1] == 0)
