/*
 *	some definitions for using 16-bit uids under AUSAM
 */

long	getuid();	/* returns real & effective uid as per structure below */

struct
{
	unsigned	eftv_uid;
	unsigned	real_uid;
};

/*
 *	these defines can be used if only the real or effective uid
 *	is required.
 */
#define	REALUID()	getuid()
#define	EFTVUID()	(getuid()>>16)

