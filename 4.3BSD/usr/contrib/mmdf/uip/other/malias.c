#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include "dm.h"
#include <pwd.h>

extern char *index();
extern char *rindex();
extern char *multcat();
extern struct passwd *getpwmid();

main (argc, argv)
int argc;
char *argv[];
{
	register int     i;
	register char	*key;

	mmdf_init (argv[0]);
	for (i=1; i < argc; i++) {
		key = multcat (argv[i], "-outbound", (char *)0);
		if (do_arg(key) == 0)
			if (do_arg(argv[i]) == 0)
				printf ("no aliases for '%s'\n", argv[i]);
		free (key);
	}
	exit(0);
}

do_arg(arg)
register char *arg;
{
	char    buf[LINESIZE];
	char	alstr[LINESIZE];
	register char    *p, *q;
	FILE    *fp;
	int	ret;
	int	bypassable = 0;

	if ((ret = aliasfetch(TRUE, arg, buf, 1)) != OK) {
		if (ret == MAYBE) {
			printf("%s: Nameserver timeout\n");
			return(0);
		}
		if ((ret = aliasfetch(TRUE, arg, buf, 0)) != OK) {
			if (ret == MAYBE) {
				printf("%s: Nameserver timeout\n");
				return(0);
			}
			if (getpwmid (arg) != NULL) {
				printf ("%s is a user\n", arg);
				return(1);
			}
			return(0);
		}
		bypassable++;
	}

	strcpy(alstr, arg);
	if (bypassable)
		strcat(alstr, " (bypassable)");
	if( buf[0] == '~' ) {
		printf("%s is a non-recursive alias: %s\n",
			alstr, buf);
		return(1);
	}
	if (index (buf, ',') != 0 && strindex (":include:", buf) < 0
		&& index (buf, '<') == 0 && index (buf, '|') == 0)
	{
		printf ("%s is list of aliases: %s\n", alstr, buf);
		return(1);
	}
	if (((p = index (buf, '@')) == 0) && (index (buf, '/') == 0))
	{
		printf ("%s has alias %s\n", alstr, buf);
		return(1);
	}

	/* Assume if multiple entries, that     */
	/* only the first one is used.		*/
	if ((q= index (buf, ',')) != 0)
		*q = '\0';
	if (p) {
		*p++ = '\0';
		if (ch_h2chan (p, 1)  != (Chan *) OK) {
			printf ("%s is a list expanded on machine %s\n",
					alstr, p);
			return(1);
		}
	} else
		p = buf;
	if ((p = index (buf, '/')) == 0) {
		printf ("bad format for alias '%s': value '%s'\n",
				alstr, buf);
		return(1);
	}
	if ((q = index (buf, '|')) != 0) {
		*q++ = '\0';
		printf ("%s is a pipe alias which runs %s as user %s\n",
				alstr, q, buf[0] ? buf : "root");
		return(1);
	}
	if (index (buf, '<') == 0 && strindex (":include:", buf) < 0)
	{
		*p++ = '\0';
		printf ("mail for %s is filed in %s as user %s\n",
				alstr, p, buf[0] ? buf : "root");
		return(1);
	}
	printf ("%s expands to contents of list file %s:\n", 
		alstr, p);
	if ((fp = fopen (p, "r")) == NULL) {
		printf ("unable to open file %s\n", p);
		return(1);
	}
	while (fgets (buf, LINESIZE, fp) != NULL)
		printf ("%s", buf);
	fclose (fp);
	return(1);
}
