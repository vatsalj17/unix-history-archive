/*
 * A clist structure is the head
 * of a linked list queue of characters.
 * The characters are stored in 4-word
 * blocks containing a link and ***12*** characters.
 * The routines getc and putc (cio.c)
 * manipulate these structures.
 */
struct clist
{
	int	c_cc;		/* character count */
	int	c_cf;		/* pointer to first block */
	int	c_cl;		/* pointer to last block */
};

/*
 * The structure of a character block in a clist
 */
struct cblock {
	struct cblock *c_next;
	char info[12];		/***/
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
	char	t_state;	/* internal state, not visible externally */
	char	t_char;		/* character temporary */
	int	t_speeds;	/* output+input line speed */
	int	t_dev;		/* device name */
};

extern char partab[];		/* ASCII table: parity, character class */

#define	TTIPRI	10
#define	TTOPRI	20

#define	CERASE	'#'		/* default special characters */
#define	CEOT	004
#define	CKILL	'@'
#define	CQUIT	034		/* FS, cntl shift L */
#define	CINTR	0177		/* DEL */

/* limits */
#define	TTHIWAT	100
#define	TTLOWAT	60
#define	TTYHOG	256

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
#define	TBDELAY	006000
#define	CRDELAY	030000
#define	VTDELAY	040000
#define BREAK	(01<<16)	/* Use BREAK for interrupt, not RUBOUT */

/* Baud rates */
#define	B300	7
#define	B600	8
#define	B1200	9
#define	B1800	10
#define	B2400	11
#define	B4800	12
#define	B9600	13

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
#define SUSPEND	0200		/*** suspend transmission (buffer full) ***/
