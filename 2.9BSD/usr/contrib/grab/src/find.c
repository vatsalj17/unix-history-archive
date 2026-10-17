/*
 * find.c
 *
 * Routines for descending the directory tree of a pathname.
 *
 *---------------------------------------------------------------------------
 *
 * $Header$
 * $Log$
 */

# include	"grab.h"



/*
 * tail( path ) -- return pointer to the terminal name of path.
 */
char *
tail( path )
    register char *path;
{
	register char *name	= path;

	while ( *path != '\0' )
		if ( *path++ == '/' )
			name	= path;
	return ( name );
}



/*
 * find( path ) -- return inode number corresponding to path.
 */
int
find( path )
    char   *path;
{
	register char  *p, *q;
	char		tmp;
	int		ino;
	struct inode	i;

	if ( path == NULL || *path == '\0' ) {
		fprintf( stderr, "grab: null filename\n" );
		return ( 0 );
	}
	p	= path;

	/*
	 * Fetch the root inode information from the disk.
	 */
	geti( ROOTINO, &i );

	/*
	 * Search the directories in the path for
	 * each pathname element in turn.
	 */
	while ( *p != '\0' ) {

		/*
		 * Find and separate out a pathname element.
		 */
		while ( *p == '/' )
			++p;
		q	= p;
		while ( *q != '/' && *q != '\0' )
			++q;
		tmp	= *q;
		*q	= '\0';

		/*
		 * Look in the directory.
		 */
		if ( ! isdir( &i ) ) {
			fprintf( stderr, "grab: bad directory in %s\n", path );
			return ( 0 );
		}

		if ( (ino = dlook( p, path, &i )) == 0 )
			return ( 0 );

		if ( tmp == '\0' )
			/*
			 * Found the terminal element.
			 */
			break;
		geti( ino, &i );
		*q	= tmp;
		p	= q;
	}

	return( ino );
}



/*
 * dlook( name, path, ip ) -- look for name in directory corresponding to the
 *	inode whose incore structure is pointed to by ip.  Path is used for
 *	error reporting.
 */
int
dlook( name, path,  ip )
    char           *name;
    char           *path;
    struct inode   *ip;
{
	register int		j, k;
	int			n, dpb, ino;
	register struct direct *dp;
	struct bmap		b;

	if ( ! chkaccess( ip ) ) {
		fprintf( stderr, "grab: %s read-protected\n", path );
		return ( 0 );
	}

	ino	= 0;
	n	= ip->i_size / sizeof (struct direct);
	dpb	= fsbsiz / sizeof (struct direct);

	allocbuf( ip, &b, B_SMALL );

	for ( j = 0; j < n; ++j ) {
		k	= j % dpb;
		if ( k == 0 )
			bread( &b );
		dp	= &((struct direct *) b.b_data)[ k ];
		if ( dp->d_ino == 0 )
			continue;
		if ( strncmp( name, dp->d_name, DIRSIZ ) == 0 ) {
			ino	= dp->d_ino;
			break;
		}
	}

	freebuf( &b );

	/*
	 * Did we find it?
	 */
	if ( ino == 0 )
		fprintf( stderr, "grab: %s not found\n", path );

	return ( ino );
}



/*
 * isreg( ip ) -- return true iff ip's inode corresponds to a regular file.
 */
int
isreg( ip )
    struct inode *ip;
{
	switch ( target_system ) {
	case V7_2BSD:
	case V7_4BSD:
		return ( (ip->i_mode & V7_IFMT) == V7_IFREG );
	case V6:
		return ( (ip->i_mode & V6_IFMT) == V6_IFREG );
	}
}




/*
 * isdir( ip ) -- return true iff ip's inode corresponds to a directory file.
 */
int
isdir( ip )
    struct inode *ip;
{
	switch ( target_system ) {
	case V7_2BSD:
	case V7_4BSD:
		return ( (ip->i_mode & V7_IFMT) == V7_IFDIR );
	case V6:
		return ( (ip->i_mode & V6_IFMT) == V6_IFDIR );
	}
}



/*
 * chkaccess( ip ) -- check permissions on readability of the given
 *	inode on the foreign system.  Return 1 if readable.
 */
int
chkaccess( ip )
    struct inode *ip;
{
	static int	uid = -1, gid = -1;
	int		iuid, igid, mode, ckmode;

	if ( uid == -1 ) {
		uid	= geteuid();
		gid	= getegid();
	}
	iuid	= ip->i_uid;
	igid	= ip->i_gid;

	if ( uid == 0 )
		return ( 1 );
	
	if ( isdir( ip ) )
		ckmode	= 05;		/* Read and execute on a directory */
	else
		ckmode	= 04;		/* Read on regular files */
	mode	= ip->i_mode & 0777;

	if ( uid == iuid && ((mode >> 6) & ckmode) == ckmode )
		return ( 1 );
	if ( gid == igid && ((mode >> 3) & ckmode) == ckmode )
		return ( 1 );
	if ( (mode & ckmode) == ckmode )
		return ( 1 );
	return ( 0 );
}
