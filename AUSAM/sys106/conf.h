/*
 * Used to dissect integer device code
 * into major (driver designation) and
 * minor (driver parameter) parts.
 */
struct
{
	char	d_minor;
	char	d_major;
};

/*
 * Declaration of block device
 * switch. Each entry (row) is
 * the only link between the
 * main unix code and the driver.
 * The initialization of the
 * device switches is in the
 * file conf.c.
 */
struct	bdevsw
{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_strategy)();
#ifndef	HASHED_BUFFERS
	int	*d_tab;
#else
	int	d_nodel;	/* if no delayed write on device e.g. tape */
#endif	HASHED_BUFFERS
#ifdef	POWER_FAIL
	int	(*d_bpowf)();
#endif	POWER_FAIL
} bdevsw[];

/*
 * Nblkdev is the number of entries
 * (rows) in the block switch. It is
 * set in binit/bio.c by making
 * a pass over the switch.
 * Used in bounds checking on major
 * device numbers.
 */
int	nblkdev;

/*
 * Character device switch.
 */
struct	cdevsw
{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_read)();
	int	(*d_write)();
	int	(*d_sgtty)();
#ifdef	TTY_CONNECT
	struct tty *d_tty;
#endif	TTY_CONNECT
#ifdef	POWER_FAIL
	int	(*d_cpowf)();
#endif	POWER_FAIL
} cdevsw[];

/*
 * Number of character switch entries.
 * Set by cinit/tty.c
 */
int	nchrdev;
