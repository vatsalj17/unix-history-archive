#include "stdio.h"
#define	TSIZE	50		/* maximum configuration table size */
#define	DSIZE	30		/* maximum device table size */
#define	ASIZE	20		/* maximum alias table size */
#define	PSIZE	15		/* maximum parameter table size */
#define	BSIZE	16		/* maximum block device table size */
#define	CSIZE	32		/* maximum character device table size */
#define	IMMED	16		/* immediate allocation of device */
#define	BLOCK	8		/* block type device */
#define	CHAR	4		/* character type device */
#define	FIX	1		/* interrupt vector in range 0 - 0274 */
#define	FLT	2		/* interrupt vector in range 0300 - 0774 */
#define	OPEN	16		/* open routine exists */
#define	CLOSE	8		/* close routine exists */
#define	READ	4		/* read routine exists */
#define	WRITE	2		/* write routine exists */
#define	SGTTY	1		/* sgtty routine exists */

struct	t	{
	char	*devname;	/* pointer to device name */
	int	vector;		/* interrupt vector location */
	int	address;	/* device address */
	int	buslevel;	/* bus request level */
	int	addrsize;	/* number of bytes at device address */
	int	vectsize;	/* number of bytes needed for interrupt vector */
	int	type;		/* IMMED,BLOCK,CHAR,FIX,FLT specification */
	char	*handler;	/* pointer to interrupt handler */
	int	count;		/* sequence number for this device */
	int	blk;		/* major device number if block type device */
	int	chr;		/* major device number if char. type device */
	int	mlt;		/* number of devices on controller */
}	table	[TSIZE]
{
	"dl11", 060, 0777560, 4, 8, 8, 5, "kl", 0, 0, 0, 1	/* console */
};

struct	t2	{
	char	dev[9];		/* device name */
	int	vsize;		/* interrupt vector size */
	int	asize;		/* device address size */
	int	type2;		/* IMMED,BLOCK,CHAR,FIX,FLT specification */
	int	block;		/* major device number if block type device */
	int	charr;		/* major device number if char. type device */
	int	mult;		/* maximum number of devices per controller */
	int	busmax;		/* maximum allowable bus request level */
	int	mask;		/* device mask indicating existing handlers */
	char	hndlr[5];	/* handler name */
	int	dcount;		/* number of controllers present */
	char	entries[3][9];	/* conf. structure definitions */
}	devinfo	[DSIZE];

struct	t3	{
	char	new[9];		/* alias of device */
	char	old[9];		/* reference name of device */
}	alias	[ASIZE];

struct	t4	{
	char	*indef;		/* input parameter keyword */
	char	*oudef;		/* output definition symbol */
	int	value;
}	parms	[PSIZE]
{
	"buffers",	"NBUF",		-1,
	"inodes",	"NINODE",	-1,
	"files",	"NFILE",	-1,
	"mounts",	"NMOUNT",	-1,
	"coremap",	"CMAPSIZ",	-1,
	"swapmap",	"SMAPSIZ",	-1,
	"calls",	"NCALL",	-1,
	"procs",	"NPROC",	-1,
	"texts",	"NTEXT",	-1,
	"clists",	"NCLIST",	-1
};

struct	t	*lowptr[128]		/* low core double-word pointers */
{
	1,2,3,4,5,6,7,8,9,10,11,12,	/* trap vectors/fillers */
	&table[0],20,			/* console entry */
	0,0,				/* free entries */
	17,18,				/* trap vectors/fillers */
	0,				/* free entry */
	13,				/* trap vector/filler */
	0,0,0,0,0,0,0,0,0,0,		/* free entries */
	0,0,0,0,0,0,0,0,0,0,		/* free entries */
	14,15,16			/* trap vectors/fillers */
};

struct	t2	*bdevices[BSIZE];	/* pointers to block devices */
struct	t2	*cdevices[CSIZE];	/* pointers to char. devices */
struct	t	*p;			/* configuration table pointer */
struct	t2	*q;			/* master device table pointer */
struct	t3	*r;			/* alias table pointer */
struct	t4	*kwdptr;		/* keyword table pointer */

int	eflag	 0;	/* error in configuration */
int	bmax	-1;	/* dynamic max. major device number for block device */
int	cmax	-1;	/* dynamic max. major device number for char. device */
int	blockex	 0;	/* dynamic end of block device table */
int	charex	 0;	/* dynamic end of character device table */
int	subv	 0;	/* low core double-word subscript */
int	abound	-1;	/* current alias table size */
int	dbound	-1;	/* current device table size */
int	tbound	 0;	/* current configuration table size */
int	pbound	 9;	/* current keyword table size */
int	rtmaj	-1;	/* major device number for root device */
int	swpmaj	-1;	/* major device number for swap device */
int	dmpmaj	-1;	/* major device number for dump device */
int	rtmin	-1;	/* minor device number for root device */
int	swpmin	-1;	/* minor device number for swap device */
int	dmpmin	-1;	/* minor device number for dump device */
int	swplo	-1;	/* low disc address for swap area */
int	nswap	-1;	/* number of disc blocks in swap area */
int	fdconf;		/* configuration file descriptor */
int	fdlow;		/* low core file descriptor */
char	*dmphndlr;	/* pointer to dump handler routine */
char	*mfile;		/* master device file */
char	*cfile;		/* output configuration table file */
char	*lfile;		/* output low core file */
char	*infile;	/* input configuration file */
char	line[101];	/* input line buffer area */
char	dump[81];	/* trap filler for dump routine */
char	xbuff[101];	/* buffer for external symbol definitions */

char	*cputrap[]	/* trap/filler text */
{
	"",
	"\tjmp\tstart\t\t/ Initial startup",
	"\ttrap4;\tbr7+0.\t\t/ Bus error",
	"\ttrap;\tbr7+1.\t\t/ Illegal instruction",
	"\ttrap14;\tbr7+2.\t\t/ Bpt-trace trap",
	"\ttrap;\tbr7+3.\t\t/ Iot trap",
	"\ttrap24;\tbr7+4.\t\t/ Power failure",
	"\ttrap;\tbr7+5.\t\t/ Emulator trap",
	"\ttrap;\tbr7+6.\t\t/ System entry",
	"\t_pwbname;\t\t/ Name of system\n\tbr .",
	&dump[0],
	"\t.+2;\tbr .\t\t/ Trap catcher",
	"\t.+2;\tbr .",
	"\ttrap;\tbr7+10.\t\t/ Memory system error",
	"\ttrap;\tbr7+7.\t\t/ Programmed interrupt",
	"\ttrap;\tbr7+8.\t\t/ Floating point",
	"\ttrap250;\t\t/ Segmentation violation\n\tbr7+9.",
	"\tclio;\tbr6+0.",
	"\tclio;\tbr6+1."
};

char	*out[2]			/* low core text */
{
	".globl\t_%srint, _%sxint\n%sin:\n\tjsr\tr0,call; _%srint\n",
	"%sou:\n\tjsr\tr0,call; _%sxint\n\n"
};

char	*opterror		/* option error message */
{
	"Option re-specification\n"
};

char	*dmpform		/* format for dump routine */
{
	"dump:\t\t\t\t/ Memory dump\n.globl\tdump_%s\n\tjmp\t*$dump_%s"
};

main(argc,argv) 
int argc;
char *argv[];
{
	int l,a,v,bl,fd,i,ml,number;
	char d[9], input[11], buff[9], c, capitals[9];
	char *argv2;
	struct t *pt;
	argc--;
	argv++;
	argv2 = argv[0];
	argv++;
/*
 * Scan off the 'c', 'l', and 'm' options.
 */
	while ((argc > 1) && (argv2[0] == '-')) {
		for (i=1;(c=argv2[i]) != NULL;i++) {
			switch (c) {
/*
 * Output configuration file specification
 */
			case 'c':	if (cfile) {
						printf (opterror);
						exit(1);
					}
					cfile = argv[0];
					argv++;
					argc--;
					break;
/*
 * Output low core file specification
 */
			case 'l':	if (lfile) {
						printf (opterror);
						exit(1);
					}
					lfile = argv[0];
					argv++;
					argc--;
					break;
/*
 * Input master table specification
 */
			case 'm':	if (mfile) {
						printf (opterror);
						exit(1);
					}
					mfile = argv[0];
					argv++;
					argc--;
					break;
			default:	printf ("Unknown option\n");
					exit(1);
			}
		}
		argc--;
		argv2 = argv[0];
		argv++;
	}
	if (argc != 1) {
		printf ("Usage: config [-l file] [-c file] [-m file] file\n");
		exit(1);
	}
/*
 * Set up defaults for unspecified files.
 */
	if (cfile == 0)
		cfile = "conf.c";
	if (lfile == 0)
		lfile = "low.s";
	if (mfile == 0)
		mfile = "/etc/master";
	infile = argv2;
	fd = fopen (infile,"r");
	if (fd == NULL) {
		printf ("Open error for file -- %s\n",infile);
		exit(1);
	}
/*
 * Open configuration file and set modes.
 */
	fdconf = fopen (cfile,"w");
	if (fdconf == NULL) {
		printf ("Open error for file -- %s\n",cfile);
		exit(1);
	}
	chmod (cfile,0644);
/*
 * Print configuration file heading.
 */
	fprintf (fdconf,"/*\n *  Configuration information\n */\n\n\n");
	p = &table[1];
/*
 * Read in the master device table and the alias table.
 */
	file();
/*
 * Set up the console, which must be present.
 */
	q = find("dl11");
	subv = (q->charr);
	cdevices[subv] = q;
	cmax = max (cmax,subv);
	q->dcount++;
/*
 * Start scanning the input.
 */
	while (fgets(line,100,fd) != NULL) {
		if (line[0] == '*')
			continue;
		l = sscanf(line,"%8s",&input[0]);
		if (l == 0) {
			error ("Incorrect line format");
			continue;
		}
		if (equal(&input[0],"root")) {
/*
 * Root device specification
 */
			l = sscanf(line,"%*8s%8s%o",&buff[0],&number);
			if (l != 2) {
				error ("Incorrect line format");
				continue;
			}
			if ((pt=locate(&buff[0])) == 0) {
				error ("No such device");
				continue;
			}
			if ((pt->type & BLOCK) == 0) {
				error ("Not a block device");
				continue;
			}
			if (number > 255) {
				error ("Invalid minor device number");
				continue;
			}
			if (rtmin >= 0) {
				error ("Multiple root specification");
				continue;
			}
			rtmin = number;
			rtmaj = pt->blk;
		}
		else
		    if (equal(&input[0],"swap")) {
/*
 * Swap device specification
 */
			l = sscanf(line,"%*8s%8s%o%d%d",
					&buff[0],&number,
					&swplo,&nswap);
			if (l != 4) {
				error ("Incorrect line format");
				continue;
			}
			if ((pt=locate(&buff[0])) == 0) {
				error ("No such device");
				continue;
			}
			if ((pt->type & BLOCK) == 0) {
				error ("Not a block device");
				continue;
			}
			if (number > 255) {
				error ("Invalid minor device number");
				continue;
			}
			if (swpmin >= 0) {
				error ("Multiple swap specification");
				continue;
			}
			if (nswap < 1) {
				error ("Invalid nswap");
				continue;
			}
			if (swplo < 1) {
				error ("Invalid swplo");
				continue;
			}
			swpmin = number;
			swpmaj = pt->blk;
		}
		else
		    if (equal(&input[0],"dump")) {
/*
 * Dump device specification
 */
			l = sscanf(line,"%*8s%8s%o",&buff[0],&number);
			if (l != 2) {
				error ("Incorrect line format");
				continue;
			}
			if ((pt=locate(&buff[0])) == 0) {
				error ("No such device");
				continue;
			}
			if (number > 255) {
				error ("Invalid minor device number");
				continue;
			}
			if (dmpmin >= 0) {
				error ("Multiple dump specification");
				continue;
			}
			dmpmin = number;
			dmpmaj = 0;
			dmphndlr = pt->handler;
		}
		else {
/*
 * Device or parameter specification other than root, swap, or dump
 */
			kwdptr = lookup(&input[0]);
			if (kwdptr) {
				l = sscanf(line,"%10s%d",&input[0],&number);
				if (l != 2) {
					error ("Incorrect line format");
					continue;
				}
				kwdptr->value = number;
				continue;
			}
			tbound++;
			if (tbound == TSIZE) {
				printf ("Configuration table overflow\n");
				exit(1);
			}
			l = sscanf(line,"%8s%o%o%d%d",&d[0],&v,&a,&bl,&ml);
			if (l < 4) {
				error ("Incorrect line format");
				continue;
			}
/*
 * Does such a device exist, and if so, do we allow its specification?
 */
			if (((q=find(&d[0]))==0)||((q->type2&(FIX+FLT))==0)) {
				error ("No such device");
				continue;
			}
/*
 * Is the bus request level a valid one for this device?
 */
			if (bl<4 || bl>q->busmax) {
				error ("Invalid bus level");
				continue;
			}
/*
 * Is the address valid?
 */
			if (a < 0160000) {
				error ("Invalid address");
				continue;
			}
/*
 * Is the interrupt vector on an appropriate boundary?
 */
			if (v%q->vsize) {
				error ("Vector alignment");
				continue;
			}
/*
 * Is the interrupt vector in low core?
 */
			if (v<0 || v>0774) {
				error ("Vector not in low core range");
				continue;
			}
			switch (q->type2 & (FIX+FLT)) {
/*
 * This interrupt vector should be in the 0 - 0274 range.
 */
			case 1:	if (v > 0274) {
					error ("Vector not in 0-0274 range");
					continue;
				}
				break;
/*
 * This interrupt vector should be in the 0300 - 0774 range.
 */
			case 2: if (v < 0300) {
					error ("Vector not in 0300-0774 range");
					continue;
				}
				break;
			}
/*
 * Get the double-word offset for the interrupt vector.
 */
			subv = v/4;
/*
 * If vector is not already allocated, mark it as now allocated.
 */
			if (lowptr[subv]) {
				error ("Vector allocated");
				continue;
			}
			if (q->vsize == 8) {
				if (lowptr[subv+1]) {
					error ("Vector collision");
					continue;
				}
				lowptr[subv+1] = 20;
			}
/*
 * See if the optional device multiplier was specified, and if so, see
 * if it is a valid one.
 */
			if (l == 5) {
				if ((ml > q->mult) || (ml < 1)) {
					error ("Invalid device multiplier");
					continue;
				}
				p->mlt = ml;
			}
/*
 * Multiplier not specified, so take a default value from master device table.
 */
			else
				p->mlt = q->mult;
/*
 * Set low core configuration table pointer to the device.
 */
			lowptr[subv] = p;
/*
 * Fill in the contents of the configuration table node for this device.
 */
			setq();
			p->devname = q->dev;
			p->vector = subv;
			p->address = a;
			p->buslevel = bl;
			p->addrsize = q->asize;
			p->vectsize = q->vsize;
			p->type = q->type2;
			p->handler = q->hndlr;
			p->count = q->dcount;
			p->blk = q->block;
			p->chr = q->charr;
			q->dcount;
			p++;
/*
 * Write upper case define statement and sequence number for device.
 */
			fprintf (fdconf,"#define\t%s_%d\n",
				uppermap (&d[0],&capitals[0]),
				q->dcount++);
		}
	}
/*
 * Make sure that the root, swap and dump devices were specified.
 */
	if (rtmaj < 0) {
		printf ("root device not specified\n");
		eflag++;
	}
	if (swpmaj < 0) {
		printf ("swap device not specified\n");
		eflag++;
	}
	if (dmpmaj < 0) {
		printf ("dump device not specified\n");
		eflag++;
	}
	for (kwdptr= &parms[0]; kwdptr<=&parms[pbound]; kwdptr++) {
		if (kwdptr->value < 0) {
			printf ("%s not specified\n",kwdptr->indef);
			eflag++;
		}
	}
/*
 * See if configuration is to be terminated.
 */
	if (eflag) {
		printf ("\nConfiguration aborted.\n");
		exit (1);
	}
/*
 * Configuration is okay, so write the two files and quit.
 */
	sprintf (&dump[0],dmpform,dmphndlr,dmphndlr);
	prtconf();
	prtlow();
	printf ("Configuration completed successfully.\n");
	exit (0);
}

/*
 * This routine writes out the low core assembler program.
 */
prtlow()
{
	register int i;
	register char *ptr;
	register struct t *tptr;
	int ktemp;
/*
 * Open low core file and set modes.
 */
	fdlow = fopen (lfile,"w");
	if (fdlow == NULL) {
		printf ("Open error for file -- %s\n",lfile);
		exit(1);
	}
	chmod (lfile,0644);
/*
 * Print some headings.
 */
	fprintf (fdlow,"\t\t/ PWB UNIX - Bell Laboratories /\n\n");
	fprintf (fdlow,"/ Low core\n.data\n\nbr4 = 200\nbr5 = 240\n");
	fprintf (fdlow,"br6 = 300\nbr7 = 340\n\n");
	fprintf (fdlow,".globl\tstart, dump, call, _pwbname\n");
	fprintf (fdlow,".globl\ttrap, trap4, trap14, trap24, trap250\n\n");
/*
 * Go through low core in double-word increments.
 */
	for(i=0;i<128;i++) {
		ktemp = lowptr[i];
/*
 * If the pointer is 0, set vector up for stray interrupt catcher.
 */
		if (ktemp == 0)
			fprintf (fdlow,"\t.+2;\tbr .\n");
		else {
			fprintf (fdlow,". = %o^.\n",i*4);
/*
 * See if pointer refers to a filler message.
 */
			if (ktemp < 20)
				fprintf (fdlow,"%s\n",cputrap[ktemp]);
/*
 * Pointer refers to a device, so set up the handler routine address with
 * the bus level and device sequence number.
 */
			else {
				ptr = ktemp;
				if (ptr->vectsize == 8) {
					fprintf (fdlow,"\t%sin;\tbr%d+%d.\n",
						ptr->handler,
						ptr->buslevel,
						ptr->count);
					fprintf (fdlow,"\t%sou;\tbr%d+%d.\n",
						ptr->handler,
						ptr->buslevel,
						ptr->count);
					i++;
				}
				else {
					fprintf (fdlow,"\t%sio;\tbr%d+%d.\n",
						ptr->handler,
						ptr->buslevel,
						ptr->count);
				}
			}
		}
	}
/*
 * Print headings for C interface code.
 */
	fprintf (fdlow,"\n. = 1000^.\n\n/ Interface code to C\n\n");
	fprintf (fdlow,".globl\t_clock\nclio:\n\tjsr\tr0,call; _clock\n\n");
/*
 * Go through configuration table and print out all required handler references.
 */
	for (tptr= &table[0];tptr->devname!=0;tptr++) {
		if (tptr->count == 0) {
			if (tptr->vectsize == 4) {
				fprintf (fdlow,".globl\t_%sintr\n%sio:\n\tjsr\tr0,call; _%sintr\n\n",
					tptr->handler,
					tptr->handler,
					tptr->handler);
			}
			else {
				fprintf (fdlow,out[0],
					tptr->handler,
					tptr->handler,
					tptr->handler,
					tptr->handler);
				fprintf (fdlow,out[1],
					tptr->handler,
					tptr->handler);
			}
		}
	}
}

/*
 * This routine is used to read in the master device table and the
 * alias table.  In the master device file, these two tables are separated
 * by a line which contains an asterisk in column 1.  Certain special type
 * devices such as memory, require immediate allocation in the character
 * and/or block tables.  They are specially marked as requiring this.
 */
file() 
{
	int l,fd;
	fd = fopen(mfile,"r");
	if (fd == NULL) {
		printf ("Open error for file -- %s\n",mfile);
		exit(1);
	}
	q = &devinfo[0];
	
	while (fgets(line,100,fd) != NULL) {
/*
 * Check for the delimiter that indicates the beginning of the
 * alias table entries.
 */
		if (line[0] == '$')
			break;
/*
 * Check for comment.
 */
		if (line[0] == '*')
			continue;
		dbound++;
		if (dbound == DSIZE) {
			printf ("Device table overflow\n");
			exit(1);
		}
		l = sscanf(line,"%8s%d%d%d%4s%d%d%d%d%d%8s%8s%8s",
			q->dev,&(q->vsize),&(q->mask),&(q->type2),
			q->hndlr,&(q->asize),&(q->block),
			&(q->charr),&(q->mult),&(q->busmax),
			q->entries[0],q->entries[1],q->entries[2]);
		if (l < 10) {
			printf ("device table error -- %d\n",l);
			exit(1);
		}
/*
 * Update the ends of the block and character device tables.
 */
		if (q->type2 & BLOCK)
			blockex = max (blockex,q->block);
		if (q->type2 & CHAR)
			charex = max (charex,q->charr);
/*
 * See if the device requires immediate allocation.
 */
		if (q->type2 & IMMED)
			setq();
		q++;
	}
	r = &alias[0];
	while (fgets(line,100,fd) != NULL) {
/*
 * Check for comment.
 */
		if (line[0] == '*')
			continue;
		abound++;
		if (abound == ASIZE) {
			printf ("Alias table overflow\n");
			exit(1);
		}
		l = sscanf(line,"%8s%8s",r->new,r->old);
		if (l < 2) {
			printf ("alias table error -- %d\n",l);
			exit(1);
		}
		r++;
	}
	return(0);
}

/*
 * This routine compares two null terminated strings for equality.
 */
equal(s1,s2)
char *s1,*s2;
{
	while (*s1++ == *s2) {
		if (*s2++ == 0)
			return(1);
	}
	return(0);
}

/*
 * This routine is used to print a configuration time error message
 * and then set a flag indicating a contaminated configuration.
 */
error(message)
char *message;
{
	printf ("%s%s\n\n",&line[0],message);
	eflag++;
	return;
}

/*
 * This routine is used to search through the master device table for
 * some specified device.  If the device is found, we return a pointer to 
 * the device.  If the device is not found, we search the alias table for
 * this device.  If the device is not found in the alias table, we return a
 * zero.  If the device is found, we change its name to the reference name of
 * the device and re-initiate the search for this new name in the master
 * device table.
 */
int find(device)
char *device;
{
	register struct t2 *q;
	register struct t3 *r;
	for (; ;) {
/*
 * Search the master device table for the specified device.
 */
		for (q= &devinfo[0];q<=&devinfo[dbound];q++) {
			if (equal(device,q->dev))
				return(q);
		}
/*
 * Since the device was not found in the master device table,
 * we now search through the alias table.
 */
		for (r= &alias[0];r<=&alias[abound];r++) {
			if (equal(device,r->new)) {
				device = r->old;
				break;
			}
		}
/*
 * See if the device was found in the alias table. If it wasn't, return a 0.
 */
		if (r > &alias[abound])
			return(0);
/*
 * It was found in the alias table, so we change its name back to the
 * reference name of the device and re-initiate the master device table search.
 */
	}
}

/*
 * This routine is used to set the character and/or block table pointers
 * to point to an entry of the master device table.
 */
setq()
{
	register char *ptr;
/*
 * Do a switch on the type of device.
 */
	switch (q->type2 & (BLOCK+CHAR)) {
/*
 * Device is not a block or character device.
 */
	case 0:	break;
/*
 * Device is a character type device.
 */
	case 4:	subv = q->charr;
		ptr = cdevices[subv];
		if (ptr) {
			if (!equal(ptr->dev,q->dev)) {
				charex++;
				if (charex == CSIZE) {
					printf("Character table overflow\n");
					exit(1);
				}
				q->charr = subv = charex;
			}
		}
		cdevices[subv] = q;
		cmax = max (cmax,subv);
		break;
/*
 * Device is a block type device.
 */
	case 8:	subv = q->block;
		ptr = bdevices[subv];
		if (ptr) {
			if (!equal(ptr->dev,q->dev)) {
				blockex++;
				if (blockex == BSIZE) {
					printf ("Block table overflow\n");
					exit(1);
				}
				q->block = subv = blockex;
			}
		}
		bdevices[subv] = q;
		bmax = max (bmax,subv);
		break;
/*
 * Device is both a character and block type device.
 */
	case 12:subv = q->charr;
		ptr = cdevices[subv];
		if (ptr) {
			if (!equal(ptr->dev,q->dev)) {
				charex++;
				if (charex == CSIZE) {
					printf("Character table overflow\n");
					exit(1);
				}
				q->charr = subv = charex;
			}
		}
		cdevices[subv] = q;
		cmax = max (cmax,subv);
		subv = q->block;
		ptr = bdevices[subv];
		if (ptr) {
			if (!equal(ptr->dev,q->dev)) {
				blockex++;
				if (blockex == BSIZE) {
					printf ("Block table overflow\n");
					exit(1);
				}
				q->block = subv = blockex;
			}
		}
		bdevices[subv] = q;
		bmax = max (bmax,subv);
		break;
	}
	return;
}

/*
 * This routine writes out the configuration file (C program.)
 */
prtconf()
{
	int i,j,counter;
	char *ptr;
/*
 * Print some headings.
 */
	fprintf (fdconf,"\n");
	for (kwdptr= &parms[0]; kwdptr<=&parms[pbound]; kwdptr++) {
		fprintf (fdconf,"#define\t%s\t%d\n",
			kwdptr->oudef, kwdptr->value);
	}
	fprintf (fdconf,"\n#include\t\"../hd\/io.h\"\n");
	fprintf (fdconf,"#include\t\"../hd/space.h\"\n\n");

/*
 * Print the extern statement for the nodev and nulldev entries.
 */
	fprintf (fdconf,"extern nodev(), nulldev();\n");

/*
 * Search the configuration table and generate an extern statement for
 * any routines which are needed.
 */
	for (p= &table[0];p<=&table[tbound];p++) {
		if (p->count)
			continue;
		switch (p->type & (BLOCK+CHAR)) {
		case 4:	q = find(p->devname);
			sprintf (&xbuff[0],"extern");
			if (q->mask & OPEN)
				sprintf (&(xbuff[strlen(xbuff)]),
					" %sopen(),",&(q->hndlr));
			if (q->mask & CLOSE)
				sprintf (&(xbuff[strlen(xbuff)]),
					" %sclose(),",&(q->hndlr));
			if (q->mask & READ)
				sprintf (&(xbuff[strlen(xbuff)]),
					" %sread(),",&(q->hndlr));
			if (q->mask & WRITE)
				sprintf (&(xbuff[strlen(xbuff)]),
					" %swrite(),",&(q->hndlr));
			if (q->mask & SGTTY)
				sprintf (&(xbuff[strlen(xbuff)]),
					" %ssgtty(),",&(q->hndlr));
			xbuff[strlen(xbuff)-1] = ';';
			fprintf (fdconf,"%s\n",&xbuff[0]);
			break;
		case 8:	fprintf (fdconf,"extern %sopen(), %sclose(),",
				p->handler,p->handler);
			fprintf (fdconf," %sstrategy(), %stab();\n",
				p->handler,p->handler);
			break;
		case 12:q = find(p->devname);
			sprintf (&xbuff[0],"extern %sopen(), %sclose(),",
				&(q->hndlr),&(q->hndlr));
			if (q->mask & READ)
				sprintf (&(xbuff[strlen(xbuff)]),
					" %sread(),",&(q->hndlr));
			if (q->mask & WRITE)
				sprintf (&(xbuff[strlen(xbuff)]),
					" %swrite(),",&(q->hndlr));
			if (q->mask & SGTTY)
				sprintf (&(xbuff[strlen(xbuff)]),
					" %ssgtty(),",&(q->hndlr));
			sprintf (&(xbuff[strlen(xbuff)]),
				" %sstrategy(), %stab();",
				&(q->hndlr),&(q->hndlr));
			fprintf (fdconf,"%s\n",&xbuff[0]);
			break;
		}
	}

/*
 * Search the master device table and generate an extern statement for
 * any routines which were not covered by the previous section of code.
 * These routines are generally associated with those devices which require
 * immediate allocation (thus, they are not in the configuration table).
 */
	for (q= &devinfo[0];q<=&devinfo[dbound];q++) {
		if (q->type2 & IMMED) {
			switch (q->type2 & (BLOCK+CHAR)) {
			case 4:	sprintf (&xbuff[0],"extern");
				if (q->mask & OPEN)
					sprintf (&(xbuff[strlen(xbuff)]),
						" %sopen(),",&(q->hndlr));
				if (q->mask & CLOSE)
					sprintf (&(xbuff[strlen(xbuff)]),
						" %sclose(),",&(q->hndlr));
				if (q->mask & READ)
					sprintf (&(xbuff[strlen(xbuff)]),
						" %sread(),",&(q->hndlr));
				if (q->mask & WRITE)
					sprintf (&(xbuff[strlen(xbuff)]),
						" %swrite(),",&(q->hndlr));
				if (q->mask & SGTTY)
					sprintf (&(xbuff[strlen(xbuff)]),
						" %ssgtty(),",&(q->hndlr));
				xbuff[strlen(xbuff)-1] = ';';
				fprintf (fdconf,"%s\n",&xbuff[0]);
				break;
			case 8:	fprintf (fdconf,"extern %sopen(), %sclose(),",
					&(q->hndlr),&(q->hndlr));
				fprintf (fdconf," %sstrategy(), %stab();\n",
					&(q->hndlr),&(q->hndlr));
				break;
			case 12:sprintf (&xbuff[0],"extern %sopen(), %sclose(),",
				&(q->hndlr),&(q->hndlr));
				if (q->mask & READ)
					sprintf (&(xbuff[strlen(xbuff)]),
						" %sread(),",&(q->hndlr));
				if (q->mask & WRITE)
					sprintf (&(xbuff[strlen(xbuff)]),
						" %swrite(),",&(q->hndlr));
				if (q->mask & SGTTY)
					sprintf (&(xbuff[strlen(xbuff)]),
						" %ssgtty(),",&(q->hndlr));
				sprintf (&(xbuff[strlen(xbuff)]),
					" %sstrategy(), %stab();",
					&(q->hndlr),&(q->hndlr));
				fprintf (fdconf,"%s\n",&xbuff[0]);
				break;
			}
		}
	}

	fprintf (fdconf,"\nint\t(*bdevsw[])()\n{\n");
/*
 * Go through block device table and indicate addresses of required routines.
 * If a particular device is not present, fill in "nodev" entries.
 */
	for (i=0;i<=bmax;i++) {
		ptr = bdevices[i];
		fprintf (fdconf,"/*%2d*/\t",i);
		if (ptr) {
			fprintf (fdconf,"&%sopen,\t&%sclose,\t&%sstrategy,\t&%stab,\n",
				&(ptr->hndlr),
				&(ptr->hndlr),
				&(ptr->hndlr),
				&(ptr->hndlr));
		}
		else {
			fprintf (fdconf,"&nodev, \t&nodev, \t&nodev, \t0, \n");
		}
	}
	fprintf (fdconf,"};\n\n");
	fprintf (fdconf,"int\t(*cdevsw[])()\n{\n");
/*
 * Go through character device table and indicate addresses of required
 * routines, or indicate "nulldev" if routine is not present.  If a 
 * particular device is not present, fill in "nodev" entries.
 */
	for (j=0;j<=cmax;j++) {
		ptr = cdevices[j];
		fprintf (fdconf,"/*%2d*/",j);
		if (ptr) {
			if (ptr->mask & OPEN)
				fprintf (fdconf,"\t&%sopen,",&(ptr->hndlr));
			else
				fprintf (fdconf,"\t&nulldev,");
			if (ptr->mask & CLOSE)
				fprintf (fdconf,"\t&%sclose,",&(ptr->hndlr));
			else
				fprintf (fdconf,"\t&nulldev,");
			if (ptr->mask & READ)
				fprintf (fdconf,"\t&%sread,",&(ptr->hndlr));
			else
				fprintf (fdconf,"\t&nodev, ");
			if (ptr->mask & WRITE)
				fprintf (fdconf,"\t&%swrite,",&(ptr->hndlr));
			else
				fprintf (fdconf,"\t&nodev, ");
			if (ptr->mask & SGTTY)
				fprintf (fdconf,"\t&%ssgtty,\n",&(ptr->hndlr));
			else
				fprintf (fdconf,"\t&nodev, \n");
		}
		else
			fprintf (fdconf,"\t&nodev, \t&nodev, \t&nodev, \t&nodev, \t&nodev,\n");
	}
/*
 * Print out block and character device counts, root, swap, and dump device
 * information, and the swplo, and nswap values.
 */
	fprintf (fdconf,"};\n\n");
	fprintf (fdconf,"int\tbdevcnt\t%d;\nint\tcdevcnt\t%d;\n\n",i,j);
	fprintf (fdconf,"int\trootdev\t{(%d<<8)|0%o};\n",rtmaj,rtmin);
	fprintf (fdconf,"int\tswapdev\t{(%d<<8)|0%o};\n",swpmaj,swpmin);
	fprintf (fdconf,"int\tswplo\t%d;\nint\tnswap\t%d;\n",swplo,nswap);
	fprintf (fdconf,"int\tdumpdev\t{(%d<<8)|0%o};\n\n",dmpmaj,dmpmin);
/*
 * Go through the block device table, ignoring any zero entries,
 * as well as those that do not have interrupt vectors.
 */
	for (i=0;i<=bmax;i++) {
		if ((q=bdevices[i]) == 0)
			continue;
		if ((q->type2 & (FIX+FLT)) == 0)
			continue;
		counter = 0;
		fprintf (fdconf,"\nint\t%s_addr[]\n{\n",q->hndlr);
/*
 * For each of these devices, go through the configuration table and
 * look for matches. For each match, print the device address, and keep
 * a count of the number of matches.
 */
		for (p= &table[0];p<=&table[tbound];p++) {
			if (equal(q->dev,p->devname)) {
				fprintf (fdconf,"\t0%o,\n",p->address);
				counter =+ p->mlt;
			}
		}
		fprintf (fdconf,"};\n");
		fprintf (fdconf,"int\t%s_cnt %d;\n",
			q->hndlr,counter);
/*
 * Print any required structure definitions.
 */
		for (j=0;j<3;j++) {
			if (*(q->entries[j]) != NULL)
				fprintf (fdconf,"struct\t%s\t%s_%s[%d];\n",
					q->entries[j],q->hndlr,
					q->entries[j],counter);
		}
	}
/*
 * Go through the character device table, ignoring any zero entries,
 * as well as those that do not have interrupt vectors, and those that
 * are not strictly character devices.
 */
	for (i=0;i<=cmax;i++) {
		if ((q=cdevices[i]) == 0)
			continue;
		if ((q->type2 & (FIX+FLT)) == 0)
			continue;
		if ((q->type2 & (BLOCK+CHAR)) != CHAR)
			continue;
		counter = 0;
		fprintf (fdconf,"\nint\t%s_addr[]\n{\n",q->hndlr);
/*
 * For each of these devices, go through the configuration table and
 * look for matches. For each match, print the device address, and keep
 * a count of the number of matches.
 */
		for (p= &table[0];p<=&table[tbound];p++) {
			if (equal(q->dev,p->devname)) {
				fprintf (fdconf,"\t0%o,\n",p->address);
				counter =+ p->mlt;
			}
		}
		fprintf (fdconf,"};\n");
		fprintf (fdconf,"int\t%s_cnt %d;\n",
			q->hndlr,counter);
/*
 * Print any required structure definitions.
 */
		for (j=0;j<3;j++) {
			if (*(q->entries[j]) != NULL)
				fprintf (fdconf,"struct\t%s\t%s_%s[%d];\n",
					q->entries[j],q->hndlr,
					q->entries[j],counter);
		}
	}
/*
 * Repeat the procedure for the dm11 - modem control which is a somewhat
 * special device.
 */
	if (locate("dm11")) {
		fprintf (fdconf,"\nint\tdm_addr[]\n{\n");
		for (p= &table[0];p<=&table[tbound];p++) {
			if (equal (p->devname,"dm11"))
				fprintf (fdconf,"\t0%o,\n",p->address);
		}
		fprintf (fdconf,"};\n");
	}
	return;
}

max(a,b)
int a,b;
{
	return (a>b ? a:b);
}

/*
 * This routine is used to search the configuration table for some
 * specified device.  If the device is found we return a pointer to
 * that device.  If the device is not found, we search the alias
 * table for this device.  If the device is not found in the alias table
 * we return a zero.  If the device is found, we change its name to
 * the reference name of the device and re-initiate the search for this
 * new name in the configuration table.
 */

int locate(device)
char *device;
{
	register struct t *p;
	register struct t3 *r;
	for (; ;) {
/*
 * Search the configuration table for the specified device.
 */
		for (p= &table[0];p<=&table[tbound];p++) {
			if (equal(device,p->devname))
				return(p);
		}
/*
 * Since the device was not found in the configuration table, we now
 * search through the alias table.
 */
		for (r= &alias[0];r<=&alias[abound];r++) {
			if (equal(device,r->new)) {
				device = r->old;
				break;
			}
		}
/*
 * See if the device was found in the alias table. If it wasn't, return a 0.
 */
		if (r > &alias[abound])
			return(0);
/*
 * It was found in alias table, so we change its name back to the
 * reference name of the device and re-initiate the configuration
 * table search.
 */
	}
}

/*
 * This routine is used to search the parameter table
 * for the keyword that was specified in the configuration.  If the
 * keyword cannot be found in this table, a value of zero is returned.
 * If the keyword is found, a pointer to that entry is returned.
 */
int lookup(keyword)
char *keyword;
{
	register struct t4 *kwdptr;
	for (kwdptr= &parms[0]; kwdptr<=&parms[pbound]; kwdptr++) {
		if (equal(keyword,kwdptr->indef))
			return (kwdptr);
	}
	return(0);
}

/*
 * This routine is used to map lower case alphabetics into upper case.
 */
int uppermap(device,caps)
char *device;
char *caps;
{
	register char *ptr;
	register char *ptr2;
	ptr2 = &caps[0];
	for (ptr= &device[0];*ptr!=NULL;ptr++) {
		if ('a' <= *ptr && *ptr <= 'z')
			*ptr2++ = *ptr + 'A' - 'a';
		else
			*ptr2++ = *ptr;
	}
	*ptr2 = NULL;
	return (&caps[0]);
}
