/*
**		S _ A R G I N I T
**
**  This function parses and uses the information found on the command
**  line.
**
**
**	R E V I S I O N  H I S T O R Y
**
**	03/31/83  GWH	Split the SEND program into component parts
**
*/

#include "./s.h"
#include "./s_externs.h"

char *UsageMessage = 
"Usage:  send addrs -a hdr:val -{bct} addrs -d{ev} -f file -h host -s subj -n\n";

arginit (argc, argv)              /* parse and use the argument list    */
int     argc;
char   *argv[];
{
    char *curfld;
    char *cp;
    register short curarg;

    strcpy (host, locname);       /* setup default host reference       */

/*  if an argument is not a switch, assume that it is an address.
 *  several of the switches merely change which header addresses are
 *  to be added to.  after an argument is processed, the pointer to
 *  it is nulled, so that it will no longer be visible by a system-
 *  status command ("ss" on the UDel machine).
 */

    for (curfld = to, curarg = 1; curarg < argc; )
    {                             /* regular scan of arg list           */
				  /* default to adding to To field      */
	if (argv[curarg][0] != '-')
	    addrcat (curfld, argv[curarg], host);
				  /* add it to current field            */
	else {                    /* a switch                           */ 
	    switch (argv[curarg][1])
	    {
	    	case 'a':
	    	    if (++curarg >= argc)
	    		break;
	    	    if (cp = index(argv[curarg], ':'))
	    		*cp++ = '\0';
		    addheader(argv[curarg], cp);
		    break;

		case 'b':         /* add to end of bcc field            */
		    curfld = bcc;
		    break;

		case 'c':         /* add to end of cc field             */
		    ccflag = FALSE;
		    curfld = cc;
		    break;

		case 'd':	  /* alter directedit option		*/
		    switch (argv[curarg][2]) {
			case '\0':
			    dflag = 0;
			    break;
			case 'e':
			    dflag = 2;
			    break;
			case 'v':
			    dflag = 1;
			    break;
			default:
				printf("bad direct edit option\n");
			}
			break;

		case 'f':         /* add file to end of message body    */
	    	    if (++curarg >= argc)
	    		break;
		    strcpy (inclfile, argv[curarg]);
		    break;        /* file is named in next argument     */

		case 'h':         /* default hostname for addresses     */
	    	    if (++curarg >= argc)
	    		break;
		    strcpy (host, argv[curarg]);
		    break;        /* host is named in next argument     */

	    	case 'n':
	    	    noinputflag++;
	    	    break;

		case 's':         /* add to end of Subject field        */
		    subjflag = FALSE;
	    	    if (++curarg >= argc)
	    		break;
		    strncat (subject, argv[curarg],
				(S_BSIZE - strlen(subject) - 2));
		    break;        /* Subject is contained in next arg   */

		case 't':         /* add to end of To field             */
		    toflag = FALSE;
		    curfld = to;
		    break;

		default:
		    snd_abort ("unknown flag in '%s'\n%s", argv[curarg],
				UsageMessage);
	    }
	}
	if (argv[curarg])
		argv[curarg++][0] = '\0';
    }
    if (to[0] != '\0')            /* at least one To was specified      */
	toflag = FALSE;           /* so don't prompt for it             */
}
