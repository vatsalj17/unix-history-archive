#include "plot.h"








number(ptr)
char	*ptr;
{

/*	This routine checks for the validity of a number.
**	It can do as little or as much checking as you like
*/

	if(((*ptr <= '9')&&(*ptr >= '0')) || (*ptr == '.')
		|| (*ptr == '+') || (*ptr == '-'))
		return(YES);
	else
		return(NO);
}



save_stdin(file)
char	*file;
{

	extern FILE *yyin;
	extern yylineno;

	name->linenum = yylineno +1;
	yylineno = 0;
	name++;
	yyin = 0;
	if(zopen(file) == NULL)
	{
		flush_warnings();
		perror(file);
		gexit();
	}
}




restore_stdin()
{
	extern FILE *yyin;
	extern yylineno;

#ifdef	DEBUG
	fprintf(debug,"\nClosing file %s ...\n",name->fname);
#endif
	fclose(name->fptr);
	name--;
	yylineno = name->linenum;
	yyin = name->fptr;
#ifdef	DEBUG
	fprintf(debug,"And returning to %s\n\n",name->fname);
#endif
}



char *skip(ptr,n)
	register char *ptr;
	register int n;
{
	while((*ptr == ' ')||(*ptr == '\t')||(*ptr == ','))ptr++;
	if((*ptr == '\n') || (*ptr == 0 ))return(ERROR);
	while(--n)
	{
		while((*ptr != ' ')&&(*ptr != '\t')&&(*ptr != '\n')&&(*ptr != ','))ptr++;
		if(*ptr == '\n') return(ERROR);
		while((*ptr == ' ')||(*ptr == '\t')||(*ptr == ','))ptr++;
		if((*ptr == '\n')||(*ptr == 0)) return(ERROR);
	}
	return(ptr);
}




pause()
{

	g_put(007); /* BELL */
	if(gstatus & TO_HARD_COPY)
	{
		fprintf(stdout,"\033AK");
		fflush(stdout);
		/*
		** THE IDEA OF THIS ROUTINE IS TO STOP THE
		** PLOTTING AT STRATEGIC PLACES SO THAT PEN
		** CHANGES (DIFFERENT COLOURS) MAY OCCUR.
		** THE TEKTRONIX PLOTTER HAS A CALL BUTTON
		** THAT WHEN PRESSED WILL SEND SOME BYTES
		** BACK TO THE HOST COMPUTER.
		** THUS PLOTTING WILL NOT RESUME TILL HE
		** PRESSES THAT BUTTON
		*/
		read(fileno(stdout),"THIS IS A DUMMY BUFFER",8);
	
		fprintf(stdout,"\033AL");
		fflush(stdout);
	}
	else
	{
		fflush(stdout);
		read(fileno(stdout),"THIS IS BUMMY",1);
	}
}




reset()
{
	/*
	** THIS ROUTINE RESETS MANY GLOBAL VARIABLE BACK TO
	** THEIR POWER UP VALUES
	*/
	x_column = 1;
	y_column = 2; /* THESE DEFAULTS WILL PROBABLY BE MOVED */
	sumx = 0.0;
	sumy = 0.0;
	sumxy = 0.0;
	sumx2 = 0.0;
	sumy2 = 0.0;
	n = 0;
	point_type = CROSS;
	line_type = SOLID;
	mark_type = SOLID;
	xscale = 1;
	yscale = 1;
	gstatus =& ~(POINT_EXPLICIT | LINE_EXPLICIT | XMARK | YMARK);
	status =& ~(REG | XINC | YINC);
	tokens_found =& ~(MARK | PLOT);
}
#ifdef	DEBUG
dump(s)
char *s;
{
	/*
	** This dumps out some global variables,
	** But unfortunatly the good one dissapeared
	** and this is just and interum one.
	** (OH WELL, IT WOULD BE HARD FOR YOU TO DEBUG ANYWAY)
	*/
	fprintf(debug,"\n\n%s\n",s);
}
#endif	DEBUG
