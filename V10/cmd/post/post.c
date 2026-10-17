#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <pwd.h>

#define	OPTS	"a:m:o:q:wxyDSVdef:mpr#"
#define	MAILX	"/bin/mail"
#define	UPDATE	path("update")
#define	DBINFO	path("dbinfo")
#define	DELPAP	"del=p"
#define	ALLYES	"all=y"
#define	ASKYES	"ask=y"

#define	PREMA(e)	printf("%s%s", e, yflg ? ", " : "\n")
#define	PRMULT(m, p)	PREMA(m[*m[EMAIL] && !p ? EMAIL : PAPER])

#define	WHO	0
#define	EMAIL	1
#define	PAPER	2
#define	NARGS	3
#define	MULT	10

typedef char	*strings[512];
typedef char	buffer[BUFSIZ];

char	usage[] = "usage:\
	post [-a attr] [-m mods] [-o fmt] [-q query]\n\
	     [-w] [mail options] [addresses...]\n\
	     [-D] [-S] [-V]\n";

strings	qdef;
strings	postattr;
buffer	postofmt;
int	postetc;
int	wflg, xflg, yflg;
int	askyes = 0;
int	exitval = 0;
char	*mult[MULT][NARGS];
char	*pq;
void	resolve();

extern char	emp_fmt[], ema_fmt[], who_fmt[];
extern char	emp_def[], emp_seq[], emp_uni[];
extern char	*pq_open();
extern char	*getenv(), *strdup();
struct passwd	*getpwnam();

char	*ofmt = emp_fmt, *attr = emp_def, *mods = "", *qopt = "";

main(argc, argv)
int	argc;
char	*argv[];
{
	extern char	*optarg;
	extern int	optind;
	strings	args;
	buffer	buf;
	char	**ap = args;
	int	c;

	*ap++ = argv[0];

	while ((c = getopt(argc, argv, OPTS)) != EOF)
		switch (c) {
		case 'a':
			attr = optarg;
			break;
		case 'm':
			mods = optarg;
			break;
		case 'o':
			ofmt = optarg;
			break;
		case 'q':
			qopt = optarg;
			break;
		case 'w':
			wflg++;
			break;
		case 'x':
			xflg++;
			break;
		case 'y':
			yflg++;
			break;
		case 'D':
			execvp(DBINFO, argv);
			perror(DBINFO);
			return 1;
		case 'S':
			execvp(UPDATE, argv);
			perror(UPDATE);
			return 1;
		case 'V':
			puts("research post 4.0 9/1/88");
			return 1;
		case '?':
			fputs(usage, stderr);
			return 1;
		default:
			sprintf(*ap++ = malloc(3), "-%c", c);
			if (optarg)
				*ap++ = optarg;
		}

	if (!wflg && !xflg && !yflg) {
		while (optind < argc)
			*ap++ = argv[optind++];
		*ap++ = 0;
		execv(MAILX, args);
		perror(MAILX);
		return 1;
	}
	if (xflg || yflg) {
		strcpy(buf, ofmt);
		strcat(buf, ema_fmt);
		ofmt = buf;
	}

	postetc = !!getenv("POSTETC");
	strvec(attr, postattr, "/:");
	fmtcomp(postofmt, ofmt, qdef);
	smerge(getenv("POSTQUAL"), qdef);
	smerge(qopt, qdef);

	if (option(ASKYES, qdef))
		askyes++;
	if (xflg || yflg)
		smerge(emp_uni, qdef);

	if (wflg && optind == argc)
		argv[argc++] = "";
	while (optind < argc) {
		resolve(argv[optind++]);
		if (yflg)
			putchar('\n');
	}
	if (pq && pq_close(pq) == -1)
		pqerr("pq");
	return yflg ? 0 : exitval;
}

void
resolve(name)
char	*name;
{
	strings	q, n, rec;
	int	delpap, allyes;
	int	c, r;
	char	*p;
	buffer	out;

	p = strpbrk(name, "!@%/:=");
	if (*name && strchr("+|", *name) || p && !strchr("/:=", *p)
	 || !p && !strpbrk(name, "._") && postetc && getpwnam(name)) {
		PREMA(name);
		return;
	}

	if (!dodef(name, n, postattr)) {
		eomatch(0, 0, 0, name);
		return;
	}
	for (c = 0; q[c] = qdef[c]; c++);
	merge(n, q);
	delpap = option(DELPAP, q);
	allyes = option(ALLYES, q);
	if (option(ASKYES, q))
		askyes++;

	c = 0;
	if ((r = pqowrite(q)) != -1) {
		while ((r = pq_read(pq, rec)) > 0) {
			fmtexec(out, postofmt, rec);
			if (match(c++, out, delpap, allyes, name, q))
				return;
		}
		if (r == -1)
			pqerr(name);
	} else
		pqerr(name);

	if (c == 0 && r == 0)
		fprintf(stderr, "post: %s: Not found\n", name);
	eomatch(c, delpap, allyes, name);
}

dodef(name, vars, defs)
char	*name;
strings	vars, defs;
{
	static buffer	tmp, buf;
	register char	*bp = buf;
	register int	m, i, c;

	strcpy(tmp, name);
	strvec(tmp, vars, "/:");
	for (m = 0; defs[m]; m++);

	for (i = 0; vars[i]; i++)
		if (!strchr(vars[i], '='))
			if (i < m) {
				c = sprintf(bp, "%s=%s", defs[i], vars[i]);
				vars[i] = bp;
				bp += c + 1;
			} else {
				fprintf(stderr, "post: %s: %s: No attribute\n",
					name, vars[i]);
				return 0;
			}
	return 1;
}

match(c, out, delpap, allyes, name, q)
int	c;
char	*out;
int	delpap, allyes;
char	*name;
strings	q;
{
	buffer	buf;
	int	i, m = c % MULT;
	char	*s;

	if (wflg) {
		fputs(out, stdout);
		return 0;
	}

	s = strdup(out);
	for (i = 0; i < NARGS && (mult[m][i] = s); i++)
		if (s = strchr(s, '|'))
			*s++ = 0;
	skname(mult[m][PAPER]);
	if (s = strchr(mult[m][EMAIL], '('))
		if (xflg)
			*--s = 0;
		else
			prname(s);

	if (allyes) {
		if (xflg) {
			PRMULT(mult[m], delpap);
			return 0;
		}
		if (yflg) {
			s = buf;
			s += sprintf(s, "!");
			for (i = 0; q[i]; i++)
				if (strchr(q[i], '=') && strcmp(q[i], emp_uni))
					s += sprintf(s, "%s:", q[i]);
			if (delpap)
				s += sprintf(s, "%s:", DELPAP);
			strcpy(s, ALLYES);
			PREMA(buf);
			return 1;
		}
	}
	if (++m < MULT)
		return 0;
	return session(c, m, delpap, name);
}

eomatch(c, delpap, allyes, name)
int	c, delpap, allyes;
char	*name;
{
	if (wflg || (xflg && allyes && c > 0))
		return 1;
	if (c == 1) {
		PRMULT(mult[0], delpap);
		return 1;
	}
	session(c, c % MULT, delpap, name);
}

session(c, m, delpap, name)
int	c, m, delpap;
char	*name;
{
	strings	addr;
	buffer	ans;
	int	i;

	if (c > 0 && c < MULT)
		fprintf(stderr, "post: %s: Ambiguous\n", name);
	for (i = 0; i < m; i++)
		fprintf(stderr, "%d %s\n", i, mult[i][WHO]);
	if (xflg || !isatty(0))
		exit(1);

	for (;;) {
		fprintf(stderr, "Which one? [");
		if (m > 0)
			fprintf(stderr, "0-%d, ", m-1);
		if (m == MULT)
			fprintf(stderr, "CR(more), ");
		fprintf(stderr, "o(mit), new address(es), q(uit)] ");

		if (!fgets(ans, sizeof ans, stdin))
			exit(1);
		ans[strlen(ans)-1] = 0;
		if (strlen(ans) <= 1)
			switch (*ans) {
			case 'o':
				return 1;
			case 'q':
				exit(1);
			case 0:
				if (m == MULT)
					return 0;
				else
					continue;
			default:
				i = *ans - '0';
				if (i >= 0 && i < m) {
					PRMULT(mult[i], delpap);
					return 1;
				}
			}

		strvec(ans, addr, " \t\n");
		for (i = 0; addr[i]; i++)
			resolve(addr[i]);
		return 1;
	}
}

merge(qs, qd)
strings	qs, qd;
{
	int	i, j, l, m;
	char	*s;

	for (l = m = 0; qd[l]; l++, m++);
	for (i = 0; qs[i]; i++) {
		for (j = 0; j < l; j++)
			if ((s = strchr(qd[j], '='))
			 && !strncmp(qs[i], qd[j], s-qd[j]+1)) {
				qd[j--] = qd[--l];
				qd[l] = qd[--m];
			}
		qd[m++] = qs[i];
	}
	qd[m] = 0;
}

smerge(s, qd)
char	*s;
strings	qd;
{
	strings	qs;

	if (s) {
		strvec(s, qs, "/:");
		merge(qs, qd);
	}
}

option(a, q)
char	*a;
strings	q;
{
	char	*s = strchr(a, '=');
	int	i, m, rv = 0;

	for (m = 0; q[m]; m++);
	for (i = 0; i < m; i++)
		if (!strncmp(q[i], a, s-a+1)) {
			if (!strncmp(q[i], a, strlen(a)))
				rv++;
			q[i--] = q[--m];
		}
	q[m] = 0;
	return rv;
}

pqowrite(q)
strings	q;
{
	static	tried = 0;
	strings	argv;

	if (!tried++) {
		strvec(mods, argv, " \t\n");
		pq = pq_open(argv);
	}
	return pq ? pq_write(pq, q) : -1;
}

pqerr(msg)
char	*msg;
{
	extern char pq_error[];
	fprintf(stderr, "post: %s: %s\n", msg, pq_error);
	exitval++;
}

prname(s)
char	*s;
{
	for (; *s; s++)
		if (*s == '_') {
			if (s[1] == '_')
				*s++ = ',';
			*s = ' ';
		}
}

skname(s)
char	*s;
{
	char	*ns;

	for (ns = s; *s; s++)
		if (*s != ' ' && *s != '\'')
			*ns++ = *s;
	*ns = 0;
}
