#
/*
**	dlscan - scan all mounted file-systems for disk usage by users & groups
**
**		update passwd file entries appropriately,
**		note any discrepancies on file "dlnotef"
*/


#include	<local-system>		/* global parameters */
#include	<sys/param.h>		/* system parameters */
#include	<sys/ino.h>		/* disk inode structure */
#include	<sys/inode.h>		/* inode mode definitions */
#include	<sys/filsys.h>		/* super-block structure */
#include	<mtab.h>		/* mounted file table structure */
#include	<passwd.h>		/* "passwd" entry structure */
#include	<stdio.h>		/* standard i/o streams */


#define	TEST		1		/* for testing ! */

#define	NBLOCKS		30		/* read NBLOCKS of inodes at a time */
#define	F_READ		0		/* file read parameter for open sys call */

char	dlnotef[]	= "/usr/adm/dlnotes";
char	gusagef[]	= "/usr/adm/gusages";
char	rawname[]	= "/dev/r";




main( argc, argv )
	int		argc;
	char		*argv[];
{
	extern char	*getfs();
	register char	*fs;

#	ifndef	TEST
	close( 0 );
	close( 1 );
#	endif

	while ( (fs = getfs()) != NULL )
		chkfs( fs );

	updtpasswd();

#	ifndef	TEST
	updtgroups();
#	endif

	return 0;
}




dusage_t	dusages[NUSERS+1];		/* real usage per user */
#ifndef	TEST
long		gdusages[NGROUPS+1];		/* real usage per group */
#endif
struct mtab	mtabent;			/* entry from "mtabf" */
#define	NINODES	(INOPB*NBLOCKS)
struct dinode	inbuf[NINODES];			/* read NBLOCKS of inodes at a time */




/*
**	getfs - returns pointer to next file system name
*/

char *
getfs()
{
	static	 	init, fd;
	register	n;
	static char	s[(sizeof mtabent.m_spec)+(sizeof rawname)];

	if ( !init )
	{
		init++;

		if ( (fd = open( mtabf, F_READ )) == SYSERROR )
		{
			perror( mtabf );
			exit( 1 );
		}
	}

	if ( (n = read( fd, &mtabent, sizeof mtabent )) != sizeof mtabent )
	{
		close( fd );
		return NULL;
	}

	return strcatn( strcpy( s, rawname ), mtabent.m_spec, sizeof mtabent.m_spec );
}




/*
**	chkfs - check file-system "fs" for usage per uid & group
*/

chkfs( fs )
	char		*fs;
{
	int		fd;
	register	ninodes, n;
	struct filsys	super;

#	ifdef	TEST
	printf( "%s\n", fs );
#	endif

	if ( (fd = open( fs, F_READ )) == SYSERROR )
	{
		perror( fs );
		return;
	}

	lseek( fd, SUPERB*BSIZE, 0 );

	if ( read( fd, &super, sizeof super ) != sizeof super )
	{
		perror( fs );
		close( fd );
		return;
	}

	lseek( fd, (SUPERB+1)*BSIZE, 0 );

	for ( ninodes = super.s_isize * INOPB ; ninodes > 0 ; ninodes -= NINODES )
	{
		register		i = sizeof inbuf;

		if ( ninodes < NINODES )
			i = ninodes * (sizeof (struct dinode));

#		ifdef	TEST
		printf( "read %d\n", i );
#		endif

		if ( read( fd, &inbuf, i ) != i )
		{
			perror( fs );
			close( fd );
			return;
		}

		i /= sizeof (struct dinode);

		while ( i-- )
			switch ( inbuf[i].di_mode & IFMT )
			{
				register dusage_t	d = IWEIGHT;
				register		u;
				extern dusage_t		fsize();

			 case 0:	/** Unallocated **/
					break;

			 case IFDIR:
			 case IFREG:
			 case IFLOK:
			 case IFALK:
					d += fsize( &inbuf[i] );
			 default:
					dusages[(u = inbuf[i].di_uid) >= NUSERS ? NUSERS : u] += d;
#					ifndef	TEST
					gdusages[(u = inbuf[i].di_gid) >= NGROUPS ? NGROUPS : u] += d;
#					endif
			}
	}

	close( fd );
}




/*
**	fsize - return block usage of file described by ip
*/

dusage_t
fsize( ip )
	struct dinode		*ip;
{
	register unsigned	b, ib, ib2;

	b = (ip->di_size + BMASK) >> BSHIFT;

	if ( b > 10 )
		if ( (ib = (b-10+NMASK)>>NSHIFT) > 1 )
			if ( (ib2 = (ib+NMASK)>>NSHIFT) > 1 )
				return (dusage_t)(1+ib2+ib+b);
			else
				return (dusage_t)(1+ib+b);
		else
			return (dusage_t)(1+b);
	else
		return (dusage_t)b;
}




/*
**	updtpasswd - updates passwd file entries with disk usage
**		and checks consistency
*/

updtpasswd()
{
	register	i;
	char		buf[SSIZ];
	register FILE	*nfd;
	extern FILE	*fopen();

	if ( (nfd = fopen( dlnotef, 'a' )) == NULL )
		perror( dlnotef );

	for ( i = 0 ; i < NUSERS ; i++ )
	{
		register dusage_t	u, d;

		if ( u = dusages[i] )
		{
			struct pwent	pwe;

			pwe.pw_limits.l_uid = i;

			if ( getpwlog( &pwe, buf, SSIZ ) == PWERROR )
			{
				if ( nfd != NULL )
					fprintf( nfd, "uid %u has %u disk units and DOES NOT EXIST\n", i, u );
			}
			else
			{
				if ( pwe.pw_limits.l_duse != u )
				{
					register	f;
					struct lnode	user;

					user.l_uid = i;

					if ( ((f = limits( &user, L_OTHLIM )) == SYSERROR )
						|| ((pwe.pw_limits.l_duse = user.l_duse) != u )
					   )
					{
						if ( nfd != NULL )
							fprintf( nfd, "uid %u passwd usage %u != real usage %u%s\n",
								i
								,pwe.pw_limits.l_duse
								,u
								,f == SYSERROR ? "" : " (logged on)"
								);
						pwe.pw_limits.l_duse = u;
					}
				}

				if ( u > (d = pwe.pw_limits.l_dlimit + pwe.pw_limits.l_doverflw) )
				{
					chmod( pwe.pw_strings[DIRPATH], 0 );
					pwe.pw_limits.l_flags |= NOLOGIN;
					if ( nfd != NULL )
						fprintf( nfd, "uid %u exceeds disk limit of %u units by %u -- NOLOGIN\n",
							i, d, u-d );
				}

				updtpwent( &pwe );
			}
		}
	}

	if ( nfd != NULL )
	{
		if ( dusages[NUSERS] )
			fprintf( nfd, "ILLEGAL uids possess %u units!\n", dusages[NUSERS] );

		fclose( nfd );
	}

	pwclose();
}
#ifndef	TEST




/*
**	updtgroups - updates group disk usage file
*/

updtgroups()
{
	register	i;
	register FILE	*gufd;
	extern FILE	*fopen();

	if ( (gufd = fopen( gusagef, 'w' )) == NULL )
	{
		perror( gusagef );
		return;
	}

	for ( i = 0 ; i <= NGROUPS ; i++ )
	{
		if ( gdusages[i] )
		{
			fprintf( gufd, "gid %u: disk usage = %U units\n", i, gdusages[i] );
		}
	}

	fclose( gufd );
}
#else	TEST




/*
**	SAFE TEST
*/




updtpwent( pe )
	struct pwent	*pe;
{
	printf( "updtpwent on uid %d\n", pe.pw_limits.l_uid );
}




chmod( s, m )
	char	*s;
	int	m;
{
	printf( "chmod on %s to %o\n", s, m );
}
#endif	TEST
