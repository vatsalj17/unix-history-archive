/*
 *	pexec(name, argv)
 *	PWB interface to execv, performs standard directory search,
 *	using .path, if available. Corresponds to sh routine texec.
 *	uses sequence found in pathstr, if non-null.
 *	Much of code exists for reliability in large systems having
 *	many processes and heavy use of shell procedures.
 */
#define	E2BIG	7
#define	ENOEXEC	8
#define	ENOMEM	12
#define	EACCES	13
#define	ETXTBSY	26
extern	errno;
char	pathstr[128];	/* directory sequence */
char	shellnam[16];	/* name of shell */

pexec(f, at)
register *at;
{
	register char *cp;
	char line[48];
	char txe2big, txeacces, txtbsy;

	if (pexinit())
		return;
	txe2big = txeaccess = txtbsy = 0;
	cp = !pexany('/', f) ? pathstr : "";
	do {
		cp = pexcat(cp, f, line);
	retry:
		execv(line, at);
		switch (errno) {
		case ENOEXEC:
			*at = line;
			*--at = shellnam;
			execv(*at, at);
			pexerr(shellnam, "No shell!");
			return;
		case EACCES:
			txeacces++;	/* file there, missing x (probably) */
			break;
		case ENOMEM:
			pexerr(f, "too large");
			return;
		case E2BIG:
			txe2big++;
			break;
		case ETXTBSY:
			if ((txtbsy =+ 10) > 60) {
				pexerr(f, "text busy");
				return;
			}
			sleep(txtbsy);
			goto retry;
		}
	} while (cp);
	if (txe2big)
		pexerr(f, "arg list too long");
	else if (txeacces)
		pexerr(f, "file not executable");
	else
		pexerr(f, "not found");
}

/*
 *	pexinit: fills in pathstr and shellnam as needed.
 *	may be invoked before fork to avoid unnecessary .path opening.
 *	returns 0 if OK, -1 if any error.
 */
pexinit()
{
	char pathbuf[sizeof pathstr + sizeof shellnam];
	register n, f;
	char *newpath, *newshell;
	char *p;

	if (pathstr[0] && shellnam[0])
		return(0);
	newshell = "/bin/sh";
	pexcat(logdir(), ".path", pathbuf);
	if ((f = open(pathbuf, 0)) < 0)
		newpath = (getuid() & 0377) ? ":/bin:/usr/bin" : "/bin:/etc:/";
	else {
		n = read(f, pathbuf, sizeof pathbuf);
		close(f);
		if (n <= 0) {
			pexerr("cannot read .path", 0);
			return(-1);
		}
		if (pexline(pathbuf, pathbuf + n, sizeof pathstr, &newpath, &p)
		|| pexline(p, pathbuf + n, sizeof shellnam, &newshell, &p))
			return(-1);
	}
	if (!pathstr[0])
		pexcopy(newpath, pathstr);
	if (!shellnam[0])
		pexcopy(newshell, shellnam);
	return(0);
}

/*
 *	pexline: scan for a line (if any) beginning at ptr,
 *	ending at ptrlim -1, for line up to psize bytes long.
 *	convert it to string, return beginning addr in pret
 *	and addr of next line in pnext
 *	return 0 if OK (or not present), -1 if runs off end in middle of line
 *	or if line too long.
 */
pexline(ptr, ptrlim, psize, pret, pnext)
register char *ptr, *ptrlim;
int psize;
char **pret, **pnext;
{
	if (ptr >= ptrlim)
		return(0);
	*pret = ptr;
	if (ptrlim > ptr + psize)
		ptrlim = ptr + psize;
	for (; ptr < ptrlim; ptr++)
		if (*ptr == '\n') {
			*ptr++ = '\0';
			*pnext = ptr;
			return(0);
		}
	pexerr(".path too long", 0);
	return(-1);
}

pexcopy(source, sink)
register char *source, *sink;
{
	while (*sink++ = *source++);
}

pexcat(so1, so2, si) char *so1, *so2, *si; {
	register char *r1, *r2, *s;

	r1 = so1; r2 = so2; s = si;
	while (*r1 != ':' && *r1 != '\0') *s++ = *r1++;
	if (si != s) *s++ = '/';
	while (*r2) *s++ = *r2++;
	*s = '\0';
	return *r1 ? ++r1 : 0;
}

pexerr(s1, s2)
char *s1, *s2;
{
	pexprs(s1);
	if (s2) {
		pexprs(": ");
		pexprs(s2);
	}
	pexprs("\n");
}

pexprs(s)
register char *s;
{
	while (*s)
		write(2, s++, 1);
}

pexany(c, s)
int c;
register char *s;
{
	while (*s)
		if (*s++ == c)
			return(1);
	return(0);
}
