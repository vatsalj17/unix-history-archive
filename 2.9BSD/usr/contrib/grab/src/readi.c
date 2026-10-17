/*
 * readi.c
 *
 * Routines dealing with file system I/O.
 *
 *---------------------------------------------------------------------------
 *
 * $Header:   RCS/readi.c.v  Revision 1.3  83/08/01  17:09:48  donn  Exp$
 * $Log:	RCS/readi.c.v $
 * Revision 1.3  83/08/01  17:09:48  donn
 * Added code for symbolic links, made MPX code conditionally compiled;
 * did some general prettying up.
 * 							Donn
 * 
 * Revision 1.2  83/07/01  19:21:48  donn
 * Added code to support -i, -I and -L flags.  This allows you to grab by
 * inode number and to display the contents of inodes in a useful way.
 * 							Donn
 * 
 */

# include	"grab.h"



/*
 * readi( ino, name ) -- read file corresponding to ino off of fsys and
 *	put it in file "name".
 */
readi( ino, name )
    int     ino;
    char   *name;
{
	int		ofile;
	struct inode	i;
	struct bmap	b;
	char		iname[DIRSIZ + 1];

	/*
	 * Get the inode.
	 * Make sure that the permissions are ok.
	 */
	if ( ino == 0 )
		return;
	geti( ino, &i );
	if ( ! chkaccess( &i ) ) {
		fprintf( stderr, "grab: %s read-protected\n", name );
		return;
	}

	/*
	 * Is it a directory? If so, then recurse.
	 */
	if ( isdir( &i ) ) {
		dodirs( name, ino, &i, &b );
		return;
	}

	/*
	 * Take care of listings for individual files.
	 */
	if ( Lflag ) {
		idump( &i, ino, name );
		return;
	}
	if ( lflag ) {
		fprintf( stderr, "grab: %s not a directory\n", name );
		return;
	}

	/*
	 * If we have no name for a file, we call it by its inode number.
	 */
	if ( name == NULL ) {
		if ( ! iflag ) {
			fprintf( stderr, "grab: Internal error (see a guru)\n" );
			exit( -1 );
		}
		name	= &iname[0];
		sprintf( name, "%u", ino );
	}

	/*
	 * Deal with links to files.
	 */
	if ( i.i_nlink > 1 && dolinks( name, ino ) )
		return;

# ifdef	SYMLINKS
	if ( (target_system == V7_4BSD || target_system == V7_2BSD) &&
	     (i.i_mode & V7_IFMT) == V7_IFLNK ) {
		dosymlinks( name, &i, &b );
		return;
	}
# endif	SYMLINKS

	/*
	 * Deal with non-directory special files.
	 */
	if ( ! isreg( &i ) ) {
		maknode( name, &i );
		return;
	}

	/*
	 * Copy the file.
	 */
	if ( (ofile = bopen( name, &i, &b )) == -1 )
		return;

	while ( bread( &b ) > 0 )
		bput( ofile, &b );

	bclose( ofile, name, &i, &b );
}



/*
 * geti( ino, ip ) -- read inode number ino and use it to fill the generic
 *	inode structure pointed to by ip.
 */
geti( ino, ip )
    int                     ino;
    register struct inode  *ip;
{
	char	       *malloc();
	char	       *ilist;
	register int	n;
	long		bn;
	register struct v6dinode *v6dp;
	register struct v7dinode *v7dp;

	ilist	= malloc( fsbsiz );
	if ( ilist == NULL ) {
		fprintf( stderr, "grab: out of memory\n" );
		exit( 5 );
	}
	bn	= itod( ino );
	bn	= fsbtodb( bn );
	lseek( fsys, bn * DBSIZE, 0 );

	if ( read( fsys, ilist, fsbsiz ) != fsbsiz ) {
		fprintf( stderr, "grab: read I/O error\n" );
		exit( 6 );
	}

	switch ( target_system ) {

	case V7_2BSD:
		v7dp		= (struct v7dinode *) ilist;
		v7dp		= &v7dp[ itoo( ino ) ];
		ip->i_mode	= v7dp->v7di_mode;
		ip->i_nlink	= v7dp->v7di_nlink;
		ip->i_size	= v7dp->v7di_size;
		ip->i_uid	= v7dp->v7di_uid;
		ip->i_gid	= v7dp->v7di_gid;
		l3tol( (char *)ip->i_addr, (char *)v7dp->v7di_addr, V7NADDR );
		ip->i_atime	= v7dp->v7di_atime;
		ip->i_mtime	= v7dp->v7di_mtime;
# ifndef PDP
		wswap( (char *)&ip->i_size, 1 );
		wswap( (char *)ip->i_addr, V7NADDR );
		wswap( (char *)&ip->i_atime, 1 );
		wswap( (char *)&ip->i_mtime, 1 );
# endif
		break;

	case V7_4BSD:
		v7dp		= (struct v7dinode *) ilist;
		v7dp		= &v7dp[ itoo( ino ) ];
		ip->i_mode	= v7dp->v7di_mode;
		ip->i_nlink	= v7dp->v7di_nlink;
		ip->i_size	= v7dp->v7di_size;
		ip->i_uid	= v7dp->v7di_uid;
		ip->i_gid	= v7dp->v7di_gid;
		l3tol( (char *)ip->i_addr, (char *)v7dp->v7di_addr, V7NADDR );
		ip->i_atime	= v7dp->v7di_atime;
		ip->i_mtime	= v7dp->v7di_mtime;
# ifdef PDP
		wswap( (char *)&ip->i_size, 1 );
		wswap( (char *)ip->i_addr, V7NADDR );
		wswap( (char *)&ip->i_atime, 1 );
		wswap( (char *)&ip->i_mtime, 1 );
# endif
		break;

	case V6:
		v6dp		= (struct v6dinode *) ilist;
		v6dp		= &v6dp[ itoo( ino ) ];
		ip->i_mode	= v6dp->v6di_mode;
		ip->i_nlink	= v6dp->v6di_nlink & 0xff;
		ip->i_size	= (unsigned short) v6dp->v6di_sz1;
		ip->i_size	+= ((long) (v6dp->v6di_sz0 & 0xff)) * 0x10000L;
		ip->i_uid	= v6dp->v6di_uid & 0xff;
		ip->i_gid	= v6dp->v6di_gid & 0xff;
		for ( n = 0; n < V6NADDR; ++n )
			ip->i_addr[n]	= (unsigned short) v6dp->v6di_addr[n];
		ip->i_atime	= v6dp->v6di_atime;
		ip->i_mtime	= v6dp->v6di_mtime;
# ifndef PDP
		wswap( (char *)&ip->i_atime, 1 );
		wswap( (char *)&ip->i_mtime, 1 );
# endif
		break;
	}

	free( ilist );
}



/*
 * dodirs( name, ino, ip, bp ) -- Recursively extract directory contents.
 */
dodirs( name, ino, ip, bp )
    char         *name;
    int		  ino;
    struct inode *ip;
    struct bmap  *bp;
{
	register struct direct *dp;
	register int		j, k, l = 0;
	struct inode	       *dirip;
	int			n, dpb;
	int			dcomp();
	char		       *s	= NULL;
	char                   *malloc();


	/*
	 * If we are doing listings, list the directory itself.
	 */
	if ( Lflag ) {
		fprintf( stderr, "Directory:\n\n" );
		idump( ip, ino, name );
		fprintf( stderr, "Contents of directory:\n\n" );
		dirip	= (struct inode *) malloc( sizeof (struct inode) );
	}
	if ( lflag > 1 )
		printf( "\n%s:\n", name );

	/*
	 * If we only have the inode, we must simulate the name.
	 */
	if ( name == NULL ) {
		if ( ! iflag ) {
			fprintf( stderr, "grab: Internal error (see a guru)\n" );
			exit( -1 );
		}
		name	= malloc( DIRSIZ + 1 );
		sprintf( name, "%u", ino );
	}

	/*
	 * Tar header for a directory.
	 */
	if ( tflag )
		theader( TFILE, name, ip );

	/*
	 * Allocate a buffer to hold the pathname.
	 */
	j	= strlen( name ) + DIRSIZ + 2;
	s	= malloc( j );
	if ( s == NULL ) {
		fprintf( stderr, "grab: out of memory\n" );
		exit( 7 );
	}
	s[j-1]	= '\0';

	/*
	 * If we aren't merely printing files, make the directory.
	 */
	if ( ! (xflag || lflag || Lflag || tflag ) )
		if ( makdir( name, ip ) < 0 )
			return;

	/*
	 * Iterate over the target directory's entries.
	 */
	n	= ip->i_size / sizeof (struct direct);
	if ( lflag || Lflag ) {
		allocbuf( ip, bp, B_BIG );
		dpb	= (nblock * TBLOCK) / sizeof (struct direct);
	} else {
		allocbuf( ip, bp, B_SMALL );
		dpb	= fsbsiz / sizeof (struct direct);
	}

	for ( j = 0; j < n; ++j ) {
		/*
		 * Read the directory (if necessary).
		 */
		k	= j % dpb;
		if ( k == 0 ) {
			bread( bp );
			if ( lflag )
				/*
				 * Sort by directory name.
				 */
				qsort( bp->b_data, min( dpb, n-j ), sizeof (struct direct), dcomp );
		}

		/*
		 * Find directory entry.
		 */
		dp	= &((struct direct *) bp->b_data)[ k ];
		if ( dp->d_ino == 0 )
			continue;
		if ( strncmp( ".", dp->d_name, DIRSIZ ) == 0 ||
		     strncmp( "..", dp->d_name, DIRSIZ ) == 0 )
			continue;

		/*
		 * If -l or -L flag was selected, list the directory.
		 */
		if ( lflag ) {
			if ( ++l % 5 != 0 )
				printf( "%-14.14s  ", dp->d_name );
			else
				printf( "%-.14s\n", dp->d_name );
			continue;
		}
		if ( Lflag ) {
			geti( dp->d_ino, dirip );
			idump( dirip, dp->d_ino, dp->d_name );
			continue;
		}

		/*
		 * Recursively extract files.
		 */
		strcpy( s, name );
		strncat( s, "/", 2 );
		strncat( s, dp->d_name, DIRSIZ );
		readi( (unsigned int) dp->d_ino, s );
	}

	if ( lflag && l % 5 != 0 )
		putchar( '\n' );

	freebuf( bp );

	free( s );
	if ( Lflag )
		free( dirip );

	/*
	 * Repair volatile attributes of the directory.
	 */
	if ( xflag || lflag || Lflag || tflag )
		return;
# ifdef CLR_SETUID
	if ( pflag && (ip->i_mode & (ISUID|ISGID|ISVTX)) )
		chmod( name, ip->i_mode & 07777 );
# endif CLR_SETUID
# ifndef PDPV6
	if ( pflag )
		utime( name, &ip->i_atime );
# endif

	return;
}



/*
 * dcomp( d1, d2 ) -- Order two directory entries by name.
 */
dcomp( d1, d2 )
    register struct direct *d1, *d2;
{
	return ( strncmp( d1->d_name, d2->d_name, DIRSIZ ) );
}



/*
 * dolinks( name, ino ) -- Copy links.  Return 1 if link was made.
 *	Strategy: search link records, each containing an inumber,
 *	a link to the next record and a pathname.
 */

struct link
{
	short		ino;
	short		pad;
	struct link    *next;
	char		path;
};

int
dolinks( name, ino )
    char   *name;
    int     ino;
{
	char		       *malloc();
	static struct link     *root	= NULL;
	struct link	       *lp	= root;

	if ( xflag )
		return ( 0 );

	/*
	 * Look for ino in list.
	 */
	while ( lp != NULL ) {
		if ( lp->ino == (short) ino ) {
			/*
			 * Found it.  Emit a "tar" link or a real link.
			 */
			if ( tflag )
				tlink( TFILE, ino, name, &(lp->path) );
			else
			if ( link( &(lp->path), name ) == -1 ) {
				fprintf( stderr, "grab: couldn't link %s to %s\n",
					name, &(lp->path) );
				return ( 0 );
			}
			if ( vflag ) {
				printf( "%s (linked to %s)\n", name, &(lp->path) );
				fflush( stdout );
			}
			return ( 1 );
		}
		lp	= lp->next;
	}

	/*
	 * A new name.  Store it away.
	 */
	lp		= (struct link *) malloc( (2 * sizeof (short))
				+ sizeof (struct link *) + strlen( name ) + 1 );
	if ( lp == NULL ) {
		fprintf( stderr, "grab: out of memory\n" );
		exit( 8 );
	}
	lp->ino		= (short) ino;
	lp->next	= root;
	strcpy( &(lp->path), name );
	root		= lp;

	return ( 0 );
}



/*
 * makdir( name, ip ) -- create a directory named name, using mkdir.
 */
makdir( name, ip )
    char         *name;
    struct inode *ip;
{
	short	statbuf[16];
	int	pid, deadpid, status;

	if ( stat( name, statbuf ) == 0 ) {
		/*
		 * File exists -- if it's a directory, okay;
		 * otherwise we complain.
		 */
# ifdef	PDPV6
		if ( (((struct inode *) statbuf)->i_mode & V6_IFMT) == V6_IFDIR )
# else
		if ( (((struct inode *) statbuf)->i_mode & V7_IFMT) == V7_IFDIR )
# endif
			return ( 0 );
		fprintf( stderr, "grab: %s: not a directory\n", name );
		return ( -1 );
	}

	/*
	 * File doesn't exist: fork and exec mkdir.
	 */
	pid	= fork();
	if ( pid == 0 ) {
		execl( "/bin/mkdir", "mkdir", name, 0 );
		execl( "/usr/bin/mkdir", "mkdir", name, 0 );
		fprintf( stderr, "grab: can't exec mkdir\n" );
		exit( 9 );
	}
	while ( (deadpid = wait( &status )) >= 0 && deadpid != pid )
		;
# ifdef PDPV6
	/*
	 * Sigh.  V6 mkdir doesn't return a useful status.
	 */
	if ( deadpid < 0 ) {
# else
	if ( deadpid < 0 || status != 0 ) {
# endif
		fprintf( stderr, "grab: can't create directory %s\n", name );
		return ( -1 );
	}

	/*
	 * Copy directory permissions.
	 */
	chmod( name, ip->i_mode & (pflag ? 07777 : 0777 ) );
	if ( pflag )
		CHOWN( name, ip->i_uid, ip->i_gid );

	return ( 0 );
}



# ifdef	SYMLINKS
/*
 * dosymlinks( name, ip, bp ) -- Make a symbolic link from name to the file
 *	whose name is contained in the file with inode *ip, using bmap *bp.
 */
dosymlinks( name, ip, bp )
    register char         *name;
    register struct inode *ip;
    register struct bmap  *bp;
{
	allocbuf( ip, bp, B_SMALL );
	bread( bp );
	if ( symlink( bp->b_data, name ) == -1 )
		fprintf( stderr, "grab: couldn't make symbolic link from %s to %s\n", name, bp->b_data );
	else if ( vflag ) {
		printf( "%s (symbolic link to %s)\n", name, bp->b_data );
		fflush( stdout );
	}
	freebuf( bp );
}
# endif	SYMLINKS



/*
 * maknode( name, ip ) -- Create a device entry.
 */
maknode( name, ip )
    char         *name;
    struct inode *ip;
{
	long	addr		= ip->i_addr[0];
	short	nodemode	= ip->i_mode & (pflag ? 07777 : 0777);

	if ( xflag ) {
		fprintf( stderr, "grab: Can't print special file %s!\n", name );
		return;
	} else if ( tflag ) {
		fprintf( stderr, "grab: %s: Can't put on tar tape\n", name );
		return;
	}

	/*
	 * Check legality of modes.
	 */
# ifndef MPXFILES
	if ( target_system != V6 &&
	     ( (ip->i_mode & V7_IFMT) == V7_IFMPC ||
	       (ip->i_mode & V7_IFMT) == V7_IFMPB ) ) {
		fprintf( stderr, "grab: %s: can't make multiplex files\n", name );
		return;
	}
# endif MPXFILES
# ifndef SYMLINKS
	if ( (target_system == V7_4BSD || target_system == V7_2BSD) &&
	     (ip->i_mode & V7_IFMT) == V7_IFLNK ) {
		fprintf( stderr, "grab: %s: can't make symbolic links\n", name );
		return;
	}
# endif	SYMLINKS

	/*
	 * Convert modes.
	 */
	nodemode	|= CONVMODE( ip->i_mode );

	/*
	 * Create the special file.
	 */
	if ( mknod( name, nodemode, addr ) == -1 ) {
		fprintf( stderr, "grab: can't make special file %s\n", name );
		return;
	}
	if ( pflag ) {
		CHOWN( name, ip->i_uid, ip->i_gid );
# ifdef CLR_SETUID
		if ( ip->i_mode & (ISUID|ISGID|ISVTX) )
			chmod( name, ip->i_mode & 07777 );
# endif CLR_SETUID
# ifndef PDPV6
		utime( name, &ip->i_atime );
# endif
	}

	if ( vflag )
		printf( "%s (special file)\n", name );
}



/*
 * l3tol( lp, cp, n ) -- convert the n 3-byte integers pointed to by cp
 *	into longs and put them in the array pointed to by lp.
 *
 * PDP long integer brain damage strikes again -- on a VAX a 3-byte long
 * is the first three bytes of a true long but on a PDP it's got a hole in
 * it...  thus the library routine can't be used.
 */
l3tol(lp, cp, n)
    long   *lp;
    char   *cp;
    int     n;
{
	register i;
	register char *a, *b;

	a = (char *)lp;
	b = cp;
	for(i=0;i<n;i++) {
		if ( target_system != V7_4BSD ) {
			*a++ = *b++;
			*a++ = 0;
			*a++ = *b++;
			*a++ = *b++;
		} else {
			*a++ = *b++;
			*a++ = *b++;
			*a++ = *b++;
			*a++ = 0;
		}
	}
}



/*
 * wswap( longlist, count ) -- swap the words of count long integers residing
 *	in longlist.
 */
wswap( longlist, count )
    char          *longlist;
    register int   count;
{
	register short *wptr, tmp;

	wptr	= (short *) longlist;
	while ( count-- ) {
		tmp	= *wptr;
		*wptr	= *(wptr+1);
		*++wptr	= tmp;
		++wptr;
	}
}
