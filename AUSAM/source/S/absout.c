#
/*
 * absolute loader format to a.out format conversion
 */ 

/*
 * function:
 *	convert files in absolute loader format (paper-tape)
 *	to unix a.out format.
 * usage:
 *	absout infile outfile
 */ 

#define	MEMLIM	0100000	/* 32768 - 16k */

struct ubuf
{
	int fildes;	/* file desriptor */ 
	int nleft;	/* chars left in buffer */ 
	char *nextp;	/* ptr. to next character */ 
	char buff[512];	/* the BUFFER! */ 
};

struct
{
	char lobyte, hibyte;
};

struct ubuf fin;
int fout_file;

char mem[MEMLIM];
int hdr[]
{

	0407, 0, 0, 0, 0, 0, 0, 1
};
char *maxmem;
int chksum;

main(argc, argv)
int argc;
char *argv[];

{
	int register stat;


	if(argc != 3)
	{
		prints(2, "Usage: absout input-file output-file\n");
		exit(1);
	}
	if(fopen(argv[1], &fin) < 0)
	{
		perror(argv[1]);
		exit(1);
	}
	if((fout_file = creat(argv[2], 0700)) < 0)
	{
		perror(argv[2]);
		exit(1);
	}
	clear();
	while((stat = loadrec()) > 0);
	if(stat < 0)
	{
		prints(2, "bad input, end assumed\n");
	}
	if(!maxmem)
	{
		prints(2, "no program\n");
		exit(1);
	}
	aoutit();
	close(fout_file);
}

clear()
{
	register char *i;
	register int *ptr;


	i = (sizeof(mem)>>1)&077777;
	ptr = &mem[0];
	do
	{
		*ptr++ = 0;
	}
	while(--i);
	maxmem = 0;
}

loadrec()
{
	int count, adr;	/* note: these must be memory locns. */ 
	int register ct;
	char register *ptr;
	register int c;
	int flg;

	/* skip leader */ 


	while(!(c = getbyte()))
		chksum = 0;

	if((c != 1) || getbyte())
		return(-1);

	if(getword(&count) || getword(&adr))
		return(-1);

	if((count =- 6) < 0)
	{
		/* bad count */ 
		return(-1);
	}
	if(count)
	{
		/* real data record */ 
		ct = count;
		ptr = &mem[adr];
		if((adr =+ count) > maxmem)
		{
			maxmem = adr;
			if(maxmem > MEMLIM)
			{
				prints(2, "memory limit exceeded\n");
				return(-1);
			}
		}
		do
		{
			if((c = getbyte()) < 0)
				return(-1);
			*ptr++ = c;
		}
		while(--ct);
		flg = 1;
	}
	else
	{
		/* start address */ 
		if(adr >= maxmem)
		{
			prints(2, "start address outside program\n");
			return(-1);
		}
		/*
		 * room here for putting start address
		 * in a.out file header, but unix can't cope
		 * at moment so refrain.
		 * hdr[5] = adr ;
		 *
		 */ 
		hdr[5] = adr|1;
		flg = 0;
	}
	getbyte();	/* check-sum */ 
	if(chksum&0377)
		return(-1);
	return(flg);
}

aoutit()
{
	int register ct, i;
	int register *ptr;


	ct = (maxmem+1)&~1;
	hdr[2] = ct;
	write(fout_file, hdr, 16);
	write(fout_file, mem, ct);
}

getbyte()
{
	register int c;


	if((c = getc(&fin)) < 0)
		return(-1);
	chksum =+ c;
	return(c);
}

getword(word)
int *word;

{
	register int c;


	if((c = getbyte()) < 0)
		return(-1);
	word->lobyte = c;
	if((c = getbyte()) < 0)
		return(-1);
	word->hibyte = c;
	return(0);
}
