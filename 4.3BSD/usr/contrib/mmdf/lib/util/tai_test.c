#include "util.h"

main ()
{
	static char initfile[] = "init.tai";
	char *taiargs[100];
	int nargs;
	register int ind;

	if (tai_init (initfile) < OK)
	{
		perror ("init");
		exit ();
	}
	while ((nargs = tai_get (100, taiargs)) > 0)
	{
		for (ind = 0; ind < nargs; ind++)
			printf ("'%s' ", taiargs[ind]);
		(void) putchar ('\n');
	}
	if (nargs < 0)
		perror ("tai_get");
	tai_end ();
}
