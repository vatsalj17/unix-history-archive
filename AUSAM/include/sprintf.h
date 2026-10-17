/*
 *	This version of "printf" is considerably smaller than the c_library version.
 *
 *	Provides "%s" and optionally "%c", "%d", "%D", "%o"; ( all numbers unsigned ).
 *	Output is on file 1 unless "fout" is reassigned.
 */

/*#define	CHARACTERS	/* if you want %c */
/*#define	NUMBERS		/* if you want %d %o %D */

#ifdef	NUMBERS
/*#define	OCTAL		/* if you want %o */
/*#define	LONG		/* if you want %D */

#ifdef	OCTAL
static	base	10;
#else
#define	base	10
#endif

struct	{ int hiint; unsigned loint; };
#endif	NUMBERS

int	fout	1;

printf( s , a )
  register char *s;
  unsigned a;
{
	register c;
	register unsigned *p = &a;
#ifdef	NUMBERS
	long n;
#endif	NUMBERS

  while ( c = *s++ )
  {
	if ( c == '%' )  switch ( c = *s++ )
	{
#ifdef	CHARACTERS
		case 'c':	c = *p++;
				break;
#endif
		case 's':	printf( *p++ );	/* u r warned */
				continue;
#ifdef	NUMBERS
#ifdef	OCTAL
		case 'o':	base = 8;
#endif
		case 'd':	n.loint = *p++;
				n.hiint = 0;
#ifdef	LONG
				goto nn;
		case 'D':	n.hiint = *p++;  n.loint = *p++;
		nn:
#endif
				printl( n );
#ifdef	OCTAL
				base = 10;
#endif
				continue;
#endif
		case 0:		return( s );
	}
	putchar( c );
  }

  return( s );
}


putchar( c )
{
  write( fout , &c , 1 );
}


#ifdef	NUMBERS
#ifndef	LONG
printl( n )
  long n;
{
	register q, r;
	extern	ldivr;

  q = ldiv( n, base );
  r = ldivr;

  if ( q )
	printl( 0, q );

  putchar( r+'0' );
}

#else	LONG

printl( n )
  long n;
{
	register i;

  i = n%base;

  if ( n =/ base )
	printl( n );

  putchar( i+'0' );
}
#endif	LONG
#endif	NUMBERS
