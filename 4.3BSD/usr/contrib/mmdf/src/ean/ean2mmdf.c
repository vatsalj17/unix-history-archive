#include "util.h"
#include "mmdf.h"

/* Copyright University College London - 1984 */

/*
 *  Steve Kille - November 1984
 *
 *  Routine to pass a message from EAN to MMDF
 *
 *  Usage  ean2mmdf -f sender dest dest .....
 */

extern char *supportaddr;
extern char *mmdflogin;
extern LLog *logptr;

char *submitargs = "vmtiean*";
char *signature = "EAN to MMDF Agent";

char *template = "/tmp/ean.XXXXXX";
FILE  *newfp;

int infile;
int  gotone;

main (argc, argv)
int argc;
char *argv[];
{
	char buf[BUFSIZ];
	struct rp_bufstruct thereply;
	int i;
	int len;
	int firstone;
	char *sender;


	mmdf_init (argv[0]);
	siginit ();

	if (lexequ (argv[1], "-f"))
	{
		if (argc < 4)
		{
			ll_log (logptr, LLOGTMP, "Insufficient arguments");
			exit (NOTOK);
		}
		sender = argv[2];
		firstone = 3;
	}
	else
	{
		if (argc < 2)
		{
			ll_log (logptr, LLOGTMP, "Insufficient arguments");
			exit (NOTOK);
		}
		sender = supportaddr;
		firstone = 1;
	}


	ll_log (logptr, LLOGFST, "EAN message from '%s'", sender);

	infile = FALSE;


	if (rp_isbad (mm_init ()) || rp_isbad (mm_sbinit()))
	{
		ll_log (logptr, LLOGTMP, "Failed to start submit");
		byebye (NOTOK);
	}

	if (rp_isbad (mm_winit ((char *)0, submitargs, sender))
	  || rp_isbad (mm_rrply (&thereply, &len)))
	{
		ll_log (logptr, LLOGTMP, "Failed on mm_winit");
		byebye (NOTOK);
	}
	switch (rp_gbval(thereply.rp_val)) {
	case RP_BNO:
	case RP_BTNO:
		ll_log (logptr, LLOGTMP, "mm_winit reply RP_NO");
		byebye (NOTOK);
	}

	for (i = firstone; i < argc; i++)
		do_adr (sender, argv[i]);


	if (!gotone)
	{
		ll_log (logptr, LLOGBST, "No valid addresses");
		byebye (OK);
	}

	if (rp_isbad (mm_waend ()))
	{
		ll_log (logptr, LLOGTMP, "Failed on mm_waend");
		byebye (NOTOK);
	}


			/* If Strat of messsage is in file, get it */
	if (infile)
	{
		while ((i = fread(buf, sizeof(char), sizeof buf, newfp))
			!= NULL)
		{
			if (rp_isbad (mm_wtxt (buf, i)))
			{
				ll_log (logptr, LLOGTMP, "Failed on mm_wtxt");
				byebye (NOTOK);
			}
		}
	}


	while ((i = fread(buf, sizeof(char), sizeof buf, stdin)) != NULL)
	{
		if (rp_isbad (mm_wtxt (buf, i)))
		{
			ll_log (logptr, LLOGTMP, "Failed on mm_wtxt");
			byebye (NOTOK);
		}
	}

	if (rp_isbad (mm_wtend ()))
	{
		ll_log (logptr, LLOGTMP, "Failed on mm_wtend");
		byebye (NOTOK);
	}

	if (rp_isbad (mm_rrply (&thereply, &len)))
	{
		ll_log (logptr, LLOGTMP, "Failed on mm_rrply");
		byebye (NOTOK);
	}


	if (rp_isbad (thereply.rp_val))
	{
		ll_log (logptr, LLOGTMP, "Bad reply value (%s) '%s'",
			rp_valstr (thereply.rp_val), thereply.rp_line);
		byebye (NOTOK);
	}


	if (rp_isbad (mm_sbend ()))
	{
		ll_log (logptr, LLOGTMP, "Failed on mm_sbend");
		byebye (NOTOK);
	}

	ll_log (logptr, LLOGFST, "Mail success  from EAN to MMDF");

	byebye (OK);
}
/* */

				/* process one address                  */
do_adr (sender, adr)
char *sender;
char *adr;
{
	int retval;
	struct rp_bufstruct thereply;
	int len;


	ll_log (logptr, LLOGFST, "EAN doing address  (%s)", adr);


	if (rp_isbad (retval = mm_wadr ("", adr)) ||
		rp_isbad (retval = mm_rrply (&thereply, &len)))
	{
		ll_log (logptr, LLOGTMP, "Failed to write adr '%s' for reason '%s'",
			adr, rp_valstr (retval));
		byebye (NOTOK);
	}
	switch (rp_gval (thereply.rp_val))
	{
		case RP_AOK:
			ll_log (logptr, LLOGFST, "Good address %s",
					adr);
			gotone = TRUE;
			break;
		default:
			ll_log (logptr, LLOGBST, "Bad address '%s' [%s] %s",
				adr, rp_valstr (thereply.rp_val),
				thereply.rp_line);

			if (!infile)
			{
					/* put citatin into file */
				getcite ();
				infile = TRUE;
			}

			ml_1adr (TRUE, FALSE, signature,
				"Failed Mail Transfer", sender);
			ml_txt ("Message not delivered to address: ");
			ml_txt (adr);
			ml_txt ("\n\nThe reason for the failure was:\n    [");
			ml_txt (rp_valstr (thereply.rp_val));
			ml_txt ("] ");
			ml_txt (thereply.rp_line);
			ml_txt ("\n\nYour Message begins as follows: \n\n");
			ml_file (newfp);
			fseek (newfp, 0L, 0);
			ml_txt ("\n.......\n");
			if (ml_end (OK) == OK)
			{
			     ll_log (logptr, LLOGFST, "Sent error to '%s'",
					sender);
			     break;
			}

			     ll_log (logptr, LLOGTMP, "Failed to send warning to '%s'", sender);

					/* Now try support */
			ml_1adr (TRUE, FALSE, signature,
				"Failed Mail Transfer", supportaddr);
			ml_txt ("Message not delivered to address: ");
			ml_txt (sender);
			ml_txt ("\n\n Failure message for transfer to adr: ");
			ml_txt (adr);
			ml_txt ("\n\nThe reason for the failure was:\n  [");
			ml_txt (rp_valstr (thereply.rp_val));
			ml_txt ("] ");
			ml_txt (thereply.rp_line);
			ml_txt ("\n\nMessage begins a follows: \n\n");
			ml_file (newfp);
			fseek (newfp, 0L, 0);
			ml_txt ("\n.......\n");
			if (ml_end (OK) != OK)

			     ll_log (logptr, LLOGTMP, "Failed to send warning to '%s'", supportaddr);
			else
			      ll_log (logptr, LLOGFST, "Sent error report to '%s'",
				supportaddr);
			break;
	}
}


/* */

getcite()
{
	char buf[LINESIZE];
	int i;

	file_init();
	i =0;
	while (fgets (buf, LINESIZE, stdin) != NULL)
	{
		fputs (buf, newfp);
		if (strlen (buf) > 2)
			i++;
		if (i > 300)
			break;
	}
	fclose (newfp);
	if ((newfp =  fopen (template, "r")) == NULL)
	{
		ll_log (logptr, LLOGTMP, "failed to reopen template '%s'",
			template);
		byebye (NOTOK);
	}
}
				/* misc routines  */
file_init ()
{
	int fd;

	mktemp (template);
	if ((fd = creat (template, 0666)) < 0)
	{
		ll_log (logptr, LLOGTMP, "Failed to create temp file %s",
			template);
		exit (NOTOK);
	}
	newfp = fdopen (fd, "w");
}

byebye (retval)
int retval;
{
	if (infile)
	{
		fclose (newfp);
		unlink (template);
	}
	if (retval  == OK)
		mm_end (OK);
	else
		mm_end (NOTOK);
	exit (retval);
}
