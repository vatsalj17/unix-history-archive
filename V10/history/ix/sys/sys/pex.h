/*
 * bits in i_pexflag
 */
/* #define POISON 3 /* in log.h */
#define PEX_STATE	12		/* this end's state */
# define	PEX0	 0			/* unpexed */
# define	PEX1	 4			/* fully pexed */
# define	PEX2	 8			/* becomming unpexed */
#define PEX_IMPURE	16		/* pipe ends differ */
#define PEX_SLEEP	32		/* this end sleeps */
#define PEX_APX		64		/* this device auto-pexable */
/* #define ISLOGFILE	128 /* in log.h */

/*
 * process-exclusive access: FIOPX, FIONPX, FIOQX
 */
struct pexclude {
	int oldnear;	/* FIOPX or FIONPX: state at begining of call */
	int newnear;	/* FIOPX or FIONPX: state at end of call */
	int farpid;	/* -1 if not pipe, 0 is not restricted, >0 if restr */
	int farcap;	/* if farpid>0, capabilities of far process; see getplab(2) */
	int faruid;	/* if farpid>0, uid of far process */
};

#define FIONBUF		(('f'<<8)|99)
#define	FIOPX		(('f'<<8)|101)
#define	FIONPX		(('f'<<8)|102)
#define	FIOQX		(('f'<<8)|103)
#define	FIOAPX		(('f'<<8)|104)
#define	FIOANPX		(('f'<<8)|105)
