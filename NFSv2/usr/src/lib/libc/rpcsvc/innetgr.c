#ifndef lint
static  char sccsid[] = "@(#)innetgr.c 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif

/* 
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <ctype.h>
#include <rpcsvc/ypclnt.h>

/* 
 * ingroup: test whether I'm in /etc/netgroup
 * 
 */

static char *any(), *index();
static struct list {			/* list of names to check for loops */
	char *name;
	struct list *nxt;
} *list;
static char *name, *machine, *domain;
static char thisdomain[256];

innetgr(grp, mach, nm, dom)
	char *grp;
	char *mach;
	char *nm;
	char *dom;
{
	if (getdomainname(thisdomain, sizeof(thisdomain)) < 0) {
		fprintf(stderr, 
		    "innetgr: getdomainname system call missing\n");
	    exit(1);
	}
	list = NULL;
	machine = mach;
	name = nm;
	domain = dom;
	return doit(grp);
}
		
/* 
 * calls itself recursively
 */
static
doit(group)
	char *group;
{
	int reason, err;
	char *key, *newkey, *val;
	int keylen, newkeylen, vallen;
	char *p, *q, *r;
	struct list *ls;
	int match;
	
	newkey = group;
	newkeylen = strlen(group);
	if ((err = yp_match(thisdomain, "netgroup",
	    newkey, newkeylen, &val, &vallen)) != 0) {
#ifdef DEBUG
		if (err == YPERR_KEY)
			fprintf(stderr, "no such group as %s\n", group);
		else
			fprintf(stderr, "error: yp_match\n");
#endif
		return(0);
	}
	/* 
	 * check for recursive loops
	 */
	for (ls = list; ls != NULL; ls = ls->nxt)
		if (strcmp(ls->name, group) == 0) {
			fprintf(stderr, "%s called recursively\n", group);
			return(0);
		}
	ls = (struct list *) malloc(sizeof (struct list));
	ls->name = (char *) malloc(newkeylen+1);
	strcpy(ls->name, group);
	ls->nxt = list;
	list = ls;
	
	p = val;
	p[vallen] = 0;
	while (p != NULL) {
		match = 0;
		while (*p == ' ' || *p == '\t')
			p++;
		if (*p == 0 || *p == '#')
			break;
		if (*p == '(') {
			p++;
			while (*p == ' ' || *p == '\t')
				p++;
			r = q = index(p, ',');
			if (q == NULL) {
				fprintf(stderr,
				    "syntax error in /etc/netgroup\n");
				return(0);
			}
			if (p == q || machine == NULL)
				match++;
			else {
				while (*(q-1) == ' ' || *(q-1) == '\t')
					q--;
				if (strncmp(machine, p, q-p) == 0)
					match++;
			}
			p = r+1;

			while (*p == ' ' || *p == '\t')
				p++;
			r = q = index(p, ',');
			if (q == NULL) {
				fprintf(stderr,
				    "syntax error in /etc/netgroup\n");
				return(0);
			}
			if (p == q || name == NULL)
				match++;
			else {
				while (*(q-1) == ' ' || *(q-1) == '\t')
					q--;
				if (strncmp(name, p, q-p) == 0)
					match++;
			}
			p = r+1;

			while (*p == ' ' || *p == '\t')
				p++;
			r = q = index(p, ')');
			if (q == NULL) {
				fprintf(stderr,
				    "syntax error in /etc/netgroup\n");
				return(0);
			}
			if (p == q || domain == NULL)
				match++;
			else {
				while (*(q-1) == ' ' || *(q-1) == '\t')
					q--;
				if (strncmp(domain, p, q-p) == 0)
					match++;
			}
			p = r+1;
			if (match == 3) {
				return 1;
			}
		}
		else {
			q = any(p, " \t\n#");
			if (q && *q == '#')
				break;
			if (q)
				*q = 0;
			if (doit(p))
				return 1;
			if (q)
				*q = ' ';
		}
		p = any(p, " \t");
	}
	return 0;
}

/* 
 * scans cp, looking for a match with any character
 * in match.  Returns pointer to place in cp that matched
 * (or NULL if no match)
 */
static char *
any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return ((char *)0);
}
