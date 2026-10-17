#
#include <local-system>
/* Expand Huffman coded input to
 * Input file format:
 *      PACKED flag defined below (integer)
 *      Number of chars in expanded file (float or long)
 *      Number of words in expanded tree (integer)
 *      Tree in 'compressed' form:
 *              If 0<=byte<=0376, expand by zero padding to left
 *              If byte=0377, next two bytes for one word
 *          Terminal nodes: First word is zero; second is character
 *          Non-terminal nodes: Incremental 0/1 pointers
 *      Code string for number of characters in expanded file
 */

#define	SUF0	'.'
#define	SUF1	'z'

struct iobuf
{
	int fildes;
	int nleft;
	char *nextp;
	char buf[512];
} buf, obuf;

struct
{
	char minor;
	char major;
	int inumber;
	int flags;
#ifdef	AUSAM
	unsigned uid;
#endif	AUSAM
	char nlinks;
#ifndef	AUSAM
	char uid;
	char gid;
#endif	AUSAM
	char size0;
	int size1;
	int addr[8];
	long actime;
	long modtime;
} status;

#define LNAME  80
#define PACKED 017437 /* <US><US> - Unlikely value */

long size;
long size;
union
{
	int hilo[2];
	float fsize;
	long lsize;
} q;

struct {int hi, lo;};
int tree[1024];

main(argc, argv)
int argc; char *argv[];
{
	register int i, k, *t;
	int sep, keysize;
	char filename[LNAME], *cp;
/**/ int *u;

	for (k = 1; k<argc; k++)
	{
		sep = -1;  cp = filename;
		for (i=0; i < (LNAME-3) && (*cp = argv[k][i]); i++)
			if (*cp++ == '/') sep = i;

		sep = i - sep - 1;		/* nr of chars in name */
		if (i >= (LNAME-3) || sep > 14)
		{
namerr:			printf ("File name too long -- %s\n", argv[k]);
			continue;
		}
		if (cp[-1] == SUF1 && cp[-2] == SUF0 && fopen(filename, &buf) >= 0) argv[k][i - 2] = '\0';
		else
		{
			if (sep > 12) goto namerr;
			*cp++ = SUF0;  *cp++ = SUF1;  *cp = '\0';
			if (fopen(filename, &buf) < 0)
			{
				printf ("Unable to open %s\n", filename);
				continue;
			}
		}

		if (getw(&buf) != PACKED) 
		{
			printf ("Unable to unpack %s\n", filename);
			close(buf.fildes);
			continue;
		}

		if( stat(argv[k], &status) != -1 )
		{
			printf("%s: Already exists\n", argv[k]);
			close(buf.fildes);
			continue;
		}
#ifdef	AUSAM
		newfstat(buf.fildes, &status);
#endif	AUSAM
#ifndef	AUSAM
		fstat(buf.fildes, &status);
#endif	AUSAM
		if ( status.nlinks != 1)
			printf("Warning: '%s' has links\n", filename);
		if (fcreat(argv[k], &obuf) == -1)
		{
			printf ("Unable to create %s\n", argv[k]);
			close(buf.fildes);
			continue;
		}

		chmod(argv[k], status.flags&07777);
#ifdef	AUSAM
		chown( argv[k], status.uid); /* IAN J Jan '77 */
#endif	AUSAM

		q.hilo[0] = getw(&buf);
		q.hilo[1] = getw(&buf);
		if( q.hilo[0] > 040000)		/* a float */
			size = q.fsize;
		else
			size = q.lsize;		/* a long  */
		t = tree;
		for (keysize = getw(&buf); keysize--; )
		{
			if ((i = getc(&buf)) == 0377)
				*t++ = getw(&buf);
			else
				*t++ = i;
		}
/**//* for (u=tree; u<t; u++ ) printf("%4d: %6d  <%3o> %c\n",   */
/**//*            u-tree, *u, *u&0377, *u);                     */

		if ( expand() == 0 )
		{
			fflush(&obuf);
			unlink(filename);
			smdate( argv[k] , status.modtime ); /* preserve modified date */
		}
		else
		{
			perror( argv[k] );
			printf( "%s: I/O Error - File unchanged\n", filename );
			unlink( argv[k] );
		}

		close(buf.fildes);
		close(obuf.fildes);
	}
}

expand()
{
	register int tp, bit, word;
	extern	errno;

	bit = tp = 0;
	for (;;)
	{
		if (bit == 0)
		{
			word = getw(&buf);
			bit = 16;
		}
		tp =+ tree[tp + (word<0)];
		word =<< 1;  bit--;
		if (tree[tp] == 0)
		{
			putc(tree[tp+1], &obuf);
			if ( errno )
				return( 1 );
			tp = 0;
			if ((size =- 1) == 0) return( 0 );
		}
	}
}
