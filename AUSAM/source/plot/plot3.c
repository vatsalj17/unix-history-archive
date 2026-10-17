#include "plot.h"






border()
/*
	DOES THE SCALING, DRAWING OF AXES ETC
*/
{

	float x,y;
	register int	xint,yint;
	/*
	** SOME VARIABLE ARE KEPT AS FLOATS BY NECCESSITY,
	** SO THE ABOVE MAKES THEM INTEGERS
	*/
	register int l;
	int k, xpoints,ypoints;
	float temp1, temp2;
	int temp3;
	/* these temps are used because adjust fixes all
	** 3 parameters (lo, hi, dist) but only some will
	** be put in the global locations, and these are temp-
	** orary storage
	*/





	if( status & NO_BORDER )
		return;
	status =| NO_BORDER;
	if(gstatus & TO_HARD_COPY)
	{
#ifdef	ELEC
		if(! (gstatus & HARD_COPY_ON))
		{
			puts(PLOTTER_ON,stdout);
			gstatus =| HARD_COPY_ON;
		}
		/* and a stty to turn echo off should be here */
#else
		fprintf(stderr,"\nGo to the plotter.\n%s\n",
		"Press the 'CALL' button to start and continue plotting");
#endif
	}
	if((status & XINC) && !(status & XMAN_SCALE))
	{
		temp1 = inc_min;
		temp2 = inc_max;
	}
	else
	{
		temp1 = xmin;
		temp2 = xmax;
	}
#ifdef	DEBUG
	fprintf(debug,"Before adjust xmin= %10.5f xmax = %10.5f\n",temp1,temp2);
#endif
	adjust(&temp1,&temp2,&temp3);
#ifdef	DEBUG
	fprintf(debug,"after : xmin = %10.5f xmax = %10.5f tick = %10d\n",temp1,temp2,temp3);
#endif
	if(! (status & XMAN_SCALE ))
	{
		xmin = temp1;
		xmax = temp2;
	}
	if( xtick_gap == 0.0)
	{
		xtick_gap = (xmax - xmin)/temp3;
		xpoints = temp3;
	}
	else
		xpoints = (xmax - xmin)/xtick_gap + 0.5;

	/*
	** THE USER SUPPLIES A DISTANCE BETWEEN NUMBERS
	** ON THE AXES, AND THIS IS CONVERTED TO THE
	** DISTANCE BETWEEN THE LITTLE TICKS
	*/
	xtick_gap =/ TICKS_PER_NUM;

	if((status & YINC) && !(status & YMAN_SCALE))
	{
		temp1 = inc_min;
		temp2 = inc_max;
	}
	else
	{
		temp1 = ymin;
		temp2 = ymax;
	}
#ifdef	DEBUG
	fprintf(debug,"before: ymin = %10.5f ymax = %10.5f\n",temp1,temp2);
#endif
	adjust(&temp1,&temp2,&temp3);
#ifdef	DEBUG
	fprintf(debug,"after: ymin= %10.5f ymax= %10.5f ticks= %10d\n",temp1,temp2,temp3);
#endif
	if(! (status & YMAN_SCALE ))
	{
		ymin = temp1;
		ymax = temp2;
	}
	if(ytick_gap == 0.0 )
	{
		ytick_gap = (ymax - ymin)/temp3;
		ypoints = temp3;
	}
	else
		ypoints = (ymax - ymin)/ytick_gap + 0.5;

	ytick_gap =/ TICKS_PER_NUM;


	/*
	** UNLESS THE USER MOVED THEM, THE AXES
	** ARE AT THE LEFT AND BOTTOM OF THE GRAPH
	*/
	if(! (status & MOVE_XAXIS))
		ypos = ymin;
	if(! (status & MOVE_YAXIS))
		xpos = xmin;


	uwindow(xmin - 0.20*(xmax - xmin), xmax + 0.1*(xmax - xmin),
		ymin - 0.1*(ymax - ymin), ymax + 0.1*(ymax - ymin));

	/*	XAXIS MARKER	*/
	if((gstatus & TO_HARD_COPY) || (gstatus & PAUSE_ON))
		pause();
	headings();	/* NO ERRORS SO FAR, SO DRAW THE HEADINGS OUT */
	if(gstatus & PAUSE_ON)
		pause();
	umovea(xpos,ymin);
	udrawa(xpos,ymax);
	umovea(xmin,ypos);
	udrawa(xmax,ypos);

	umovea(x = xmin,ypos);
	smover(0,TICK_SIZE*2);
	sdrawr(0,-TICK_SIZE*2);
	xprint(x);
	for(k=0; k<xpoints; k++)
	{
		for(l=1; l< TICKS_PER_NUM; l++)
		{
			/*
			** WE LOOP AROUND DOING
			** THE LITTLE TICKS
			*/
			umovea(x=+ xtick_gap,ypos);
			smover(0,TICK_SIZE);
			sdrawr(0,-TICK_SIZE);
		}
		umovea(x=+ xtick_gap,ypos);
		smover(0,TICK_SIZE*2);
		sdrawr(0,-TICK_SIZE*2);
		xprint(x);
	}
	/*	YAXIS	*/

	umovea(xpos,y = ymin);
	smover(TICK_SIZE*2,0);
	sdrawr(-TICK_SIZE*2,0);

	yprint(y);
	for(k=0; k<ypoints; k++)
	{
		for(l=1; l< TICKS_PER_NUM; l++)
		{
			umovea(xpos,y=+ ytick_gap);
			smover(TICK_SIZE,0);
			sdrawr(-TICK_SIZE,0);
		}
		umovea(xpos,y=+ ytick_gap);
		smover(TICK_SIZE*2,0);
		sdrawr(-TICK_SIZE*2,0);
		yprint(y);
	}
	/* THE OLD KEY BOX WAS HERE */
	/* draw in key if wanted */
	if((key_quadrant) && (key_quadrant != END))
	{
		if(gstatus & PAUSE_ON)
			pause();
		xint = xkey_pos;
		yint = ykey_pos;
		smovea(xint, yint=- 22);
		ykey_pos = yint;
		alpha();
		fputs("     KEY",stdout);
		smover(-5,22);
		sdrawr(HOR_BOX_SIZE*14 +52, 0);
		sdrawr(0,-(VER_BOX_SIZE*22 + 52));
		sdrawr(-(HOR_BOX_SIZE*14 + 52), 0);
		sdrawr(0,VER_BOX_SIZE*22 + 52);
		smover(0, -26);
		sdrawr(HOR_BOX_SIZE*14 +52,0);
	}
	/* next we put up extra axes if wanted	*/
	if(gstatus & XMARK)
	{
		if(gstatus & PAUSE_ON)
			pause();
		/*
		** xint IS JUST A SPARE VARIABLE
		*/
		for( xint = 0; xint < xmark_count; xint++)
		{
			if((xmark[xint]>=xmin) && (xmark[xint]<=xmax))
			{
				umovea(xmark[xint],ymin);
				switch(mark_type)
				{
				case SOLID:
					udrawa(xmark[xint],ymax);
					break;
				case SHORT:
					udasha(xmark[xint],ymax,1);
					break;
				case SHORT_LONG:
					udasha(xmark[xint],ymax,3);
					break;
				case LONG:
					udasha(xmark[xint],ymax,2);
					break;
				case OTHER:
				case NOTHING:
					break;
				}
			}
		}
	}
	if(gstatus & YMARK)
	{
		if(gstatus & PAUSE_ON)
			pause();
		for( xint = 0; xint < ymark_count; xint++)
		{
			if((ymark[xint]>=ymin) && (ymark[xint]<=ymax))
			{
				umovea(xmin,ymark[xint]);
				switch(mark_type)
				{
				case SOLID:
					udrawa(xmax,ymark[xint]);
					break;
				case SHORT:
					udasha(xmax,ymark[xint],1);
					break;
				case SHORT_LONG:
					udasha(xmax,ymark[xint],3);
					break;
				case LONG:
					udasha(xmax,ymark[xint],2);
					break;
				case OTHER:
				case NOTHING:
					break;
				}
			}
		}
	}
	/* This is so that the scaling
	** will not change on the plotted
	** lines after the border has been
	** drawn
	*/
	status =| (XMAN_SCALE | YMAN_SCALE);
}

xprint(num)
float num;
{
	char	buf1[30];	/* A TEMPORARY HOME OF THE NUMBERS BEFORE THEY ARE PRINTED */
	char	buf2[30];	/* A TEMPORARY HOME OF THE NUMBERS BEFORE THEY ARE PRINTED */
	if(status & NO_NUMBERS)
		return;
	if(( fabs((num-xpos)/(ymax-ymin)) <0.02) && (ypos != ymin))
		return;
	if( fabs(num/(xmax-xmin)) < 0.00001)
	{
		num =  0.0;
	}
	sprintf(buf1,xformat,num);
	strcpy(buf1,skip(buf1,1));	/* SKIP OVER ANY LEADING BLANKS */
	if(buf1[strlen(buf1)-1] == '.')
		buf1[strlen(buf1)-1] = 0;
	smover(-strlen(buf1)*XCHRSZ/2, -(YCHRSZ+5));
	alpha();
	printf("%s",buf1);

}

yprint(num)
float num;
{
	char	buf1[30];	/* A TEMPORARY HOME OF THE NUMBERS BEFORE THEY ARE PRINTED */
	char	buf2[30];	/* A TEMPORARY HOME OF THE NUMBERS BEFORE THEY ARE PRINTED */



	if(status & NO_NUMBERS)
		return;
	if(( fabs((num-ypos)/(xmax-xmin)) <0.02) && (xpos != xmin))
		return;
	if( fabs(num/(ymax-ymin)) < 0.00001)
	{
		num = 0.0;
	}
	sprintf(buf1,yformat,num);
	strcpy(buf1,skip(buf1,1));
	if(buf1[strlen(buf1)-1] == '.')
		buf1[strlen(buf1)-1] = 0;
	smover(-(strlen(buf1)*XCHRSZ +5), -7);
	alpha();
	printf("%s",buf1);
}



/*
** THIS IS THE HEART OF PLOTS AUTOMATIC SCALING
** ABILITY. GIVEN A LO AND HI VALUE, IT WILL ADJUST
** THESE AND THE NUMBER OF TICKS BETWEEN THEM TO
** PRODUCE WHAT IT CONSIDERS TO BE A NICE SCALING
** THE ROUTINE IS ARBITARY AND FIDDLEY.
** BE MY GUEST TO SUPPLY A NEW ONE AND IF IT WORKS
** WELL THEN TELL THE AUTHOR ABOUT IT !
** (RICHARD GREVIS, AGSM U.N.S.W)
*/

adjust(lo,hi,divisions)
	register float *lo, *hi;
	int	*divisions;
{
	/*
	** THE NORMALISED LOW VALUE
	*/
	int nlo;
	float diff,sdiff;
	int   ndiff;


	diff =  *hi -  *lo;
	if(diff == 0.0)
	{
		/*	WE CAN'T SCALE THE DATA REALLY
		**	SO RATHER THAN BOMBING WE CHOOSE
		**	+- 10% TO BE THE SCALE THEN PLOUGH
		**	ON FROM THERE
		*/

		*hi =+ *hi/10.0;
		*lo =- *lo/10.0;
		diff = *hi - *lo;
		warning("Data constant, Arbitary scaling done");
	}

	sdiff = size(diff);
#ifdef	DEBUG
	fprintf(debug,"first diff = %10.5f	sdiff = %10.5f\n",diff,sdiff);
#endif
	 *lo = floor( *lo*SENSITIVITY/sdiff + 1.0e-2)*sdiff/SENSITIVITY;
	 *hi = ceil( *hi*SENSITIVITY/sdiff - 1.0e-2)*sdiff/SENSITIVITY;
	diff =  *hi - *lo;
	sdiff = size(diff);
	/*
	** WE MUST NORMALISE THE DIFFERANCE TO GET
	** BETWEEN 1 AND 20 FOR THE SWITCH
	*/
	if((diff*SENSITIVITY/sdiff +0.5) >= 10)
	{
		ndiff = SENSITIVITY*diff/sdiff + 0.5;
		sdiff =/ 2.0;
	}
	else
		ndiff = SENSITIVITY*2.0*diff/sdiff + 0.5;
	if(ndiff >20)
		ndiff =/ 5;
	if( *lo<0.0)nlo =  *lo/(sdiff) -0.5;
	else nlo =  *lo/(sdiff) + 0.5;
	nlo =% 10;
	if(nlo<0)nlo =+ 10;
#ifdef	DEBUG
	fprintf(debug," diff = %10.5f	sdiff = %10.5f\n",diff,sdiff);
	fprintf(debug,"ndiff = %10d	nlo = %10d\n",ndiff,nlo);
#endif
	switch(ndiff)
	{
		case 1:
		case 10:
		case 2:
		case 5:
		case 20:
			*divisions = 5;
			break;
		case 9:
			if((nlo == 1)) *lo =- sdiff;
			else  *hi =+ sdiff;
			*divisions = 5;
			break;
		case 18:
			if((nlo <= 2))
			{
				*lo =- nlo*sdiff;
			}
			else
				*hi =+ sdiff;
			*divisions = 5;
			break;
		case 8:
		case 4:
		case 16:
			*divisions = 4;
			break;
		case 15:
			if(nlo == 1)
				*lo =- sdiff;
			else
				*hi =+ sdiff;
			*divisions = 4;
			break;
		case 3:
		case 6:
		case 12:
			*divisions = 6;
			break;
		case 7:
			if(nlo<=3)
			{
				 *lo =- nlo*sdiff;
				*divisions = (14 -nlo)/2;
			}
			else
				*divisions = 7;	/* OR PERHAPS 7 */
			break;
		case 14:
			*divisions = 7;
			break;
		case 11:
			if(nlo == 1)
				*lo =- sdiff;
			else
				*hi =+ sdiff;
			*divisions = 6;
			break;
		case 13:
			if(nlo == 1)
				*lo =- sdiff;
			else
				*hi =+ sdiff;
			*divisions = 7;
			break;
		case 17:
			if(nlo <= 3)
			{
				*lo =- nlo*sdiff;
				*hi =+ (3-nlo)*sdiff;
			}
			else
				*hi =+ 3*sdiff;
			*divisions = 5;
			break;
		case 19:
			if(nlo ==1)
				*lo =- sdiff;
			else
				*hi =- sdiff;
			*divisions = 5;
			break;
		default:
			flush_warnings();
			error("Unknown scaling error, check data and consult Richard Grevis");
			gexit();
			break;
	}
}




headings()
{
	int	down;
	int k;

	k = 0;
	down = 0;


	if(gstatus & PAUSE_ON)
		pause();
	for( k=0; k< head_count; k++)
	{
		smovea(500 -strlen(head_str[k])*XCHRSZ/2, 750 -YCHRSZ*down++);
		alpha();
		fputs(head_str[k], stdout);
	}
	if(x_str)
	{
		smovea(500 -strlen(x_str)*XCHRSZ/2,10);
		alpha();
		fputs(x_str,stdout);
	}
	if(y_str)
		syputsa(10,380 + strlen(y_str)*YCHRSZ/2, y_str);
	if(date_str)
	{
		smovea(1022-(strlen(date_str)*XCHRSZ), 767);
		alpha();
		fputs(date_str,stdout);
	}
}
