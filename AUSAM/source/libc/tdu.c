#
/*
**	Tdu calculates total disk usage (inodes and blocks) for the given directory
**
**		the total number of files is available in the global "t_files"
**		and likewise the total number of blocks in "t_blocks"
*/



#include	<local-system>
#include	<passwd.h>
#include	<stat16.h>

struct	statbuf	__statb;

extern	errno;
int	__derror;
long	_tdu();
long	_malformed();
char	*_nextpath();
unsigned _size();
long	t_blocks;
long	t_files;
char	__pathname[200];
#define	PATHEND	(&__pathname[sizeof __pathname])

struct { int hiword; unsigned loword; };



tdu( dir )
  register char *dir;		/** pointer to directory to be totalled **/
{
	int			parino;
	/*
	**	get inode of parent directory of "dir" (N.B. ".." is not good enough)
	*/
	{
		register char	*dp, *np;
		char		dbuf[SSIZ];

		if ( (np = dir) == 0 || *np == 0 )
			np = "/";

		for ( dp = dbuf ; *dp++ = *np++ ; )
			if ( dp >= &dbuf[SSIZ] )
			{
				printf( "%s: too long!\n", dir );
				return( -1 );
			}

		while ( *--dp != '/' )
			if ( dp <= dbuf )
			{
				printf( "Bad directory path: %s\n", dir );
				return( -1 );
			}

		if ( dp == dbuf )	/* root! */
			dp++;
		*dp = 0;

		if ( newstat( dbuf, &__statb ) < 0 )
		{
			printf( "Parent directory: " );
			perror( dbuf );
			return( -1 );
		}

		parino = __statb.sb_inumber;
	}

	/*
	**	move over to the user's home directory
	*/
	if ( newstat( dir , &__statb ) < 0 || chdir(dir) < 0 )
	{
		perror( dir );
		return( -1 );
	}

	__derror = 0;
	errno = 0;
	t_blocks = 0;
	t_files = 0;
	return( _tdu( _nextpath( __pathname, dir ), parino ) >> 16 );	/* N.B. __statb set up above */
}




#define	ERROR	0XFFFFFFFFL

struct
{
	int f_ino;
	char f_name[14];
	char f_end;
}
	__dirbuf;

/*
**	"tdu" returns the disc usage of the current directory
**	      and all sub-directories.
**		Inodes are weighted at IWEIGHT blocks.
**		Links are shared equally.
*/

long _tdu( pathend, parino )
  char *pathend;
  unsigned parino;
{
	register	fd, i, posn;
	long		inodes, blocks, t;
	unsigned	dino;

  if ( pathend == 0 )
	return( ERROR );

  dino = __statb.sb_inumber;

  inodes = 0X10000;
  t_files =+ 0X10000;
  blocks.loword = 0;
  blocks.hiword = _size();
  t_blocks =+ blocks;

  posn = 0;
  __dirbuf.f_end = '\0';

loop:
  if ( ((fd = open( __pathname , 0)) >= 0) && (seek( fd , posn , 0) >= 0) )
  {
	while ( (i = read( fd , &__dirbuf , 16 )) == 16 )
	{
		posn =+ i;

		if ( __dirbuf.f_ino )
			if ( newstat( __dirbuf.f_name , &__statb ) >= 0 )
			{
				if ( (__statb.sb_flags & IFTYP) == IFDIR )
				{
					if ( __statb.sb_inumber != dino && __statb.sb_inumber != parino )
					{
						if ( _dots() )
							return( _malformed( "separate directory!" ) );

						if ( posn <= 32 )
							return( _malformed( "\".\" or \"..\" displaced!" ) );

						if ( chdir( __dirbuf.f_name ) >= 0 )
						{
							close( fd );
							blocks =+ _tdu( _nextpath( pathend, __dirbuf.f_name ), dino );
							*pathend = 0;
							chdir( __pathname );
							if ( __derror )
								return( ERROR );
							goto loop;
						}
						else
							return( _malformed() );
					}
					else
						if ( posn > 32 )
							return( _malformed( "link displaced" ) );
						else
							if ( !_dots() )
								return( _malformed( "\".\" or \"..\" renamed!" ) );
				}
				else
				{
					/* fixed point arithmetic */
					t = (0X10000 / (__statb.sb_nlinks&0377));
					t_files =+ t;
					inodes =+ t;
					t = ((0X10000 * _size()) / (__statb.sb_nlinks&0377));
					t_blocks =+ t;
					blocks =+ t;
				}
			}
			else
				return( _malformed() );
	}

	close( fd );

	if ( i )
		return( _malformed( "bad length" ) );

	return( blocks + inodes*IWEIGHT );
  }
  else
  {
	__dirbuf.f_name[0] = 0;
	return( _malformed() );
  }
}


unsigned _size()
{
	register unsigned b, ib;

  b = ((__statb.sb_size0 & 0377) << 7) + (((__statb.sb_size1 + 0777) >> 9) & 0177);
  if ( ib = b>>3 )
  {
	b++;
	if ( ib =>> 5 )
	{
		b =+ ib;	/* indirect */
		if ( ib > 8 )
			b++;	/* double indirect */
	}
  }

  return( b );
}


long _malformed( s )
  char *s;
{
	extern	errno;

  putchar( '\n' );
  printf( "MALFORMED DIRECTORY in %s\n\t", __pathname );
  if ( errno )
	perror( __dirbuf.f_name );
  else
	printf( "%s: %s\n", __dirbuf.f_name, s );
  __derror++;
  return( ERROR );
}


_dots()
{
	register char	*sp = __dirbuf.f_name;

	if ( *sp++ == '.'
	     && ( *sp == '\0'
		  || ( *sp++ == '.' && *sp == '\0' )
		)
	   )
		return( 1 );

	return( 0 );
}


char *_nextpath( at, name )
  register char *at, *name;
{
  if ( *name != '/' )
	*at++ = '/';

  do
	if ( at >= PATHEND )
	{
		__derror++;
		return( 0 );
	}
  while
	( *at++ = *name++ );

  return( --at );
}
