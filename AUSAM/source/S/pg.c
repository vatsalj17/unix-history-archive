#
/*		PG, page through a file
 *
 *		13-jun-78
 *		Copyright Sape Mullender
 *		Informatics staff Vrije Universiteit Amsterdam
 *
 *	This program allows the user to page through a file.
 */ 

#define MAXBLOCKS 8192
#define BLOCKSIZ 2048
#define COREBLOCK 512
#define DOUBLE 2
#define TRUE 1
#define FALSE 0
#define out	1

/* Pg divides a file in blocks of size BLOCKSIZ characters. For each
 * block read pg remembers the line number of the first character of
 * that block. Thus pg can find a line of the file without much trouble.
 */ 

int fildes;
int printfile;	/* file descriptor of print file */ 
int printing;	/* non zero when printing is on */ 
int filpos;	/* current position on the file */ 
char *arg "standard input";	/* pointer to filename */ 
char instr[128];
char searchstr[128];

int screensize 22;	/* number of lines per screen */ 
int scrshft;	/* distance of curline from top of screen */ 
int anchored;	/* true for anchored searches */ 
long *lines;		/* lines[x] is line# of first char of blck x */ 
long curline -100;	/* line currently on display */
char block[2][BLOCKSIZ];	/* Two blocks can be kept in core */ 
int charcnt[2];	/* number of chars in each block */ 
int blockno[2];	/* block no of in core blocks */ 
long lastline;	/* number of last known line of the file */ 
int lastblock;	/* number of the highest block read in */ 
int lastknown;	/* true if last line of file is known */ 
int oldestblock;	/* number of oldest block in core */ 

main(argc, argv)
char **argv;
{
	register i, c;
	register char *ptr;
	long l, l1;
	int adr1[DOUBLE], adr2[DOUBLE];
	int reset();


	signal(1, 1);
	signal(2, 1);	/* signal(3,1);*/ 
	while(--argc && **++argv == '-')
	{
		flags(*argv);
	}
	if(argc > 1)
	{
		prints(2, "Usage: pg [-num] [file]\n");
		exit(1);
	}
	if(argc)
	{
		if((fildes = open(*argv, 0)) < 0)
		{
			prints(2, *argv);
			prints(2, ": cannot open\n");
			exit(1);
		}
		arg = *argv;
	}
	lines = sbrk(4);
	screensize = 22;
	lines[0] = 1;
	setexit();
	signal(2, &reset);
	while(getline())
	{
		ptr = instr;
		l = curline;
		if((c = *ptr++) >= '0' && c <= '9')
			ptr = readnum(instr, &l);
		else
			switch(c)
			{
			case '!':
				shell(ptr);
				continue;
			case '+':
			case '-':
				if(*ptr == '\n')
					if(c == '+')
						l =+ screensize;
					else
						l =- screensize;
				else
				{
					ptr = readnum(ptr, &l);
					if(c == '+')
					{
						l =+ curline;
					}
					else
					{
						l = curline - l;
					}
				}
				break;
			case '/':
				if(*ptr != '/')
					if(setsearch(ptr) == FALSE)
					{
						prints(2, "bad search string\n");
						continue;
					}
				if(search(&l) == FALSE)
				{
					prints(2, "can't find string\n");
					continue;
				}
				*ptr = '\n';
				break;
			case '\n':
				l =+ screensize;
				--ptr;
				break;
			case '$':
				l = 017777777777l;	/* somewhat large */
				break;
			case '.':
				break;
			case 'p':
				if((c = *ptr++) == ' ')
				{
					if(printfile)
						close(printfile);
					i = ptr;
					while(*ptr++ != '\n');
					*--ptr = 0;
					if((printfile = creat(i, 0644)) < 0)
					{
						prints(2, i);
						prints(2, ": cannot create\n");
						printfile = 0;
						continue;
					}
					printing++;
				}
				else
				{
					if(c == '+')
						if(printfile)
							printing++;
						else
						{
							prints(2, "no printfile\n");
							continue;
						}
					else if(c == '-')
						printing = 0;
					else
					{
						prints(2, "bad print cmd\n");
						continue;
					}
				}
			case 'r':
				if((i = atoi(ptr)) >= 0 && i < screensize)
					scrshft = i;
				continue;
			case 's':
				if((i = atoi(ptr)) > 0)
					screensize = i;
				continue;
			case 'q':
				if(*ptr == '\n')
					return 0;
			default:
				prints(2, "unknown command\n");
				continue;
			}
		if(*ptr != '\n')
		{
			prints(2, "bad format\n");
			continue;
		}
		getaddress(&l, adr1);
		curline = l;
		l =- scrshft;
		getaddress(&l, adr1);
		l1 = l;
		l =+ screensize;
		if(getaddress(&l, adr2) == FALSE)
		{
			/* line not on file */ 
			l1 = l - screensize;
			getaddress(&l1, adr1);
		}
		display(adr1, adr2);
		prints(2, "\t\t\t\t\tlines ");
		prints(2, locv(l1));
		prints(2, " to ");
		prints(2, locv(l));
		prints(2, ": ");
		prints(2, arg);
		prints(2, "\r");
	}
	prints(2, "\n");
	return 0;
}

getaddress(line, address)
int address[];
long *line;
{
	/* Find out the file address of line,
	 * Return 1 if successfull, 0 if not
	 * If line not on file, change line to closest line
	 * that is on the file and set address accordingly.
	 */ 

	register i, b;
	register char *j;
	long l;


	if(*line <= 0)
	{
		address[0] = address[1] = 0;
		*line = 1;
		/* get the appropriate block in core */ 
		getblock(0);
		return(FALSE);
	}
	if(*line > lastline)
	{
		if(lastknown)
		{
			/* The line is not on the file */ 
		not_found:
			*line = lastline;
			getaddress(line, address);
			return(FALSE);
		}
		while(newblock() && *line > lastline);
		if(*line > lastline)
		{
			/* The line is not on the file */ 
			goto not_found;
		}
	}
	/* The line is in the file, search to the right block */ 
	if(*line == 1)
	{
		/* line 1 is a special case,
		 * Prevent pg from reading block -1
		 */ 
		address[0] = address[1] = 0;
		getblock(0);
		return(TRUE);
	}
	for(i = 0; *line > lines[i+1]; i++);
	/* Now i contains the block number of the line-feed before
	 * the desired line, (the line itself might be in the next
	 * block) read the block in
	 */ 
	b = getblock(i);
	l = lines[i];
	j = block[b];
	do
	{
		while(*j++ != '\n');
		l++;
	}
	while(l < *line);
	address[0] = i;
	address[1] = j-block[b];
	return(TRUE);
}

newblock()
{
	long l;
	register int i, b;
	register char *p;

	/* Read a new block, update the lines table */ 


	l = lines[lastblock];
	/*
	 * We're going to read block "lastblock"
	 * If neccessairy seek to the right position first
	 */ 
	if(lastblock != filpos)
	{
		if(seek(fildes, lastblock*(BLOCKSIZ/512), 3) < 0)
		{
			/* seek failed */ 
			prints(2, "can't seek");
			reset();
		}
	}
	b = oldestblock++&1;
	charcnt[b] = 0;
	while((i = read(fildes, block[b]+charcnt[b], BLOCKSIZ-charcnt[b])) > 0 && (charcnt[b] =+ i) < BLOCKSIZ);
	if(charcnt[b])
	{
		/* at least something was read, it may not have
		 * been a whole block though
		 */ 
		p = block[b];
		for(i = 0; i < charcnt[b]; i++)
		{
			if(*p++ == '\n')
				l++;
		}
		blockno[b] = lastblock;
		if((lastblock%(COREBLOCK/4)) == 0)
		{
			/* ask for more memory */ 
			sbrk(COREBLOCK);
		}
		lastblock++;
		filpos++;
		lastline = lines[lastblock] = l;
		return(TRUE);
	}
	else
	{
		lastknown = TRUE;
		return(FALSE);
	}
}

getblock(num)
{
	/*
	 * get block nr num
	 * return the number of the block used
	 */ 
	register int i, b;


	if(num == blockno[0])
	{
		/* already in core */ 
		return(0);
	}
	if(num == blockno[1])
	{
		/* already in core */ 
		return(1);
	}
	if(filpos != num)
	{
		if(seek(fildes, num*(BLOCKSIZ/512), 3) < 0)
		{
			/* seek failed */ 
			prints(2, "bad seek\n");
			reset();
		}
	}
	b = oldestblock++&1;
	charcnt[b] = read(fildes, block[b], BLOCKSIZ);
	blockno[b] = num;
	filpos = num+1;
	return(b);
}

readnum(str, var)
char *str;
long *var;
{
	register char *p, c;
	register long *x;


	x = var;
	p = str;
	*x = 0;
	while((c = *p++) >= '0' && c <= '9')
	{
		*x =* 10;
		*x =+ c-'0';
	}
	return(p-1);
}

getline()
{
	register char *p;
	register c;


	prints(2, "* ");
	p = instr;
	c = read(2, p, sizeof instr - 1);
	p[c] = 0;
	return c;
}

display(adr1, adr2)
int *adr1, *adr2;
{
	register *a1, *a2, b;


	a1 = adr1;
	a2 = adr2;
	b = getblock(a1[0]);
	if(a1[0] == a2[0])
	{
		write2(&block[b][a1[1]], a2[1]-a1[1]);
	}
	else
	{
		write2(&block[b][a1[1]], BLOCKSIZ-a1[1]);
		do
		{
			b = getblock(blockno[b]+1);
			if(blockno[b] == a2[0])
			{
				write2(block[b], a2[1]);
			}
			else
			{
				write2(block[b], BLOCKSIZ);
			}
		}
		while(blockno[b] < a2[0]);
	}
}

flags(string)
{
}

search(line)
long *line;
{
	int addr[DOUBLE];
	int b, ii;
	char *ptr;
	register i;
	register char *s, *p;


	if(curline <= 0)
	{
		curline = 1;
	}
	*line = curline + 1;
	for(;;)
	{
		if(*line == curline)
		{
			/* not found */ 
			return(FALSE);
		}
		(*line)++;
		if(getaddress(line, addr) == FALSE)
		{
			/* eof, back to beginning */ 
			*line = 1;
			addr[0] = addr[1] = 0;
		}
		b = getblock(addr[0]);
		ptr = &block[b][addr[1]];
		ii = charcnt[b]-addr[1];
		do
		{
			s = searchstr;
			if((i = ii) == 0)
			{
				b = getblock(addr[0]++);
				i = ii = charcnt[b];
				ptr = block[b];
			}
			p = ptr;
			do
			{
				if(*s == 0)
					return(TRUE);
				if(i == 0)
				{
					/* get another block */ 
					b = getblock(addr[0]+1);
					p = block[b];
					i = charcnt[b];
				}
				i--;
			}
			while(*s++ == *p++);
			ii--;
		}
		while(anchored == FALSE && *ptr++ != '\n');
	}
}

shell(string)
char *string;
{
	register int pid;
	int status;
	int reset();


	signal(2, 1);
	if((pid = fork()) < 0)
	{
		prints(2, "Try again\n");
		return;
	}
	if(pid)
	{
		while(pid != wait(&status));
		signal(2, &reset);
		return;
	}
	execl("/bin/sh", "sh", "-c", string, 0);
	prints(2, "can't execute %s\n", string);
	exit(1);
}

setsearch(string)
char *string;
{
	register char c, *s, *ss;
	char *end;


	s = string;
	while(*s != '\n')
		s++;
	if(*--s != '/')
	{
		*searchstr = 0;
		return(FALSE);
	}
	if((end = s) <= string)
		return(FALSE);
	if(s[-1] == '$')
		s[-1] = '\n';
	if(*string == '^')
	{
		anchored = TRUE;
		string++;
	}
	else
		anchored = FALSE;
	ss = searchstr;
	for(s = string; s < end; *ss++ = *s++);
	*ss++ = 0;
	return(TRUE);
}

write2(buf, cnt)
{

	write(out, buf, cnt);
	if(printing)
		write(printfile, buf, cnt);
}
