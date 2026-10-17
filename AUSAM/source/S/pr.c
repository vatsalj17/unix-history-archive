#
#include <local-system>
/*
 *   print file with headings
 *  2+head+2+page[PAGESIZE-10]+5
 *
 *	additional flags by dave h:
 *
 *	-f:		use form feed rather than new lines
 *			to take a new page (save time & space)
 *
 *	-pn:		page 1 is renumbered "n"
 *
 *	-b string:	specify a subtitle to be printed
 *			instead of blank line after title
 *
 *	also, funny flags are now ignored
 */ 

int ncol 1;
char *header;
int col;
int icol;
int file;
char *bufp;
int maxbufs;
int bufs;
extern end[];
char *buffer end;
#define	FF	014
int line;
char *colp[72];
int nofile;
char isclosed[10];
int peekc;
int fpage;
int page;
int colw;
int nspace;
int width 72;
int length PAGESIZE;	/* defined in local-system */ 
#define DEFMARGIN 10		/* default margin size */
int plength PAGESIZE-DEFMARGIN+5;
int margin DEFMARGIN;
int ntflg;
int mflg;
int tabc;
char *tty;
int mode;
int form;	/* daveh */ 
int pinit 1;	/* daveh */ 
char *subhdr "";	/* daveh */ 
extern long time();

struct inode
{
	int dev;
	int inum;
	int flags;
	char nlink;
	char uid;
	char gid;
	char siz0;
	int size;
	int ptr[8];
	long atime;
	long mtime;
};

main(argc, argv)
char **argv;
{
	int nfdone;
	int onintr();
	extern fout;


	tty = "/dev/ttyx";
	fout = dup(1);
	close(1);
	if((signal(2, 1)&01) == 0)
		signal(2, onintr);
	fixtty();
	for(nfdone = 0; argc > 1; argc--)
	{
		argv++;
		if(**argv == '-')
		{
			switch(*++*argv)
			{
			case 'h':
				if(argc >= 2)
				{
					header = *++argv;
					argc--;
				}
				continue;

			case 't':
				ntflg++;
				continue;

			case 'l':
				length = getn(++*argv);
				continue;

			case 'w':
				width = getn(++*argv);
				continue;

			case 's':
				if(*++*argv)
					tabc = **argv;
				else
					tabc = '\t';
				continue;

			case 'm':
				mflg++;
				continue;

			case 'f':	/* daveh */ 
				form++;
				continue;

			case 'b':	/* daveh */ 
				if(argc >= 2)
				{
					subhdr = *++argv;
					argc--;
				}
				continue;

			case 'p':	/* daveh */ 
				pinit = getn(++*argv);
				continue;

			default:
				if(**argv >= '0' && **argv <= '9')
					ncol = getn(*argv);	/* daveh */ 
				continue;
			}
		}
		else if(**argv == '+')
		{
			fpage = getn(++*argv);
		}
		else
		{
			print(*argv, argv);
			nfdone++;
			if(mflg)
				break;
		}
	}
	if(nfdone == 0)
		print(0);
	flush();
	onintr();
}

onintr()
{

	chmod(tty, mode);
	exit(0);
}

fixtty()
{
	struct inode sbuf;
	extern fout;


	tty[8] = ttyn(fout);
	fstat(fout, &sbuf);
	mode = sbuf.flags&0777;
	chmod(tty, 0600);
}

print(fp, argp)
char *fp;
char **argp;
{
	struct inode sbuf;
	register int sncol, sheader;
	register char *cbuf;
	extern fout;


	if(ntflg)
		margin = 0;
	if(length <= margin)
		length = PAGESIZE;
	if(width <= 0)
		width = 72;
	if(ncol > width)
	{
		prints(2, "Too many columns.\n");
		exit(1);
	}
	bufs = length*width + 259*2;
	if( bufs > maxbufs )
	{
		/* allocate more space */
		if(brk(end + bufs) == -1)
		{
			perror("Buffer alloc");
			exit(1);
		}
		maxbufs = bufs;
	}
	if(mflg)
	{
		mopen(argp);
		ncol = nofile;
	}
	colw = width/ncol;
	width = colw * ncol;
	sncol = ncol;
	sheader = header;
	plength = length-5;
	if(ntflg)
		plength = length;
	if(--ncol < 0)
		ncol = 0;
	if(mflg)
		fp = 0;
	if(fp)
	{
		file = open(fp, 0);
		if(file == -1)
		{
			perror(fp);
			return;
		}
		fstat(file, &sbuf);
	}
	else
	{
		file = 0;
		sbuf.mtime = time();
	}
	if(header == 0)
		header = fp;
	cbuf = ctime(sbuf.mtime);
	cbuf[16] = '\0';
	cbuf[24] = '\0';
	page = pinit;	/* daveh */ 
	icol = 0;
	colp[ncol] = bufp = buffer;
	if(mflg == 0)
		nexbuf();
	while(mflg && nofile || (!mflg) && tpgetc(ncol) > 0)
	{
		if(mflg == 0)
		{
			colp[ncol]--;
			if(colp[ncol] < buffer)
				colp[ncol] = &buffer[bufs];
		}
		line = 0;
		if(ntflg == 0)
		{
			puts(" \n\n");
			puts(cbuf+4);
			puts(" ");
			puts(cbuf+20);
			puts("  ");
			puts(header);
			puts(" Page ");
			putd(page);
			puts("\n");
			puts(subhdr);
			puts("\n\n");	/* daveh */ 
		}
		putpage();
		if(ntflg == 0 && form == 0)	/* daveh */ 
			while(line < length)
				put('\n');
		if(form)
			putcp(FF);	/* prdl */ 
		page++;
	}
	if(file)
		close(file);
	ncol = sncol;
	header = sheader;
}

mopen(ap)
char **ap;
{
	register char **p, *p1;


	p = ap;
	while((p1 = *p++) && p1 != -1)
	{
		isclosed[nofile] = fopen(p1, &buffer[2*259*nofile]);
		if(isclosed[nofile] == -1)
		{
			perror(p1);
			nofile--;
		}
		if(++nofile >= 10)
		{
			write(2, "Too many args.\n", 15);
			exit(1);
		}
	}
}

putpage()
{
	register int lastcol, i, c;
	int j;


	if(ncol == 0)
	{
		while(line < plength)
		{
			while((c = tpgetc(0)) && c != '\n' && c != FF)
				putcp(c);
			if(c == 0 && form)
				return;	/* prdl */ 
			putcp('\n');
			line++;
			if(c == FF)
				break;
		}
		return;
	}
	colp[0] = colp[ncol];
	if(mflg == 0)
		for(i = 1; i <= ncol; i++)
		{
			colp[i] = colp[i-1];
			for(j = margin; j < length; j++)
				while((c = tpgetc(i)) != '\n')
					if(c == 0)
						break;
		}
	while(line < plength)
	{
		lastcol = colw;
		for(i = 0; i < ncol; i++)
		{
			while((c = pgetc(i)) && c != '\n')
				if(col < lastcol || tabc != 0)
					put(c);
			if(c == 0 && ntflg)
				return;
			if(tabc)
				put(tabc);
			else
				while(col < lastcol)
					put(' ');
			lastcol =+ colw;
		}
		while((c = pgetc(ncol)) && c != '\n')
			put(c);
		put('\n');
	}
}

nexbuf()
{
	register int n;
	register char *rbufp;


	rbufp = bufp;
	n = &buffer[bufs]-rbufp;
	if(n > 512)
		n = 512;
	if((n = read(file, rbufp, n)) <= 0)
		*rbufp = 0376;
	else
	{
		rbufp =+ n;
		if(rbufp >= &buffer[bufs])
			rbufp = buffer;
		*rbufp = 0375;
	}
	bufp = rbufp;
}

tpgetc(ai)
{
	register char **p;
	register int c, i;


	i = ai;
	if(mflg)
	{
		if((c = getc(&buffer[2*259*i])) < 0)
		{
			if(isclosed[i] == 0)
			{
				isclosed[i] = 1;
				if(--nofile <= 0)
					return(0);
			}
			return('\n');
		}
		if(c == FF && ncol > 0)
			c = '\n';
		return(c);
	}
loop:
	c = **(p = &colp[i])&0377;
	if(c == 0375)
	{
		nexbuf();
		c = **p&0377;
	}
	if(c == 0376)
		return(0);
	(*p)++;
	if(*p >= &buffer[bufs])
		*p = buffer;
	if(c == 0)
		goto loop;
	return(c);
}

pgetc(i)
{
	register int c;


	if(peekc)
	{
		c = peekc;
		peekc = 0;
	}
	else
		c = tpgetc(i);
	if(tabc)
		return(c);
	switch(c)
	{

	case '\t':
		icol++;
		if((icol&07) != 0)
			peekc = '\t';
		return(' ');

	case '\n':
		icol = 0;
		break;

	case 010:
	case 033:
		icol--;
		break;
	}
	if(c >= ' ')
		icol++;
	return(c);
}

puts(as)
char *as;
{
	register int c;
	register char *s;


	if((s = as) == 0)
		return;
	while(c = *s++)
		put(c);
}

putd(an)
{
	register int a, n;


	n = an;
	if(a = n/10)
		putd(a);
	put(n%10+'0');
}

put(ac)
{
	register int ns, c;


	c = ac;
	if(tabc)
	{
		putcp(c);
		if(c == '\n')
			line++;
		return;
	}
	switch(c)
	{

	case ' ':
		nspace++;
		col++;
		return;

	case '\n':
		col = 0;
		nspace = 0;
		line++;
		break;

	case 010:
	case 033:
		if(--col < 0)
			col = 0;
		if(--nspace < 0)
			nspace = 0;

	}
	while(nspace)
	{
		if(nspace > 2 && col > (ns = ((col-nspace)|07)))
		{
			nspace = col-ns-1;
			putcp('\t');
		}
		else
		{
			nspace--;
			putcp(' ');
		}
	}
	if(c >= ' ')
		col++;
	putcp(c);
}

getn(ap)
char *ap;
{
	register int n, c;
	register char *p;


	p = ap;
	n = 0;
	while((c = *p++) >= '0' && c <= '9')
		n = n*10+c-'0';
	return(n);
}

putcp(c)
{
	if(page >= fpage)
		putchar(c);
}
