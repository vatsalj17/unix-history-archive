/*
* FILE Term.c
*
* Module of mackermit: contains code for the terminal simulation
* routine.
*/

#include "term.h"  /* Terminal routine constants */

/*
 *  c o n n e c t
 *
 *  Establish a virtual terminal connection with the remote host.
*/

connection()
{
	register int index;
        int halt=FALSE,err=0,printit(), fixbuffer();
	char *addptr();


	initchars(); /* Set up the keyboard configuration */

	dumptr=NewRgn();
	PenMode(patXor);
	if (GetCRefCon(online)==21)
	{
		local();
		return;
	}
	flushio(); /* Get rid of pending characters */
	sendbreak();


	for(;;)
	{

/* Read until buffer is estimated full */
/* or until no input is immeadiately pending. */

	
			inparam.ioBuffer=inbuff;
			inparam.ioReqCount = BUFSIZE;
			PBRead(&inparam,TRUE);


/* Print and read keyboard until new input arrives */

		do
		{
			GetNextEvent(everyEvent, &myEvent);
			if ((myEvent.what==keyDown)||(myEvent.what==autoKey))
			{
				outchar= (myEvent.message & 0xff);

				/* Enter is break key */
				if(outchar==3)
				{
					sendbreak();
					flushio();
				}
				
				/* Translate typed characters */
				/* Clover leaf key is control */
				if(myEvent.modifiers&256)
					outchar=contcharmap[outchar];
				else
					outchar=charmap[outchar];
				err=PBWrite(&outparam,TRUE);
				if(err)
					printerr("Bad Writeout:",err);
			}

			/* Mouse button exits connect mode */
			else if(myEvent.what==mouseDown)
			{
				flushio();
				return;
				}
			if(*outp != 255)
			{
				if(halt<3)
					if(*addptr(outp,FULLENOUGH)!=255)
					{
						halt++;
						PBWrite(&xoffparam,FALSE);
					};
				printit();
			}
			else if (halt)
			{
				halt=FALSE;
				outchar=controlq;
				PBWrite(&outparam,TRUE);
			}
		}
		while  (inparam.ioResult!=0);
	}
}

/*
*  Printit:
*	Draws character and updates buffer
*/
	
printit()
{
	PFI funp, lookup();

	Line(-CHARWIDTH,0);	/* Erase Cursor */
	*outp &=0177;

	if (begptr!=NULL)	/* Are we in an escape sequence? */
	{
		if(funp=lookup(*outp,singescapetable,NUMSINGESCS))
			(*funp)();	/* Do escape sequence function */
		*begptr=255;
		*(++begptr)=255;
		begptr=NULL;
	}
	else if(escseq!=NULL)
	{
		if(funp=lookup(*outp,escapetable,NUMMULTESCS))
		{
			begptr=escseq;
			endptr= outp;
			(*funp)();	/* Do escape sequence function */
			for(;begptr<=endptr;begptr++)
				*begptr=255;
			escseq=NULL;
			begptr=NULL;
			endptr=NULL;
		}
	}
	else if(*outp==ESCAPE)
		begptr=outp;
	else
	{	
		MDrawChar(*outp);
		*outp=255;
		}
	Line(CHARWIDTH,0);	/* Redraw Cursor */
	if(++outp>=endbuff)   /* Find next character to be printed */
		outp=inbuff;
}	


/*
*  Fixbuffer:
*	Update input character buffer.  This is the completion routine for
*	the aynchronous reads.
*/

fixbuffer()
{	

}


	
/*
 *      KERMIT utilities.
 */
  
/*
*   Lookup:
*	Lookup a given character in the apropriate character table, and
*	return a pointer to the appropriate function, if it exists.
*/

 PFI
 lookup(data,table,length)
 char data;
 CMD table[];
 int length;
 {
 	for(length-1;length>=0;length--)
 	{
 		if(table[length].name==data)
 			return(table[length].funct);
 	};
 	return((PFI)NULL);
 }


/*
*   Flushio:
*	Initialize some communications constants, and clear screen and
*	character buffers.
*/

flushio()
{
	int i,err;



	for (i=0;i<=BUFSIZE;i++)
		inbuff[i]=255;
	controls='\023';
	controlq='\021';
	numinbuffer=0;
	begptr=NULL;
	endptr=NULL;
	escseq=NULL;
	outp=inbuff;
	err=KillIO(-6);
	if (err)
		printerr("Bad input clear",err);
	err=KillIO(-7);
	if(err)
		printerr("Bad ouput clear",err);

/* Initialize the asynchronous io paramater blocks */
	
	SetupIO();
	inparam.ioCompletion=fixbuffer;
	inparam.ioRefNum=innum ;
    	inparam.ioActCount=0;
	inparam.ioPosMode=0;
	inparam.ioPosOffset=0;
	inparam.ioResult=0;
	inparam.ioBuffer= inbuff;

	xoffparam.ioCompletion=(ProcPtr) NULL;
	xoffparam.ioRefNum=outnum;
    	xoffparam.ioReqCount=1;
	xoffparam.ioPosMode=0;
	xoffparam.ioPosOffset=0;
	xoffparam.ioBuffer= &controls;

	outparam.ioCompletion = (ProcPtr) NULL;
	outparam.ioRefNum=outnum;
	outparam.ioReqCount=1;
	outparam.ioPosMode=0;
	outparam.ioPosOffset=0;
	outparam.ioBuffer= &outchar;

	home_cursor();

	EraseRect((Rect *)inrect);

}	

   
/*
*  Sendbreak:
*	Sends a break across the communications lines.
*/
 
sendbreak()
{ 
	int xx;

	Control(outnum,12,&controlparam);
	for( xx=0; xx<150;xx++)
		;
	Control(outnum,11,&controlparam);
}

     
MDrawChar(chr)
char chr;
{
	PFI funp;

	/* If its a control char, do the apropriate function. */	
	

	if(chr<'!')   /* Space is a control character, because mac's space
		         does not wipe out the character beneath it. */
	{ 
			
		if (funp=lookup( chr,controltable,NUMSINGCMDS))
			(*funp)();
	}
	else
	{
		GetPen (&p);
		if(p.h>RIGHTMARGIN)
		{
		overrun=FALSE;
			carriage_return();
			line_feed();
			overrun=TRUE;
		}
		if(insert)
			insert_char();
		space();
		back_space(); 
		DrawChar(chr);
	}
}	

MDrawString(str)
char *str;
{
	while (*str)
		MDrawChar(*str++);
}


/* Increment pointer in circular buffer */
char *
addptr(ptr,num)
char *ptr;
int num;
{

	ptr+=num;
	if(ptr>endbuff)
		ptr=(char* )((long)ptr-BUFSIZE);
	else if(ptr<inbuff)
		ptr=(char *)((long)ptr+BUFSIZE);
	return(ptr);
}



myatoi(strptr)
char *strptr;
{
	int sign=1, n=0;

	if(*strptr=='+'||*strptr=='-')
		sign=(*strptr++=='+') ? 1 : -1;

	while( *strptr>='0'&& *strptr<='9')
		{
			n=10*n+*strptr-'0';
			if(++strptr>endbuff)  
				strptr=inbuff;
		};
	return(sign*n);
}
	
local()
{
	char *charp=inbuff;

	flushio();
	SysBeep(10);
	for(;;)
	{
		
			GetNextEvent(everyEvent, &myEvent);
			if ((myEvent.what==keyDown)||(myEvent.what==autoKey))
			{
				GetKeys(&theKeys);
				outchar= (myEvent.message & 0xff);

				
				/* Translate typed characters */
				/* Clover leaf key is control */
				if(theKeys.kmap[1] & (long)  0x8000) 
					outchar=contcharmap[outchar];
				else
					outchar=charmap[outchar];
				*charp=outchar;
				numinbuffer++;
				charp=addptr(charp,1);
				printit();
			}
			else if (myEvent.what==mouseDown)
				return;
	};
}



/*
* Initchars:
*	Do a mock reconfiguration of the keyboard by setting
*	values in an array of ascii chars.
*/

initchars()
{
	int index;


	/* Start with all charcters normal */
	for(index=0;index<256;index++)
	{
		charmap[index]=index;
		contcharmap[index]=index;
	};
	
	charmap['\140']='\033'; /* Replace ` with escape */
	charmap['\010']='\177'; /* Replace backspace with delete */

	/* Insert control characters */
	for(index='a';index<='z';index++)
		contcharmap[index] -=96;

}


/*
*	Control character functions:
*		Each of the following allow the mac to simulate
*		the behavior of a terminal when given the proper
*		control character.
*/

int
back_space()
{
	GetPen( &p);
	if (p.h-CHARWIDTH>0)
		Move(-CHARWIDTH,0);
	else
		carriage_return();
}

int
space()
{
	/* Regular quickdraw space does not cover previous characters */
	Point p; 

	GetPen(&p);
	currect[0]=p.v-LINEHEIGHT+3;
	currect[1]=p.h;
	currect[2]=p.v+3;
	currect[3]=p.h+CHARWIDTH;
	if (invert)
		FillRect((Rect* ) currect,&QD->black);
	else
		EraseRect((Rect*) currect);
	Move(CHARWIDTH,0);

}



tab()
{
	int index, tabstop, remember=999;

	GetPen(&p);

	for(index=0;index<NUMTABS;index++)
	{
		tabstop=(tabs[index])*CHARWIDTH;
		if (tabstop>p.h)
			if(tabstop<remember)
				remember=tabstop;
	};
	if (remember !=999)
		MoveTo(remember,p.v);
}


line_feed()
{
	if(!overrun)
	{
		GetPen(&p);
		if(p.v>=bottommargin)
		{
			ScrollRect((Rect *)scrollrect,0,-LINEHEIGHT,dumptr);
			EraseRgn(dumptr);
		}
		else
			MoveTo(p.h,p.v+LINEHEIGHT);
	}
	else
		overrun=FALSE;
}

carriage_return()
{
	GetPen(&p);
	MoveTo(LEFTMARGIN,p.v);
}

clear_screen()
{
	EraseRect((Rect *)inrect);
}


home_cursor()
{
	MoveTo(LEFTMARGIN,TOPMARGIN);
}


bell()
{
	SysBeep(3);
}





/* Esacpe Sequence Functions---basically behave like vt100 */

clear_line()
{
	GetPen(&p);

	currect[0]= p.v-LINEHEIGHT;
	currect[1]=p.h-CHARWIDTH;
	currect[2]=p.v+3;
	currect[3]=RIGHTMARGIN;

	EraseRect((Rect*) currect);


}

int
erase_display()
{
	int ermode;

	ermode=myatoi(addptr(begptr,2));
	GetPen(&p);

	switch(ermode)
	{
		case 0:
			clear_line();
			currect[0]=p.h+3;
			currect[1]=LEFTMARGIN;
			currect[2]=BOTTOMMARGIN;
			currect[3]=RIGHTMARGIN;
			EraseRect((Rect *) currect);
		case 1:
			currect[0]=p.v-LINEHEIGHT;
			currect[1]=LEFTMARGIN;
			currect[2]=p.v+3;
			currect[3]=p.h;
			EraseRect((Rect *) currect);
			currect[0]=TOPMARGIN;
			currect[2]=p.v-LINEHEIGHT;
			currect[3]=RIGHTMARGIN;
			EraseRect((Rect *) currect);
			break;
		case 2:
			clear_screen();
			break;
		}
}

			
cursor_right()
{
	Move(CHARWIDTH,0);
}

set_scroll_region()
{
	char *tmptr;
	int t,b;
	for(tmptr=begptr;*tmptr!=';';)
	{
		if (tmptr==endptr)
			return;
		tmptr=addptr(tmptr,1);
	};
	t=myatoi(addptr(begptr,2));
	b=myatoi(addptr(tmptr,1));
	if(t<=0)
		t=1;
	if (b<=0)
		b=1;
	topmargin= --t*LINEHEIGHT+TOPMARGIN;
	bottommargin= TOPMARGIN+(--b)*LINEHEIGHT;
	scrollrect[0]= (--t)*LINEHEIGHT+TOPMARGIN;
		if(scrollrect[0]<=0)
			scrollrect[0]=0;
	scrollrect[2]=bottommargin+LINEHEIGHT;
}

cursor_position()
{
	int hort=0,vert=0;
	char *tmptr;

	for(tmptr=begptr;*tmptr!=';';)
	{
		if (tmptr==endptr)
		{
			home_cursor();
			return;
		}
		tmptr=addptr(tmptr,1);
	};
	vert=myatoi(addptr(begptr,2));
	hort=myatoi(addptr(tmptr,1));

	if(--hort<0)
		hort=0;
	if(--vert<0)
		vert=0;	
	MoveTo(LEFTMARGIN+hort*CHARWIDTH,TOPMARGIN+vert*LINEHEIGHT);
}



text_mode()
{
	int style=myatoi(addptr(begptr,2));

	switch(style)
	{
		case 0: 
			invert=FALSE;
			textstyle=0;
			TextFace(0);
			TextMode(srcOr);
			break;
		case 1:
			textstyle +=boldStyle;
			TextFace(textstyle);
			break;
		case 4:
			textstyle+=underlineStyle;
			TextFace(textstyle);
			break;
		case 7:
			invert=TRUE;
			TextMode(srcBic);
			break;
		case 22:
			if (textstyle>=boldStyle)
			{
				TextFace(textstyle-boldStyle);
				textstyle -= boldStyle;
			}
			break;
		case 24:
			if (textstyle>=underlineStyle)
			{
				TextFace(textstyle-underlineStyle);
				textstyle -= underlineStyle;
			}
			break;
		case 27:
			invert=FALSE;
			TextMode(srcOr);
			break;
		default:;
	}
}	

reverse_line_feed()
{
	GetPen(&p);

	
	if(p.v<=topmargin)
	{
		ScrollRect((Rect *)scrollrect,0,LINEHEIGHT,dumptr);
		EraseRgn(dumptr);
	}
	else
		MoveTo(p.h,p.v-LINEHEIGHT);
}

insert_delete_line()
{
	char mode = *endptr;
	int scrolldir;
	int numlin=myatoi(addptr(begptr,2));

	if (numlin==0)
		numlin=1;

	currect[1]=LEFTMARGIN;
	currect[2]=bottommargin;
	currect[3]=RIGHTMARGIN;

	GetPen(&p);
	if(numlin> (bottommargin-p.v)/LINEHEIGHT)
		numlin=(int) ((bottommargin-p.v)/LINEHEIGHT);

	if (mode=='L')
	{
		scrolldir=LINEHEIGHT;
		currect[0]=p.v-LINEHEIGHT+3;
	}
	else
	{
		scrolldir= -LINEHEIGHT;
		currect[0]=p.v-LINEHEIGHT+3;
	}

	while(numlin--)
	{
		ScrollRect((Rect *)currect,0,scrolldir,dumptr);
		EraseRgn(dumptr);
	}
}


delete_char()
{
	int numchars=myatoi(addptr(begptr,2));

	if (numchars==0)
		numchars=1;

	GetPen(&p);

	currect[0]=p.v-LINEHEIGHT+3;
	currect[1]=p.h;
	currect[2]=p.v+3;
	currect[3]=RIGHTMARGIN;

	if(numchars> (RIGHTMARGIN-p.h)/CHARWIDTH)
		numchars =(int) ((RIGHTMARGIN-p.h)/CHARWIDTH);


	while(numchars--)
	{
		ScrollRect((Rect *)currect,-CHARWIDTH,0,dumptr);
		EraseRgn(dumptr);
	}
}

insert_char()
{
	GetPen(&p);
	
	currect[0]=p.v-LINEHEIGHT+3;
	currect[1]=p.h;
	currect[2]=p.v+3;
	currect[3]=RIGHTMARGIN;

	ScrollRect((Rect *) currect, CHARWIDTH,0,dumptr);
	EraseRgn(dumptr);
}

insert_mode()
{
	int mode=myatoi(addptr(begptr,2));
	if(mode==4)
	insert=TRUE;
}

end_insert_mode()
{
	int mode=myatoi(addptr(begptr,2));
	if(mode==4)
	insert=FALSE;
}

unknown()
{
	for(;begptr!=endptr;begptr=addptr(begptr,1))
	{
		MDrawChar(*begptr);
	}
}

dummy()
{
}
csi()
{
	escseq=begptr;
}
