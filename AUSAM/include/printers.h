#define PARALLEL 0
#define BAUD2400  ((11 << 8) + 11)
#define BAUD1200  ((9 << 8) + 9)
#define BAUD4800  ((12 << 8) + 12)

#define EVENP	0200
#define CRMOD	020
#define XTABS	02

#define DELNL1	0400
#define DELNL3	01400
#define DELCR2 020000
#define DELFF1	040000

struct printer
{
	int	pagewidth;
	char	translate;	/* whether to translate curly brackets etc */
	char	pagemult;	/* multiplication factor for the page limit */
	int	baudrate;	/* baud rate for  LA180 type printers,
				   0 for parallel interface type things. */
	int	modes;		/* modes for la180 types, e.g. parity. */
}
printer[]
{
#ifdef EECF
	{136, 1, 10, PARALLEL, 0},
	{132, 0, 3, BAUD2400, EVENP | XTABS},
	{132, 0, 3, BAUD2400, EVENP | XTABS},
	{132, 0, 3, BAUD1200, EVENP | XTABS | CRMOD | DELNL3 | DELCR2 | DELFF1 }
#endif EECF
#ifdef AGSM
	{132, 1, 127, PARALLEL},
	{132, 0, 20, BAUD4800, EVENP | XTABS}
#endif AGSM
};
#define NPRINTERS (sizeof printer/sizeof printer[0])
