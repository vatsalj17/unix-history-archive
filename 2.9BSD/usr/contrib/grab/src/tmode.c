/*
 * tmode.c
 *
 * Routines for producing blocked "tar"-style output.
 *
 *---------------------------------------------------------------------------
 *
 * $Header$
 * $Log$
 */

# include	"grab.h"



/*
 * theader( ofile, name, ip ) -- Write a "tar" header for a file called "name"
 *	on file "ofile" using inode information pointed to by "ip".
 */
theader( ofile, name, ip )
    int             ofile;
    char           *name;
    struct inode   *ip;
{
	union hblock    h;
	register char  *cp;
	register int	n	= TBLOCK;

	/*
	 * Clear out the buffer.
	 */
	cp	= h.dummy;
	CLRBUF( cp, n );

	/*
	 * Insert name.
	 */
	n	= strlen( name );
	if ( n > NAMSIZ ) {
	    fprintf( stderr, "grab: name too long for tar output: %s\n", name );
	    fprintf( stderr, "grab: name truncated to: %.*s\n", NAMSIZ, name );
	}
	strncpy( h.dbuf.name, name, NAMSIZ );
	if ( isdir( ip ) )
		h.dbuf.name[ min( n, NAMSIZ-1 ) ]	= '/';

	/*
	 * Insert modes.
	 */
	tomodes( &h, ip );

	tput( ofile, h.dummy, TBLOCK );
}



/*
 * tlink( ofile, ino, name, linkname ) -- Write a "tar" header for a file named
 *	name with inode ino which is linked to an earlier file called linkname.
 */
tlink( ofile, ino, name, linkname )
    int     ofile;
    int     ino;
    char   *name;
    char   *linkname;
{
	union hblock	h;
	struct inode	i;
	register char  *cp;
	register int	n	= TBLOCK;

	/*
	 * Clear out the buffer.
	 */
	cp	= h.dummy;
	CLRBUF( cp, n );

	/*
	 * Insert names.
	 */
	if ( strlen( name ) > NAMSIZ ) {
	    fprintf( stderr, "grab: name too long for tar output: %s\n", name );
	    fprintf( stderr, "grab: name truncated to: %.*s\n", NAMSIZ, name );
	}
	strncpy( h.dbuf.name, name, NAMSIZ );
	strncpy( h.dbuf.linkname, linkname, NAMSIZ );
	h.dbuf.linkflag	= '1';

	/*
	 * Get modes.
	 */
	geti( ino, &i );
	tomodes( &h, &i );

	tput( ofile, h.dummy, TBLOCK );
}



/*
 * tomodes( hp, ip ) -- Store modes from inode ip in header block hp.
 */
tomodes( hp, ip )
    union hblock *hp;
    struct inode *ip;
{
	sprintf( hp->dbuf.mode, "%6o ", ip->i_mode & 07777 );
	sprintf( hp->dbuf.uid, "%6o ", ip->i_uid );
	sprintf( hp->dbuf.gid, "%6o ", ip->i_gid );
	sprintf( hp->dbuf.size, "%11lo ", isdir( ip ) ? 0 : ip->i_size );
	sprintf( hp->dbuf.mtime, "%11lo ", ip->i_mtime );
	checksum( hp );
}



/*
 * checksum( hp ) -- Insert tar-style checksum in header block *hp.
 */
checksum( hp )
    union hblock *hp;
{
	register int	i;
	register char  *cp	= hp->dummy;
	register char  *last	= hp->dummy + TBLOCK;

	for ( i = 0; i < sizeof hp->dbuf.chksum; ++i )
		hp->dbuf.chksum[i]	= ' ';

	i	= 0;
	do
		i	+= *cp++;
	while ( cp < last );

	sprintf( hp->dbuf.chksum, "%6o", i );
}



/*
 * tput( ofile, data, size ) -- Twiddle the buffer queuing mechanism to output
 *	tape data on ofile.
 */
tput( ofile, data, size )
    int              ofile;
    register char   *data;
    int              size;
{
	struct inode	i;
	struct bmap	b;
	register int	n;
	register char  *cp;

	/*
	 * Get access to the big data buffer.
	 * Notice that it really doesn't matter much what's in inode i!
	 */
	i.i_size	= size;
	allocbuf( &i, &b, B_BIG );

	/*
	 * Get the data in place and fiddle the bmap parameters to make
	 * bput think it's got something to do.
	 */
	cp		= b.b_data + b.b_cc;
	n		= size;
	CPYBUF( cp, data, n );

	b.b_offset	= 0;
	b.b_cc		+= size;

	/*
	 * Send out the data.
	 */
	bput( ofile, &b );

	/*
	 * Wrap up.  Simple, huh?
	 */
	freebuf( &b );
}



/*
 * tflush( ofile ) -- Flush remaining tape record, with two zero blocks
 *	appended.
 */
tflush( ofile )
    int     ofile;
{
	char		buf[TBLOCK];
	register char  *cp;
	register int	n;
	extern int	savetcc;

	/*
	 * Make an empty buffer.
	 */
	cp	= buf;
	n	= TBLOCK;
	CLRBUF( cp, n );

	/*
	 * Cleverly put out 2 empty buffers and write.
	 * The chicanery in the final tput forces a write because it makes
	 * bput think it has a full buffer.
	 */
	tput( ofile, buf, TBLOCK );
	tput( ofile, buf, TBLOCK );
	savetcc	= nblock * TBLOCK;
	tput( ofile, buf, 0 );
}
