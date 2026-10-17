/*	@(#)getmap.c	2.1	SCCS id keyword	*/
#include <whoami.h>
#include <sys/types.h>
#include <pwtable.h>
#define NAMSIZ	8

static int	tbf = -1;			/* table file */
static char	TBL[] = UCB_PWHASH;	/* name of map table */
static unsigned	nusers;				/* number of users */

long	lseek();

static struct pwtable	mapentry;		/* table entry from file */

static
setmapent()
{
	if (tbf == -1) {
		if ((tbf = open (TBL, 0)) < 0)
			return (0);
						/* compute number of entries */
		nusers = lseek(tbf, 0L, 2) / (2 * sizeof(struct pwtable));
	}
	return(1);
}

endmapent()
{
	if (tbf >= 0) {
		close(tbf);
		tbf = -1;
	}
}

struct pwtable *
getmapuid(uid)
register uid;
{
	unsigned		high;			/* high offset to file */
	unsigned		low;			/* low offset to file */
	unsigned		test;			/* current offset to file */

	if (!setmapent())
		return(0);
	low = 0;				/* initialize pointers */
	high = nusers;

	do {
		test = (low + high) / 2;			/* find midpoint */
		lseek (tbf, (long) test * sizeof (struct pwtable), 0);
		if (read(tbf, &mapentry, sizeof (struct pwtable)) !=
			sizeof (struct pwtable))
			return (0);
		if (mapentry.pwt_uid == uid)
			return (&mapentry);
		if (mapentry.pwt_uid < uid)
			low = test + 1;
		else
			high = test;
	} while (low != high);
	return(0);
}

struct pwtable *
getmapnam(name)
char *name;
{
	unsigned		high;			/* high offset to file */
	unsigned		low;			/* low offset to file */
	unsigned		test;			/* current offset to file */
	int	differ;

	if (!setmapent())
		return(0);
	low = 0;				/* initialize pointer */
	high = nusers;

	do {
		test = (low + high) / 2;			/* find midpoint */
		lseek (tbf, ((long) test + nusers) * sizeof (struct pwtable), 0);
		if (read(tbf, &mapentry, sizeof (struct pwtable)) !=
			sizeof (struct pwtable))
			return (0);

		if ((differ = strncmp (mapentry.pwt_name,name,NAMSIZ)) == 0)
			return (&mapentry);
	
		if (differ < 0)				/* adjust pointers */
			low = test + 1;
		else
			high = test;
	} while (low != high);
	return(0);
}
