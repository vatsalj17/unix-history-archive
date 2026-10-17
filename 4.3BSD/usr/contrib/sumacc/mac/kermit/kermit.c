
 /*
 *  File Kermit
 *
 *  UNIX Kermit, Columbia University, 1981, 1982, 1983
 *      Bill Catchings, Bob Cattani, Chris Maio, Frank da Cruz, Alan Crosswell
 *
 *  Also:   Jim Guyton, Rand Corporation
 *          Walter Underwood, Ford Aerospace
 *
 *  Macintosh adaptation: Stephen Engel
 */


#include "kermit.h"

/*
*	Main routine:
*			Initialize windows, communications, and wait for a
*		menu selection.
*/

main()
{
   
    int err,code;
    struct QDVar QDVar;

    QD = &QDVar;
    eol = CR;                           /* EOL for outgoing packets */
    quote = '#';                        /* Standard control-quote char "#" */
    pad = 0;                            /* No padding */
    padchar = NULL;                     /* Use null if any padding wanted */

     


    InitGraf(&thePort);
    InitFonts();
    InitWindows();
    InitDialogs((ProcPtr) NULL);
    SetupMenus();
    SetCursor(&QD->arrow);
    ControlWindow=NewWindow(&CWRecord, (Rect *) ControlRect, "Controls",
				0,userKind,(WindowPtr) -1,1,1);
	DebugWindow=NewWindow(&DWRecord, (Rect *) DebugRect, "Debugger",
			0,userKind,(WindowPtr) -1,1,1);
    TabWindow=NewWindow(&TWRecord, (Rect *) TabRect,
		"Tabs--Press any  key to exit",
				0,userKind,(WindowPtr) -1,1,1);
    theWindow=NewWindow(&wRecord, (Rect *)wirect, "MacKermit",VIS,
			documentProc, (WindowPtr) -1,1,0);
    SetupControls();
    SetPort(theWindow);
    theWindow->txFont=4;   /* Monaco font with non-proportional spacing */
    theWindow->txSize=9;
    PenSize(1,1);
    FlushEvents(everyEvent,0);
    err=GetVol(volname,&volnum);
    if (err)
	printerr("Bad volume name: ",err);

    config=0x4c0a;	   /* Initial Conditions:
				9600 baud, eight bit, no parity, one stop bit */
	SetupIO();

/* Main loop:  look for events, ignore unless its a menu selection
*              If it is call DoCommand to do the dispatching */

    for(;;)
    {
	SystemTask();
	GetNextEvent(everyEvent,&myEvent);
	switch(myEvent.what)
	{
		case mouseDown:
			code=FindWindow(&myEvent.where,&whichWindow);
			switch(code)
			{
				case inMenuBar:
					DoCommand(MenuSelect(&myEvent.where));
					break;
			}
	}
   }
}

SetupIO()
{
	int err;
			   /* Set up IO drivers */
    innum=OpenDriver(".AIn");
    outnum=OpenDriver(".AOut");
    controlparam.serConfig=config;
    err=Control(innum,8,&controlparam);
    if (err)
	fatal("Can't open drivers: ",err);
    err=Control(outnum,8,&controlparam);	
    if (err)
	fatal("Can't open drivers: ",err);

    			  /* Replace driver buffer with larger one */
    controlparam.serInBuff.serBPtr=mybuff;
    controlparam.serInBuff.serBLen=(short)MYBUFSIZE;
    err=Control(innum,9,&controlparam);
    if (err)
	printerr("Trouble making io buffer:",err);

			  /* Specify handshake options */
    controlparam.serShk.fXOn=(char)FALSE;
    controlparam.serShk.fCTS=(char)FALSE;
    controlparam.serShk.xOn=(char) 17;
    controlparam.serShk.xOff=(char) 19;
    controlparam.serShk.errs=(char)FALSE;
    controlparam.serShk.evts=(char)FALSE;
    controlparam.serShk.fInX=(char)FALSE;
    controlparam.serShk.null=(char)FALSE;
    if (err)
	printerr("Trouble with handshake: ",err);
    err=Control(outnum,10,&controlparam);
    if (err)
	printerr("Trouble with handshake: ",err);
}


/*
*   SetupMenus:
*		Install command menus.
*/

SetupMenus()
{
	int i;
 	char fnamcnv[13];

 	stringcpy(fnamcnv,"Filenamecnv!");
 	fnamcnv[12]=(char) 18;

	InitMenus();
	myMenus[0]=NewMenu(filemenu,"File");
	AppendMenu(myMenus[0], "Exit");
	myMenus[1]=NewMenu(modemenu,"Mode");
	AppendMenu(myMenus[1],"Connect");
	AppendMenu(myMenus[1],"Transmit");
	AppendMenu(myMenus[1],"Receive");
	AppendMenu(myMenus[1],"Controls");
	myMenus[2]=NewMenu(optionmenu,"Option");
	AppendMenu(myMenus[2],"Image");
	AppendMenu(myMenus[2],"Macwrite");
	AppendMenu(myMenus[2],"Executable");
	AppendMenu(myMenus[2],fnamcnv);
	AppendMenu(myMenus[2],"Debug");
	for (i=0;i<numMenu;i++)
		InsertMenu(myMenus[i],0);
	DrawMenuBar();
}


/*
*	SetupControls:
*		Read in and install the terminal controls
*/

SetupControls()
{
	ControlHandle theControl;
	int i;
	Handle tabstring;

	/* Read in controls from resource file */

	for(i=0;i<NUMCNTLS;i++)	
	{
		theControl=GetNewControl(i,ControlWindow);
		if(GetCtlValue(theControl))
		{
			config&= 
				masks[i].andpart;
			config|= 
				masks[i].orpart;
			if(i<11)
				baud=theControl;
			else if(i<13)
				bits=theControl;
			else if (i<16)
				parity=theControl;
			else if (i<18)
				stopb=theControl;
			else if (i<20)
			{
				xon=theControl;
				flowctl=(i==18);
			}
			else if (i < 22)
				online=theControl;
		}
	};
	MoveTo(25,25);
	tabstring=GetResource("TABS",0);
	stringcpy(tabs,*tabstring);
}


/*
*	DoCommand:
*		Either dispatch to a routine or change constant values,
*		according to menu selection.
*/

DoCommand(mResult)
int mResult;
{
	int theMenu=mResult>>16, theItem=(mResult &0xffff);

	if (mResult !=0)
		switch(theMenu)
		{
			case filemenu: /* Exit is only possibility */
				flushio();
				ExitToShell();
				break;
			case modemenu:
					/* Dispatch to the correct routines */
				switch(theItem)
				{
					case CONNECT:
						connection();
						break;
					case TRANSMIT:
						sendsw();
						break;
					case RECEIVE:
						recsw();
						break;
					case CONTROLS:
						DoControls();
						break;
					default:
						break;
				};
				break;
			case optionmenu:
					/* Update the selected constant */
					/* Selecting negates the current setting*/
				switch(theItem)
				{
					case IMAGE:
						image = !image; 

 /* Checking with last paramater=TRUE places checkmark, otherwise its erased */
						CheckItem(myMenus[2],IMAGE,image);
						break;
					case MACWRITE:
						macwrite = !macwrite; 
						CheckItem(myMenus[2],MACWRITE,macwrite);
						break;
					case EXECUTABLE:
						exec=!exec; 
						CheckItem(myMenus[2],EXECUTABLE,exec);
				/* Need all eight bits for executable code */
						image=exec;
						CheckItem(myMenus[2],IMAGE,image);
						break;
 
 				case FILENAMECNV:
 					filnamcnv=!filnamcnv;
 					CheckItem(myMenus[2],FILENAMECNV,filnamcnv);
 					break;
				case DEBUG:
					debug=!debug;
					CheckItem(myMenus[2],DEBUG,debug);
					break;
				};
		};
	HiliteMenu(0);
}		
    
/*
*	DoControls:
*		Draw contol labels, find which control was selected, and
*		update control panel and serial driver accordingly. 
*/
 
DoControls()
{
	int i,res,whichone;
	Handle myResource;

	ShowWindow(ControlWindow);
	SelectWindow(ControlWindow);	
	SetPort(ControlWindow);
	PenNormal();
	DrawControls(ControlWindow);
	TextFace(boldStyle);
	MoveTo(20,20);
	DrawString("Baud Rate");
	MoveTo(190,20);
	DrawString("Bits Sent");
	MoveTo(190,100);
	DrawString("Parity");
	MoveTo(280,20);
	DrawString("Stop Bits");
	MoveTo(280,100);
	DrawString("Xon/Xoff Flow");
	for(;;) /* Loop until exit button is pressed */
	{
		GetNextEvent(everyEvent,&myEvent);
		FindWindow(&myEvent.where,&whichWindow);
		GlobalToLocal(&myEvent.where);
		if (whichWindow==ControlWindow)
		switch(myEvent.what)
		{
		case mouseDown:
			res=FindControl(&myEvent.where,ControlWindow,&theControl);
		/* res=TRUE if a control was selected */
			if ( res)
			{
				TrackControl(theControl,&myEvent.where,(ProcPtr) NULL);
				whichone=(*theControl)->contrlRfCon;

				/* Update apropriate controls */
				if(whichone<11)
					resetcontrol(&baud);
				else if(whichone<13)
					resetcontrol(&bits);
				else if (whichone<16)
					resetcontrol(&parity);
				else if (whichone<18)
					resetcontrol(&stopb);
				else if (whichone<20)
				{
					resetcontrol(&xon);
				/* Need to update global variable since its
					not part of the driver */
					flowctl=(whichone==18);
				}
				else if (whichone < 22)
					resetcontrol(&online);
				else if (whichone==22)
				{
					ShowWindow(TabWindow);
					SelectWindow(TabWindow);
					SetPort(TabWindow);
					change_tabs();
					HideWindow(TabWindow);
					SelectWindow(ControlWindow);
					SetPort(ControlWindow);
				}	
				else /* Must be exit button */
				{
					HideWindow(ControlWindow);
					SelectWindow(theWindow);
					SetPort(theWindow);
					return;
				};

	/* Set selected control, the manager automatically fills it in. */
			if(whichone<22)
			{
				myResource=GetResource("CNTL",(int)(*theControl)->contrlRfCon);
				SetCtlValue(theControl,1);
				(*myResource)[9]=1;
				ChangedResource(myResource);
				WriteResource(myResource);
			}

	/* Reconfigure the drivers */
				if(whichone<18)
				/* Don't reconfigure if xon was thecontrol */
				{
				config&= 
					masks[whichone].andpart;
				config|= 
					masks[whichone].orpart;
				sendbreak();
				controlparam.serConfig=config;
				Control(innum,8,&controlparam);
				Control(outnum,8,&controlparam);	
				}
		}

	}
	};
}		

resetcontrol(cont)
ControlHandle *cont;
{
	Handle myResource;

	myResource=GetResource("CNTL",(int)(**cont)->contrlRfCon);
	(*myResource)[9]=0;
	ChangedResource(myResource);
	WriteResource(myResource);
	HNoPurge(myResource);
	SetCtlValue(*cont,0); /* Unselect the old control value */
	*cont=theControl;     /* Set it to the selected one */
}
	

		
 		
/*
 *  s e n d s w
 *
 *  Sendsw is the state table switcher for sending files.  It loops until
 *  either it finishes, or an error is encountered.  The routines called
 *  by sendsw are responsible for changing the state.
 *
 */
     
sendsw()
{
    int packno=0;  /* Actual (not mod 64) packet count */
    char sinit(), sfile(), sdata(), seof(), sbreak();
    state = 'S';                        /* Send initiate is the start state */
    n = 0;                              /* Initialize message number */
    fp=NULL;   				/* No file yet */
    numtry = 0;                         /* Say no tries yet */

    EraseRect((Rect *) inrect);
    MoveTo(15,50);
    MDrawString("SENDING...");
    MoveTo(15,200);
    MDrawString("Packet ");
    if(debug)
	ShowWindow(DebugWindow);
		
    while(TRUE)                         /* Do this as long as necessary */
    {
	if(n!=(packno%64))              /* Change in packet number? */
		packno++;
	EraseRect((Rect *) myrect);     /* Erase old packet number */
	MoveTo(80,200);	
	printnum(packno);
        switch(state)
        {
            case 'S':   state = sinit();  break; /* Send-Init */
            case 'F':   state = sfile();  break; /* Send-File */
            case 'D':   state = sdata();  break; /* Send-Data */
            case 'Z':   state = seof();   break; /* Send-End-of-File */
            case 'B':   state = sbreak(); break; /* Send-Break */
            case 'C': 	MoveTo(15,260);          /* Complete */ 
	    		DrawString("File Successfully Transmitted. ");
			return;
            case 'A':   		         /* "Abort" */
            default:	if(fp!=NULL)
			{
				FSClose(fp); 
				FlushVol(volname,volnum);	
			}
	 		MoveTo(15,260);
			MDrawString("File transmission failed."); 
			return;
        }
    }
}
     
     
/*
 *  s i n i t
 *
 *  Send Initiate: send this host's parameters and get other side's back.
 */
     
char sinit()
{
    int num, len;                       /* Packet number, length */
    if (numtry++ > MAXTRY) 
	{
		printerr("In init-- too many tries",0);
		return('A'); /* If too many tries, give up */
	};
	GetNextEvent(everyEvent,&myEvent);
	if(myEvent.what==mouseDown)
	{
		printerr("\nMousedown Aborted\n",0);
		return('A');
	};
    spar(packet);                       /* Fill up init info packet */
     
     
    spack('S',n,6,packet);              /* Send an S packet */
    switch(rpack(&len,&num,recpkt))     /* What was the reply? */
    {
        case 'N':  return(state);       /* NAK, try it again */
     
        case 'Y':                       /* ACK */
            if (n != num)               /* If wrong ACK, stay in S state */
                return(state);          /* and try again */
            rpar(recpkt);               /* Get other side's init info */
     
            if (eol == 0) eol = '\n';   /* Check and set defaults */
            if (quote == 0) quote = '#';
     
            numtry = 0;                 /* Reset try counter */
            n = (n+1)%64;              /*  Bump packet count */
            return('F');                /* OK, switch state to F */
     
        case FALSE: return(state);      /* Receive failure, try again */
     
	    /* Anything else, just "abort" */
        default: printerr("Bad packet type--sinit",0); 
					return('A');       
   }
 }
     
     
/*
 *  s f i l e
 *
 *  Send File Header.
 */
     
char sfile()
{
    int num,len;                   /* Packet number, length */
    char     *cp;                            /* char pointer */
     
    if (numtry++ > MAXTRY) 
	{
		printerr("Too many tries-- in sfile",0);
		return('A'); /* If too many tries, give up */
	};
	GetNextEvent(everyEvent,&myEvent);
	if(myEvent.what==mouseDown)
	{
		printerr("\nMousedown Aborted\n",0);
		return('A');
	};
     
    if (fp == NULL)                     /* If not already open, */
    {
	fp=open_file(filnam);
	if(!fp)
		fatal("Kermit Cancelled",0);
    
    if (filnamcnv)                      /* Convert lower case to upper  */
        for (cp =filnam; *cp != '\0'; cp++)
		{
            if (*cp >= 'a' && *cp <= 'z')
                *cp ^= 040;
		if ((*cp=='\0x20')||(*cp=='0xca'))
			*cp='.';
		};
     
    }
    len = stringlen(filnam);                /* Compute length of new filename */
     
    spack('F',n,len,filnam);         /* Send an F packet */
    switch(rpack(&len,&num,recpkt))     /* What was the reply? */
    {
        case 'N':                       /* NAK, just stay in this state, */
            num = (--num<0 ? 63:num);   /* unless it's NAK for next packet */
            if (n != num)               /* which is just like an ACK for */
                return(state);          /* this packet so fall thru to... */
     
        case 'Y':                       /* ACK */
            if (n != num) return(state); /* If wrong ACK, stay in F state */
            numtry = 0;                 /* Reset try counter */
            n = (n+1)%64;             /*  Bump packet count */
            size = bufill(packet);      /* Get first data from file */
            return('D');                /* Switch state to D */
     
            case FALSE: return(state);  /* Receive failure, stay in F state */
     
 			/* Something else, just "abort" */
            default:    printerr("Bad packet type--in sfile",0);
						return('A');   
        }
}
     
     
/*
 *  s d a t a
 *
 *  Send File Data
 */
     
char sdata()
{
    int num, len;                       /* Packet number, length */
    if (numtry++ > MAXTRY) 
	{
		printerr("Too many tries--in sdata",0);
		return('A'); /* If too many tries, give up */
	};
    GetNextEvent(everyEvent,&myEvent);
    if(myEvent.what==mouseDown)
    {
    	printerr("\nMousedown Aborted\n",0);
    	return('A');
    };
     
    spack('D',n,size,packet);           /* Send a D packet */
    switch(rpack(&len,&num,recpkt))     /* What was the reply? */
    {
        case 'N':                       /* NAK, just stay in this state, */
            num = (--num<0 ? 63:num);   /* unless it's NAK for next packet */
            if (n != num)               /* which is just like an ACK for */
                return(state);          /* this packet so fall thru to... */
     
        case 'Y':                       /* ACK */
            if (n != num) return(state); /* If wrong ACK, fail */
            numtry = 0;                 /* Reset try counter */
            n = (n+1)%64;                /*Bump packet count */
            if ((size = bufill(packet)) == EOF) /* Get data from file */
                return('Z');            /* If EOF set state to that */
            return('D');                /* Got data, stay in state D */
     
        case FALSE: return(state);      /* Receive failure, stay in D */
     
    	/* Anything else, "abort" */
        default: printerr("Bad packet type--sdata",0);
			     return('A');  
    }
}
     
     
/*
 *  s e o f
 *
 *  Send End-Of-File.
 */
     
char seof()
{
    int num, len;                       /* Packet number, length */
    if (numtry++ > MAXTRY) 
	{
		printerr("Too many tries--seof",0);
		return('A'); /* If too many tries, "abort" */
	};
    GetNextEvent(everyEvent,&myEvent);
    if(myEvent.what==mouseDown)
    {
    	printerr("\nMousedown Aborted\n",0);
    	return('A');
    };

     
    spack('Z',n,0,packet);              /* Send a 'Z' packet */
    switch(rpack(&len,&num,recpkt))     /* What was the reply? */
    {
        case 'N':                       /* NAK, just stay in this state, */
            num = (--num<0 ? 63:num);   /* unless it's NAK for next packet, */
            if (n != num)               /* which is just like an ACK for */
                return(state);          /* this packet so fall thru to... */
     
        case 'Y':                       /* ACK */
            if (n != num) return(state); /* If wrong ACK, hold out */
            numtry = 0;                 /* Reset try counter */
            n = (n+1)%64;              	/* and bump packet count */
            FSClose(fp);                /* Close the input file */
	    FlushVol(volname,volnum);	
            fp = NULL;                  /* Set flag indicating no file open */
            return('B');                /* if not, break, EOT, all done */
     
        case FALSE: return(state);      /* Receive failure, stay in Z */
     
   		/* Something else, "abort" */
        default:   printerr("Bad packet type--seof",0);
			 	   return('A');     
    }
}
     
     
/*
 *  s b r e a k
 *
 *  Send Break (EOT)
 */
     
char sbreak()
{
    int num, len;                       /* Packet number, length */
    if (numtry++ > MAXTRY) 
		{
			printerr("Too many tries--sbreak",0);
			return('A'); /* If too many tries "abort" */
		};
     
    GetNextEvent(everyEvent,&myEvent);
    if(myEvent.what==mouseDown)
    {
    	printerr("\nMousedown Aborted\n",0);
    	return('A');
    };
    spack('B',n,0,packet);              /* Send a B packet */
    switch (rpack(&len,&num,recpkt))    /* What was the reply? */
    {
        case 'N':                       /* NAK, just stay in this state, */
            num = (--num<0 ? 63:num);   /* unless NAK for previous packet, */
            if (n != num)               /* which is just like an ACK for */
                return(state);          /* this packet so fall thru to... */
     
        case 'Y':                       /* ACK */
            if (n != num) return(state); /* If wrong ACK, fail */
            numtry = 0;                 /* Reset try counter */
            n = (n+1)%64;                	/*and bump packet count */
            return('C');                /* Switch state to Complete */
     
        case FALSE: return(state);      /* Receive failure, stay in B */
     
        default:    return ('A');       /* Other, "abort" */
   }
}
     
     
/*
 *  r e c s w
 *
 *  This is the state table switcher for receiving files.
 */
     
recsw()
{
    int packno=0;			/* Actual (not mod 64) packet count */
    char rinit(), rfile(), rdata();     /* Use these procedures */
     
    state = 'R';                        /* Receive-Init is the start state */
    n = 0;                              /* Initialize message number */
    numtry = 0;                         /* Say no tries yet */
    fp=NULL; 

    EraseRect((Rect *) inrect);		/* Clear screen */
    MoveTo(15,50);
    MDrawString("RECEIVING...");
    MoveTo(15,200);
    MDrawString("Packet ");
    if(debug)
	ShowWindow(DebugWindow);
		
    while(TRUE)                         /* Do this as long as necessary */
    {
	if (n!=(packno%64))		/* Change in packet number? */
		packno++;
	EraseRect((Rect *) myrect);	/* Erase old packet count */
	MoveTo(80,200);	
	printnum(packno);
        switch(state)                   /* Do until done */
        {
            case 'R':   state = rinit(); break; /* Receive-Init */
            case 'F':   state = rfile(); break; /* Receive-File */
            case 'D':   state = rdata(); break; /* Receive-Data */
            case 'C':  	MoveTo(15,260);	        /* Complete */
 			DrawString("File Successfully Received.");
	 		return; 
            case 'A':  
			if( fp!=NULL)           /* Abort state */
			{
				FSClose(fp); 
				FlushVol(volname,volnum);	
			}
			MoveTo(15,260);
			DrawString("File Transfer Failed."); 
			return;        
        }
    }
}
     
     
/*
 *  r i n i t
 *
 *  Receive Initialization
 */
     
char rinit()
{
    int len, num;                       /* Packet length, number */
     
    if (numtry++ > MAXTRY) 
    {
	printerr("Too many tries--rinit",0);
	return('A'); /* If too many tries, "abort" */
    };
     
    switch(rpack(&len,&num,packet))     /* Get a packet */
    {
        case 'S':                       /* Send-Init */
            rpar(packet);               /* Get the other side's init data */
            spar(packet);               /* Fill up packet with my init info */
            spack('Y',n,6,packet);      /* ACK with my parameters */
            oldtry = numtry;            /* Save old try count */
            numtry = 0;                 /* Start a new counter */
            n = (n+1)%64;             /*  Bump packet number, mod 64 */
            return('F');                /* Enter File-Receive state */
     
        case FALSE:                     /* Didn't get packet */
            spack('N',n,0,(char *)0);           /* Return a NAK */
            return(state);              /* Keep trying */
     
        default:   printerr("Bad packet type--rinit",0);
		  return('A');       /* Some other packet type, "abort" */
    }
}
     
     
/*
 *  r f i l e
 *
 *  Receive File Header
 */
     
char rfile()
{
    int num, len, err;                       /* Packet number, length */
    char *filnam1;                   /* Holds the converted file name */
     
    if (numtry++ > MAXTRY) 
    {
	printerr("Too many tries--rfile",0);
	return('A'); /* "abort" if too many tries */
    };
     
    GetNextEvent(everyEvent,&myEvent);
    if(myEvent.what==mouseDown)
    {
    	printerr("\nMouse Aborted\n",0);
    	return('A');
    };
    switch(rpack(&len,&num,packet))     /* Get a packet */
    {
        case 'S':                       /* Send-Init, maybe our ACK lost */
            if (oldtry++ > MAXTRY) return('A'); /* If too many tries "abort" */
            if (num == ((n==0) ? 63:n-1)) /* Previous packet, mod 64? */
            {                           /* Yes, ACK it again with  */
                spar(packet);           /* our Send-Init parameters */
                spack('Y',num,6,packet);
                numtry = 0;             /* Reset try counter */
                return(state);          /* Stay in this state */
            }
            else return('A');           /* Not previous packet, "abort" */
     
        case 'Z':                       /* End-Of-File */
            if (oldtry++ > MAXTRY) return('A');
            if (num == ((n==0) ? 63:n-1)) /* Previous packet, mod 64? */
            {                           /* Yes, ACK it again. */
                spack('Y',num,0,(char* )0);
                numtry = 0;
                return(state);          /* Stay in this state */
            }
            else return('A');           /* Not previous packet, "abort" */
     
        case 'F':                       /* File Header (just what we want) */
            if (num != n) return('A');  /* The packet number must be right */
            	stringcpy(filnam, packet);    /* Copy the file name */
     
            if (filnamcnv)              /* Convert upper case to lower */
                for (filnam1=filnam; *filnam1 != '\0'; filnam1++)
                    if (*filnam1 >= 'A' && *filnam1 <= 'Z')
                        *filnam1 |= 040;
		MDrawString("\n\n\n\rFile Name: ");
		MDrawString(filnam);
		if (!exec)
		{	/* Place data in data fork of mac text file */

			err=Create(filnam,0,"KERMIT","TEXT");
			if(err != noErr)
			{
				printerr("Couldn't create file:",err);
				return('A');
			}
			err=FSOpen(filnam,0,&fp);
			if(err !=noErr)
			{
				printerr("Can't open file:",err);
				return('A'); 
			}
		}
		else
		{	/* Executable file---place data in resource fork
			   of application file. */

			err=Create(filnam,0,"KERMIT","APPL");
			if(err != noErr)
			{
				printerr("Couldn't create file:",err);
				return('A');
			}
			err=OpenRF(filnam,0,&fp);
			if(err != noErr)
			{
				printerr("Couldn't open resources: ",err);
				return('A');
			}
			
		};
            spack('Y',n,0,(char *)0);           /* Acknowledge the file header */
            oldtry = numtry;            /* Reset try counters */
            numtry = 0;                 /* ... */
            n = (n+1)%64;                /* Bump packet number, mod 64 */
            return('D');                /* Switch to Data state */
     
        case 'B':                       /* Break transmission (EOT) */
            if (num != n) return ('A'); /* Need right packet number here */
            spack('Y',n,0,(char *)0);   /* Say OK */
            return('C');                /* Go to complete state */
     
        case FALSE:                     /* Didn't get packet */
            spack('N',n,0,(char *)0);           /* Return a NAK */
            return(state);              /* Keep trying */
     
        default:    return ('A');       /* Some other packet, "abort" */
    }
}
     
/*
 *  r d a t a
 *
 *  Receive Data
 */
     
char rdata()
{
    int num, len;                       /* Packet number, length */
    if (numtry++ > MAXTRY) 
	{
		printerr("Too many tries--rdata",0);
		return('A'); /* "abort" if too many tries */
		};
    GetNextEvent(everyEvent,&myEvent);
    if(myEvent.what==mouseDown)
    {
    	printerr("\nMouse Aborted\n",0);
    	return('A');
    };

    switch(rpack(&len,&num,packet))     /* Get packet */
    {
        case 'D':                       /* Got Data packet */
            if (num != n)               /* Right packet? */
            {                           /* No */
                if (oldtry++ > MAXTRY)
                    {
			printerr("Too many tries--rdata",0);
			return('A');        /* If too many tries, abort */
				};
                if (num == ((n==0) ? 63:n-1)) /* Else check packet number */
                {                       /* Previous packet again? */
                    spack('Y',num,6,packet); /* Yes, re-ACK it */
                    numtry = 0;         /* Reset try counter */
                    return(state);      /* Don't write out data! */
                }
                else 
		{
			printerr("Bad Packet no--rdata",0);
			return('A');       /* sorry, wrong number */
		};
            }

	    bufemp(packet,len);
            spack('Y',n,0,(char *)0);   /* Acknowledge the packet */
            oldtry = numtry;            /* Reset the try counters */
            numtry = 0;                 /* ... */
            n = (n+1)%64;               /* Bump packet number, mod 64 */
            return('D');                /* Remain in data state */
     
        case 'F':                       /* Got a File Header */
            if (oldtry++ > MAXTRY)
	    {
		printerr("Too many tries--rdata",0);	
               	return('A');            /* If too many tries, "abort" */
	    };
            if (num == ((n==0) ? 63:n-1)) /* Else check packet number */
            {                           /* It was the previous one */
                spack('Y',num,0,(char *)0);     /* ACK it again */
                numtry = 0;             /* Reset try counter */
                return(state);          /* Stay in Data state */
            }
            else
	    {
		printerr("Not previous packet-rdata",0);
		return('A');           /* Not previous packet, "abort" */
	    };
     
        case 'Z':                       /* End-Of-File */
            if (num != n) return('A');  /* Must have right packet number */
            spack('Y',n,0,(char *)0);           /* OK, ACK it. */
            FSClose(fp);                 /* Close the file */
	    FlushVol(volname,volnum);	
            n = (n+1)%64;               /*  Bump packet number */
            return('F');                /* Go back to Receive File state */
     
        case FALSE:                     /* Didn't get packet */
            spack('N',n,0,(char *)0);           /* Return a NAK */
            return(state);              /* Keep trying */
     
        default:     return('A');       /* Some other packet, "abort" */
    }
}


/*
*   KERMIT Utilities
*/

stringcpy(str1,str2)
char *str1,*str2;
{
	while(*str1++ = *str2++)
	;
}

stringlen(str)
char *str;
{
	int i=0;

	while(*str++)
		i++;
	return(++i);
}

/*
 *  r p a c k
 *
 *  Read a Packet
 */
     
rpack(len,num,data)
int *len, *num;                         /* Packet length, number */
char *data;                             /* Packet data */
{
    int i, done;                /* Data character number, loop exit */
    char chksum,t='a',type;
    IOParam  inparam;

    inparam.ioCompletion=(ProcPtr) NULL;
    inparam.ioRefNum=innum ;
    inparam.ioReqCount=1;
    inparam.ioPosMode=0;
    inparam.ioPosOffset=0;
    inparam.ioBuffer= &t;

    if (debug)
    {
        SelectWindow(DebugWindow);
    	SetPort(DebugWindow);
    	EraseRect((Rect *) inrect);
    	MoveTo(50,12);
    	MDrawString("Packet Coming In\n\n\r");
    }
    while (t != SOH)                    /* Wait for packet header */
    {
        PBRead(&inparam,TRUE);
	while(inparam.ioResult!=0)
	{
		GetNextEvent(everyEvent,&myEvent);
		if (myEvent.what==keyDown)
			return(FALSE);
	};
        t &= 0177;                      /* Handle parity */
    }
    if (debug)
	MDrawString("Got SOH \n\r");
    done = FALSE;                       /* Got SOH, init loop */
    while (!done)                       /* Loop to get a packet */
    {
        PBRead(&inparam,TRUE);
	while(inparam.ioResult!=0)
	{
		GetNextEvent(everyEvent,&myEvent);
		if (myEvent.what==keyDown)
			return(FALSE);
	};
        if (!image) t &= 0177;          /* Handle parity */
        if (t == SOH) continue;         /* Resynchronize if SOH */
        chksum = t;                     /* Start the checksum */
        *len = unchar(t)-3;             /* Character count */
	if (debug)
	{
		MDrawString("Number of characters: ");
		printnum(*len);
	}
        PBRead(&inparam,TRUE);
	while(inparam.ioResult!=0)
	{
		GetNextEvent(everyEvent,&myEvent);
		if (myEvent.what==keyDown)
			return(FALSE);
	};
        if (!image) t &= 0177;          /* Handle parity */
        if (t == SOH) continue;         /* Resynchronize if SOH */
        chksum = chksum + t;            /* Update checksum */
        *num = unchar(t);               /* Packet number */
	if(debug)
	{
		MDrawString("\n\rPacket Number: ");
		printnum(*num);
	}
     
        PBRead(&inparam,TRUE);
	while(inparam.ioResult!=0)
	{
		GetNextEvent(everyEvent,&myEvent);
		if (myEvent.what==keyDown)
			return(FALSE);
	};
        if (!image) t &= 0177;          /* Handle parity */
        if (t == SOH) continue;         /* Resynchronize if SOH */
        chksum = chksum + t;            /* Update checksum */
        type = t;                       /* Packet type */
    	if(debug)
	{
		MDrawString("\n\rPacket type: ");
		MDrawChar(type);
	}
        for (i=0; i<*len; i++)          /* The data itself, if any */
        {                               /* Loop for character count */
        	PBRead(&inparam,TRUE);
		while(inparam.ioResult!=0)
		{
			GetNextEvent(everyEvent,&myEvent);
			if (myEvent.what==keyDown)
				return(FALSE);
		};
            if (!image) t &= 0177;      /* Handle parity */
            if (t == SOH) continue;     /* Resynch if SOH */
            chksum = chksum + t;        /* Update checksum */
            data[i] = t;                /* Put it in the data buffer */
        }
        data[*len] = 0;                 /* Mark the end of the data */
     
        PBRead(&inparam,TRUE);
	while(inparam.ioResult!=0)
	{
		GetNextEvent(everyEvent,&myEvent);
		if (myEvent.what==keyDown)
			return(FALSE);
	};
        if (!image) t &= 0177;          /* Handle parity */
        if (t == SOH) continue;         /* Resynchronize if SOH */
        done = TRUE;                    /* Got checksum, done */
    }

     
                                        /* Fold in bits 7,8 to compute */
    chksum = (((chksum&0300) >> 6)+chksum)&077; /* final checksum */
     
    if (chksum != unchar(t)) 
    {
	if (debug)
	{
		MDrawString("\n\rBad Checksum: ");
		printnum(unchar(t));
	}
	type=FALSE;
    }
    if (debug)
    {
	MDrawString("\n\rHere's the data:\n\n\r");
	MDrawString(data);
	do
	{
		GetNextEvent(everyEvent,&myEvent);
	}
	while(myEvent.what !=mouseDown);
	SelectWindow(theWindow);
	SetPort(theWindow);
    }

     
    return(type);                       /* All OK, return packet type */
}

/*
 *  s p a c k
 *
 *  Send a Packet
 */
     
spack(type,num,len,data)
char type, *data;
int num, len;
{
    int i, count;                            /* Character loop counter */
    char chksum, buffer[100];           /* Checksum, packet buffer */
    register char *bufp;                /* Buffer pointer */
    bufp = buffer;                      /* Set up buffer pointer */
		/* Issue any padding */
    for (i=1; i<=pad; i++) FSWrite(outnum,&count, &padchar); 
     
    *bufp++ = SOH;                      /* Packet marker, ASCII 1 (SOH) */
    *bufp++ = tochar(len+3);            /* Send the character count */
    chksum  = tochar(len+3);            /* Initialize the checksum */
    *bufp++ = tochar(num);              /* Packet number */
    chksum += tochar(num);              /* Update checksum */
    *bufp++ = type;                     /* Packet type */
    chksum += type;                     /* Update checksum */
     
    for (i=0; i<len; i++)               /* Loop for all data characters */
    {
        *bufp++ = data[i];              /* Get a character */
        chksum += data[i];              /* Update checksum */
    }
    chksum = (((chksum&0300) >> 6)+chksum)&077; /* Compute final checksum */
    *bufp++ = tochar(chksum);           /* Put it in the packet */
    *bufp = eol;                        /* Extra-packet line terminator */
    count=bufp-buffer+1;
    FSWrite(outnum,&count, buffer); /* Send the packet */
	if (debug)
	{
	SelectWindow(DebugWindow);
	SetPort(DebugWindow);
	EraseRect((Rect *) inrect);
	MoveTo(50,12);
	MDrawString("Packet To Be Sent Out\n\n\r");
	MDrawString("Packet Number: ");
	printnum(num);
	MDrawString("\n\rPacket Type: ");
	MDrawChar(type);
	MDrawString("\n\rCharacter Count: ");
	printnum(len+3);
	MDrawString(" \n\rThe Packet: \n\n\r");
	MDrawString( buffer );
	do
	{
		GetNextEvent(everyEvent,&myEvent);
	}
	while(myEvent.what !=mouseDown);
	SelectWindow(theWindow);
	SetPort(theWindow);
	}
}
     

/*
 *  b u f i l l
 *
 *  Get a bufferful of data from the file that's being sent.
 *  Only control-quoting is done; 8-bit & repeat count prefixes are
 *  not handled.
 */
     
bufill(buffer)
char buffer[];                          /* Buffer */
{
    int i;                              /* Loop index */
    char t;				/* Character read from file */
    int 
        t7,                             /* 7-bit version of above */
	count=1;
    i = 0;                              /* Init data buffer pointer */
    while( FSRead(fp,&count,&t) != eoFErr)        /* Get the next character */
    {
        t7 = t & 0177;                  /* Get low order 7 bits */
     
        if (t7 < SP || t7==DEL || t7==quote) /* Does this char require */
        {                                   /* special handling? */
    	    if(t7==CR && macwrite)	/* Map CR to LF if macwrite */
		t=t7='\n';
            if (t=='\n' && !image) 
            {                           /* Do LF->CRLF mapping if !image */
                buffer[i++] = quote;
		buffer[i++]=ctl('\r');
            } 
            buffer[i++] = quote;        /* Quote the character */
            if (t7 != quote)
            {
                t = ctl(t);             /* and uncontrolify */
                t7 = ctl(t7);
            }
        }
        if (image)
            buffer[i++] = t;            /* Deposit the character itself */
        else
            buffer[i++] = t7;
        if (i >= spsiz-8) return(i);    /* Check length */
    }
    if (i==0) 
		return(EOF);              /* Wind up here only on EOF */
    return(i);                          /* Handle partial buffer */
}
     
     
/*
 *      b u f e m p
 *
 *  Put data from an incoming packet into a file.
 */
     
bufemp(buffer,len)
char  *buffer;                         /* Buffer */
int   len;                              /* Length */
{
    int i,err,count=1;                              /* Counter */
    char t;                             /* Character holder */
     
    for (i=0; i<len; i++)               /* Loop thru the data field */
    {
        t = buffer[i];                  /* Get character */
        if (t == MYQUOTE)               /* Control quote? */
        {                               /* Yes */
            t = buffer[++i];            /* Get the quoted character */
            if ((t & 0177) != MYQUOTE)  /* Low order bits match quote char? */
                t = ctl(t);             /* No, uncontrollify it */
        }
        if (t=='\n' && macwrite)	/* If macwrite filter out LFs */
            continue;
	err=FSWrite(fp,&count,&t);	/* Write out the character */
	if (err != noErr)
		fatal("Error in writing :", err);
	}
}
     
     
/*
*   SPAR:
*		Fill a packet with our communication paramaters 
*/

spar(data)
char data[];
{
	data[0]=tochar(MAXPACKSIZ);
	data[1]=tochar(MYTIME);
	data[2]=tochar(MYPAD);
	data[3]=ctl(MYPCHAR);
	data[4]=tochar(MYEOL);
	data[5]=MYQUOTE;
}


/*
*   RPAR:
*		Read in the other kermits paramaters
*/

rpar(data)
char data[];
{
	spsiz=unchar(data[0]);
	timint=unchar(data[1]);
	pad=unchar(data[2]);
	padchar=ctl(data[3]);
	eol=unchar(data[4]);
	quote=data[5];
}


/*
*  Printerr:
*	Display error message and number in standard error box
*/

printerr(str,err)
char *str;
int err; 
{
	int i;
	char error[10];

	if (err) /* Err=0 signals message only */
	{
		/* Make sure string will be null terminated */
		for(i=0;i<10;error[i++]='\0');

		NumToString(err,error);  /* Convert err number */
		ParamText(str,error,"",""); /* Insert strings into error box */
	}
	else
		ParamText(str,"","","");
	StopAlert(1,(ProcPtr) NULL);

}


/*
*   Fatal:
*	Printerr, close file, and exit to shell.
*/

fatal(str,err)
char *str;
int err;
{
	printerr(str,err);

	if(fp!=NULL)
	{
		FSClose(fp);
		FlushVol(volname,volnum);	
	}
	ExitToShell();
}

/*
*   Printnum:
*	Print out number as an ascii string.
*/

printnum(num)
int num;
{
	int i;
	char numstr[10];

	/* Make sure string will be null terminated. */

	for(i=0;i<10;numstr[i++]='\0');
	NumToString(num,numstr);
	DrawString(numstr);

}

draw_tabs()
{
	int index,tabindex,lin;

	MoveTo(CHARWIDTH,TOPMARGIN);
	for(index=1;index<=(RIGHTMARGIN-LEFTMARGIN)/CHARWIDTH;index++)
	{
		for (tabindex=0;tabindex<NUMTABS;tabindex++)	
			if(tabs[tabindex]==index)
			{
				DrawChar('T');
				Move(-CharWidth('T'),0);
				break;
			};
		Line(CHARWIDTH,0);
		lin=3+3*((index%5)==0);
		Line(0,-lin);
		Move(0,lin);
	}
}

change_tabs()
{
	int ind;
	Handle tabstring;

	clear_screen();
	draw_tabs();
	FlushEvents(everyEvent,0);
	for (;;)
	{
		GetNextEvent(everyEvent, &myEvent);
		if (myEvent.what==mouseDown)
		{
			GlobalToLocal(&myEvent.where);
			for(ind=0;ind<NUMTABS;ind++)
			{
				if(((myEvent.where.h-tabs[ind]*CHARWIDTH)<CHARWIDTH)&&
				   (myEvent.where.h-tabs[ind]*CHARWIDTH>0))
				{
					SysBeep(3);
					tabs[ind]=0;
					break;
				}
			}
			if (ind>=NUMTABS)
				for(ind=0;ind<NUMTABS;ind++)
				 	if(tabs[ind]==0)
					{
						tabs[ind]= myEvent.where.h/(int) CHARWIDTH;
						break;
					}
		clear_screen();
		draw_tabs();
		}
		else if(myEvent.what==keyDown)
			break;
	};
	tabstring=GetResource("TABS",0);
	stringcpy(*tabstring,tabs);
	ChangedResource(tabstring);
	WriteResource(tabstring);
}

/*
*	open_file  -- used to open the file for reading
*	   returns value of refNum if successful, NULL if not
*	   returns file name in fNamePtr if successful, NULL if not
*/

int open_file (fNamePtr)	/*returns refNum if file opened, NULL if not*/
char fNamePtr[];
{
	SFReply reply;
	Point where;
	int i, err=0, length, refNum;

	/*position of dialog box*/
	SetPt (&where,30,30);

	/*initializes fNamePtr to NULLs for safety*/
	for (i=0; i<64; i++)
		reply.fName[i] = NULL;

	/*handles user interface*/
	SFGetFile (&where, NULL, NULL, -1, NULL, NULL, &reply);

	/*
	*	convert Pascal string to C string --
	*	remove leading number and add a trailing NULL
	*/
	length = reply.fName[0];
	for (i=0; i< length; i++)
		fNamePtr[i] = reply.fName[i + 1];
	fNamePtr[i] = NULL;

	for (i=0; i< length; i++)
		reply.fName[i] = reply.fName[i + 1];
	reply.fName[i] = NULL;

	if (reply.good)		/*user did not press Cancel Button*/
		{
		err = FSOpen (reply.fName, (int) reply.vRefNum, &refNum);
		if(err)
			printerr("Bad SFgetopen",err);
	}

	else			/*user did press Cancel Button*/
		{
		refNum = NULL;
		fNamePtr[0] = NULL;
		}
	
	return (refNum);
}
