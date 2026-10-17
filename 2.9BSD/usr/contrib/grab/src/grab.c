/*
 * grab.c
 *
 * Name:	grab
 * Purpose:	extract files from remote filesystems
 * Usage:	grab [-246pxvilLtb#] <filesystem> <files...>
 * Environment:	V6 or V7 (2.9BSD, 4.1BSD) Unix
 * Compile:	cc -O -s -D<type> grab.c find.c readi.c bread.c tmode.c -o grab
 * Date:	4/29/82
 * Author:	Donn Seeley RRCF UCSD
 * Remarks:
 *	This is meant to be used with dual-ported disks or controllers
 *	when it is either impossible or undesirable to mount another
 *	file system on that medium.  It should allow any Version 6 or
 *	Version 7 (2.9BSD, 4.1BSD) system to read filesystems from any
 *	Version 6 or Version 7 (2.9BSD, 4.1BSD) system even if the systems
 *	are not of the same type.  Version 6 systems will need at least
 *	a Phototypesetter level C compiler to make "grab".  "Grab" will
 *	copy any conceivable (?) object on a filesystem including directories
 *	(recursively), links, file holes, setuid/setgid/sticky files and
 *	device nodes.  "Grab" tries to prevent unauthorized readers from
 *	gazing at remote files but the only reliable way of maintaining
 *	security is to make "grab" setgid and then make fs's readable by
 *	group and not by other.  ("Df" should work this way too.)
 *
 *	To compile a V6 grab, define PDPV6.  To compile a 2.9 grab define
 *	PDP, and to compile a 4.1 grab define VAX.  If your machine has
 *	multiplexed files, define MPXFILES, and if it has symbolic links
 *	you should define SYMLINKS.
 *
 *	Options:
 *		-246	2.9, 4.1 or V6 system on the remote end.
 *		-p	Preserve all permissions and ownerships, instead of
 *			just some of them (should be su).
 *		-x	Act like "cat(1)" from the remote fs (but still
 *			recursively on directories).
 *		-v	Verbose mode: print filenames as they are copied.
 *		-i	The arguments are inode numbers; treat them as
 *			files with those inode numbers and named by the numbers.
 *		-l	Act like a rudimentary "ls(1)" on a remote fs.
 *		-L	Long version of -l: gives a gruesome inode dump.
 *		-t	Act like "tar(1)" on a remote fs.  Output goes to
 *			the standard output.  Digits following the "t" are
 *			interpreted as a blocksize.
 *		-b	Set blocksize independently: this manipulates the
 *			internal write buffer size.  Digits following, etc.
 *
 *---------------------------------------------------------------------------
 *
 * $Header:   RCS/grab.c.v  Revision 1.2  83/07/01  19:29:13  donn  Exp$
 * $Log:	RCS/grab.c.v $
 * Revision 1.2  83/07/01  19:29:13  donn
 * Added -i and -L flags for inode grabbing and inode dumping.
 * 							Donn
 * 
 */

# include	"grab.h"

int	target_system	= TARGET_DEFAULT;
int	fsbsiz		= FSB_DEFAULT;
int	fsys;
int	pflag		= 0;
int	xflag		= 0;
int	vflag		= 0;
int	iflag		= 0;
int	lflag		= 0;
int	Lflag		= 0;
int	tflag		= 0;
int	nblock		= B_BUFSIZE;




main( argc, argv )
    int	    argc;
    char   *argv[];
{
	char   *tail();
	char   *name;
	int	ino;

	if ( argc < 3 ) {
		fprintf( stderr, "grab: too few arguments\n" );
		fprintf( stderr, "Usage: grab [-246pxvilLtb] <file-system> <files>\n" );
		exit( 1 );
	}

	/*
	 * Process the command line.
	 */
	while ( *argv[1] == '-' ) {

		while ( *++argv[1] != '\0' )
			switch ( *argv[1] ) {

			/*
			 * Select type of filesystem to read.
			 */
			case '2':
				target_system	= V7_2BSD;
				fsbsiz		= 1024;
				break;
			case '4':
				target_system	= V7_4BSD;
				fsbsiz		= 1024;
				break;
			case '6':
				target_system	= V6;
				fsbsiz		= 512;
				break;

			/*
			 * Restore previous owner, group & protections.
			 */
			case 'p':
				++pflag;
# ifndef PDPV6
				umask( 0 );
# endif
				break;

			/*
			 * Put files on the standard output.
			 */
			case 'x':
				++xflag;
				break;

			/*
			 * Print names of copied files.
			 */
			case 'v':
				++vflag;
				break;

			/*
			 * Grab by inode number.
			 */
			case 'i':
				++iflag;
				break;

			/*
			 * List directories.
			 */
			case 'l':
				/*
				 * If only one file then we leave out header.
				 */
				lflag	= argc - 3;
				Lflag	= 0;
				break;

			/*
			 * Long listing.
			 */
			case 'L':
				++Lflag;
				lflag	= 0;
				break;

			/*
			 * Output files in "tar" format.
			 */
			case 't':
				++tflag;
				nblock	= TAR_BS_DEFAULT;
				/* FALL THRU */

			/*
			 * Pick up blocking factor (if any).
			 */
			case 'b':
				if ( *(argv[1] + 1) >= '0' &&
				     *(argv[1] + 1) <= '9' )
					nblock	= atoi( ++argv[1] );
				while ( *(argv[1]+1) >= '0' && *(argv[1]+1) <= '9' )
					++argv[1];

				break;

			default:
				fprintf( stderr, "grab: unknown option -%c\n", argv[1][1] );
				fprintf( stderr, "Usage: grab [-246pxvilLtb] <file-system> <files>\n" );
				exit( 2 );
			}
		++argv;
		--argc;
	}

	if ( argc < 3 ) {
		fprintf( stderr, "grab: too few arguments\n" );
		fprintf( stderr, "Usage: grab [-246pxvilLtb] <file-system> <files>\n" );
		exit( 3 );
	}



	/*
	 * If the standard output is a file, assume -x by default.
	 * This is for backward compatibility with Arthur Olsen's grab
	 * and may go away.
	 */
	if ( ! (xflag || tflag || lflag || Lflag) ) {
		int	statbuf[32];

		if ( fstat( STDOUT, statbuf ) == 0
# ifdef PDPV6
		 && (((struct inode *) statbuf)->i_mode & V6_IFMT) == V6_IFREG )
# else
		 && (((struct inode *) statbuf)->i_mode & V7_IFMT) == V7_IFREG )
# endif
			++xflag;
	}



	/*
	 * Try to separate file output and verbose mode output!
	 * The method here is a bit nonportable...
	 */
	if ( tflag || xflag )
		fileno( stdout )	= fileno( stderr );



	/*
	 * Open the file system device.
	 */
	fsys = open( argv[1], 0 );
	if ( fsys < 0 ) {
		fprintf( stderr, "grab: can't open %s\n", argv[1] );
		exit( 4 );
	}



	/*
	 * Set id to real id.  (V6 isn't setuid because nobody cares.)
	 */
# ifndef PDPV6
	setuid( getuid() );
	setgid( getgid() );
# endif



	/*
	 * Find the files corresponding to the file arguments
	 * and read them off.
	 */
	while ( argc > 2 ) {
		if ( iflag ) {
			name	= NULL;
			ino	= atoi( argv[2] );
		} else {
			name	= tail( argv[2] );
			ino	= find( argv[2] );
		}
		readi( ino, name );
		++argv, --argc;
	}



	/*
	 * Clean up on tape output.
	 * Tar puts out two blank records followed by junk
	 * from the buffer when it flushes the buffer...
	 */
	if ( tflag )
		tflush( TFILE );

	exit( 0 );
}
