#include "plot.h"
#include "lex.yy.c"






/*
**
**	PLOT		R.Grevis
**
**
*/


main(ac,av)
int	ac;
char	**av;
{
	extern	yylineno;
	int k,m;
	int dunna_file;
	char	*strptr;
	char	stringbuff[STRING_SPACE];
	/*
	**	THIS IS WHERE ALL STRINGS GO
	*/
	extern FILE *yyin;

#ifdef	ELEC
	/*
	** The terminal should not echo when
	** hard copy is requested.
	** Normally if the plotter is a separate
	** device the terminal mode is -echo by default
	*/
	extern	struct g_ttystruct g_ttyb;
#endif

#ifdef	DEBUG

	if(( debug = fopen(BUGFILE,"w")) == NULL)
	{
		fprintf(stderr,"Cannot open plot.bug: The debug file\n");
		perror("REASON");
		gexit();
	}
#ifndef	RUN_BUG
	setbuf(debug,0);
#endif
#endif

	/*
	** ITS BETTER IF WE BLOCK BUFFER, EVEN
	** TO A TERMINAL, BUT IF THE ALLOC FAILS,
	** THEN WHO ARE WE TO QUESTION, WE'LL JUST
	** DO UNBUFFERED IO
	*/
	setbuf(calloc(BUFSIZ, sizeof *strptr ),stdout);
	xerr_pos = XERR_INIT;
	yerr_pos = YERR_INIT;
	strcpy(xformat,XFORM_INIT);
	strcpy(yformat,YFORM_INIT);
	name = infiles;	/* ADDRESS OF THE FIRST ELEMENT	*/
	sbuff_ptr = stringbuff;

	xmin = ymin = MAX_FLOAT;
	xmax = ymax =  -MAX_FLOAT;
	date_str = add_str(ctime(time()));
	*(date_str +11) = 0;
	strcat(date_str, date_str + 20);
	x_column = 1;
	y_column = 2; /* THESE DEFAULTS WILL PROBABLY BE MOVED */
	point_type = NOTHING;
	line_type = SOLID;
	mark_type = SOLID;
	xscale = 1;
	yscale = 1;
	status =& ~NO_SPECS;
	gstatus = 0;




	k = 1;
	dunna_file = NO;
	while( k < ac )
	{
		m = 1;
		if( *av[k] == '-' )
		{
			if(dunna_file)
			{
				/* IF WE DISCOVER A FLAG AFTER
				** SOME FILES HAVE BEEN DONE THEN
				** RESET MOST VARIABLES TO POWER
				** UP VALUES.
				*/


				reset();
			}
			while( av[k][m] )
			{
				switch( av[k][m++] )
				{
				case 'p':
#ifdef	DEBUG
					fprintf(debug,"We processed flag -p-\n");
#endif
						if(gstatus & INITIALISED)
							finish();
#ifndef	ELEC
						if((freopen(PLOTTER,"w",stdout) == NULL))
						{
							perror(PLOTTER);
							gexit();
						}
						/*
						** THIS IS A RATHER SIGNIFICANT
						** CLUDGE. STDOUT CANNOT BE READ
						** FROM AND THE PLOTTER HAS INODE LOCKING
						** AND THIS STDIO LIBRARY DOES NOT
						** ALLOW "WR" FROM A FILE
						*/
						close(fileno(stdout));
						fileno(stdout) = open(PLOTTER,2);
#endif
						gstatus =| TO_HARD_COPY;
						initialise();
#ifdef FAST
						syncs(0);
#endif
					break;

				case 's':
					gstatus =| PAUSE_ON;
					/*
					** This will cause the device
					** to stop at all stages of the plot
					** Mainly useful for hard copy
					** to facilitate pen changes
					*/
					break;


				case 'c':
					/*
					** Perform a syntax check on the input
					** Pretty yuky the way it does this really
					*/
					if(gstatus & INITIALISED)
						finish();
					if(freopen("/dev/null","w",stdout)==NULL)
					{
						perror("/dev/null");
						fprintf(stderr,"Cannot perform syntax check\n");
						gexit();
					}
					init(stdout);
#ifdef FAST
					syncs(0);
#endif
					gstatus =| INITIALISED;
					break;


				case 'b':
#ifdef	DEBUG
					fprintf(debug,"We processed flag -b-\n");
#endif

					/*
					** THIS STOPS THE NUMBERS BUT NOT
					** THE AXES AND HEADINGS FROM BEING
					** PRINTED. IF YOU WANT THIS FLAGS TO STOP
					** ALL OF THE BORDER FROM BEING PRINTED THEN
					** REPLACE NO_NUMBERS WITH NO_BORDER.
					** PS.
					** SUPRESSING OF THE NUMBERS BEING
					** PRINTED CAN BE ACHEIVED ANYWAY WITH
					** FORMAT ""
					** IE A NULL FORMAT ON EITHER OR BOTH AXES
					*/
					status =| NO_NUMBERS;
					break;
				case 'w':
#ifdef	DEBUG
					fprintf(debug,"We processed flag -w-\n");
#endif
					status =| NO_WARNINGS;
					break;
				case 'n':
#ifdef	DEBUG
					fprintf(debug,"We processed flag -n-\n");
#endif
					status=| NO_SPECS;
					break;
				case 'g':
					strcpy(date_str,AUTHOR);
					break;
				default:
					fprintf(stderr,"%s %s\n",
					  "Unknown flag - Consult",
					  "the documentation for the NEW plot");
#ifdef	DEBUG
					fprintf(debug,"Default process %c\n",av[k][(m-1)]);
#endif
					gexit();
					break;
				}
			}
		}
		else
		{
			/*
			** SET UP GRAPHICS AND SO ON
			*/
			initialise();
			if(zopen( av[k]) == NULL)
			{
				flush_warnings();
				perror(av[k]);
				gexit();
			}
			yylineno = 1;
			crunch();
			dunna_file = YES;
		}
		k++;
	}
	if(dunna_file == NO)	/* NO FILE IS GIVEN HENCE PIPES */
	{
		initialise();
		name->fptr = yyin;
		strcpy(name->fname,"");
		strcpy(name->pname,"");
		crunch();
	}
	gexit();
}
gexit( stat )
int stat;
{
	if(! (status & WARNS_FLUSHED ))
		flush_warnings();
	if(gstatus & TO_HARD_COPY)
	{
#ifdef	ELEC
		if(gstatus & HARD_COPY_ON)
		{
			puts(PLOTTER_OFF,stdout);
			gstatus =& ~HARD_COPY_ON;
		}
		/* and a stty to turn echo on should be here */
#else
		puts(CALL_LIGHT_OFF,stdout);
#endif
	}
#ifdef	RUN_BUG
	fflush(debug);
#endif
	if(gstatus & INITIALISED)
		finish();
	exit( stat );
}
FILE	*zopen(s)
char	*s;
{
	FILE *newfile;

	strcpy(name->pname,s);
	/*
	** LAST BIT RETURNS THE FILE COMPONENT OF A PATH-NAME
	*/
	strcpy(name->fname,last_bit(s));
#ifdef	DEBUG
	fprintf(debug,"Opening file %s now...\n\n",name->fname);
#endif
	fclose(yyin);
	/*
	** THE ABOVE CLOSE IS RENDERED INOPERATIVE WHEN
	** NECCESSARY BY MAKING YYIN = 0
	*/

	if(name+1 >= &infiles[INCLUDE_DEPTH])
	{
		flush_warnings();
		error("Unreasonable nesting depth of files");
		fprintf(stderr,"\n\t\tFile backtrace is :\n");
		for(; name >= infiles; name--)
			fprintf(stderr,"\t\t\t%s\n",name->pname);
		gexit();
	}
	newfile = fopen(s,"r");
	yyin = name->fptr = newfile;
	/* WE SET THE TOKEN TO ANYTHING EXCEPT EOF
	** BECAUSE IF EOF WAS TRUE, IT IS NOT NOW
	*/
	token = YES;
	return(newfile);
}
error(s)
char *s;
{
	extern yylineno;
#ifdef	ELEC
	if(gstatus & HARD_COPY_ON)
	{
		puts(PLOTTER_OFF,stdout);
		gstatus =& ~HARD_COPY_ON;
	}
	if(! (gstatus & TO_HARD_COPY))
	{
		smovea(xerr_pos,yerr_pos=- YCHRSZ);
		alpha();
	}
#else
	smovea(xerr_pos,yerr_pos=- YCHRSZ);
	alpha();
#endif
#ifdef	VERBOSE_ERRORS
	fprintf(stderr,"Error in file \"%s\" at line %d: %s\n",name->fname,yylineno,s);
#else
	fprintf(stderr,"Error: %s:%d: %s\n",name->fname,yylineno,s);
#endif
	status =| NO_PLOT;	/* SUPPRESS PLOTTING */
}

warning(s)
char *s;
{
	extern yylineno;

	if(status & NO_WARNINGS)
		return;

#ifdef	VERBOSE_ERRORS
	sprintf(warn_buf[warncount++],"Warning in file \"%s\" at line %d: %s\n",name->fname,yylineno-1,s);
#else
	sprintf(warn_buf[warncount++],"Warning: %s:%d: %s\n",name->fname,yylineno-1,s);
#endif
	/* YYLINENO HAS 1 SUBTRACTED FOM IT BECAUSE GETLINE IS 1 LINE AHEAD */
	if(warncount >= WARN_LIMIT)
	{
		flush_warnings();
		error("Warning limit exeeded. (Use -w)");
		gexit();
	}
}

flush_warnings()
{
	register int	count;

#ifdef	ELEC
		if(gstatus & HARD_COPY_ON)
		{
			puts(PLOTTER_OFF,stdout);
			gstatus =& ~HARD_COPY_ON;
		}
#endif
	for(count=0; count != warncount; count++)
	{
#ifdef	ELEC
		if(! (gstatus & TO_HARD_COPY))
		{
			smovea(xerr_pos, yerr_pos=- YCHRSZ);
			alpha();
		}
#else
		smovea(xerr_pos,yerr_pos=- YCHRSZ);
		alpha();
#endif
		fprintf(stderr,"%s",warn_buf[count]);
	}
	status =| WARNS_FLUSHED;
}
last_bit(s)
char	*s;
{
	/* returns a pointer to the last component of a path-name */

	char *old;

	if(!s) return;
	old = s;
	while(*s) s++;
	while( s >= old )
		if(*s == '/') return(s+1);
		else s--;
	return(old);
}
char	*add_str(s)
register char *s;
{
	register char	*temp;

	temp = sbuff_ptr;
	while(*s)
		*sbuff_ptr++ = *s++;
	*sbuff_ptr++ = 0;
	return(temp);
}
initialise()
{
	/*
	**	SET UP GRAPHICS
	*/
	if(gstatus & INITIALISED)
		return;
	init(stdout);
#ifdef	ELEC
	/*
	**	IF THE HARD COPY IS IN PARALLEL THEN
	**	TURN ECHO OFF
	*/
	g_ttyb.mode =& ~010;	/* this is echo but we don't want a conflict with lex */
	stty(fileno(stdout),&g_ttyb);
#endif
	if(! (gstatus & TO_HARD_COPY))
		erase();
#ifdef FAST
	syncs(0);
#endif
	gstatus =| INITIALISED;
}
