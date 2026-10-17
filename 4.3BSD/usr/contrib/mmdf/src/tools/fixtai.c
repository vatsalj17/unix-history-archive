/*****************************************************************************
* Program to suck up the 'mchan' lines from the tailoring file and produce   *
* a suitable block of structure initializers for inclusion in conf_chan.c    *
*                                                                            *
* This program relies on having an existing installation with all channels   *
* defined in a tailoring file (whose name is supplied as the first argument).*
* Pre-compiled channels and tables are ignored.                              *
*                                                                            *
* This program uses mmdf_init to read in the (alternate) tailor file and     *
* then passes the channel and table structure arrays to an internal          *
* structure-initializer-printer.  This output is then #included into         *
* conf_chan.c and forms the basis for the next installation of MMDF.         *
*                                                                            *
* Fri Apr 13 13:07:49 EST 1984, bpc (BBN)                                    *
* Tue Nov 27 11:45:21 EST 1984, dbl (BBN)                                    *
/*****************************************************************************
* Added processing of 'mtbl' lines, also.                                    *
* Mon Apr 23 09:59:59 EST 1984, bpc                                          *
*****************************************************************************/

#include "util.h"
#include "cmd.h"
#include "ch.h"
#include <sys/stat.h>
#include "ap.h"
#include "conf.h"

#define MAXARG 50

extern int errno;
extern char * malloc();
extern int tb_numtables;
extern int ch_numchans;
extern Table **tb_list;
extern Chan **ch_tbsrch;
extern char *mmtailor;

/**/
main (argc, argv)
    char * argv[];
{
    if (argc != 2)
    {
	fprintf (stderr, "usage: %s tailoringfile_name\n", argv[0]);
	exit (1);
    }

    (void) strcpy(mmtailor, argv[1]);  /* use alternate tailor file */
    tb_numtables = 0;           /* zero the pre-compiled table array */
    tb_list[0] = (Table *) 0;
    ch_numchans = 0;            /* zero the pre-compiled channel array */
    ch_tbsrch[0] = (Chan *) 0;

    mmdf_init(argv[0]);
    dump_all();
    exit (0);
}

/*****************************************************************************
*                            | d u m p _ a l l |                             *
*                             *****************                              *
* this routine is the real point of all this: it is the routine to           *
* pass over the channels and tables we've defined and appropriately          *
* initialize them                                                            *
*****************************************************************************/
dump_all ()
{
    dump_tables();
    printf ("/*\f*/\n");
    dump_channels();
}

/*****************************************************************************
*                         | d u m p _ t a b l e s |                          *
*                          ***********************                           *
* Print out all of the pre-initialized tables.                               *
*****************************************************************************/
dump_tables()
{
    register i;

    printf ("LOCVAR Table _tblist[]");
    printf ("\t/* Data blocks for the pre-initialized tables */\n");
    printf ("\t=\n");
    printf ("{\n");
    for (i = 0; i < tb_numtables; i++)
      set_tb (tb_list[i]);
    printf ("};\n");
    printf ("LOCVAR Table * tblist[NUMTABLES+1]\t/* all known tables */\n");
    printf ("\t=\n");
    printf ("{\n");
    for (i = 0; i < tb_numtables; i++)
      printf ("    &_tblist[%d],\n", i);
    printf ("    (Table *) 0\n};\n");
    printf ("int\ttb_numtables = %d;\t/* Number of preinitialized tables */\n",
		tb_numtables);
}

/*****************************************************************************
*                       | d u m p _ c h a n n e l s |                        *
*                        ***************************                         *
* Now print out all the channel definitions.  The 'table' pointer for        *
* each channel will simply be the same pointer to the _tblist array          *
* that tblist uses                                                           *
*****************************************************************************/
dump_channels()
{
    register i;

    printf ("LOCVAR Chan _chsrch[]");
    printf ("\t/* Data blocks for the pre-initialized channels */\n");
    printf ("\t=\n");
    printf ("{\n");
    for (i = 0; i < ch_numchans; i += 1)
      set_ch (ch_tbsrch[i]);
    printf ("};\n");
    printf ("LOCVAR Chan * chsrch[NUMCHANS+1]\t/* order chan tables searched */\n");
    printf ("\t=\n");
    printf ("{\n");
    for (i = 0; i < ch_numchans; i += 1)
      printf ("    &_chsrch[%d],\n", i);
    printf ("    (Chan *) 0\n};\n");
    printf ("int\tch_numchans = %d;\t/* Number of preinitialized channels */\n",
		ch_numchans);
}
/**/
/*****************************************************************************
*                              | s e t _ t b |                               *
*                               *************                                *
* Set the initialization for one entry of a table                            *
*****************************************************************************/
set_tb (tptr)
    register Table * tptr;
{
    printf ("    { ");
    str_print (tptr->tb_name);
    printf (", ");
    str_print (tptr->tb_show);
    printf (", ");
    str_print (tptr->tb_file);
    printf (", (FILE *)0, 0L, 0%o },\n", (int)tptr->tb_flags);
}
/*****************************************************************************
*                         | t a b l e _ p r i n t |                          *
*                          ***********************                           *
* Given a table pointer, print it out as an pointer into the array of table  *
* structures.                                                                *
*****************************************************************************/
table_print (tptr)
    Table * tptr;
{
    register i;

    if (tptr == (Table *) 0)
	printf ("(Table *)0");
    else
    {
        for (i = 0; i < tb_numtables; i += 1)
	    if (tb_list[i] == tptr)
	    {
		printf ("&_tblist[%d]", i);
	        return;
	    }
        fprintf (stderr, "Can't locate table %o\n", tptr);
	(void) fflush (stdout);
        abort ();
    }
}
/*****************************************************************************
*                           | s t r _ p r i n t |                            *
*                            *******************                             *
* Print out a string if non-null, 0 otherwise                                *
*****************************************************************************/
str_print (ptr)
    char * ptr;
{
    if (ptr == (char *)0)
	printf ("(char *)0");
    else
	printf ("\"%s\"", ptr);
}
/**/
/*****************************************************************************
*                              | s e t _ c h |                               *
*                               *************                                *
* Initialize one channel                                                     *
*****************************************************************************/
set_ch (cptr)
    register Chan * cptr;
{
    printf ("    { ");
    str_print (cptr->ch_name);
    printf (", ");
    str_print (cptr->ch_show);
    printf (", ");
    table_print (cptr->ch_table);
    printf (", ");
    str_print (cptr->ch_queue);
    printf (", ");
    printf ("\n\t");
    printf ("0%o, ", cptr->ch_access);
    str_print (cptr->ch_ppath);
    printf (", ");
    str_print (cptr->ch_lname);
    printf (", ");
    str_print (cptr->ch_ldomain);
    printf (", ");
    str_print (cptr->ch_host);
    printf (", ");
    printf ("\n\t");
    str_print (cptr->ch_login);
    printf (", %d, ", (int)cptr->ch_poltime);
    str_print (cptr->ch_trans);
    printf (", ");
    str_print (cptr->ch_script);
    printf (", ");
    printf ("\n\t");
    table_print (cptr->ch_outsource);
    printf (", ");
    table_print (cptr->ch_insource);
    printf (", ");
    table_print (cptr->ch_outdest);
    printf (", ");
    table_print (cptr->ch_indest);
    printf (", ");
    table_print (cptr->ch_known);
    printf (", ");
    printf ("\n\t");
    str_print (cptr->ch_confstr);
    printf (", ");
    printf ("0%o, ", cptr->ch_apout);
    printf ("0%o, ", (int)cptr->ch_auth);
    printf ("(Cache *)0,"); /* ch_dead */
    printf ("\n\t");
    printf ("0%o, ", cptr->ch_ttl);
    str_print (cptr->ch_logfile);
    printf (", ");
    printf ("0%o", cptr->ch_loglevel);
    printf (" },\n");
}
