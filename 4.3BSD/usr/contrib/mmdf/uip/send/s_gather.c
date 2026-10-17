/* name:
	gather

function:
	read a single line of console input into the user specified
	buffer.

algorithm:
	Read a character:
	If it is an unescaped new line, null terminate the buffer and return.
	Otherwise, stash the character into the buffer.
	If the length of the buffer has been reached, echo a new line and
	return.

parameters:
	*char   pointer to user buffer of length S_BSIZE

returns:
	nothing

globals:
	S_BSIZE

calls:
	getchar

called by:
	main
	send

*/
/*
**	R E V I S I O N  H I S T O R Y
**
**	03/31/83  GWH	Split the SEND program into component parts
**
*/

#include "./s.h"
#include "./s_externs.h"

gather (sp, max)
char   *sp;
register int max;
{
    register int    c;
    register char   *p;

    fflush (stdout);

    for (p = sp; max-- > 0; *p++ = c)
    {
	if ((c = getchar ()) == EOF)
	    if (ferror (stdin) || feof (stdin))
	    {                   /* something really happened            */
		*p = '\0';
		return (-1);
	    }

	c = toascii (c);        /* make sure it's valid                 */

	if (c == '\n')          /* normal end on newline                */
	    break;
    }

    *p = '\0';
    return (p - sp);              /* return length                      */
}
