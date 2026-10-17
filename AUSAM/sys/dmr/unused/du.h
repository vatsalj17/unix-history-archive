/*
 *	DU11 register layout
 */

#define DUADDR		0160040
struct	dureg_read
{
	int	durcsr; 	/* r/w */
	int	durbuf;
	int	dutcsr; 	/* r/w */
};
struct	dureg_write
{
	int	dufill1;
	int	dupsr;
	int	dufill2;
	int	dutbuf;
};

#define DURCSR	DUADDR->durcsr
#define DURBUF	DUADDR->durbuf
#define DUTCSR	DUADDR->dutcsr
#define DUPSR	DUADDR->dupsr
#define DUTBUF	DUADDR->dutbuf

/*
 *	register control bits
 */
#define RING		040000		/* durcsr */
#define CLR_TO_SND	020000
#define CARRIER 	010000
#define REC_ACT 	04000
#define SEC_RCV_DAT	02000
#define DATA_SET_RDY	01000
#define STRIP_SYNC	0400
#define RX_DONE 	0200
#define RX_INTEB	0100
#define DS_INTEB	040
#define SCH_SYNC	020
#define SEC_XMIT	010
#define REQ_TO_SND	4
#define DATA_TERM_RDY	2

#define E_OVR_RUN	040000		/* durbuf */
#define E_FRAME 	020000
#define E_PARITY	010000

#define INT_SYNC	030000		/* dupsr */
#define C8BIT		04000
#define ODD_PAR 	01000
#define EVN_PAR 	01400
#define SYN		026

#define MSTRST		0400		/* duxcsr */
#define TX_DONE 	0200
#define TX_INTEB	0100
#define DNA_INTEB	040
#define SEND		020
#define HALF_DUP	010

struct	{ char lobyte; char hibyte; };

