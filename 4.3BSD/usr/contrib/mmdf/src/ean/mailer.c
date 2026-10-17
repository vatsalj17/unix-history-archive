/*----------------------------------------------------------*/
/*     EAN Mail System User Agent -- UUCP/EAN gateway mailer*/
/*                                                          */
/*                   September, 1983                        */
/*                                                          */
/*          Author: Rick Sample                             */
/*                  University of British Columbia          */
/*                                                          */
/*      This is a utility program to interface to UUCP mail */
/*  delivery, and re-route it into the EAN mail system. It  */
/*  reads the message (in ARPA RFC 822 format) from the     */
/*  standard input.                                         */
/*                                                          */
/*----------------------------------------------------------*/

#include "../util/interface.h"
#include "../util/log.h"
#include "../ccitt/interface.h"
#include "../mta/rec.h"
#include "../mta/p1.h"
#include "../mta/P1interf.h"
#include "../ua/message.h"
#include "../ua/P2.h"
#include "../ua/key.h"
#include "../ua/or.h"
#include "../ua/input.h"
#include "../nsg/A822/A822_in.h"
#include <sysexits.h>
#include <stdio.h>

#define	        DOM_STR	        "uucp"
#define	        CONNECTION      ".local"

EDESC			P1_desc =
  {
    P1_get,
    P1_getbuff,
    P1_put,
    P1_putbuff,
    0,
    0
  };


main (argc, argv)
int    argc;
char  *argv[];
  {
    LOG_DESC	log;
    char	str[100];
    char*     recip_name;
    char      userid[20];
    char      hostid[20];
    char*     sender;
    char*     dflt;
    int       i,
	      rc,
	      len,
	      count,
	      ignore;
    ENODE*    dsi;
    ENODE*    trace;
    ENODE*    tr_seq;
    ENODE*    e;
    ENODE*    env;
    ENODE*    domain;
    ENODE*    recip;
    ENODE*    reclist;
    ENODE*    orname;
    ENODE*    mpdu;
    ENODE*    content;
    FILE*     f;
    P1ID*     p1id;
    long      cont_len;
    TIME      t;
    
    if (Local_info () != OK) 
      {
	fputs ("Local info unavailable.\n", stderr);
	exit (EX_SOFTWARE);
      }

    reclist = NULL;

    count = 0;
    
    recip_name = cpystr ("");
    len = 1;

    for (i = 0; i < argc; ++i)
	if (argv[i][0] == '-')
	    switch (argv[i][1])
	      {
		case 'd': 
		    if (i++ == argc)
	                exit (EX_DATAERR);
		    while (i < argc && argv[i][0] != '-') 
		      {
			if ((orname = Bld_ORname (argv[i], &rc)) == NULL) 
			  {
			    fprintf (stderr, "Bad address argument: %s.\n",
				    argv[i]);
			    exit (EX_DATAERR);
			  }

		        recip = NULL;
			Set_add (&recip, orname);
			e = Bit_set ((ENODE*) NULL, P1_RESPONSIBLE);
			Bit_set (e, P1_URQBASIC);
			e->id = CONTEXT + 1L;
			Set_add (&recip, e);
			Set_add (&recip, Bld_int (CONTEXT+0L, ++count));
			Seq_add (&reclist, recip);
    
			catstr (&recip_name, &len, argv[i++]);
			catstr (&recip_name, &len, " ");
		      }
		    break;
	      }

    if (reclist == NULL) 
      {
	fputs ("no recipient specified.\n", stderr);
	exit (1);
      }

    reclist->id = CONTEXT + 2L;

    if (P1_open (CONNECTION, NULL, &p1id) != OK) 
      {
	fputs ("Message Transfer Error.\n", stderr);
	exit (EX_UNAVAILABLE);
      }
      
    P1_desc.id = (char*) p1id;

    Userid (userid);
    gethostname (hostid, 20);
    
    sender = cpystr (userid);
    len = strlen (sender) + 1;
    catstr (&sender, &len, "@");
    catstr (&sender, &len, hostid);
    catstr (&sender, &len, ".");
    catstr (&sender, &len, DOM_STR);

    domain = NULL;
    Seq_add (&domain, Bld_cons (P1_COUNTRY, Bld_prim (E_PRINTSTR, "")));
    Seq_add (&domain, Bld_cons (P1_ADMINDOM, Bld_prim (E_PRINTSTR, "")));
    Seq_add (&domain, Bld_prim (E_PRINTSTR, DOM_STR));;
    
    domain->id = P1_GLOBALID;

    ignore = (	   strcmp (userid, "ean")     != 0
		&& strcmp (userid, "root")    != 0
		&& strcmp (userid, "mmdf")    != 0
		&& strcmp (userid, "uucp")    != 0
		&& strcmp (userid, "daemon")  != 0
		&& strcmp (userid, "network") != 0);
    
    if ((content = A822_in (&sender, stdin, ignore, DOM_STR, &cont_len))
		 == NULL)
      {
	fprintf (stderr, "Invalid or empty message.\n");
	exit (EX_DATAERR);
      }
    
    env = NULL;

    /* create P1.MPDUIdentifier for message envelope */
    e = NULL;
    Seq_add (&e, Ecpy (domain));
    Seq_add (&e, Bld_prim (E_IA5_STRING, TS_get ()));
    e->id = P1_MPDUID;
    Set_add (&env, e);

    if ((e = Bld_recip (sender, &rc)) == NULL
         || (e = Set_find (e, CONTEXT + 0L)) == NULL
	 || (e = Set_find (e, P1_ORNAME)) == NULL)
      {
	fputs ("invalid originator.\n", stderr);
	exit (EX_DATAERR);
      }

    
    Set_add (&env, e);
    
    Set_add (&env, Bld_int (P1_CONTTYPE, 2L));
    
    if ((e = Find_heading (content, P2_MESSAGEID)) != NULL) 
	Set_add (&env, Bld_prim (P1_UACONTID, Unb_MID (e)));
    
    Set_add (&env, reclist);
    
    trace = NULL;
    Seq_add (&trace, Ecpy (domain));
    
    dsi = NULL;

    e = Bld_time (TR_get (&t));
    e->id = CONTEXT + 0L;
    Set_add (&dsi, e);
    
    Set_add (&dsi, Bld_int (CONTEXT+2L, 0L));
    
    Seq_add (&trace, dsi);
  
    tr_seq = NULL;
    Seq_add (&tr_seq, trace);
    tr_seq->id = P1_TRACEINFO;

    Set_add (&env, tr_seq);

    mpdu = NULL;
    Seq_add (&mpdu, env);

    mpdu->id = P1_USERMPDU;

    if (P1_import (p1id, mpdu) != OK) 
      {
	fputs ("Message Transfer Failure.\n", stderr);
	exit (EX_TEMPFAIL);
      }
    
    Ewrite (&P1_desc, content);

    if (P1_end (p1id, TRUE) != OK) 
      {
	fputs ("Message Transfer Failed.\n", stderr);
	exit (EX_OSERR);
      }
      
    Efree (&content);
    P1_close (&p1id);

    Log_init( LOG_SENDMAIL, &log );
    sprintf( Log_str, "in : %d %0.25s %0.25s", cont_len, sender, recip_name );
    Log_enter( &log, LOG_NORM, NULL );
    Log_term( &log );

    exit (EX_OK);
  }
