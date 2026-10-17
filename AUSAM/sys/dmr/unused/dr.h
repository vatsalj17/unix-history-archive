/*
 *	definitions for DR11-K
 */

/*
 * dr11 register layout
 */
struct	dr11_regs
{
	int	drcsr;
	int	drinbuf;
	int	droutbuf;
};

#define	DRCSR		DRADDR->drcsr
#define	DRINBUF		DRADDR->drinbuf
#define	DROUTBUF	DRADDR->droutbuf

/*
 * register control bits
 */
#define	OUTFL		0100000
#define	OUTINTEB	040000
#define	INFL		0200
#define	ININTEB		0100
