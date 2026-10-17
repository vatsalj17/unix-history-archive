/*	@(#)pwtable.c	2.1	SCCS id keyword	*/
#include <whoami.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwtable.h>
#define ENTRIES	1500			/* maximum number of entries */
#define MAXLINE BUFSIZ			/* maximum line length */
#define NAMSIZ	8

static char	PASSWD[] = "/etc/passwd";	/* password file */

#ifdef UCB_PWHASH
static char	TBL[]	= UCB_PWHASH;	/* pwtable file */
#else
static char	TBL[]	= "/etc/pw_map";
#endif

static	unsigned	filltable();
static	int	uidcmp(), namecmp();

/*
 * make tables that map user id to location in password file and
 * user name to location in password file
 */
pwtable()  {
	register	unsigned	nusers;

	struct	pwtable	table[ENTRIES];

	nusers = filltable(table);
	writetable(table, nusers);
}

static unsigned
filltable(table)
struct	pwtable	*table;
{
	register	struct	pwtable	*tp;

	char	line[BUFSIZ];
	char	*t, *s;
	unsigned	u;
	FILE	*pwf;
	struct	pwtable	*ep;
	off_t	loc = 0L;

	if ((pwf = fopen(PASSWD, "r")) == NULL) {
		perror(PASSWD);
		return(0);
	}
	ep = table + ENTRIES;
	for (tp = table; tp < ep; tp++) {
		tp->pwt_loc = loc;
		if (fgets(line, BUFSIZ, pwf) == NULL)
			break;
		loc += strlen(line);
		t = tp->pwt_name;
		for (s = line;  *s && *s != ':'; s++) {
			if (t >= tp->pwt_name + NAMSIZ)
				break;
			*t++ = *s;
		}
		while (t < tp->pwt_name + NAMSIZ)
			*t++ = '\0';
		while (*++s != ':')
			;
		u = 0;
		while (*++s != ':')
			u = 10 * u + (*s) - '0';
		tp->pwt_uid = u;
	}
	fclose(pwf);
	return(tp - table);
}

static
writetable(table, nusers)
struct	pwtable	*table;
unsigned	nusers;
{
	register	struct	pwtable	*tp;
	register	struct	pwtable	*bp;
	struct	pwtable	*ep;
	FILE	*tbf;

	if ((tbf = fopen(TBL, "w")) == NULL) {
		perror(TBL);
		return;
	}
	ep = table + nusers;

	qsort(table, nusers, sizeof(struct pwtable), uidcmp);
	for (tp = table; tp < ep; tp++) {
		for (bp = tp - 1; bp >= table; bp--)
			if (bp->pwt_uid != tp->pwt_uid)
				break;
		fwrite((char *) (bp + 1), sizeof(struct pwtable), 1, tbf);	
	}

	qsort(table, nusers, sizeof(struct pwtable), namecmp);
	for (tp = table; tp < ep; tp++) {
		for (bp = tp - 1; bp >= table; bp--)
			if (strncmp(bp->pwt_name, tp->pwt_name, NAMSIZ))
				break;
		fwrite((char *) (bp + 1), sizeof(struct pwtable), 1, tbf);	
	}
	fclose(tbf);
}

static
uidcmp(a, b)
struct	pwtable	*a, *b;
{
	if (a->pwt_uid < b->pwt_uid)
		return(-1);
	if (a->pwt_uid > b->pwt_uid)
		return(1);
	if (a->pwt_loc < b->pwt_loc)
		return(-1);
	return(1);
}

static
namecmp(a, b)
struct	pwtable	*a, *b;
{
	register	differ;
	if (differ = strncmp(a->pwt_name, b->pwt_name, NAMSIZ))
		return(differ);
	if (a->pwt_loc < b->pwt_loc)
		return(-1);
	return(1);
}
