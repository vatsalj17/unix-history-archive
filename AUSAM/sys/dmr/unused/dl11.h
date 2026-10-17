/*
 * data used in coordinating the KL driver with other drivers on the same controller
 */

#define	FIRSTDL 2	/* first DL11 involved */
#define	NDL	2	/* no. of DL11s involved */

char	dl11[NDL];	/* coordinating status */

#define TTY	1	/* value of dl11 if tty */
#define	BLK	2	/* value of dl11 if block device */
#define	LPR	4	/* value of dl11 if line printer */
#define COM	010	/* value of dl11 if communications protocol being used */

/*
 *	now assign the devices
 */
#define	VS	1	/* selects element of dl11 & dl11v */
#define	VSADDR	0175620	/* interrupt vector at 320 (dl11-d) */
#define	SPLVS	spl6	/* interrupt priority at br6 */

#define BP	1	/* selects element of dl11 */
#define BPADDR	0175620	/* interrupt vector at 320 (dl11-d) */

/* structure for accessing DL registers */
struct
{
	int	dlrs;	/* DL reading status register ( RCSR ) */
	int	dlrb;	/* "	"	buffer ( RBUF ) */
	char	dlws;	/* "  writing status register ( XCSR ) */
	char	dlfill;
	char	dlwb;	/* "	"	buffer ( XBUF ) */
};

/** dlrs status bits **/
#define RDENB		01		/* reader enable		*/
#define	DTR		02		/* data terminal ready		*/
#define	REQ_TO_SND	04		/* request to send		*/
#define	SEC_XMIT	010		/* secondary transmit data	*/
#define	DS_INTEB	040		/* data set interrupt enable	*/
#define IENABLE 	0100		/* receive interrupt enable	*/
#define DONE		0200		/* receiver done		*/
#define	SEC_REC		02000		/* secondary receive data	*/
#define	CARRIER		010000		/* carrier			*/
#define CLR_TO_SND	020000		/* clear to send		*/
#define	RING		040000		/* ringing			*/
#define	DS_INTR		0100000		/* data set interrupt		*/

/* rbuf bits */
#define E_OVRUN		040000
#define E_FRAME		020000	
#define E_PARITY	010000

/*
 *	locations defined in l.s
 */
struct 
{
	int	rcall[2];
	int	(*rintad)();
	int	wcall[2];
	int	(*wintad)();
}
	dl11v[NDL];
