#include <stdio.h>
#include <string.h>

#define USAGE	"usage: pq [-l] [-a attr] [-m mods] [-o fmt] [queries...]\n"

typedef char	*strings[512];
typedef char	buffer[BUFSIZ];

extern char	emp_def[], emp_fmt[], loc_def[], loc_fmt[];
extern char	*pq_open();

main(argc, argv)
int	argc;
char	*argv[];
{
	extern char	*optarg;
	extern int	optind;
	char	*attr = emp_def, *mods = "", *ofmt = emp_fmt;
	strings	vars, vals, defs;
	buffer	prog, out;
	int	rv = 0;
	int	c, v, r;
	char	*pq;

	while ((c = getopt(argc, argv, "a:m:o:l")) != EOF)
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
		case 'l':
			attr = loc_def;
	 		ofmt = loc_fmt;
			break;
		default:
			fputs(USAGE, stderr);
			return 1;
		}

	strvec(mods, vals, " \t\n");
	if ((pq = pq_open(vals)) == 0)
		return pqerr();

	fmtcomp(prog, ofmt, vars);
	for (v = 0; vars[v]; v++);
	strvec(attr, defs, "/:");

	if (optind == argc)
		argv[argc++] = "";
	while (optind < argc) {
		strvec(argv[optind++], vars + v, "/:");
		if (!dodef(vars + v, defs))
			continue;
		c = 0;
		if ((r = pq_write(pq, vars)) != -1)
			while ((r = pq_read(pq, vals)) > 0) {
				fmtexec(out, prog, vals);
				fputs(out, stdout);
				fflush(stdout);
				c++;
			}
		if (r == -1)
			rv += pqerr();
		if (c == 0)
			rv++;
	}
	if (pq_close(pq) == -1)
		rv += pqerr();
	return rv;
}

dodef(vars, defs)
strings	vars, defs;
{
	static buffer	buf;
	register char	*bp = buf;
	int	i, m, n;

	for (m = 0; defs[m]; m++);
	for (i = 0; vars[i]; i++)
		if (!strchr(vars[i], '='))
			if (i < m) {
				n = sprintf(bp, "%s=%s", defs[i], vars[i]);
				vars[i] = bp;
				bp += n + 1;
			} else {
				fprintf(stderr, "pq: %s: No attribute\n",
							vars[i]);
				return 0;
			}
	return 1;
}

pqerr()
{
	extern char pq_error[];
	fprintf(stderr, "pq: %s\n", pq_error);
	return 1;
}
