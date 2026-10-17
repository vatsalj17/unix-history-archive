/*
 *	global command (/etc/glob)
 *
 *	execl("/etc/glob", search-path, command, args...,0)
 *
 *	PWB version expects shell $p as arg0, so that it works same as sh.
 *
 *	glob params
 *
 *	"*" in params matches r.e ".*"
 *	"?" in params matches r.e. "."
 *	"[...]" in params matches character class
 *	"[...a-z...]" in params matches a through z.

 *	perform command with argument list
 *	constructed as follows:
 *	if param does not contain "*", "[", or "?", use it as is
 *	if it does, find all files in current directory
 *	which match the param, sort them, and use them
 *
 *	PWB version uses directory search sequence.
 */

/*
 *	new sizes taken from USG glob; old were 522 and 200
 */
#define	STRSIZ	5300
char	ab[STRSIZ];		/* generated characters */
char	*ava[500];		/* generated arguments */
char	**av &ava[1];
char	*string ab;
int	ncoll;
char	*pathstr;

main(argc, argv)
char *argv[];
{
	register char *cp;

	if (argc < 3) {
		write(2, "Arg count\n", 10);
		exit(1);
	}
	pathstr = argv[0];
	argv++;
	*av++ = *argv;
	while (--argc >= 2)
		expand(*++argv);
	if (ncoll==0) {
		write(2, "No match\n", 9);
		exit(1);
	}
	texec(ava[1], &ava[1]);
	exit(1);
}

expand(as)
char *as;
{
	register char *s, *cs;
	register int dirf;
	char **oav;
	static struct {
		int	ino;
		char	name[16];
	} entry;

	s = cs = as;
	while (*cs!='*' && *cs!='?' && *cs!='[') {
		if (*cs++ == 0) {
			*av++ = cat(s, "");
			return;
		}
	}
	for (;;) {
		if (cs==s) {
			dirf = open(".", 0);
			s = "";
			break;
		}
		if (*--cs == '/') {
			*cs = 0;
			dirf = open(s==cs? "/": s, 0);
			*cs++ = 0200;
			break;
		}
	}
	if (dirf<0) {
		write(2, "No directory\n", 13);
		exit(1);
	}
	oav = av;
	while (read(dirf, &entry, 16) == 16) {
		if (entry.ino==0)
			continue;
		if (match(entry.name, cs)) {
			*av++ = cat(s, entry.name);
			ncoll++;
		}
	}
	close(dirf);
	sort(oav);
}

sort(oav)
char **oav;
{
	register char **p1, **p2, **c;

	p1 = oav;
	while (p1 < av-1) {
		p2 = p1;
		while(++p2 < av) {
			if (compar(*p1, *p2) > 0) {
				c = *p1;
				*p1 = *p2;
				*p2 = c;
			}
		}
		p1++;
	}
}


match(s, p)
char *s, *p;
{
	if (*s=='.' && *p!='.')
		return(0);
	return(amatch(s, p));
}

amatch(as, ap)
char *as, *ap;
{
	register char *s, *p;
	register scc;
	int c, cc, ok, lc;

	s = as;
	p = ap;
	if (scc = *s++)
		if ((scc =& 0177) == 0)
			scc = 0200;
	switch (c = *p++) {

	case '[':
		ok = 0;
		lc = 077777;
		while (cc = *p++) {
			if (cc==']') {
				if (ok)
					return(amatch(s, p));
				else
					return(0);
			} else if (cc=='-') {
				if (lc<=scc && scc<=(c = *p++))
					ok++;
			} else
				if (scc == (lc=cc))
					ok++;
		}
		return(0);

	default:
		if (c!=scc)
			return(0);

	case '?':
		if (scc)
			return(amatch(s, p));
		return(0);

	case '*':
		return(umatch(--s, p));

	case '\0':
		return(!scc);
	}
}

umatch(s, p)
char *s, *p;
{
	if(*p==0)
		return(1);
	while(*s)
		if (amatch(s++,p))
			return(1);
	return(0);
}

compar(as1, as2)
char *as1, *as2;
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while (*s1++ ==  *s2)
		if (*s2++ == 0)
			return(0);
	return (*--s1 - *s2);
}

cat(as1, as2)
char *as1, *as2;
{
	register char *s1, *s2;
	register int c;

	s2 = string;
	s1 = as1;
	while (c = *s1++) {
		if (s2 > &ab[STRSIZ]) {
			err("Arg list too long");
			exit(1);
		}
		c =& 0177;
		if (c==0) {
			*s2++ = '/';
			break;
		}
		*s2++ = c;
	}
	s1 = as2;
	do {
		if (s2 > &ab[STRSIZ]) {
			err("Arg list too long");
			exit(1);
		}
		*s2++ = c = *s1++;
	} while (c);
	s1 = string;
	string = s2;
	return(s1);
}
/*
 *	texec(name, argv)
 *	pexec routine modified for glob environment.
 */
#define	E2BIG	7
#define	ENOEXEC	8
#define	ENOMEM	12
#define	EACCES	13
#define	ETXTBSY	26
texec(f, at)
int *at;
{
	extern errno;
	register int np;
	register char *cp;
	static char line[48];
	char txe2big, txeacces;
	int txtbsy;

	txeacces = txe2big = 0;
	txtbsy = 0;
	cp = !any('/', f) ? pathstr : "\0";
	do {
		cp = pcat(cp, f, line);
	retry:
		execv(line, at);
		if (errno == ENOEXEC) {
			*at = line;
			*--at = "/bin/sh";
			execv(*at, at);
			pexerr("No shell!", 0);
			return;
		}
		else if (errno == EACCES)
			txeacces++;	/* file there, missing x (probably) */
		else if (errno == ENOMEM) {
			pexerr(f, "too large");
			return;
		}
		else if (errno == E2BIG)
			txe2big++;
		else if (errno == ETXTBSY) {
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

pexerr(s1, s2)
char *s1, *s2;
{
	prs(s1);
	if (s2) {
		prs(": ");
		prs(s2);
	}
	prs("\n");
}

pcat(so1, so2, si) char *so1, *so2, *si; {
	register char *r1, *r2, *s;

	r1 = so1; r2 = so2; s = si;
	while(*r1 != ':' && *r1 != '\0') *s++ = *r1++;
	if(si != s) *s++ = '/';
	while(*r2) *s++ = *r2++;
	*s = '\0';
	return *r1 ? ++r1 : 0;
}

prs(s)
register char *s;
{
	while(*s)
		putc(*s++);
}

putc(c)
{

	write(2, &c, 1);
}
err(s)
char *s;
{
	prs(s);
	prs("\n");
}

any(c, s)
register char c;
register char *s;
{
	while(*s)
		if(*s++ == c)
			return(1);
	return(0);
}
