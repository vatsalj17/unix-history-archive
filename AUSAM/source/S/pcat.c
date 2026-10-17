#
/* Expand Huffman coded input to standard output
 * Input file format:
 *      PACKED flag defined below (integer)
 *      Number of chars in expanded file (float)
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

#define NNODES 512
struct iobuf
{       int fildes;
	int nleft;
	char *nextp;
	char buf[512];
} buf, obuf;

extern fout;
#define LNAME  80
#define PACKED 017437 /* <US><US> - Unlikely value */

long size;
union {
	int hilo[2];
	float fsize;
	long lsize;
} q;
int tree[1024];

main(argc, argv)
int argc; char *argv[];
{       register int i, k, *t;
	int sep, keysize;
	char filename[LNAME], *cp;

	fout = 2;
	obuf.fildes = 1;
	obuf.nleft = 512;
	obuf.nextp = obuf.buf;

	for (k = 1; k<argc; k++)
	{       sep = -1;  cp = filename;
		for (i=0; i < (LNAME-3) && (*cp = argv[k][i]); i++)
			if (*cp++ == '/') sep = i;
		if (cp[-1]==SUF1 && cp[-2]==SUF0)
		{       argv[k][i-2] = '\0'; /* Remove suffix and try again */
			k--;
			continue;
		}

		if (i >= (LNAME-3) || (i-sep) > 13)
		{       printf ("File name too long -- %s\n",argv[k]);
			continue;
		}
		*cp++ = SUF0;  *cp++ = SUF1;  *cp = '\0';
		if (fopen(filename,&buf) < 0)
		{       printf ("Unable to open %s\n", filename);
			continue;
		}

		if (getw(&buf) != PACKED)
		{	printf ("Unable to unpack %s\n", filename);
			close(buf.fildes);
			continue;
		}
		q.hilo[0] = getw(&buf);
		q.hilo[1] = getw(&buf);
		if( q.hilo[0] > 040000)		/* a float */
			size = q.fsize;
		else
			size = q.lsize;		/* a long  */
		t = tree;
		for (keysize = getw(&buf); keysize--; )
		{       if ((i = getc(&buf)) == 0377)
				*t++ = getw(&buf);
			else
				*t++ = i;
		}

		expand();

		fflush(&obuf);

		close(buf.fildes);
	}
}

expand()
{       register int tp, bit, word;

	bit = tp = 0;
	for (;;)
	{       if (bit == 0)
		{       word = getw(&buf);
			bit = 16;
		}
		tp =+ tree[tp + (word<0)];
		word =<< 1;  bit--;
		if (tree[tp] == 0)
		{       putc(tree[tp+1],&obuf);
			tp = 0;
			if (--size == 0) return;
		}
	}
}
