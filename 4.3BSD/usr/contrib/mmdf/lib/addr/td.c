#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include "dm.h"

extern LLog *logptr;

main ()
{
    char adr [LINESIZE];
    char result [LINESIZE];
    Dmn_route rbuf;
    Dmn_route *route = &rbuf;
    Domain dbuf;
    Domain *dmn = &dbuf;
    Chan *tchan;

    mmdf_init ("TEST-DM");
    siginit ();
    logptr -> ll_level = LLOGFTR;

    while (TRUE)
    {
	int i;

	printf ("\ngive test string (q to quit):");
	scanf ("%s", adr);
	if (lexequ (adr, "q"))
	{
		printf ("quitting\n");
		exit(0);
	}
	printf ("received: '%s'\n", adr);


	printf ("dm_v2route (%s) gives: ",adr);
	dmn = dm_v2route (adr, result, route);
	if (dmn  == (Domain *) NOTOK)
	{
		printf ("Error \n\n\n");
	}
	else
	{
		printf ("%s \n", dmn->dm_show);
		printf ("domain in full is '%s'\n", result);
		printf ("Route: '%s'\n", route->dm_buf);
		for (i=0; i <= route->dm_argc; i++)
		  printf ("    Component (%d): '%s'\n", i, route->dm_argv[i]);
	}
	fflush (stdout);


	for (i=1;;i++)
	{
	    if ((tchan = ch_h2chan (adr, i)) == (Chan *) NOTOK)
	    {
		printf ("ch_h2chan returns NOTOK\n");
		break;
	    }
	    else
	    {
		if (tchan == (Chan *) OK)
			printf ("local reference (%d)\n", i);
		else
		    printf ("ch_h2chan (%d) gives channel '%s'\n", i, tchan -> ch_show);
	    }
	}


	printf ("\n\n");
   }
}

err_abrt ()
{
printf ("aborting\n");
}
