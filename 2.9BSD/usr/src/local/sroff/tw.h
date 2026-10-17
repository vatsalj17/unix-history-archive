/* sanders driving table structure*/

extern struct fonts {
	char lab;
	char phys;
	int Char;
	int Em;
	char mount[10];
} fonts[NFONTS];

extern struct {
#ifndef TTY
	int bflags;
	int bspeed;
#else
	int bset;
	int breset;
#endif
	int Newline;
	int Halfline;
	char *twinit;
	char *twrest;
	char *twnl;
#ifdef ADJ
	char *twfp;
#endif
	char *hlr;
	char *hlf;
	char *flr;
	char *bdon;
	char *bdoff;
	char *ploton;
	char *plotoff;
	char *up;
	char *down;
	char *right;
	char *left;
	char *ulon;     /* underlining enables */
	char *uloff;    /* underlining disabled */
	int zzz;
} t;
