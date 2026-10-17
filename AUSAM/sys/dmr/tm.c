#
/*
 *	tm11 driver
 *
 *	minor devices 0-7  get rewound on close
 *	minor devices 8-15 don't
 *
 *	indutry standard 2 tape marks for EOF
 *
 *	stty functions for skip forward/reverse file/block
 *			   rewind
 *			   write tape mark
 *			   put drive off line.
 */


/*#define	ERRORS	01		/* set debug bit 0 to enable error reports */
/*#define	TRACE	02		/* set debug bit 1 to enable trace */
#ifdef	ERRORS | TRACE
int	tmdebug;			/* non-zero to debug */
#endif

#include "../defines.h"
#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../buf.h"
#include "../reg.h"

/*
 * Structure of device registers
 */

#define	TMADDR	0172520		/* register base	*/
struct	{
	int	tmer;
	int	tmcs;
	int	tmbc;
	int	tmba;
	int	tmdb;
	int	tmrd;
};
#define	TMER	TMADDR->tmer
#define	TMCS	TMADDR->tmcs
#define	TMBC	TMADDR->tmbc
#define	TMBA	TMADDR->tmba
#define	TMDB	TMADDR->tmdb
#define	TMRD	TMADDR->tmrd

struct	devtab	tmtab;
#ifndef	RAW_BUFFER_POOL
struct	buf	rtmbuf;
#endif

#define	NTM11	1		/* drives on controller	*/
/* #define MANY_DRIVES		/* if NTM11 > 1		*/

/* tmcs */
#define	GO	01		/* hardware go bit 	*/
#define	RCOM	03		/* read command 	*/
#define	WCOM	05		/* write command	*/
#define	WIRG	015		/* write extended gap	*/
#define	IENABLE	0100		/* interrupt enable	*/
#define	CRDY	0200		/* controller ready	*/
#define	UNIT	03400		/* unit select		*/
#define	PCLR	010000		/* power clear		*/
#define	DENS	060000		/* 9-track 800 bpi	*/

/* tmrd */
#define	GAPSD	010000		/* gap slow down	*/

/* tmer */
#define	TUR	01		/* tape driver ready	*/
#define	HARD	0102200		/* hard error		*/
#define	EOF	0040000		/* end-of-file		*/
#define	RWS	02		/* rewind started	*/
#define	WRL	04		/* write locked		*/
#define	TAPSD	010		/* tape settle down	*/
#define	SELR	0100		/* tape unit on line	*/

/* driver states */
#define	SSEEK	1		/* positioning		*/
#define	SIO	2		/* read/write		*/
#define	SBSP	3		/* error recovery BS	*/
#define	SPHYS	4		/* stty functions	*/

/* special commands */
#define	SSFB	011		/* skip forward block	*/
#define	SSRB	013		/* skip reverse block	*/
#define	SSFF	001		/* skip forward file	*/
#define	SSRF	003		/* skip reverse file	*/
#define	SSREW	017		/* rewind on-line	*/
#define	SSWEOF	007		/* write eof		*/
#define	SSOFFL	020000		/* put drive off-line	*/
#define	SSWRL	040000		/* check tape write lock */
#define	SSONLN	0100000		/* check tape on-line	*/

/* drive states */
#define	OPEN	01		/* drive open		*/
#define	HARDE	02		/* drive hard error	*/
#define	LASTW	04		/* driver last command was write */


struct	{
	char	t_status[NTM11];
	unsigned t_blkno[NTM11];
	unsigned t_nxrec[NTM11];
} tm11;


/*
 * Open: check for legal drive number, and
 * already open drive.
 */

tmopen(dev, flag)
{
	register dminor;

	dminor = dev.d_minor & 07;
	if(dminor >= NTM11 || tm11.t_status[dminor]&OPEN)
		u.u_error = ENXIO;
	else 
	{
		tm11.t_status[dminor] = OPEN;
		tmcommand(dev, flag ? SSWRL|SSONLN : SSONLN);
		tm11.t_blkno[dminor] = 0;
		tm11.t_nxrec[dminor] = 0177777;	/* last block written set at 64k */
		if(u.u_error)
			tm11.t_status[dminor] = 0;
	}
}

/*
 * Close: if the last command on the drive was
 * a write, then close off with a WEOF
 */

tmclose(dev, flag)
{
	register dminor;


	dminor = dev.d_minor&07;

	if(tm11.t_status[dminor]&LASTW)  
	{
		tmcommand(dev, SSWEOF, -2);
		if ( dev.d_minor & 010 )
			tmcommand(dev, SSRB, -1);
	}

	if ( (dev.d_minor & 010) == 0 )
		tmcommand(dev, SSREW);

	tm11.t_status[dminor] = 0;
}

/*
 * Strategy: the tm supports three basically distinct types
 * of I/O. As a UNIX style block device, it behaves just like
 * a random access device, with the exception that writes
 * must be sequential, and invalidate what lies beyond them.
 * As a raw device, it can read and write records of any
 * blocksize. It also supports tape positioning functions:
 * skip [forward/reverse] [blocks/files], rewind, and weof.
 */

tmstrategy(bp)
register struct buf *bp;
{
	register unsigned *p;

	if((bp->b_flags&(B_NOTIO|B_PHYS))==0) 	/* i.e. block device */
	{
		p = &tm11.t_nxrec[bp->b_dev.d_minor&07];
		if(*p <= bp->b_blkno) 
		{
			if(*p < bp->b_blkno) 	/* attempt to read/write non-existent block */
			{
				bp->b_flags =| B_ERROR;
				iodone(bp);
				return;
			}

			if(bp->b_flags&B_READ) /* attempt to read block immediately beyond last block written */
			{
				clrbuf(bp);
				iodone(bp);
				return;
			}
		}

		if((bp->b_flags&B_READ)==0)
			*p = bp->b_blkno + 1;
	}

#	ifdef	_1170
	if(bp->b_flags&B_PHYS)
		mapalloc(bp);
#	endif

	bp->av_forw = NULL;

	spl5();

	if(tmtab.d_actf == NULL) 
	{
		tmtab.d_actf = bp;
		tmstart();
	}
	else 
	{
		tmtab.d_actl->av_forw = bp;
	}

	tmtab.d_actl = bp;

	spl0();
}


/*
 * Start: If the requested function is on a drive with
 * a hard error, shoot it down.  turn skip-files into
 * the skip-blocks that the hardware supports. Do positioning
 * for block-type i/o.  Write with extended IRG for
 * crudy tapes.
 */

tmstart()
{
	register struct buf *bp;
	register com;
	register blkno;
	unsigned unit;

loop:
	if((bp = tmtab.d_actf) == NULL)
		return;

	unit = bp->b_dev.d_minor & 07;

	if(tm11.t_status[unit]&HARDE) 
	{
		bp->b_flags =| B_ERROR;
deq:
		tmtab.d_actf = bp->av_forw;
		iodone(bp);
		goto loop;
	}

	tm11.t_status[unit] =& ~LASTW;

	com = (unit << 8) | ((bp->b_xmem&03) << 4) | IENABLE | DENS;

	if(bp->b_flags&B_NOTIO) 
	{
		tmtab.d_active = SPHYS;
		if ((blkno = bp->b_blkno) & (SSWRL|SSONLN)) 
		{
#			ifdef	MANY_DRIVES
			TMCS = (com & (UNIT|IENABLE));
			com = 100;
			do; while(--com);
#			endif	MANY_DRIVES

			if ( (com = TMER) & RWS )
				return;		/* await rewind completed */

			if ( (com&(TUR|SELR))!=(TUR|SELR) || ((com&WRL)&&(blkno&SSWRL)) )
				bp->b_flags =| B_ERROR;
			goto deq;
		}
		if ( blkno & SSOFFL )  
		{
			TMCS = com|GO;
			goto deq;
		}
		if(blkno == SSFF || blkno ==SSRF) 
		{
			com =| 010;
			TMBC = 0;
		}
		else
			TMBC = bp->b_wcount;

		if ( blkno == SSRF || blkno == SSRB )  
		{
			TMCS = (com&03400);
			while ( TMER & TAPSD );
		}

		TMCS = com | blkno;
	}
	else  
	{
		if((bp->b_flags&B_PHYS)==0 && (blkno = tm11.t_blkno[unit] - bp->b_blkno)) 
		{
			tmtab.d_active = SSEEK;

			if(blkno < 0) 
			{
				com =| SSFB;
			}
			else 
			{
				blkno = -blkno;
				if(bp->b_blkno == 0)
					com =| SSREW;
				else 
				{
					com =| SSRB;
				}
				while ( TMER & TAPSD );
			}

			TMBC = blkno;
		}
		else 
		{
			tmtab.d_active = SIO;

			TMBC = (bp->b_wcount << 1);
			TMBA = bp->b_addr;

			if ( bp->b_flags & B_READ )
				com =| RCOM;
			else  
			{
				tm11.t_status[unit] =| LASTW;
				if ( tmtab.d_errcnt )
					com =| WIRG;
				else
					com =| WCOM;
			}
		}

		TMCS = com;
	}
}


/* Interrupt: If there is nothing to do, just return.
 * Fix up soft errors on read and write, continue
 * processing of Skip files.
 * Note that physical limitations mean that a mag-tape
 * could not have more than about 50000 records .
 *
 *	rewritten - Piers Lauder	Jan '78
 */

#define	iotyp	error	

tmintr()
{
	register struct buf *bp;
	register unsigned unit, error;

#	ifdef	TRACE
	if ( tmdebug & TRACE )
		printf( "\nTMER %o CS %o BC %o BA %o DB %o RD %o IOTYP %d\n"
			      ,TMER, TMCS, TMBC, TMBA, TMDB, TMRD, tmtab.d_active
		      );
#	endif

  if ( (bp = tmtab.d_actf) == NULL )	/* nothing doing */
	return;

  unit = bp->b_dev.d_minor & 07;

  bp->b_resid = TMBC;

  if ( TMCS < 0 )	/* error */
  {
	error = TMER;
#	ifdef	ERRORS
	if ( tmdebug & ERRORS )
		printf( "\nTMER %o CS %o BC %o BA %o DB %o RD %o IOTYP %d\n"
			      ,error, TMCS, TMBC, TMBA, TMDB, TMRD, tmtab.d_active
		      );
#	endif

	/*while ( TMRD & GAPSD );*/

	if ( error & HARD )
		tm11.t_status[unit] =| HARDE;
	else
		if ( error & EOF )  
		{
			if ( tmtab.d_active == SPHYS )  
			{
				if (((error = bp->b_blkno) == SSFF) || (error == SSRF) || (error == SSWEOF))
					if ( ++bp->b_wcount < 0 )
					{
						TMBC = 0;
						TMCS =| GO;
						return;
					}
			}
			else
				if ( bp->b_flags & B_PHYS )
					bp->b_resid = bp->b_wcount;	/* indicate EOF by 0 bytes read */
				else
				{
					bp->b_flags =| B_ERROR;		/* indicate EOF by error */
					goto backsp;
				}

			goto done;
		}
		else
			if ((tmtab.d_active == SIO) && (++tmtab.d_errcnt < 10))  
			{
backsp:
				tmtab.d_active = SBSP;
				while ( TMER & TAPSD );
				TMBC = -1;
				TMCS = (unit << 8)|IENABLE|DENS|SSRB;
				if ( bp->b_flags & B_ERROR )
				{
					tmtab.d_actf = bp->av_forw;
					iodone( bp );
				}
				return;
			}

	bp->b_flags =| B_ERROR;
	tmtab.d_active = SIO;
  }

  if ((iotyp = tmtab.d_active) == SSEEK)
	tm11.t_blkno[unit] = bp->b_blkno;
  else
	if ( iotyp != SBSP )  
	{
		tm11.t_blkno[unit]++;
		if ( iotyp == SIO )
			bp->b_resid =>> 1;
done:
		tmtab.d_actf = bp->av_forw;

		iodone( bp );

		tmtab.d_errcnt = 0;
	}

  tmstart();
}


/*
 * TM stty: fetch code and count from user, and pass then
 * into raw buffer header. Obey buffer protocols.
 * Codes:
 *	0	Skip forward file
 *	1	Skip reverse file
 *	2	Clear ( super-users only )
 *	3	Write end-of-file
 *	4	Skip forward block
 *	5	Skip reverse block
 *	6	Drive off line
 *	7	Rewind
 */

tmsgtty(dev, v)
{
	register cnt, com;

	if ( v || (u.u_arg[0] & ~07) )
	{
		u.u_error = ENXIO;
		return;
	}
	com = ((u.u_arg[0] & 07) << 1) | GO;
	if(com == 05) 
	{
		if ( suser() )
			TMCS = PCLR;
		return;
	}
	if (com == 015)
		com = SSOFFL;
	if ((cnt = -u.u_arg[1]) >= 0)
		cnt = -1;
	u.u_ar0[R0] = tmcommand(dev, com, cnt);
}

/*
 *	queue a non-io type request to the tapedrive
 */

tmcommand(dev, com, cnt)
  register com, cnt;
{
	register struct buf *bp;

	if ( u.u_error )
		goto out;

#	ifndef	RAW_BUFFER_POOL
	bp = &rtmbuf;
	while(bp->b_flags&B_BUSY) 
	{
		bp->b_flags =| B_WANTED;
		sleep(bp, PRIBIO);
	}
#	else	RAW_BUFFER_POOL
	bp = getrb(dev , B_NOTIO);
#	endif	RAW_BUFFER_POOL

	bp->b_flags = B_BUSY | B_NOTIO;
	bp->b_dev = dev;
	bp->b_blkno = com;
	bp->b_wcount = cnt;

	tmstrategy(bp);

#	ifndef	RAW_BUFFER_POOL
	spl5();
#	else	RAW_BUFFER_POOL
	spl6();
#	endif	RAW_BUFFER_POOL

	while((bp->b_flags&B_DONE)==0)
		sleep(bp, PRIBIO);

#	ifndef	RAW_BUFFER_POOL
	if(bp->b_flags&B_WANTED)
		wakeup(bp);
	spl0();
	bp->b_flags =& ~(B_BUSY|B_WANTED);
#	else	RAW_BUFFER_POOL
	freerb(bp);
#	endif	RAW_BUFFER_POOL

	geterror(bp);

out:
	if(com == SSFB || com == SSRB)
		return(bp->b_resid);
	return(bp->b_wcount);
}

#ifndef	RAW_BUFFER_POOL
tmread(dev)
{
	physio(tmstrategy, &rtmbuf, dev, B_READ);
}

tmwrite(dev)
{
	physio(tmstrategy, &rtmbuf, dev, B_WRITE);
}
#else	RAW_BUFFER_POOL

tmread(dev)
{
	physio(tmstrategy, 0, dev, B_READ);
}

tmwrite(dev)
{
	physio(tmstrategy, 0, dev, B_WRITE);
}
#endif	RAW_BUFFER_POOL


#ifdef	POWER_FAIL
/*
 *	restart controller after a power restore
 */
tmpowerf()
{
	register char *cp;

  for ( cp=tm11.t_status ; cp < &tm11.t_status[NTM11] ; cp++ )
	*cp =| HARDE;

  tmstart();
}
#endif	POWER_FAIL
