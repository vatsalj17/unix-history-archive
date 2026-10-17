/*	c-version of tp?.s
 *
 *	M. Ferentz
 *	August 1976
 *
 * Quick & dirty emergency version for Interdata
 *
 *	Major changes:
 *		- arse-backwards 16-bit integers for PDP-11 compatibility
 *		- 'o' flag: output goes to filename given by next argument
 *			(since Wollongong is too poor to afford a tape drive)
 *					R. Miller
 *					Dec 1977
 * 	For Ian Johnson's AGSM version (48-char pathnames), define
 *	the preprocessor variable:
 *		#define	AGSM	0
 */

#define	MDIRENT	496		/* must be zero mod 8 */
#define DIRSIZE	16

#ifdef	AGSM
#define	NMSIZE	48
#else
#define	NMSIZE	32
#endif


#define	tapeblk	tpentry
#define tapeb	tpentry

EXTERN struct 	tent	{	/* Structure of a tape directory block */
	char	pathnam[NMSIZE];
	char	mode[2];
	char	uid;
	char	gid;
	char	spare;
	char	size0;
	char	size1[2];
	char	time0[2];
	char	time1[2];
	char	tapea[2];	/* tape address */
#ifndef	AGSM
	char	unused[16];
#endif
	char	cksum[2];
}	tpentry[8];

EXTERN struct	dent {	/* in core version of tent with "unused" removed
		 * and pathname replaced by pointer to same in a
		 * packed area (nameblock).
		 */
	char	*d_namep;
	int	d_mode;
	char	d_uid;
	char	d_gid;
	char	d_spare;
	char	d_size0;
	int	d_size1;
	int	d_time[2];
	int	d_tapea;
}  dir[MDIRENT];

EXTERN struct {
	int	s_skip[2];
	int	s_flags;
	char	s_nlinks;
	char	s_uid;
	char	s_gid;
	char	s_size0;
	int	s_size1;
	int	s_addr[8];
	int	s_active[2];
	int	s_modtime[2];
	} statb;

/**/

EXTERN char	map[4096];
EXTERN char	catlb[16];
EXTERN char	name[NMSIZE];
EXTERN char	name1[NMSIZE];
extern char	tc[]	;
extern char	mt[]	;

EXTERN int	narg, rnarg;
EXTERN char	**parg;
EXTERN int	wseeka,rseeka;
EXTERN int	tapsiz;
EXTERN int	fio;
EXTERN int	ndirent, ndentd8;
EXTERN char	*edir;
EXTERN struct dent *lastd;		/* for improvement */
EXTERN int	(*command)();

EXTERN char	*nameblk;
EXTERN char	*top;
EXTERN char	*nptr;

EXTERN int	   flags	;	/* default is flu */
#define	flc	0001
#define	flf	0002
#define	fli	0004
#define	flm	0010
#define	flu	0020
#define	flv	0040
#define	flw	0100

/*
 * PDP-11 arse-backwards integers
 */
#define rab(from)	(from[1]<<8)|from[0]
#define wab(to, from)	to[0] = from&0377; to[1] = from>>8;
#define	NCPW	4
