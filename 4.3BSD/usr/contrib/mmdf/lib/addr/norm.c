#include "util.h"
#include "mmdf.h"
#include "ap.h"
#include "ch.h"

extern LLog *logptr;

extern int ap_outtype;

main (argc, argv)
int argc;
char *argv[];
{
    char adr [LINESIZE];
    char *p;

    AP_ptr *ap;
    AP_ptr *group;
    AP_ptr *name;
    AP_ptr *route;
    AP_ptr *domain;
    AP_ptr *local;
    AP_ptr *norm;
    int i;

    Chan *thechan;
    char *dfldomain = (char *) 0;
    char *dflhost = (char *) 0;


    mmdf_init ("NORM");
    logptr -> ll_level = LLOGFTR;
    siginit ();

    if ((thechan = ch_nm2struct ("foo")) == (Chan *) NOTOK)
    {
	printf ("Chan foo not found\n");
	thechan = (Chan *) 0;
    }
    else
    {
	printf ("channel '%s' used\n", thechan -> ch_show);
	dfldomain = thechan -> ch_ldomain;
	dflhost = thechan -> ch_lname;
    }
    fflush (stdout);


    for (i=1; i < argc; i++)
    {
	printf ("received: '%s'\n", argv[i]);
	fflush (stdout);

	ap = ap_s2tree (argv[i]);
	if (ap == (AP_ptr) NOTOK)
	{
		printf ("Parse failed\n\n");
		continue;
	}
	ap_outtype = AP_822;
	ap_t2s (ap, &p);

	printf ("tree in full is: '%s'\n", p);
	fflush (stdout);
	free (p);

	norm = ap_normalize (dflhost, dfldomain, ap, thechan);
	ap_t2s (norm, &p);
	printf ("normalised tree (822, little) is: '%s' \n", p);
	fflush (stdout);
	free (p);

	ap_t2parts (ap, &group, &name, &local, &domain, &route);
	p = ap_p2s ((AP_ptr) 0, (AP_ptr) 0, local, domain, route);
	printf ("real  bits: '%s'\n", p);
	fflush (stdout);
	free (p);

	ap_outtype = AP_733;
	ap_t2s (norm, &p);
	printf ("normalised tree (jnt, little) is: '%s' \n", p);
	fflush (stdout);
	free (p);
	ap_outtype = (AP_822  | AP_BIG);
	ap_t2s (norm, &p);
	printf ("normalised tree (822, big) is: '%s' \n", p);
	free (p);
	fflush (stdout);
	if (thechan == NULL)
	    continue;
	ap_outtype = thechan -> ch_apout;
	ap_t2s (norm, &p);
	printf ("normalised tree (from channel type) is: '%s' \n", p);
	free (p);

	printf ("\n\n");
	fflush (stdout);

    }
}

err_abrt (code, fmt, b, c, d)
char code,
	*fmt, *b, *c, *d;
{
printf ("norm aborting (%s)\n", rp_valstr (code));
printf (fmt, b, c, d);
fflush (stdout);
exit (0);
}

