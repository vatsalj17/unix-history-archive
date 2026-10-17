/*   Routine to test nn_mail.o                                  */
/*   Note that this does not use any MMDF libraries etc         */
/*   This is to emulate linking into the NIFTP                  */
/*                                                              */
/*      Steve Kille             August 1982                     */

int ipcs_print = 7;

main ()
{
    char        fname[50];
    char        queue[50];
    char        TSname[50];
    int         maxtries;
    char        *result;
    int         tmp;

    printf ( "\nInput filename: " );
    scanf ( "%s", fname);
    printf ( "Input queue: " );
    scanf ( "%s", queue );
    printf ( "TSname: " );
    scanf ("%s", TSname );
    printf ( "maxtries: " );
    scanf ( "%d", &maxtries );

    printf ( "\n\nni_mail (%s, %s, %s, %d, , )\n", fname, queue, TSname,
		maxtries, result);

    tmp = ni_mail (fname, queue, TSname, maxtries, &result);

    if (tmp == 0)
	printf ("\nResult is good");
    else
	printf ("\nResult is bad");

    printf ("\n\nResult string is: '%s'\n", result);

}

