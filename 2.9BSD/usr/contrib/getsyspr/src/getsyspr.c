/*
 * getsyspr --
 *
 * Read the system profile from memory.
 *
 * Usage:
 *	getsyspr [incr] [outfile]
 *
 * $Log$
 */

static char rcsid[] =
    "$Header$";

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <a.out.h>

struct nlist names[] =
{
    "_proloc",	0,	0,
    "_outside",	0,	0,
    "_mode",	0,	0,
    "_etext",	0,	0,
    0,		0,	0
};

#define	PROLOC	names[0].n_value
#define	OUTSIDE	names[1].n_value
#define	MODE	names[2].n_value
#define	ETEXT	names[3].n_value

char devmem[] = "/dev/mem";
char slashunix[] = "/unix.namelist";

#define	CLICKSIZE	64
#define	PROBUFCLICKS	256

int memfile;
unsigned outside, lowpc, highpc, proloc, nwords;
int probuf[PROBUFCLICKS*CLICKSIZE];
struct {
    long md_kernel;
    long md_super;
    long md_unused;
    long md_user;
} modes;
int zero = 0;
off_t bufoffset;
extern off_t lseek();
char sysprtmp[] = "/tmp/syspraXXXXXX";

main(argc, argv)
    char *argv[];
{
    int numincr = 0;
    char *outname, *incr = (char *) 0;


    outname = "mon.out";
    incr = NULL;
    if (argc > 1)
    {
	if (argv[1][0] == '-')
	{
	    if (argc > 3) goto usage;
	    incr = &argv[1][1];
	    if (argc > 2)
		outname = argv[2];
	}
	else
	{
	    if (argc > 2) goto usage;
	    outname = argv[1];
	}
    }

    if (incr)
    {
	if (isdigit(incr[0]))
	{
	    numincr = atoi(incr);
	    if (numincr <= 0)
	    {
		printf("Increment must be greater than zero\n");
		exit (1);
	    }
	}
    }

    memfile = open(devmem, 0);
    if (memfile < 0)
    {
	perror(devmem);
	exit (1);
    }

    nlist(slashunix, names);
    if (names[0].n_type == 0)
    {
	printf("No entry for _proloc in %s\n", slashunix);
	exit (1);
    }

    lseek(memfile, (long) PROLOC, 0);
    if (read(memfile, &proloc, sizeof proloc) != sizeof proloc)
    {
	printf("Can't read profile offset\n");
	exit (1);
    }
    bufoffset = (long) proloc * (long) CLICKSIZE;

    lowpc = 0;
    highpc = ETEXT;


    readprof();
    if (incr)
    {
	if (numincr == 0)
	{
	    diffprof(incr);
	    printf("Incremental kernel profile over counts in \"%s\"\n",
			incr);
	}
	else
	{
	    printf("Incremental kernel profile over %d minutes\n", numincr);
	    mktemp(sysprtmp);
	    writeprof(sysprtmp);
	    sleep(numincr * 60);
	    readprof();
	    diffprof(sysprtmp);
	    unlink(sysprtmp);
	}
    }

    writeprof(outname);

    if (incr == 0)
    {
	printf("Kernel:     %ld\n", modes.md_kernel);
	printf("Supervisor: %ld\n", modes.md_super);
	printf("User:       %ld\n", modes.md_user);
	printf("\n");
	printf("Overflow count: %u\n", outside);
	printf("Lowpc = 0%o Highpc = 0%o\n", lowpc, highpc);
    }

    printf("\nWARNING: times reported by prof(1) must multiplied\n");
    printf("by 0.60 in order to determine real time\n");
    exit (0);

usage:
    printf("Usage: %s [-incr] [file]\n", argv[0]);
    exit (1);
}


readprof()
{
    lseek(memfile, bufoffset, 0);
    if (read(memfile, probuf, sizeof probuf) != sizeof probuf)
    {
	printf("Can't read profile table\n");
	exit (1);
    }

    lseek(memfile, (long) MODE, 0);
    if (read(memfile, &modes, sizeof modes) != sizeof modes)
    {
	printf("Can't read mode table\n");
	exit (1);
    }

    lseek(memfile, (long) OUTSIDE, 0);
    if (read(memfile, &outside, sizeof outside) != sizeof outside)
    {
	printf("Can't read overflow count\n");
	exit (1);
    }
}

writeprof(outname)
    char *outname;
{
    FILE *outfile;

    outfile = fopen(outname, "w");
    if (outfile == NULL)
    {
	perror(outname);
	exit (1);
    }

    fwrite(&lowpc, sizeof lowpc, 1, outfile);
    fwrite(&highpc, sizeof highpc, 1, outfile);
    fwrite(&zero, sizeof zero, 1, outfile);
    nwords = (highpc - lowpc) >> 3;
    fwrite(probuf, nwords, sizeof (unsigned), outfile);
    fclose(outfile);
}

diffprof(filename)
    char *filename;
{
    FILE *infile;
    int dlow, dhigh, dzero, w;
    register i;

    infile = fopen(filename, "r");
    if (infile == NULL)
    {
	perror(filename);
	exit (1);
    }

    if (fread(&dlow, sizeof dlow, 1, infile) <= 0) goto bad;
    if (fread(&dhigh, sizeof dhigh, 1, infile) <= 0) goto bad;
    if (fread(&dzero, sizeof dzero, 1, infile) <= 0) goto bad;
    if (dlow != lowpc || dhigh != highpc || dzero != zero)
    {
	printf("Version mismatch in monitor files\n");
	exit (1);
    }

    for (i = 0; ; i++)
    {
	if (fread(&w, sizeof w, 1, infile) <= 0)
	    break;
	probuf[i] -= w;
    }
    return;

bad:
    printf("Bad format in incremental file\n");
    exit (1);
}
