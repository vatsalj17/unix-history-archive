#include "plot.h"







plotdata()
{

	int	xint, yint;	/* DUMMY VARIABLES USED TO MAKE INTEGERS */
	float temp , ycept, slope,r2;
	float temp1;
	char	numbuf[30];
	/*
	** THE ABOVE INITIALIZES THE AUTO INCREMENT IF IT APPLIES
	** IF IT DOES NOT, THEN THE VALUES ARE CHANGED
	*/

#ifdef	DEBUG
	dump("PLOTDATA STARTS");
#endif

	if(status & NO_DATA)
		return;
	/* this windowing first scales the window so
	   the data completely fills the screen,
	   then reduces it with the truncating screen
	   windowing to the correct size.
	   This is done so that all data outside the
	   confines of the borders is not plotted
	*/

	uwindow(xmin,xmax,ymin,ymax);
	swindow(WXL,WXH,WYL,WYH);
	if(gstatus & POINT_EXPLICIT)
		display(point_type);
	else if(status & REG)
		display(CROSS);
	/*
	** The revolting expression below
	** determines whether a line should
	** be drawn through the points.
	** It should never be drawn when a regression is done.
	** It should be done if he asks for a line explicitly.
	** It should be done if he does not ask for either
	** lines or points explicity.
	*/
	if( ((gstatus & LINE_EXPLICIT) ||
		( !(gstatus & POINT_EXPLICIT) && !(gstatus & LINE_EXPLICIT) ))
		&& !(status & REG))
		display(line_type);
	if( status & REG )
	{
		temp = sumxy - (sumx*sumy/n);
		slope = temp/( sumx2 - sumx*sumx/n);
		ycept = sumy/n - slope*sumx/n;
		r2    = temp*temp/((sumx2 - sumx*sumx/n)*(sumy2 - sumy*sumy/n));
		temp  = xmin*slope + ycept;


		if( (temp>= ymin) && (temp<= ymax)) umovea(xmin,temp);
		else if(temp<ymin) umovea((ymin - ycept)/slope,ymin);
		else umovea((ymax - ycept)/slope, ymax);
		temp = xmax*slope + ycept;
		/*
		** temp AND temp2 ARE USED TO STORE THE POSITION
		** GENERATED FOR THE SWITCH. I AM SORRY THAT THEY
		** ARE SUCH GROTTY NAMES
		*/

		if(gstatus & PAUSE_ON)
			pause();
		if(( temp>=ymin) && (temp<= ymax))
		{
			temp1 = temp;
			temp = xmax;
		}
		else if(temp < ymin)
		{
			temp = (ymin - ycept)/slope;
			temp1 = ymin;
		}
		else
		{
			temp = (ymax - ycept)/slope;
			temp1 = ymax;
		}
		switch(line_type)
		{
			case SOLID:
				udrawa(temp,temp1);
				break;
			case SHORT:
				udasha(temp,temp1,1);
				break;
			case LONG:
				udasha(temp, temp1,2);
				break;
			case SHORT_LONG:
				udasha(temp, temp1, 3);
				break;
			case OTHER:
			case NOTHING:
				break;

			default:
#ifdef	DEBUG
			dump("Line symbol got past lex but not at sw for REG");
#endif
			error("BUG IN PROGRAM, See Richard Grevis AGSM");
			gexit();
		}


		if(status & KEY)
		{
			labels(); /* LABELS MUST APPEAR AT THE END OF THE LINE */
			/*
			** THIS LITTLE FLAGS STOPS THE LABELS
			** BEING DONE AGAIN JUST BELOW HERE
			*/
			gstatus =| DID_LABEL;
		}
		if(reg_quadrant)	/* if he wants slope and intercept */
		{
			if(gstatus & PAUSE_ON)
				pause();
			/*
			** This is all very bad, but cplot
			** at the time of writing this has
			** save and restore missing (to save environment).
			** Thus I use previous knowledge of
			** what the screen window is and expand
			** it. (If i leave the window at the 
			** actual graph edge, then some of below will
			** not get printed)
			*/
			swindow(0,1023,0,780);
			if((ycept<0)&&(ycept>=-0.005))ycept = -ycept;
			sprintf(numbuf,"   Y'=%.4g",ycept);
			sxputsa(xint=xreg_pos, yint=yreg_pos, numbuf);
			sprintf(numbuf,"SLOPE=%.4g",slope);
			sxputsa(xint,yint=- YCHRSZ,numbuf);
			sprintf(numbuf,"  R^2=%.4f",r2);
			sxputsa(xint,yint=- YCHRSZ,numbuf);
			swindow(WXL,WXH,WYL,WYH);
		}
	}
	if( (status & KEY) && !(gstatus & DID_LABEL))
	{
		labels();
		gstatus =& ~DID_LABEL;
	}
}

labels()
{
	/*
	**
	** PRINT OUT THE LABELS IN THE RIGHT SPOT
	**
	*/
	int	xint, yint;	/* THEY ONLY MAKE FLOATS TO INTEGERS */

	/*
	** Expand the window to suppress
	** truncation for the labels
	*/
	swindow(0,1023,0,780);
	if(label_str != NULL)
	{
		if(key_quadrant == END)
			smover(11,-7);
		else
		{
			xint = xkey_pos;
			yint = ykey_pos;
			/*
			** This piece of yukkyness resets the
			** dash vector cycling so that they key
			** will contain a better example of the dashed
			** line (hopefully)
			*/
			smovea(2000,2000);
			sdasha(2100,2100,2);

			smovea(xint,(yint=- YCHRSZ)+7);
			ykey_pos = yint;
			if((line_type == NOTHING) ||
			  (((gstatus & POINT_EXPLICIT) && !(gstatus & LINE_EXPLICIT))
			  || (status & REG)))
			switch(point_type)
			{
			case STAR:
				smover(20,0);
				star();
				smover(REG_LINE_LEN - 20,0);
				break;
			case CIRCLE:
				smover(20,0);
				circ();
				smover(REG_LINE_LEN - 20,0);
				break;
			case CROSS:
				smover(20,0);
				cross();
				smover(REG_LINE_LEN - 20,0);
				break;
			case DOT:
				spointr(20,0);
				smover(REG_LINE_LEN - 20,0);
				break;
			}
			else
			 switch(line_type)
			{
			case SOLID:
				sdrawr(REG_LINE_LEN,0);
				break;
			case SHORT:
				sdashr(REG_LINE_LEN,0,1);
				break;
			case LONG:
				sdashr(REG_LINE_LEN,0,2);
				break;
			case SHORT_LONG:
				sdashr(REG_LINE_LEN,0,3);
				break;
			case OTHER:
				break;
			}

			smover(14,-7);
		}
		alpha();
		fputs(label_str,stdout);
		label_str = NULL;
	}
	label_str = NULL;
	/*
	** restore to old screen window
	*/
	swindow(WXL,WXH,WYL,WYH);
}

display(sym_type)
register int sym_type;
{
	register float *p;
	float	increment;
	float	x, y;

	if(sym_type == NOTHING)
		return;
	if(gstatus & PAUSE_ON)
		pause();
	p = buffer ;

	increment = (inc_max - inc_min + 1)/(saved - buffer);
	y = x = inc_min;
	if( status & XINC )
	{
		y = *p;
	}
	else  if( status & YINC )
	{
		x = *p;
	}
	else
	{
		x = *p;
		y = *(p+1);
	}
	umovea(x,y);
	/*
	** I'd have a great deal of trouble explaining why these
	** are here
	** If they were not, the plain draws would work, but the first
	** symbol would be ommitted (because of another fix)
	*/
	x =- increment;
	y =- increment;
	while(p < saved)
	{
		if( status & XINC )
		{
			x =+ increment;
			y = *p++;
		}
		else  if( status & YINC )
		{
			x = *p++;
			y=+ increment;
		}
		else
		{
			x = *p++;
			y = *p++;
		}
		switch(sym_type)
		{
			case DOT:
				upointa(x,y);
				break;
			case CROSS:
				umovea(x,y);
				cross();
				break;
			case NOTHING:
				break;
			case CIRCLE:
				umovea(x,y);
				circ();
				break;
			case STAR:
				umovea(x,y);
				star();
				break;
			case SOLID:
				udrawa(x,y);
				break;

			case SHORT:
				udasha(x,y,1);
				break;
			case LONG:
				udasha(x,y,2);
				break;
			case SHORT_LONG:
				udasha(x,y,3);
				break;
			case OTHER:
				break;
			default:
				error("Symbol confused me, This is a bug !!!!");
#ifdef	DEBUG
				dump("SYMBOL GOT PAST LEX BUT NOT MATCHED UP TOP");
#endif
				break;
		}
	}
}



cross()
{
	smover(0,CROSS_SIZE);
	sdrawr(0,-CROSS_SIZE*2);
	smover(-CROSS_SIZE,CROSS_SIZE);
	sdrawr(CROSS_SIZE*2,0);
	smover(-CROSS_SIZE,0);
}
circ()
{
	extern int g_spx,g_spy;
	scircle(g_spx,g_spy,CIRCLE_SIZE);
}
star()
{
	smover(-3*STAR_SIZE, -3 * STAR_SIZE);
	sdrawr(6 * STAR_SIZE, 4 * STAR_SIZE);
	sdrawr(-6 * STAR_SIZE, 0);
	sdrawr(6 * STAR_SIZE, -4 * STAR_SIZE);
	sdrawr(-3 * STAR_SIZE, 6 * STAR_SIZE);
	sdrawr(-3 * STAR_SIZE, -6 * STAR_SIZE);
	smover(3*STAR_SIZE,3*STAR_SIZE);
}
