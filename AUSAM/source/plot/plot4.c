#include "plot.h"









getdata()
{

	struct point *value;
	register float	*p;
	float 	x,y;
	float	increment;

	/*
	** IF ERRORS HAVE BEEN FOUND IN THE PLOT SPECS, THEN
	** READING THE DATA USING THIS INFORMATION IS A BIT
	** DANGEROUS
	*/

	if(status & NO_PLOT)
		return;
	p = buffer;	/* point to the beginning of the buffer where the points are stored */

	while((value = getxy()) != NULL)
	{
		if(! (status & XINC) )
		{
			*p = value->x;
			if( !(status & XMAN_SCALE) )
			{
				if(*p > xmax)
					xmax = *p;
				if(*p < xmin)
					xmin = *p;
			}
			p++;
		}


		if(! (status & YINC) )
		{
			*p = value->y;
			if( !(status & YMAN_SCALE) )
			{
				if(*p > ymax)
					ymax = *p;
				if(*p < ymin)
					ymin = *p;
			}
			p++;
		}
		if( p >= (buffer + NUM_OF_POINTS*2))
		{
			warning("Too many points for internal buffer");
			break;
		}
	}
	saved = p;
	if(saved == buffer)
		status =| NO_DATA;
	if( status & REG )
	{
		if( (status & XINC) && (status & YINC) || (status & NO_DATA))
		{
			/*
			** CURRENTLY THIS POINT IS ONLY REACHED
			** IF THERE IS SOME DATA IN THE FILE, BUT IT
			** ALL GARBAGE AND HE ASKS FOR REGRESSION
			** BUT IF THE PROGRAM IS CHANGED SO BOTH X AND Y
			** CAN BE AUTO INCREMENTED ( THE EFFECT IS A STRAIGHT
			** LINE BETWEEN ANY TWO POINTS, WHICH CAN BE USEFUL)
			** THEN HERE IT IS ALSO.
			*/
			flush_warnings();
			error("With no data, I can hardly do a regression");
			gexit();
		}
		/* IS THAT + 1 CORRECT ?????????	*/
		increment = (inc_max - inc_min + 1)/(saved - buffer);
		x = inc_min - increment;
		y = inc_min - increment;

		p = buffer;
		if(status & REG)
		while(p<saved)
		{
			/*
			** THE REGRESSION STATISTICS ARE OBTAINED IN A
			** SECOND PASS, BUT THIS IS NECCESSARY WHEN
			** HE USES AUTO INCREMENT MODE
			*/
			if(status & XINC)
			{
				x =+ increment;
				y = *p++;
			}
			else if(status & YINC)
			{
				x = *p++;
				y =+ increment;
			}
			else
			{
				x = *p++;
				y = *p++;
			}
			sumx =+ x;
			sumy =+ y;
			sumxy =+ x*y;
			sumx2 =+ x*x;
			sumy2 =+ y*y;
			n =+ 1;
		}
	}
}



struct point *getxy()
{
	struct point value;
	char	*ptr;
	char	*line;

nextline:
	if((line = getline()) == EOF)
		return(NULL);

	/* THE NEXT TWO LINES ARE BECAUSE AN EMPTY LINE
	** SHOULD NOT REALLY EVEN BE A WARNING, IT SHOULD
	** BE SKIPPED SILENTLY
	*/
	if(*line == '\n')
		goto nextline;
	if( status & XINC ) goto yvalue;
	if((ptr = skip(line,x_column)) == ERROR)	/* point to the column of x values */
	{
		warning("Missing column of data");
		goto nextline;
	}
	if( number(ptr) == NO )		/* is the column value illegal (not numero)*/
	{
		warning("Garbage data");
		goto nextline;
		/* wellllll, its error recovery ?	*/
	}
	value.x = atof(ptr) * xscale;

yvalue:
	if(status & YINC )
		return(&value);
	if((ptr = skip(line,y_column)) == ERROR)
	{
		warning("Missing column of data");
		goto nextline;
	}

	if( number(ptr) == NO )
	{
		warning("Garbage data");
		goto nextline;
	}
	value.y = atof(ptr) * yscale;
	return(&value);
}



# define input() (((yytchar=yysptr>yysbuf?*--yysptr:getc(yyin))=='\n'?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
getline()
{
	int	yytchar;
	extern	FILE *yyin;
	extern	char *yysptr, yysbuf[];
	extern	yylineno;
	char	line1[256];
	char	line2[256];
	char	*ptr;


	ptr = line1;
	while( (*ptr = input()) != '\n')
		if(! *ptr++) return(EOF);;
	if(got_some_data)	/* IF TRUE THEN THIS NEEDS TO BE PREPENDED TO THE LINE */
	{
		sprintf(line2,"%s %s",got_some_data,line1);
		strcpy(line1,line2);
		got_some_data = 0;
	}

	return(line1);
}




getoken()
{

	token = yylex();
	if(!token)	/*	END OF FILE	*/
		token = EOF;
	return(token);
}



nextgroup()
{
	if( ! (token & SUB_TOKEN ))
		return(token);
	while( getoken() & SUB_TOKEN )	/* WHILE NOT A MAIN TOKEN	*/
		if( token == EOF ) return(EOF);
	return(token);
}
