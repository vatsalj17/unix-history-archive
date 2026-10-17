#
/*  A set of routines in the style of getpw() to read the ttys
 *  file and return the tty info.
 *
 *  Steve Manion
 *
*/

#include	<stdio.h>
#include        "gettys.h"

#define		ETCTTYS		"/etc/ttys"

/*  the channel to read the data from  */
static FILE *input = NULL;

/*  the line number last read from the file  */
static int linenum = 0;

getttyent(userdata)
  struct ttys *userdata;
    {
    char buff[128];

    /*  if the input file is not yet open, do it  */
    if (input == NULL)
	if (openinput(ETCTTYS) < 0)
	    return(BADDATA);

    /*  read the next line from the input  */
    if (getline(buff) == EOF)
	{
	rewind(input);
	linenum = 0;
	return(EOF);
    }

    /*  transfer the data into the user struct  */
    if (process(buff, userdata) < 0)
	return(BADDATA);

    /*  return the line number we are on  */
    return(linenum);
}





openinput(file)
  char *file;
    {

    input = fopen(file, "r");
    if (input == NULL)
	{
	printf("File '%s': ", file);
	fflush(stdout);
	perror("");
	fflush(stderr);
	return(-1);
    }

    return(0);
}





static int getline(buff)
  char *buff;
    {
    register int c;

    for(;;)
	{
	c = getc(input);

	/*  look for the true end of the file  */
	if( (c == EOF) && (feof(input) != 0)   )
	{
	    *buff = '\0';
	    return(EOF);
	}

	/*  transfer character to the buffer and check for the
	 *  end of the line.
	 */
	*buff++ = c;
	if (c == '\n')
	{
	    *buff = '\0';
	    linenum++;
	    return(0);
	}
    }
}





static int process(buff, userdata)
  char *buff;
  struct ttys *userdata;
    {
    register char *cp;

    /*  check the first char of the line.  It should be 0 or 1 as
     *  the line is invalid or valid.
     */
    switch(*buff++)
	{
	case '0':
	    userdata->t_valid = 0;
	    break;

	case '1':
	    userdata->t_valid = 1;
	    break;

	default:
	    fflush(stdout);
	    fprintf(stderr, "Invalid tty file - notify system manager.\n");
	    fflush(stderr);
	    return(-1);
    }

    /*  transfer the control code directly  */
    userdata->t_code = *buff++;

    /*  transfer the tty name to the buffer  */
    for(cp = userdata->t_name;  *buff != '\n';  )
	*cp++ = *buff++;
    *cp = '\0';

    return(0);
}




getttynam(userdata, name)
  struct ttys *userdata;
  char *name;
    {
    register int curline, result;

    /*  the strategy of this routine is to cycle through the
     *  tty file, starting from the current position, until
     *  the tty 'name' is located.  Note that the getttyent()
     *  routine will automaticly rewind the input when appropriate.
     *  Note also that the call to it ultimately updates the
     *  variable 'linenum'.
     */

    curline = linenum;
    do
	{
	result = getttyent(userdata);
	if(result == EOF)
	    continue;
	if(result == BADDATA)
	    break;
	if(strcmp(name, userdata->t_name) == 0)
	    return(result);
    }
    while (curline != linenum);

    return(BADDATA);
}
