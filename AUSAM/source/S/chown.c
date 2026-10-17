#
/*
 *	chown lname file ...
 */

#include	<local-system>
#include	<passwd.h>

struct	inode
{
	int	i_dev;
	int	i_number;
	int	i_mode;
	int	i_uid;
	char	i_nlinks;
	char	i_size0;
	int	i_size1;
	int	i_addr[8];
	long	i_atime;
	long	i_mtime;
};

struct	pwent	pe;



main( c , v )
register c;
char *v[];
{
	register unsigned	uid;
	register		sflg = 0;
	struct inode		i[1];

	if ( c < 3 )
	{
		prints( 2, "chown [-s] name|uid file ...\n" );
		exit( 1 );
	}

	v++;
	if( v[0][0] == '-' && v[0][1] == 's' )
	{
		sflg++;
		v++;
		c--;
	}
	pe.pw_strings[LNAME] = *v;
	if ( getpwuid( &pe, 0, 0 ) < 0 )
	{
		if( v[0][0] >= '0' && v[0][0] <= '9' )
			uid = atoi(*v);
		else
		{
			prints( 2, "who?\n" );
			exit( 1 );
		}
	}
	else
		uid = pe.pw_uid;

	c--; v++;

	while ( --c )
	{
		if( sflg)
		{
			stat(*v, i);
		}
		if( chown ( *v , uid ) < 0 )
		{
			perror( *v );
		}
		else if( sflg)
			smdate( *v, i->i_mtime );
		v++;
	}

	return( 0 );
}
