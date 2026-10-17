#
#include <local-system>
/* Huffman code input to output with key
 * Output file format:
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
#define LNAME  80
#define PACKED 017437 /* <US><US> - Unlikely value */

struct node
{       struct node *zlink;  /* 0 link */
	struct node *olink;  /* 1 link */
#ifndef FLOATING_PACK
	long freq;
#endif FLOATING_PACK
#ifdef FLOATING_PACK
	float freq;
#endif FLOATING_PACK
	struct node *sortl;  /* Pointer to next lower frequency node */
	struct node *sorth;  /* Pointer to next higher frequency node */
} nodes[NNODES], *root, *leaves[256], sortstart, max;

struct
{       char minor;
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

int used, depth, freqflag ;
int tree[1024]; /* Stores tree in puttree; codes in gcode and encoding */
struct {int integ;};
char code[5];
extern errno;

struct iobuf
{       int fildes;
	int nleft;
	char *nextp;
	char buf[512];
} buf, obuf;

main(argc, argv)
int argc; char *argv[];
#ifndef FLOATING_PACK
{       long nchars; /* Bits, then chars in output file */
#endif FLOATING_PACK
#ifdef FLOATING_PACK
{       double nchars; /* Bits, then chars in output file */
#endif FLOATING_PACK
	register struct node *n;
	register int i;
	register char *cp;
	char filename[LNAME];
	struct node *order[256];
	int j, k, sep, treesize, ncodes;

	struct {int hi, lo;} ;
	struct {char byte;} ;

	for (k=1; k<argc; k++)
	{       if (argv[k][0] == '-' && argv[k][1] == '\0')
		{       freqflag = 1 - freqflag;
			continue;
		}

		sep = -1;  cp = filename;
		for (i=0; i < (LNAME-3) && (*cp = argv[k][i]); i++)
			if (*cp++ == '/') sep = i;
#ifdef	AGSM
		if (cp[-1]==SUF1 && cp[-2]==SUF0)
		{	printf ("%s: Already packed\n", filename);
			continue;
		}
#endif	AGSM
		if (i >= (LNAME-3) || (i-sep) > 13)
		{       printf ("%s: File name too long\n",argv[k]);
			continue;
		}
		if (fopen(filename,&buf) < 0)
		{       printf ("%s: Unable to open\n", argv[k]);
			continue;
		}

#ifdef	AUSAM
		newfstat(buf.fildes,&status);
#endif	AUSAM
#ifndef	AUSAM
		fstat(buf.fildes,&status);
#endif	AUSAM
		if( (status.flags&060000) != 0 )
		{	printf ("%s: Not a plain file\n",filename);
			goto closein;
		}
		if( status.nlinks != 1 )
		{	printf("'%s' has links\n", filename);
			goto closein;
		}

		*cp++ = SUF0;  *cp++ = SUF1;  *cp = '\0';
		if( stat(filename, obuf.buf) != -1)
		{
			printf("%s: Already exists\n", filename);
			goto closein;
		}
		if (fcreat(filename,&obuf) == -1)
		{       printf ("%s: Unable to create\n", argv[k]);
			goto closein;
		}

		chmod(filename,status.flags&07777);
#ifdef	AUSAM
		chown(filename, status.uid);
#endif	AUSAM
#ifndef	AUSAM
		chown(filename,(status.gid<<8)|status.uid); /* IAN J Jan '77 */
#endif	AUSAM
		errno = used = 0;
		for (i = 256; i--; ) leaves[i] = 0;

		sortcount();

		if (used < 2)
		{       printf ("%s: Trivial file\n", argv[k]);
			goto forgetit;
		}

		n = &max;
		for (i = ncodes = used; i--; )
			order[i] = n = n->sortl;

		formtree();
		putw(PACKED,&obuf);
		putw(root->freq.hi, &obuf);
		putw(root->freq.lo, &obuf);
		treesize = puttree();

		depth = 0;  /* Reset for reuse by gcode */
		gcode(0,root); /* leaves[i] now points to code for i */

		if (freqflag)  /* Output stats */
#ifndef FLOATING_PACK
		{       printf ("\n%s: %D Bytes\n",argv[k],root->freq);
#endif FLOATING_PACK
#ifdef FLOATING_PACK
		{       printf ("\n%s: %.0f Bytes\n",argv[k],root->freq);
#endif FLOATING_PACK
			for (i=ncodes; i--; )
			{   n = order[i];
#ifndef FLOATING_PACK
			    printf ("%10D%8D%% <%3o> = <",
			       n->freq, 100*n->freq/root->freq,
#endif FLOATING_PACK
#ifdef FLOATING_PACK
			    printf ("%10.0f%8.3f%% <%3o> = <",
			       n->freq, 100.*n->freq/root->freq,
#endif FLOATING_PACK
			       n->olink.integ&0377);
			    if (n->olink.integ<040 || n->olink.integ > 0177)
			       printf (">   ");
			    else
			       printf ("%c>  ",n->olink.integ);
			    cp = leaves[n->olink.integ];
			    for (j=0; j < *cp; j++)
			       putchar('0' +
				  ((cp[1+(j>>3)] >> (7-(j&07)))&01));
			    putchar('\n');
			}
		}

		nchars = 0;
		for (i=ncodes; i--; )
			nchars =+ (nodes[i].freq *
				((leaves[nodes[i].olink.integ]->byte)&0377));
		nchars = (nchars + 7)/8 + treesize + 8;
#ifndef	FLOATING_PACK
		if (freqflag) printf ("%s: Packed size: %D bytes\n",
#endif FLOATING_PACK
#ifdef FLOATING_PACK
		if (freqflag) printf ("%s: Packed size: %.0f bytes\n",
#endif FLOATING_PACK
			argv[k], nchars);
		/* If compression won't save a block, forget it */
		if ((i = (nchars+511)/512) >= (j = (root->freq+511)/512))
		{       printf ("%s: Not packed (no blocks saved)\n", argv[k]);
			goto forgetit;
		}

		seek(buf.fildes,0,0);
		buf.nleft = 0;

		if (compress() == 0)
		{
			unlink(argv[k]);
#ifndef FLOATING_PACK
			printf ("%s: %D%% Compression\n", argv[k],
				100*(root->freq - nchars)/root->freq);
#endif FLOATING_PACK
#ifdef FLOATING_PACK
			printf ("%s: %.0f%% Compression\n", argv[k],
				100.*(root->freq - nchars)/root->freq);
#endif FLOATING_PACK
		}
		else
		{
			perror( filename );
			printf ("%s: I/O Error - File unchanged\n", argv[k]);
	forgetit:       unlink(filename);
		}

   closeboth:	close (obuf.fildes);
     closein:	close (buf.fildes);
		smdate( filename , status.modtime ); /* preserve modified time */
	}
}

sortcount()
{       register struct node *p, *q;
	register int c;

	max.sorth = sortstart.sorth = &max;
	sortstart.sortl = max.sortl = &sortstart;
#ifndef FLOATING_PACK
	max.freq = 2000000000;
#endif FLOATING_PACK
#ifdef FLOATING_PACK
	max.freq = 1.0e+30;
#endif FLOATING_PACK

	while ((c = getc(&buf)) >= 0)
	{       if ((p = leaves[c]) == 0)
		{       p = leaves[c] = &nodes[used++];
			p->zlink = 0;
			p->olink.integ = c;
			p->freq = 1;
			q = p->sorth = sortstart.sorth;
			p->sortl = &sortstart;
			q->sortl = sortstart.sorth = p;
		}
		else
		{       if ((p->freq =+ 1) > (q = p->sorth)->freq)
			{       do
				{       q = q->sorth;
				} while (q->freq < p->freq);
				/* Move node p in front of node q */
				p->sortl->sorth = p->sorth;
				p->sorth->sortl = p->sortl;
				p->sortl = q->sortl;
				p->sorth = q;
				q->sortl->sorth = p;
				q->sortl = p;
			}
		}
	}
}

formtree()      /* Form Huffman code tree */
{       register struct node *p, *q, *r;
	p = sortstart.sorth;

	while ((q = p->sorth) != &max)
	{       /* Create a new node by combining
		 * the two lowest frequency nodes
		 */
		r = &nodes[used++];
		r->freq = p->freq + q->freq;
		r->zlink = p;
		r->olink = q;
		p = q->sorth;
		while (r->freq > p->freq)
			p = p->sorth;
		r->sortl = p->sortl;
		r->sorth = p;
		p->sortl->sorth = r;
		p->sortl = r;
		p = q->sorth;
	}
	return (root = p);
}
puttree()  /* Returns tree size (bytes) */
{       register int i,j;
	register struct iobuf *b;
	struct {char *charp;} ;
	int extra; /* full words in tree */

	extra = depth = 0;
	maketree(root);
	b = &obuf;
	putw(depth, b); /* Size of tree */

	for (i = 0; i<depth; i++)
	{       j = tree[i];
		if (j.charp < 0377)
			putc(j,b);
		else
		{       putc(0377,b);
			putw(j,b);
			extra++;
		}
	}
	return (depth + extra*2);
}

maketree(no)
struct node *no;
{       register int d;
	register struct node *n;

	n = no;
	d = depth;
	depth =+ 2;
	if (n->zlink == 0) /* Terminal node */
	{       tree[d] = 0;
		tree[d+1] = n->olink.integ;
	}
	else
	{       tree[d] = maketree(n->zlink) - d;
		tree[d+1] = maketree(n->olink) - d;
	}
	return(d);
}

gcode(len,nod)  /* Recursive routine to compute code table */
int len;  /* code length at time of call */
struct node *nod;
{       register struct node *n;
	register int l, bit;
	int i;
	char *t, *u;
	struct {char ch[];};

	n = nod;
	l = len;
	if (n->zlink == 0)
	{       /* Terminal.  Copy #bits and code, set pointer in leaves */
		/* Treat tree as char array */
		leaves[n->olink.integ] = t = &tree->ch[depth];
		*t++ = l;
		depth++;
		u = code;
		do
		{       *t++ = *u++;
			depth++;
		} while ((l =- 8) > 0);
	}
	else
	{       bit = 0200 >> (l&07);
		code[i = l>>3] =& ~bit;
		gcode(++l, n->zlink);
		code[i] =| bit;
		gcode(l,n->olink);
	}
}

compress() /* Here's the time consumer */
{       register int i, word, bits;
	int c;
	char *p;

	bits = 0;
	while ((i = getc(&buf)) >= 0)
	{       p = leaves[i];
		c = *p++ & 0377;
		for (i=0; i<c; i++) /* Output c bits */
		{       word =<< 1;
			if ((p[i>>3] << (i&07)) & 0200)
				word++;  /* Logical Or 1 */
			++bits;
			if ((bits =& 017) == 0)
			{
				putw(word, &obuf);
				if ( errno )
					return( 1 );
			}
		}
	}
	if (bits) putw(word << (16-bits), &obuf);
	fflush(&obuf);
	return( errno );
}
