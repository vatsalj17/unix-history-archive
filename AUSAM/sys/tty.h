/*
 * A clist structure is the head
 * of a linked list queue of characters.
 * The characters are stored in 4-word
 * blocks containing a link and 6 characters.
 * The routines getc and putc (m45.s or m40.s)
 * manipulate these structures.
 */
struct clist
{
	int	c_cc;		/* character count */
	int	c_cf;		/* pointer to first block */
	int	c_cl;		/* pointer to last block */
};

/*
 * A tty structure is needed for
 * each UNIX character device that
 * is used for normal terminal IO.
 * The routines in tty.c handle the
 * common code associated with
 * these structures.
 * The definition and device dependent
 * code is in each driver. (kl.c dc.c dh.c)
 */
struct tty
{
	struct	clist t_rawq;	/* input chars right off device */
	struct	clist t_canq;	/* input chars after erase and kill */
	struct	clist t_outq;	/* output list to device */
	int	t_flags;	/* mode, settable by stty call */
	int	*t_addr;	/* device address (register or startup fcn) */
	char	t_delct;	/* number of delimiters in raw q */
	char	t_col;		/* printing column of device */
	char	t_erase;	/* erase character */
	char	t_kill;		/* kill character */
#ifndef TTY_FLOW_CONTROL
	char	t_state;	/* internal state, not visible externally */
#endif
#ifdef	TTY_FLOW_CONTROL
	char	t_count;	/* count for buffering */
#endif
	char	t_char;		/* character temporary */
	int	t_speeds;	/* output+input line speed */
	int	t_dev;		/* device name */
#ifdef	TTY_FLOW_CONTROL
	unsigned t_state;	/* internal state, not visible externally */
#endif
#ifdef	TTY_CONNECT
	char	t_rtype;	/* type of redirection (see below) */
	char	t_cchar;	/* master control char (breaks connection) */
	struct tty *t_redirect;	/* redirect tty input to this tty */
#ifdef	TTY_MULTIPLEXED_PORTS
	int	t_rcount;	/* current output rate this port */
#endif	TTY_MULTIPLEXED_PORTS
#endif	TTY_CONNECT
};

char partab[];			/* ASCII table: parity, character class */

#define	TTIPRI	10
#define	TTOPRI	20

#define	CERASE	'#'		/* default special characters */
#define	CEOT	004
#define	CKILL	'@'
#define	CQUIT	034		/* FS, cntl shift L */
#define	CINTR	0177		/* DEL */
#ifdef	TTY_FLOW_CONTROL
#define ACK	6		/* Flow control bytes */
#define ETX	3
#define ESC	033		/* Control codes for some terminals */
#define HT	011
#define VT	013
#define RS	036
#define US	037
#endif

/* limits */
#define	TTHIWAT	100
#define	TTLOWAT	50
/*	#define	TTYHOG	300	/* fix041: define to param.h */
#ifdef	TTY_HISPEED
#define TTHSHIWAT 128
#define TTHSLOWAT 64
#endif

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
#ifndef	TTY_INVCASE
#define	TBDELAY	006000
#endif
#ifdef	TTY_INVCASE
#define	TBDELAY	002000
#define	INVCASE	004000
#endif
#define	CRDELAY	030000
#define	VTDELAY	040000
#ifdef	TTY_FLOW_CONTROL
#define FLOWCNTRL 0100000	/* flow control required */
#endif

/* Hardware bits */
#define	DONE	0200
#define	IENABLE	0100

/* Internal state bits */
#define	TIMEOUT	01		/* Delay timeout in progress */
#define	WOPEN	02		/* Waiting for open to complete */
#define	ISOPEN	04		/* Device is open */
#define	SSTART	010		/* Has special start routine at addr */
#define	CARR_ON	020		/* Software copy of carrier-present */
#define	BUSY	040		/* Output in progress */
#define	ASLEEP	0100		/* Wakeup when output done */
#ifdef	TTY_TRUE_RAW
#define RAWSLEEP 0200		/* Wakeup when input ready */
#endif
#ifdef	TTY_FLOW_CONTROL
#define	FCINTFL	04000		/* flow control has been interrupted */
#define ESC2FL	010000		/* 3 byte escape sequence being output */
#define ESCFL	020000		/* Escape sequence being output */
#define ACKFLAG 040000		/* ACK expected */
#define FCSLEEP 0100000		/* Wakeup when ACK received */

/* Flow control is for Diablo type terminals */
#define FCBUFZ	79		/* Diablo buffer is 158 bytes */
#endif

#ifdef	TTY_HISPEED
/* speeds */
#define B300	7		/* 300 bd per DH11 */
#define B1200	9		/* 1200 bd per DH11 */
#endif

#ifdef	TTY_CONNECT
/* modes of tty connection (t_rtype) */
#define	TCONLOG		0001
#ifdef	TTY_SPECIAL_POWERS
#define	TCONVIEW	0002
#define	TCONGRAB	0003
#define	TCONSHARE	0004
#endif	TTY_SPECIAL_POWERS
#ifdef	TTY_MULTIPLEXED_PORTS
#define	TMULTIPLEX	0005
#endif	TTY_MULTIPLEXED_PORTS
#define	TMASTER		0100
#endif	TTY_CONNECT
