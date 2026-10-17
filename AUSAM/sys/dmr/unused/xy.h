/*
**	Xylogic Phoenix 211 Disk Controller
**
**		Control Registers
*/


struct	xyregs
{
	int		xycsr;		/* control & status	*/
	int		xyush;		/* unit, sector, head	*/
	int		xycar;		/* bus address		*/
	int		xywcnt;		/* word count		*/
	int		xycyl;		/* cylinder address	*/
	int		xystat;		/* disk status		*/
	int		xyerr;		/* error		*/
};

#define	XYCSR	XYADDR->xycsr
#define	XYUSH	XYADDR->xyush
#define	XYCAR	XYADDR->xycar
#define	XYWCNT	XYADDR->xywcnt
#define	XYCYL	XYADDR->xycyl
#define	XYSTAT	XYADDR->xystat
#define	XYERR	XYADDR->xyerr


/** Control and Status Register bits **/

#define	GO		01
#define	XY_COM		076
#define	INTEB		0100
#define	READY		0200
#define	SEEKINH		040000
#define	ERRSUM		0100000


/** Commands **/

#define	XY_CLEAR	0		/* clear controller	*/	/* no interrupt */
#define	XY_SEEK		2		/* seek only		*/
#define	XY_READ		4		/* normal read		*/
#define	XY_WRITE	6		/* normal write		*/
#define	XY_FORMAT	010		/* format		*/
#define	XY_FLTCL	022		/* drive fault clear	*/	/* no interrupt */
#define	XY_RTZ		024		/* recalibrate		*/
#define	XY_NOP		036		/* nothing		*/


/** Drive Status Register bits **/

#define	SEEKID		03		/* drive id bits	*/
#define	FSTSKDN		04		/* first other seek bit	*/
#define	ANYSKDN		074		/* any other seek done	*/
#define	DRIVEXI		0100		/* disk drive connected	*/
#define	SEEKDON		0200		/* drive seek completed	*/
#define	DRVSKNG		07400		/* any drives seeking	*/
#define	PAKCHNG		010000		/* disk status change	*/
#define	WRTPROT		020000		/* drv write protected	*/
#define	DRIVRDY		0100000		/* drive ready		*/


/** Error Register bits **/

#define	CYLADER		01		/* cyl address error	*/
#define	SECADER		02		/* sector address error	*/
#define	WRTPRER		04		/* write protect error	*/
#define	DISCOVE		010		/* disk overrun error	*/
#define	NXHD		020		/* non-existent head	*/
#define	NXCYL		040		/* non-existent cyl	*/
#define	NXSEC		0100		/* non-existent sector	*/
#define	DATALT		0200		/* data late error	*/
#define	CRCERR		0400		/* crc error		*/
#define	CMDTIMO		01000		/* command timeout	*/
#define	SEEKERR		02000		/* hard seek error	*/
#define	DRVFLT		04000		/* drive fault		*/
#define	DRVUNR		010000		/* drive not ready	*/
#define	BADSECT		020000		/* bad sector ignored	*/
#define	WRTCHKE		040000		/* write check data err	*/
#define	BUSSERR		0100000		/* Bad address		*/


/** the following errors are deemed logical mistakes **/
#define	PROGERR		(WRTPRER|DISCOVE|NXHD|NXCYL|NXSEC|WRTCHKE)

/** the following errors are deemed unrecoverable **/
#define	HARDERR		(PROGERR|CMDTIMO|SEEKERR|DRVFLT|DRVUNR|BUSSERR)


/** the following write errors must be fixed to avoid generating an unreadable sector **/
#define	REWRITE		(DATALT|BUSSERR)


/** the following errors must be followed by a recalibrate command **/
#define	RECAL		(SEEKERR)


/** the following errors may possibly be recovered by retries **/
#define	RETRYER		(DATALT|CRCERR|CYLADER|SECADER)
