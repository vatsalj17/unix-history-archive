/*
 * bread.c
 *
 * Routines to sequentially retrieve blocks from a Unix file.
 *
 *---------------------------------------------------------------------------
 *
 * $Header:   RCS/bread.c.v  Revision 1.3  83/08/01  17:05:47  donn  Exp$
 * $Log:	RCS/bread.c.v $
 * Revision 1.3  83/08/01  17:05:47  donn
 * Fixed bug that led to extra zero-filled blocks in tar output,
 * ended up simplifying main loop of bput.
 * 							Donn
 * 
 * Revision 1.2  83/07/01  17:30:24  donn
 * Changed idump to make it usable with new -L and -I options -- it's much
 * more helpful now, and cleaner.
 * 							Donn
 * 
 */

# include	"grab.h"



/*
 * bopen( name, ip, bp ) -- Prepare to output a file called name, using
 *	inode ip to allocate a bmap bp.
 */
int
bopen( name, ip, bp )
    char                  *name;
    register struct inode *ip;
    register struct bmap  *bp;
{
	int	ofile;

	if ( xflag )
		/*
		 * Send file to the standard output.
		 */
		ofile	= STDOUT;
	else if ( tflag ) {
		/*
		 * Put out a "tar"-style header record.
		 */
		theader( TFILE, name, ip );
		ofile	= TFILE;
	} else {
		/*
		 * Create a file to copy into.
		 */
		ofile	= creat( name, ip->i_mode & (pflag ? 07777 : 0777) );
		if ( ofile < 0 ) {
			fprintf( stderr, "grab: can't create %s\n", name );
			return ( -1 );
		}
		if ( pflag )
			CHOWN( name, ip->i_uid, ip->i_gid );
	}

	allocbuf( ip, bp, B_BIG );

	return ( ofile );
}



/*
 * bread( bp ) -- Fill the buffer of bmap bp with blocks from its file.
 */
bread( bp )
    register struct bmap *bp;
{
	register struct bstr
	    {
		long	bn_bno;
		int	bn_index;
	    }	
		       *bb;
	long		bno;
	long		v7bnext(), v6bnext();
	register int	i;
	int		nbytes, nblocks;
	int		bcomp();
	char	       *start;
	char	       *malloc();

	/*
	 * End of file?  Buffer wraparound?
	 */
	if ( bp->b_offset >= bp->b_size )
		return ( 0 );
	if ( bp->b_cc >= bp->b_len )
		bp->b_cc	= 0;

	/*
	 * A minor optimization: blocks are sorted by block number
	 * before reading.  Here we figure out how many blocks to read and
	 * allocate a buffer to hold the block numbers for sorting.
	 */
	nbytes		= min( bp->b_size-bp->b_offset, bp->b_len-bp->b_cc );
	nblocks		= (nbytes + (fsbsiz-1)) / fsbsiz;
	bb		= (struct bstr *) malloc( nblocks * sizeof (struct bstr) );
	if ( bb == NULL ) {
		fprintf( stderr, "grab: Out of memory\n" );
		exit( 12 );
	}

	/*
	 * Find out which file system block(s) to read in.
	 */
	for ( i = 0; i < nblocks; ++i ) {
		switch ( target_system ) {
		case V7_2BSD:
		case V7_4BSD:
			bno	= v7bnext( bp );
			break;
		case V6:
			bno	= v6bnext( bp );
			break;
		}
		if ( bno == 0 )
			/*
			 * A hole -- stop here and let bput do the work.
			 */
			break;
		bb[i].bn_index	= i;
		bb[i].bn_bno	= bno;
	}
	nblocks		= i;
	nbytes		= i * fsbsiz;

	/*
	 * Sort the block numbers.
	 */
	qsort( bb, nblocks, sizeof (struct bstr), bcomp );

	/*
	 * Read the blocks off, in order.
	 */
	start		= bp->b_data + bp->b_cc;
	for ( i = 0; i < nblocks; ++i ) {
		lseek( fsys, fsbtodb( bb[i].bn_bno ) * DBSIZE, 0 );
		if ( read( fsys, start + (bb[i].bn_index * fsbsiz), fsbsiz ) != fsbsiz ) {
			fprintf( stderr, "grab: error reading filesystem\n" );
			exit( 10 );
		}
	}
	free( bb );

	/*
	 * Update the bmap.  Pad out holes.
	 */
	if ( bno == 0L ) {
		bp->b_type	= B_HOLE;
		nbytes		+= fsbsiz;
	} else
		bp->b_type	= B_NORMAL;
	bp->b_cc	+= nbytes;
	bp->b_offset	+= nbytes;
# ifdef	DEBUG
	bdump( bp );
# endif
	return ( nbytes );
}



/*
 * bcomp( b1, b2 ) -- Compare block numbers in bstr's b1 and b2.
 */
bcomp( b1, b2 )
    struct bstr
    {
	long	bn_bno;
	int	bn_index;
    }
	*b1, *b2;
{
	if ( b1->bn_bno < b2->bn_bno )
		return( -1 );
	return( b1->bn_bno > b2->bn_bno );
}



/*
 * bput( ofile, bp ) -- Do a buffered transfer to ofile using data in bp.
 */
bput( ofile, bp )
    int                   ofile;
    register struct bmap *bp;
{
	register int	n;
	register char  *cp;
	register char  *dp;
	long		count;
	int		err;
	int		saveblen;
	long		saveboffset;

# ifdef	DEBUG
	fprintf( stderr, "Entering bput\n" );
# endif
	/*
	 * Special case for holes in files...  Bleah.
	 */
	if ( bp->b_type == B_HOLE )
		if ( tflag || xflag ) {
			/*
			 * Deal with tape output.  In this case we need to zero
			 * out the last block brought in since holes in tape are
			 * inconvenient (to say the least)...
			 */
			cp		= bp->b_data+(bp->b_cc-fsbsiz);
			n		= fsbsiz;

			CLRBUF( cp, n );
		} else {
			/*
			 * Regular files need holes made in them.  Force all
			 * current buffered data out now & pick up again later.
			 * Note that b_offset, b_cc are >= fsbsiz after bread.
			 */
			saveblen	= bp->b_len;
			saveboffset	= bp->b_offset;
			bp->b_offset	-= fsbsiz;
			bp->b_len	= bp->b_cc - fsbsiz;
			bp->b_cc	= bp->b_len;
		}

	/*
	 * Output the buffer, if necessary.
	 */
	cp		= bp->b_data;
	dp		= bp->b_data + bp->b_len;
	n		= bp->b_cc - bp->b_len;
	if ( bp->b_size < bp->b_offset )
		/*
		 * Correct the count for overshooting.
		 */
		count		= bp->b_cc - (bp->b_offset - bp->b_size);
	else
		count		= bp->b_cc;

	while ( count > 0 ) {
		if ( count < bp->b_len ) {
			if ( tflag ) {
				/*
				 * Only write full buffers.
				 */
				break;
			}
			err	= write( ofile, bp->b_data, (int) count );
			count		= 0;
		} else {
			err	= write( ofile, bp->b_data, bp->b_len );
			count		-= bp->b_len;
		}
		if ( err < 0 ) {
			fprintf( stderr, "grab: write error\n" );
			exit( 11 );
		}
		if ( count > 0 )
			CPYBUF( cp, dp, n );
	}
	bp->b_cc	= count;

	/*
	 * Finish dirty work with holes.
	 */
	if ( bp->b_type == B_HOLE && ! (tflag || xflag) ) {
		lseek( ofile, min( (long) fsbsiz, bp->b_size-bp->b_offset ), 1 );
		bp->b_offset	= saveboffset;
		bp->b_len	= saveblen;
	}
}



/*
 * bclose( ofile, name, ip, bp ) -- Wrap up a file named name with output
 *	channel ofile and inode ip and deallocate the buffers of its bmap bp.
 */
bclose( ofile, name, ip, bp )
    int			   ofile;
    char		  *name;
    register struct inode *ip;
    register struct bmap  *bp;
{
	freebuf( bp );

	if ( ! xflag && ! tflag ) {
		close( ofile );
# ifdef CLR_SETUID
		if ( pflag && (ip->i_mode & (ISUID|ISGID|ISVTX)) )
			chmod( name, ip->i_mode & 07777 );
# endif CLR_SETUID
# ifndef PDPV6
		if ( pflag )
			utime( name, &ip->i_atime );
# endif
	}

	if ( vflag ) {
		printf( "%s\n", name );
		fflush( stdout );
	}
}



/*
 * v7bnext( bp ) -- get the next file system block from the list
 *	associated with bp.  [Version 7]
 */
long
v7bnext( bp )
    register struct bmap *bp;
{
	long	lbn = bp->b_lbno++;

	/*
	 * Blocks 0 thru V7NADDR-4 are direct blocks.
	 */
	if ( lbn < V7NADDR - 3 )
		return ( bp->b_iaddr[lbn] );

	/*
	 * An indirect block is needed.
	 */
	++bp->b_count[0];
	v7indir( bp, 0 );
	return ( ((long *) bp->b_indir[0]) [ bp->b_count[0] ] );
}



/*
 * v7indir( bp, level ) -- make sure that the needed block number is
 *	available from the indirect blocks.
 */
v7indir( bp, level )
    register struct bmap   *bp;
    register int            level;
{
	long	bno;
	char   *malloc();

	if ( level >= 3 ) {
		/*
		 * Max. three levels of indirection!
		 */
		fprintf( stderr, "grab: file overflow\n" );
		exit( 12 );
	}

	/*
	 * Has an indirect block of this level been looked at yet?
	 */
	if ( bp->b_indir[level] == NULL ) {
		bp->b_indir[level]	= malloc( fsbsiz );
		if ( bp->b_indir[level] == NULL ) {
			fprintf( stderr, "grab: out of memory\n" );
			exit( 13 );
		}
		bno	= bp->b_iaddr[(V7NADDR-3) + level];
	}

	/*
	 * Have we looked at all the blocks in this indirect block?
	 */
	else
	if ( bp->b_count[level] >= fsbsiz / sizeof (long) ) {
		++bp->b_count[level + 1];
		v7indir( bp, level + 1 );
		bno	= ((long *) bp->b_indir[level+1]) [ bp->b_count[level+1] ];
	}

	/*
	 * This indirect block is still valid.
	 */
	else
		return;

	/*
	 * We need to read in a new indirect block.
	 */
	lseek( fsys, fsbtodb( bno ) * DBSIZE, 0 );
	if ( read( fsys, bp->b_indir[level], fsbsiz ) < 0 ) {
		fprintf( stderr, "grab: error reading filesystem\n" );
		exit( 14 );
	}
	bp->b_count[level]	= 0;

	/*
	 * Swap words if necessary.
	 */
	switch ( target_system ) {
	case V7_2BSD:
# ifndef PDP
		wswap( bp->b_indir[level], fsbsiz / sizeof (long) );
# endif
		break;
	case V7_4BSD:
# ifdef PDP
		wswap( bp->b_indir[level], fsbsiz / sizeof (long) );
# endif
		break;
	}
}




/*
 * v6bnext( bp ) -- get the next file system block number from the list
 *	associated with bp.  [Version 6]
 */
long
v6bnext( bp )
    register struct bmap *bp;
{
	long	lbn = bp->b_lbno++;

	/*
	 * If the file has small format then we use the direct blocks.
	 */
	if ( (bp->b_mode & V6_ILARGE) == 0 ) {
		if ( lbn >= 8 ) {
			fprintf( stderr, "grab: V6 small format file too big\n" );
			return ( 0 );
		}
		return ( (unsigned short) bp->b_iaddr[lbn] );
	}

	/*
	 * An indirect block is needed.
	 */
	++bp->b_count[0];
	v6indir( bp, 0 );
	return ( ((unsigned short *) bp->b_indir[0]) [ bp->b_count[0] ] );
}



/*
 * v6indir( bp, level ) -- make sure that the needed block number is
 *	available from the indirect blocks.
 */
v6indir( bp, level )
    register struct bmap   *bp;
    register int            level;
{
	long	bno;
	char   *malloc();

	if ( level >= 2 ) {
		fprintf( stderr, "grab: file overflow\n" );
		exit( 15 );
	}

	/*
	 * Has an indirect block of this level been looked at yet?
	 */
	if ( bp->b_indir[level] == NULL ) {
		bp->b_indir[level]	= malloc( fsbsiz );
		if ( bp->b_indir[level] == NULL ) {
			fprintf( stderr, "grab: out of memory\n" );
			exit( 16 );
		}
		bno	= (unsigned short) bp->b_iaddr[ level == 0 ? 0 : 7 ];
	}

	/*
	 * Have we looked at all the blocks in this indirect block?
	 */
	else
	if ( bp->b_count[level] >= fsbsiz / sizeof (short) ) {
		register int	n = bp->b_lbno / (fsbsiz / sizeof(short));
		if ( n < 7 )
			/*
			 * One of 7 singly indirect blocks in the inode.
			 */
			bno	= (unsigned short) bp->b_iaddr[ n ];
		else {
			++bp->b_count[level + 1];
			v6indir( bp, level + 1 );
			bno	= ((unsigned short *) bp->b_indir[level+1]) [ bp->b_count[level+1] ];
		}
	}

	/*
	 * This indirect block is still valid.
	 */
	else
		return;

	/*
	 * We need to read in a new indirect block.
	 */
	lseek( fsys, fsbtodb( bno ) * DBSIZE, 0 );
	if ( read( fsys, bp->b_indir[level], fsbsiz ) < 0 ) {
		fprintf( stderr, "grab: error reading file system\n" );
		exit( 17 );
	}
	bp->b_count[level]	= 0;
}



/*
 * allocbuf( ip, bp, flag ) -- associate a block map structure bp and a buffer
 *	with ip, using flag to determine buffering mode.  Savetcc ought to be
 *	known only here but actually tflush uses it for a kluge...  Sigh.
 */

char   *bigbuf	= NULL;
int	savetcc	= 0;

allocbuf( ip, bp, flag )
    register struct inode *ip;
    register struct bmap  *bp;
    int		           flag;
{
	char	       *malloc();

	bp->b_type	= B_NORMAL;
	bp->b_mode	= ip->i_mode;
	bp->b_size	= ip->i_size;
	bp->b_offset	= 0L;
	bp->b_iaddr	= ip->i_addr;
	bp->b_lbno	= 0L;
	if ( flag == B_SMALL ) {
		bp->b_len	= fsbsiz;
		bp->b_cc	= 0;
		bp->b_data	= malloc( fsbsiz );
		if ( bp->b_data == NULL ) {
			fprintf( stderr, "grab: out of memory\n" );
			exit( 18 );
		}
	} else {
		bp->b_len	= nblock * TBLOCK;
		if ( tflag ) {
			/*
			 * Make sure to get tail end of last file in buffer.
			 * Also round up size to nearest TBLOCK.
			 */
			bp->b_size	= ((bp->b_size + (TBLOCK-1))/TBLOCK) * TBLOCK;
			bp->b_cc	= savetcc;
		} else
			bp->b_cc	= 0;
		if ( bigbuf == NULL ) {
			/*
			 * Allocate a large data buffer.  Leave enough slop for
			 * odd-numbered blocksize outputs so that input doesn't
			 * get truncated...
			 */
			bigbuf		= malloc( bp->b_len + fsbsiz );
			if ( bigbuf == NULL ) {
				fprintf( stderr, "grab: out of memory\n" );
				exit( 19 );
			}
		}
		bp->b_data	= bigbuf;
	}
	bp->b_indir[0]	= NULL;
	bp->b_indir[1]	= NULL;
	bp->b_indir[2]	= NULL;
	bp->b_count[0]	= -1;
	bp->b_count[1]	= -1;
	bp->b_count[2]	= -1;
}



/*
 * freebuf( bp ) -- release all the data and indirect block buffers
 *	that are tied up with bp.  Don't reallocate a big data buffer.
 */
freebuf( bp )
    register struct bmap *bp;
{
	register int	n;

	if ( bp->b_data == bigbuf )
		savetcc	= bp->b_cc;
	else if ( bp->b_data != NULL )
		free( bp->b_data );
	for ( n = 0; n < 3; ++n )
		if ( bp->b_indir[n] != NULL )
			free( bp->b_indir[n] );
}



/*
 * Some debugging aids.  Idump() is also used for -I and -L options.
 */


/*
 * bdump( bp ) -- Print out the printable contents of bmap bp.
 */
bdump( bp )
    register struct bmap *bp;
{
	fprintf(stderr, "type=%d, mode=%o, b_size=%D, b_offset=%D\n",
		bp->b_type, (unsigned) bp->b_mode, bp->b_size, bp->b_offset);
	fprintf(stderr, "cc=%d, len=%d, lbno=%D, data=%o\n",
		bp->b_cc, bp->b_len, bp->b_lbno, bp->b_data );
}



/*
 * ddump( dp ) -- Print out the contents of directory entry dp.
 */
ddump( dp )
    struct direct *dp;
{
	fprintf( stderr, "--%d\t%s\n", (unsigned) dp->d_ino, dp->d_name );
}



/*
 * idump( ip, ino, name ) -- Print out the contents of inode ip with i-number
 *	ino and name name.
 */
idump( ip, ino, name )
   struct inode *ip;
   int		 ino;
   char		*name;
{
	register int	n;

	if ( name != NULL )
		fprintf( stderr, "Name:\t\t%.14s\n", name );
	if ( ino != 0 )
		fprintf( stderr, "Inode number:\t%u\n", ino );
	fprintf( stderr, "\tMode:\t\t%o\n", (unsigned short) ip->i_mode );
	fprintf( stderr, "\tSize:\t\t%D\n", ip->i_size );
	fprintf( stderr, "\tLink count:\t%d\n", ip->i_nlink );
	fprintf( stderr, "\tUid:\t\t%d\n", ip->i_uid );
	fprintf( stderr, "\tGid:\t\t%d\n", ip->i_gid );
	fprintf( stderr, "\tAccess time:\t%s", ctime( &(ip->i_atime) ) );
	fprintf( stderr, "\tModify time:\t%s", ctime( &(ip->i_mtime) ) );
	fprintf( stderr, "\tBlock numbers:" );
	for ( n = 0; n < (target_system == V6 ? V6NADDR : V7NADDR); ++n ) {
		if ( n % 4 == 0 )
			fprintf( stderr, "\n\t\t" );
		fprintf( stderr, "%10D  ", ip->i_addr[n] );
	}
	fprintf( stderr, "\n\n" );
}
