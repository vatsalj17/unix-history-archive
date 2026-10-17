/*
 *		C H E C K A D D R . C
 *
 *	Checks to determine if an address(s) is valid to mail to.
 */

#include "util.h"
#include "mmdf.h"

extern	char *rindex();
extern  char *supportaddr;

main (argc, argv)
int	argc;
char	**argv;
{
	struct rp_bufstruct thereply;
	char	linebuf[LINESIZE];
	char	*cp;
	int	i;
	char	*subargs = "vm";

	mmdf_init( argv[0] );

	if (argc > 1 && strcmp (argv[1], "-w") == 0) {
		subargs = "vmW";
		argc--;
		argv++;
	}

	if (rp_isbad(mm_init()) || rp_isbad(mm_sbinit()) ||
	    rp_isbad(mm_winit ((char *)0, subargs, supportaddr)) ||
	    rp_isbad(mm_rrply( &thereply, &i )) ||
	    rp_isbad(thereply.rp_val)) {
		printf("Cannot initialize mail system.\n");
		exit(9);
	}
		
	if (argc > 1) {
		for (i = 1; i < argc; i++)
			verify( argv[i] );
	} else {
		while (fgets (linebuf, LINESIZE, stdin)) {
			if (cp = rindex(linebuf, '\n'))
				*cp-- = 0;
			verify( linebuf );
		}
	}
	mm_end (NOTOK);
	exit (0);
}

verify (addr)
char	*addr;
{
	struct rp_bufstruct thereply;
	int	len;

	printf("%s: ", addr);
	(void) fflush(stdout);

	if (rp_isbad (mm_wadr ((char *)0, addr))) {
		if( rp_isbad( mm_rrply( &thereply, &len ))) {
			printf ("Mail system problem\n");
			mm_end (NOTOK);
			exit( 8 );
		} else
			printf ("%s\n", thereply.rp_line);
	} else {
		if( rp_isbad( mm_rrply( &thereply, &len ))) {
			printf ("Mail system problem\n");
			mm_end (NOTOK);
			exit (7);
		} else if( rp_isbad( thereply.rp_val ))
			printf ("%s\n", thereply.rp_line);
		else
			printf ("OK\n");
	}
}
