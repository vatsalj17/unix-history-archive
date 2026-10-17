struct	sgttyb
{
	char	ispeed,
		ospeed,
		erase,
		kill;
	unsigned mode;
};

/* modes */
#define	HUPCL	01
#define	XTABS	02
#define	LCASE	04
#define	ECHO	010
#define	CRMOD	020
#define	RAW	040
#define	ODDP	0100
#define	EVENP	0200
#define	NLDELAY	001400
#define	TBDELAY	002000
#define	INVCASE	004000
#define	CRDELAY	030000
#define	VTDELAY	040000
#define FLOWCNTRL 0100000	/* flow control required */

/* speeds */
#define	B50	1
#define	B75	2
#define	B110	3
#define	B1345	4
#define	B150	5
#define	B200	6
#define	B300	7
#define	B600	8
#define	B1200	9
#define	B1800	10
#define	B2400	11
#define	B4800	12
#define	B9600	13
#define	EXTA	14
#define	EXTB	15
#define	B19200	15
