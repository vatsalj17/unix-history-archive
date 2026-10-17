/*
 *  Copyright University College London - 1984
 *
 *  Routines used by the EAN channel to access EAN
 *  Developed from Rick Sample's Sendmail mailer
 *
 *  Steve Kille  - November 1984
 */

#include "util/interface.h"
#include "ccitt/interface.h"
#include "mta/rec.h"
#include "mta/p1.h"
#include "mta/P1interf.h"
#include "ua/message.h"
#include "ua/P2.h"
#include "ua/key.h"
#include "ua/or.h"
#include "ua/input.h"
#include "nsg/A822/A822_in.h"
#include <sysexits.h>
#include <stdio.h>

#define NOTOK -1
#define CONNECTION ".local"


EDESC                   P1_desc =
  {
    P1_get,
    P1_getbuff,
    P1_put,
    P1_putbuff,
    0,
    0
  };


static    char *mydomain;
static    char *ean_sender;
static    int reccount;
static    ENODE*    reclist;




en_init (sender, domain)
char *sender;
char *domain;
{
    eanlog ("ean_init ('%s', '%s')", sender, domain);

    if (Local_info () != OK)
      {
	eanlog ("Local info unavailable");
	return (NOTOK);
      }

    ean_sender = cpystr (sender);
    mydomain = cpystr (domain);

    reclist = NULL;

    reccount = 0;

    return (OK);
}


en_end ()
{
	eanlog ("en_end()");

	free (ean_sender);
	free (mydomain);
	return (OK);
}

en_wadr (adr,  rplystr)
char *adr;                              /* address passed down */
char *rplystr;                          /* where to place problem string */
{
    int rc;
    ENODE*    recip;
    ENODE*    orname;
    ENODE*    e;

    eanlog ("en_wadr (%s)", adr);

    if ((orname = Bld_ORname (adr, &rc)) == NULL)
    {
	eanlog ("bad OR Name '%s' [%d]", adr, rc);
	sprintf (rplystr, "Bad OR Name '%s'", adr);
	return (NOTOK);
    }

    recip = NULL;
    Set_add (&recip, orname);
    e = Bit_set ((ENODE*) NULL, P1_RESPONSIBLE);
    Bit_set (e, P1_URQBASIC);
    e->id = CONTEXT + 1L;
    Set_add (&recip, e);
    Set_add (&recip, Bld_int (CONTEXT+0L, ++reccount));
    Seq_add (&reclist, recip);

    return (OK);
}


en_txt (fp, mydomain)
FILE *fp;
char *mydomain;
{
    ENODE*    dsi;
    ENODE*    trace;
    ENODE*    tr_seq;
    ENODE*    e;
    ENODE*    env;
    ENODE*    domain;
    ENODE*    mpdu;
    ENODE*    content;
    FILE*     f;
    P1ID*     p1id;
    long      cont_len;
    TIME      t;
    int       rc;

    eanlog ("en_txt ()");

    reclist->id = CONTEXT + 2L;

    if (P1_open (CONNECTION, NULL, &p1id) != OK)
    {
	eanlog ("P1_open failed");
	return (NOTOK);
    }

    P1_desc.id = (char*) p1id;


    domain = NULL;
    Seq_add (&domain, Bld_cons (P1_COUNTRY, Bld_prim (E_PRINTSTR, "")));
    Seq_add (&domain, Bld_cons (P1_ADMINDOM, Bld_prim (E_PRINTSTR, "")));
    Seq_add (&domain, Bld_prim (E_PRINTSTR, mydomain));;

    domain->id = P1_GLOBALID;

    if ((content = A822_in (&ean_sender, fp, FALSE, mydomain, &cont_len))
		 == NULL)
    {
	eanlog ("Failed to write contents");
	return (NOTOK);
    }

    env = NULL;

    /* create P1.MPDUIdentifier for message envelope */
    e = NULL;
    Seq_add (&e, Ecpy (domain));
    Seq_add (&e, Bld_prim (E_IA5_STRING, TS_get ()));
    e->id = P1_MPDUID;
    Set_add (&env, e);

    if ((e = Bld_recip (ean_sender, &rc)) == NULL
	 || (e = Set_find (e, CONTEXT + 0L)) == NULL
	 || (e = Set_find (e, P1_ORNAME)) == NULL)
      {
	eanlog ("Didn't like the sender");
	return (NOTOK);
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
	eanlog ("Message transfer failure (P1_import)");
	return (NOTOK);
    }

    Ewrite (&P1_desc, content);

    if (P1_end (p1id, TRUE) != OK)
    {
	eanlog ("P1_end failure");
	return (NOTOK);
    }

    Efree (&content);
    P1_close (&p1id);

    return (OK);
}
