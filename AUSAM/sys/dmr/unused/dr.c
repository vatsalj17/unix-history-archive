#
/*
 *	Driver for DR11-K as a character file
 */

#include	"../defines.h"
#include	"../param.h"
#include	"../user.h"
#include	"../tty.h"

#define	HIWAT		50	/* high water mark for i/o queues */
#define	DRINPRI		8	/* priority while waiting for input */
#define	DROUPRI		9	/* priority while waiting to output */

#define	DRADDR	0167770	/* first device address */
#define	spldr	spl4	/* BR level */

#include	"dr.h"

/*
 * flag bits
 */
#define	TEST		01	/* test mode -- 16-bit turnaround */
#define	OPEN		02	/* exclusive use */
#define	ASLEEPR		04	/* waiting for input */
#define	ASLEEPW		010	/* waiting for output */
#define	INERROR		020	/* input lost */

struct	dr11
{
	unsigned	flags;
	struct	clist	in, ou;	/* character q.s */
}
	dr
{
	TEST
};




dropen( dev, flag )
{
  if ( dr.flags & OPEN )
	u.u_error = ENXIO;
  else
  {
	dr.flags = OPEN;
	DRCSR =| ININTEB;
  }
}




drclose( dev )
{
  dr.flags = TEST;
}




drwrite( dev )
{
	register c;

  while ( (c = cpass()) >= 0 )
  {
	spldr();

	if ( dr.ou.c_cc > HIWAT )
	{
		dr.flags =| ASLEEPW;
		sleep( &dr.ou, DROUPRI );
	}

	while ( putc( c, &dr.ou ) )
		delay( HZ );

	drwint();

	spl0();
  }
}




drwint( dev )
{
	register c;

  while ( (dr.ou.c_cc > 1) && (DRCSR & OUTFL) )
  {
	DRCSR =& ~OUTFL;
	c = getc( &dr.ou ) | (getc( &dr.ou ) << 8);
	DROUTBUF = c;	/* 16-bits */
  }

  if ( dr.ou.c_cc > 1 )
	DRCSR =| OUTINTEB;

  if ( dr.flags & ASLEEPW )
  {
	dr.flags =& ~ASLEEPW;
	wakeup( &dr.ou );
  }
}




drread( dev )
{
  do
  {
	spldr();

	drrint();

	while ( !dr.in.c_cc && !(dr.flags & INERROR) )
	{
		dr.flags =| ASLEEPR;
		sleep( &dr.in, DRINPRI );
	}

	spl0();

	if ( dr.flags & INERROR )
	{
		u.u_error =| EIO;
		dr.flags =& ~INERROR;
		return;
	}
  }
  while
	( passc( getc( &dr.in ) ) >= 0 );
}




drrint( dev )
{
	register cc;

  if ( (dr.in.c_cc < HIWAT) && (DRCSR & INFL) )
  {
	do
	{
		cc = DRINBUF;	/* 16-bits */
		DRINBUF = cc;	/* clear bits */
		DRCSR =& ~INFL;
		if ( !(dr.flags & TEST) )
		{
			if ( putc( cc, &dr.in ) || putc( cc>>8, &dr.in ) )
				dr.flags =| INERROR;
		}
		else
			DROUTBUF = cc;
	}
	while
		( (DRCSR & INFL) && (dr.in.c_cc < HIWAT) );

	DRCSR =| ININTEB;
  }

  if ( dr.flags & ASLEEPR )
  {
	dr.flags =& ~ASLEEPR;
	wakeup( &dr.in );
  }
}
