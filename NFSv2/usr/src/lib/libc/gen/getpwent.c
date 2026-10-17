#ifndef lint
static  char sccsid[] = "@(#)getpwent.c 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif

/* 
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <pwd.h>
#include <rpcsvc/ypclnt.h>

#define MAXINT 0x7fffffff;
struct grouplist {
	char *gl_machine;
	char *gl_name;
	char *gl_domain;
	struct grouplist *gl_nxt;
};
static char domain[256];
static struct passwd NULLPW = {NULL, NULL, 0, 0, 0, NULL, NULL, NULL, NULL};
static char PASSWD[]	= "/etc/passwd";
static char EMPTY[] = "";
static FILE *pwf = NULL;	/* pointer into /etc/passwd */
static char *yp;		/* pointer into yellow pages */
static struct grouplist *grplstp;/* pointer into a grouplist */
static int yplen;
static char *oldyp = NULL;	
static int oldyplen;
struct grouplist *_getgroup();
static struct passwd *interpret();
static struct passwd *interpretwithsave();
static struct passwd *save();
static struct passwd *getnamefromyellow();
static struct passwd *getuidfromyellow();
static struct list {
    char *name;
    struct list *nxt
} *minuslist;			/* list of - items */

struct passwd *
getpwnam(name)
	char *name;
{
	struct passwd *pw;
	char line[BUFSIZ+1];

	setpwent();
	if (!pwf)
		return NULL;
	while (fgets(line, BUFSIZ, pwf) != NULL) {
		pw = interpret(line, strlen(line));
		if (matchname(line, &pw, name)) {
			endpwent();
			return pw;
		}
	}
	endpwent();
	return NULL;
}

struct passwd *
getpwuid(uid)
	register uid;
{
	struct passwd *pw;
	char line[BUFSIZ+1];

	setpwent();
	if (!pwf)
		return NULL;
	while (fgets(line, BUFSIZ, pwf) != NULL) {
		pw = interpret(line, strlen(line));
		if (matchuid(line, &pw, uid)) {
			endpwent();
			return pw;
		}
	}
	endpwent();
	return NULL;
}

setpwent()
{
	if (getdomainname(domain, sizeof(domain)) < 0) {
		fprintf(stderr, 
		    "setpwent: getdomainname system call missing\n");
		exit(1);
	}
	if (pwf == NULL)
		pwf = fopen(PASSWD, "r");
	else
		rewind(pwf);
	yp = NULL;
	grplstp = NULL;
	freeminuslist();
}

endpwent()
{
	if (pwf != NULL) {
		fclose(pwf);
		pwf = NULL;
	}
	yp = NULL;
	grplstp = NULL;
	freeminuslist();
}

static char *
pwskip(p)
	register char *p;
{
	while(*p && *p != ':' && *p != '\n')
		++p;
	if (*p) *p++ = 0;
	return(p);
}

struct passwd *
getpwent()
{
	char line1[BUFSIZ+1];
	static struct passwd *savepw;
	struct passwd *pw;
	struct grouplist *glp;

	if (domain[0] == 0 && getdomainname(domain, sizeof(domain)) < 0) {
		fprintf(stderr, 
		    "getpwent: getdomainname system call missing\n");
		exit(1);
	}
	if (pwf == NULL && (pwf = fopen(PASSWD, "r")) == NULL)
		return (NULL);
  again:
	if (yp) {
		pw = interpretwithsave(yp, yplen, savepw);
		getnextfromyellow();
		if (onminuslist(pw))
			goto again;
		else
			return pw;
	}
	else if (grplstp) {
		pw = getnamefromyellow(grplstp->gl_name, savepw);
		grplstp = grplstp->gl_nxt;
		if (pw == NULL)
			goto again;
		if (onminuslist(pw))
			goto again;
		else
			return pw;
	}
	else {
		if (fgets(line1, BUFSIZ, pwf) == NULL)
			return (NULL);
		pw = interpret(line1, strlen(line1));
		switch(line1[0]) {
			case '+':
				if (strcmp(pw->pw_name, "+") == 0) {
					getfirstfromyellow();
					savepw = save(pw);
					goto again;
				}
				if (line1[1] == '@') {
					savepw = save(pw);
					grplstp = _getgroup(pw->pw_name + 2);
					goto again;
				}
				/* 
				 * else look up this entry in yellow pages
				 */
				savepw = save(pw);
				pw = getnamefromyellow(pw->pw_name+1, savepw);
				if (pw == NULL)
					goto again;
				else if (onminuslist(pw))
					goto again;
				else
					return pw;
				break;
			case '-':
				if (line1[1] == '@') {
					for (glp = _getgroup(pw->pw_name+2);
					    glp != NULL; glp = glp->gl_nxt)
						addtominuslist(glp->gl_name);
				}
				else
					addtominuslist(pw->pw_name+1);
				goto again;
				break;
			default:
				if (onminuslist(pw))
					goto again;
				return pw;
				break;
		}
	}
}

static
matchname(line1, pwp, name)
	char line1[];
	struct passwd **pwp;
	char *name;
{
	struct grouplist *glp;
	struct passwd *savepw;
	struct passwd *pw = *pwp;

	switch(line1[0]) {
		case '+':
			if (strcmp(pw->pw_name, "+") == 0) {
				savepw = save(pw);
				pw = getnamefromyellow(name, savepw);
				if (pw) {
					*pwp = pw;
					return 1;
				}
				else
					return 0;
			}
			if (line1[1] == '@') {
				glp = _getgroup(pw->pw_name + 2);
				for(; glp != NULL; glp = glp->gl_nxt)
					if (strcmp(name, glp->gl_name) == 0) {
						savepw = save(pw);
						pw = getnamefromyellow(
						    glp->gl_name, savepw);
						if (pw) {
							*pwp = pw;
							return 1;
						}
						else
							return 0;
					    }
				return 0;
			}
			if (strcmp(pw->pw_name+1, name) == 0) {
				savepw = save(pw);
				pw = getnamefromyellow(pw->pw_name+1, savepw);
				if (pw) {
					*pwp = pw;
					return 1;
				}
				else
					return 0;
			}
			break;
		case '-':
			if (line1[1] == '@') {
				glp = _getgroup(pw->pw_name + 2);
				for(; glp != NULL; glp = glp->gl_nxt)
					if (strcmp(name, glp->gl_name) == 0) {
						*pwp = NULL;
						return 1;
					    }
			}
			else if (strcmp(pw->pw_name+1, name) == 0) {
				*pwp = NULL;
				return 1;
			}
			break;
		default:
			if (strcmp(pw->pw_name, name) == 0)
				return 1;
	}
	return 0;
}

static
matchuid(line1, pwp, uid)
	char line1[];
	struct passwd **pwp;
{
	struct grouplist *glp;
	struct passwd *savepw;
	struct passwd *pw = *pwp;

	switch(line1[0]) {
		case '+':
			if (strcmp(pw->pw_name, "+") == 0) {
				savepw = save(pw);
				pw = getuidfromyellow(uid, savepw);
				if (pw) {
					*pwp = pw;
					return 1;
				}
				else
					return 0;
			}
			if (line1[1] == '@') {
				glp = _getgroup(pw->pw_name + 2);
				for(; glp != NULL; glp = glp->gl_nxt)
					if (uid == uidof(glp->gl_name)) {
						savepw = save(pw);
						pw = getnamefromyellow(
						    glp->gl_name, savepw);
						if (pw) {
							*pwp = pw;
							return 1;
						}
						else
							return 0;
					    }
				return 0;
			}
			savepw = save(pw);
			pw = getnamefromyellow(pw->pw_name+1, savepw);
			if (pw && pw->pw_uid == uid) {
				*pwp = pw;
				return 1;
			}
			else
				return 0;
			break;
		case '-':
			if (line1[1] == '@') {
				glp = _getgroup(pw->pw_name + 2);
				for(; glp != NULL; glp = glp->gl_nxt)
					if (uid == uidof(glp->gl_name)) {
						*pwp = NULL;
						return 1;
					    }
			}
			else if (uid == uidof(pw->pw_name+1)) {
				*pwp = NULL;
				return 1;
			}
			break;
		default:
			if (pw->pw_uid == uid)
				return 1;
	}
	return 0;
}

static
uidof(name)
	char *name;
{
	struct passwd *pw;
	
	pw = getnamefromyellow(name, &NULLPW);
	if (pw)
		return pw->pw_uid;
	else
		return MAXINT;
}

static
getnextfromyellow()
{
	int reason;
	char *key;
	int keylen;
	
	if (reason = yp_next(domain, "passwd.byname",
	    oldyp, oldyplen, &key, &keylen,
	    &yp, &yplen)) {
#ifdef DEBUG
fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif
		yp = NULL;
	}
	oldyp = key;
	oldyplen = keylen;
}

static
getfirstfromyellow()
{
	int reason;
	char *key;
	int keylen;
	
	if (reason =  yp_first(domain, "passwd.byname",
	    &key, &keylen, &yp, &yplen)) {
#ifdef DEBUG
fprintf(stderr, "reason yp_first failed is %d\n", reason);
#endif
		yp = NULL;
	}
	oldyp = key;
	oldyplen = keylen;
}

static struct passwd *
getnamefromyellow(name, savepw)
	char *name;
	struct passwd *savepw;
{
	struct passwd *pw;
	int reason;
	char *val;
	int vallen;
	
	if (reason = yp_match(domain, "passwd.byname",
	    name, strlen(name), &val, &vallen)) {
#ifdef DEBUG
fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif
		return NULL;
	}
	else {
		pw = interpret(val, vallen);
		if (savepw->pw_passwd && *savepw->pw_passwd)
			pw->pw_passwd =  savepw->pw_passwd;
		if (savepw->pw_gecos && *savepw->pw_gecos)
			pw->pw_gecos = savepw->pw_gecos;
		if (savepw->pw_dir && *savepw->pw_dir)
			pw->pw_dir = savepw->pw_dir;
		if (savepw->pw_shell && *savepw->pw_shell)
			pw->pw_shell = savepw->pw_shell;
		return pw;
	}
}

static struct passwd *
getuidfromyellow(uid, savepw)
	int uid;
	struct passwd *savepw;
{
	struct passwd *pw;
	int reason;
	char *val;
	int vallen;
	char uidstr[20];
	
	sprintf(uidstr, "%d", uid);
	if (reason = yp_match(domain, "passwd.byuid",
	    uidstr, strlen(uidstr), &val, &vallen)) {
#ifdef DEBUG
fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif
		return NULL;
	}
	else {
		pw = interpret(val, vallen);
		if (savepw->pw_passwd && *savepw->pw_passwd)
			pw->pw_passwd =  savepw->pw_passwd;
		if (savepw->pw_gecos && *savepw->pw_gecos)
			pw->pw_gecos = savepw->pw_gecos;
		if (savepw->pw_dir && *savepw->pw_dir)
			pw->pw_dir = savepw->pw_dir;
		if (savepw->pw_shell && *savepw->pw_shell)
			pw->pw_shell = savepw->pw_shell;
		return pw;
	}
}

static struct passwd *
interpretwithsave(val, len, savepw)
	char *val;
	struct passwd *savepw;
{
	struct passwd *pw;
	
	pw = interpret(val, len);
	if (savepw->pw_passwd && *savepw->pw_passwd)
		pw->pw_passwd =  savepw->pw_passwd;
	if (savepw->pw_gecos && *savepw->pw_gecos)
		pw->pw_gecos = savepw->pw_gecos;
	if (savepw->pw_dir && *savepw->pw_dir)
		pw->pw_dir = savepw->pw_dir;
	if (savepw->pw_shell && *savepw->pw_shell)
		pw->pw_shell = savepw->pw_shell;
	return pw;
}

static struct passwd *
interpret(val, len)
	char *val;
{
	register char *p;
	static struct passwd passwd;
	static char line[BUFSIZ+1];

	strncpy(line, val, len);
	p = line;
	line[len] = '\n';
	line[len+1] = 0;

	passwd.pw_name = p;
	p = pwskip(p);
	passwd.pw_passwd = p;
	p = pwskip(p);
	passwd.pw_uid = atoi(p);
	p = pwskip(p);
	passwd.pw_gid = atoi(p);
	passwd.pw_quota = 0;
	passwd.pw_comment = EMPTY;
	p = pwskip(p);
	passwd.pw_gecos = p;
	p = pwskip(p);
	passwd.pw_dir = p;
	p = pwskip(p);
	passwd.pw_shell = p;
	while(*p && *p != '\n') p++;
	*p = '\0';
	return(&passwd);
}

static
freeminuslist() {
	struct list *ls;
	
	for (ls = minuslist; ls != NULL; ls = ls->nxt) {
		free(ls->name);
		free(ls);
	}
	minuslist = NULL;
}

static
addtominuslist(name)
	char *name;
{
	struct list *ls;
	char *buf;
	
	ls = (struct list *)malloc(sizeof(struct list));
	buf = (char *)malloc(strlen(name) + 1);
	strcpy(buf, name);
	ls->name = buf;
	ls->nxt = minuslist;
	minuslist = ls;
}

/* 
 * save away psswd, gecos, dir and shell fields, which are the only
 * ones which can be specified in a local + entry to override the
 * value in the yellow pages
 */
static struct passwd *
save(pw)
	struct passwd *pw;
{
	struct passwd *sv;
	
	sv = (struct passwd *)malloc(sizeof(struct passwd));

	sv->pw_passwd = (char *)malloc(strlen(pw->pw_passwd) + 1);
	strcpy(sv->pw_passwd, pw->pw_passwd);

	sv->pw_gecos = (char *)malloc(strlen(pw->pw_gecos) + 1);
	strcpy(sv->pw_gecos, pw->pw_gecos);

	sv->pw_dir = (char *)malloc(strlen(pw->pw_dir) + 1);
	strcpy(sv->pw_dir, pw->pw_dir);

	sv->pw_shell = (char *)malloc(strlen(pw->pw_shell) + 1);
	strcpy(sv->pw_shell, pw->pw_shell);

	return sv;
}

static
onminuslist(pw)
	struct passwd *pw;
{
	struct list *ls;
	register char *nm;
	
	nm = pw->pw_name;
	for (ls = minuslist; ls != NULL; ls = ls->nxt)
		if (strcmp(ls->name, nm) == 0)
			return 1;
	return 0;
}
