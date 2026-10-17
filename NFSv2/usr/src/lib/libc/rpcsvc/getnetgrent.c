#ifndef lint
static  char sccsid[] = "@(#)getnetgrent.c 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif

/* 
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <ctype.h>
#include <rpcsvc/ypclnt.h>

#define MAXGROUPLEN 1024

/* 
 * access members of a netgroup
 */

struct grouplist {		/* also used by pwlib */
	char *gl_machine;
	char *gl_name;
	char *gl_domain;
	struct grouplist *gl_nxt;
};

static char *any(), *index();
static struct list {			/* list of names to check for loops */
	char *name;
	struct list *nxt;
} *list;
static struct grouplist *grouplist, *grlist;
struct grouplist *_getgroup();
static char domain[256];

setnetgrent(grp)
	char *grp;
{
	static char oldgrp[256];
	
	if (strcmp(oldgrp, grp) == 0)
		grlist = grouplist;
	else {
		if (grouplist != NULL)
			endnetgrent();
		_getgroup(grp);
		grlist = grouplist;
		strcpy(oldgrp, grp);
	}
}

endnetgrent() {
	struct grouplist *gl;
	
	for (gl = grouplist; gl != NULL; gl = gl->gl_nxt) {
		free(gl->gl_name);
		free(gl->gl_domain);
		free(gl->gl_machine);
		free(gl);
	}
	grouplist = NULL;
	grlist = NULL;
}

getnetgrent(machinep, namep, domainp)
	char **machinep, **namep, **domainp;
{
	if (grlist) {
		*machinep = grlist->gl_machine;
		*namep = grlist->gl_name;
		*domainp = grlist->gl_domain;
		grlist = grlist->gl_nxt;
		return (1);
	}
	else
		return (0);
}

/* 
 * also used by pwlib
 */
struct grouplist *
_getgroup(grp)
	char *grp;
{
	if (getdomainname(domain, sizeof(domain)) < 0) {
		fprintf(stderr, 
		    "getnetgrent: getdomainname system call missing\n");
	    exit(1);
	}
	list = NULL;
	doit(grp);
	return grouplist;
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
	char buf[MAXGROUPLEN];
	struct grouplist *gpls;
	
	newkey = group;
	newkeylen = strlen(group);
	if ((err = yp_match(domain, "netgroup",
	    newkey, newkeylen, &val, &vallen)) != 0) {
#ifdef DEBUG
		if (err == YPERR_KEY)
			fprintf(stderr, "no such group as %s\n", group);
		else
			fprintf(stderr, "error: yp_match\n");
#endif
		grouplist = NULL;
		return;
	}
	/* 
	 * check for recursive loops
	 */
	for (ls = list; ls != NULL; ls = ls->nxt)
		if (strcmp(ls->name, group) == 0) {
			fprintf(stderr, "%s called recursively\n", group);
			grouplist = NULL;
			return;
		}
	ls = (struct list *) malloc(sizeof (struct list));
	ls->name = (char *) malloc(newkeylen+1);
	strcpy(ls->name, group);
	ls->nxt = list;
	list = ls;
	
	p = val;
	p[vallen] = 0;
	while (p != NULL) {
		while (*p == ' ' || *p == '\t')
			p++;
		if (*p == 0 || *p =='#')
			break;
		if (*p == '(') {
			gpls = (struct grouplist *)
			    malloc(sizeof (struct grouplist));
			p++;
			while (*p == ' ' || *p == '\t')
				p++;
			r = q = index(p, ',');
			if (q == NULL) {
				fprintf(stderr,
				    "syntax error in /etc/netgroup\n");
				grouplist = NULL;
				return 0;
			}
			if (p == q) {
				sprintf(buf, "*");
				gpls->gl_machine =
				    (char *)malloc(strlen(buf)+1);
				strcpy(gpls->gl_machine, buf);
			}
			else {
				while (*(q-1) == ' ' || *(q-1) == '\t')
					q--;
				sprintf(buf, "%.*s", q-p, p);
				gpls->gl_machine =
				    (char *)malloc(strlen(buf)+1);
				strcpy(gpls->gl_machine, buf);
			}
			p = r+1;

			while (*p == ' ' || *p == '\t')
				p++;
			r = q = index(p, ',');
			if (q == NULL) {
				fprintf(stderr,
				    "syntax error in /etc/netgroup\n");
				grouplist = NULL;
				return;
			}
			if (p == q) {
				sprintf(buf, "*");
				gpls->gl_name =
				    (char *)malloc(strlen(buf) + 1);
				strcpy(gpls->gl_name, buf);
			}
			else {
				while (*(q-1) == ' ' || *(q-1) == '\t')
					q--;
				sprintf(buf, "%.*s", q-p, p);
				gpls->gl_name =
				    (char *)malloc(strlen(buf) + 1);
				strcpy(gpls->gl_name, buf);
			}
			p = r+1;

			while (*p == ' ' || *p == '\t')
				p++;
			r = q = index(p, ')');
			if (q == NULL) {
				fprintf(stderr,
				    "syntax error in /etc/netgroup\n");
				grouplist = NULL;
				return;
			}
			if (p == q) {
				sprintf(buf, "*");
				gpls->gl_domain =
				    (char *)malloc(strlen(buf)+1);
				strcpy(gpls->gl_domain, buf);
			}
			else {
				while (*(q-1) == ' ' || *(q-1) == '\t')
					q--;
				sprintf(buf, "%.*s", q-p, p);
				gpls->gl_domain =
				    (char *)malloc(strlen(buf)+1);
				strcpy(gpls->gl_domain, buf);
			}
			p = r+1;
			gpls->gl_nxt = grouplist;
			grouplist = gpls;
		}
		else {
			q = any(p, " \t\n#");
			if (q && *q == '#')
				break;
			if (q)
				*q = 0;
			doit(p);
			if (q)
				*q = ' ';
		}
		p = any(p, " \t");
	}
	if (list) {
		free(list);
		free(list->name);
		list = list->nxt;
	}
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
