/* typewriter driving table structure*/
#ifdef NROFF
extern struct {
/***
 *** Initial stty() to set up terminal
 ***/
	int bset;	/*** bits to set  ***/
	int breset;	/*** bits to reset  ***/
/***
 *** Terminal resolution - measured in 1/240 inch units
 ***/
	int Hor;	/*** horizontal resolution ***/
	int Vert;	/*** vertical resolution ***/
	int Newline;	/*** vertical motion produced by newline ***/
	int Char;	/*** horizontal motion produced by one character ***/
	int Em;		/*** width of em ***/
	int Halfline;	/*** vertical motion produced by halfline ***/
	int Adj;	/*** resolution for space adjusting ***/
/***
 *** Special character sequences
 ***/
	char *twinit;	/*** initialize terminal ***/
	char *twrest;	/*** restore terminal to pre-nroff state ***/
	char *twnl;	/*** new line ***/
	char *hlr;	/*** half line upwards ***/
	char *hlf;	/*** half line downwards ***/
	char *flr;	/*** full line upwards ***/
	char *bdon;	/*** escape to "bold" character mode (I think?) ***/
	char *bdoff;	/*** return from "bold" character mode ***/
/***
 *** "Plot" mode:
 ***	used to build composite characters and for high-resolution motions.
 ***	It is assumed that printing a character while in plot mode does
 ***	not move the cursor.
 ***/
	char *ploton;	/*** start plot mode ***/
	char *plotoff;	/*** end plot mode ***/
	char *up;	/*** move up	one plot increment ***/
	char *down;	/***  "	down	 "    "      "     ***/
	char *right;	/***  "	right	 "    "	     "     ***/
	char *left;	/***  "	left	 "    "	     "     ***/
/***
 *** Character set:
 ***	The first byte of each string means:
 ***		1... ....	underline this character in .ul mode
 ***		.xxx xxxx	actual width of character (in ems)
 ***	Within the string a character >=0200 means "plot" motion:
 ***		100. ....	move right
 ***		101. ....	move left
 ***		110. ....	move down
 ***		111. ....	move up
 ***		...x xxxx	number of plot increments to move
 ***	The first character of a plot string must be >= 0200.
 ***/
	char *codetab[256-32];
	int zzz;	/*** dummy to mark end of structure ***/
	} t;
#endif
