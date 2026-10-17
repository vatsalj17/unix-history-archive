#include	<stdio.h>
FILE *debf;



/*			CPLOT,	A graphics package for Tektronix terminals.

				By Adrian Freed, Richard Grevis, and Graeme Elsworthy.

*/
#define	FF	12
#define NUM_DASHES 10
#define	GS	29	/*	Graphics mode	*/
#define SYNC  026
#define US	31	/*	Alpha    mode	*/
#define MASK   037	/*	00011111	*/
#define ESC    033      /*	Control code preamble */
#define BELL	07
#define ENQUIRE	"\033\005"
#define BOTTOM	2,1
#define CURSORS	"\033\032"
#define CURSREAD "\033\032"


#define ALPHA 1
#define ONSCREEN	2
#define USER	4
#define SCREEN		010
#define	DASHMOVE	010000
#define	HIY	0040	/*   100000	*/
#define	LOY	0140	/*  1100000	*/
#define	HIX	0040	/*   100000	*/
#define	LOX	0100	/*  1000000	*/

#define CURSBUFSIZE 8
#define	HOMEX	0
#define	HOMEY	767	/*	Home Co-ordinates	*/

#define	SXMAX	1023
#define	SYMAX	780	/*	Max. Screen positions	*/

#define	YCHSZ	22	/*	Character height	*/
#define XCHSZ	14	/*	Character width		*/


/*	TTY	masks	*/

#define	MAPCASE	04
#define	ECHO	010
#define	LFCR	020
#define	RAWMODE	040
#define	NLDELAY	01400
#define	FF1	040000



#define DEGTORAD 0.01745329




float g_xlo, g_ylo, g_xhi, g_yhi;	/*	User space descriptors	*/
int g_sxlo, g_sylo, g_sxhi, g_syhi;	/*	Screen descriptors	*/
float g_sxtrans, g_sytrans, g_uxtrans, g_uytrans;


int	g_spx, g_spy;	/*	Current screen position		*/	
int	g_sax, g_say;
float	g_px, g_py;		/*	Current user Co-ord. position	*/

int	g_sqx1, g_sqy1;	/*	First solved point	*/
int	g_sqx2, g_sqy2;	/*	Second solved point	*/

int	g_b1, g_b2, g_b3, g_b4;	/*	Bytes to be drawn	*/
int	g_nekmod;
int g_term;

/*
** The global variables used with the dash line routines
*/
int g_scan;
float g_dashbit ;
float g_xpos, g_ypos;
extern g_dashtable[NUM_DASHES][4];
float g_xold, g_yold;


FILE *g_out;
struct	g_ttystruct
{
	char ispeed, ospeed;
	char e, k;
	int mode;
} g_ttybuf, g_ttyb, g_outbuf;
int	g_syncs;

int	g_positioned;
/* SCALE AND ROTATION ON RELATIVE MOVES AND DRAWS */


float g_xsin, g_ysin, g_xcos, g_ycos;
float g_xscale, g_yscale;
int	g_status,g_last;

/*
**	This structure allocates the bit representation
**	of a data word in the software character set.
*/

struct	schar
{
	int vstat	:1;
	int deltax	:6;
	int deltay	:6;
};
#define		erase()		( g_put(ESC), g_put(FF),g_status =| ALPHA, g_status =& ~ONSCREEN )
#define		bell()		( g_alpha(), g_put( BELL ) )
#define		smovea(x,y)	( g_spx = (x), g_spy = (y), g_status =& ~(USER|ONSCREEN) )
#define		home()		smovea(HOMEX, HOMEY )
#define		syncs(x)	( g_syncs = (x) )
#define		sdrawr(x,y)	sdrawa( g_spx + (x), g_spy + (y) )
#define		sdashr(x,y,t)	sdasha( g_spx + (x), g_spy + (y) , (t))
#define		spointr(x,y)	spointa( g_spx + (x), g_spy + (y) )
#define		upointr( x,y ) 	( g_status & USER ) ?0: g_chtusr( g_spx, g_spy ), upointa( g_px+x, g_py+y )
#define		umovea(x,y)	( g_chtscr( g_px = (x), g_py = (y) ), g_status =| (USER), g_status =& ~ONSCREEN )
#define		udrawr(x,y)	( ( g_status & USER ) ?0:  g_chtusr( g_spx, g_spy ), udrawa(g_px + (x),g_py + (y)) )
#define		udashr(x,y,t)	( ( g_status & USER ) ?0:  g_chtusr( g_spx, g_spy ), udasha(g_px + (x),g_py + (y), (t)) )
#define		cursor()	( g_alpha(), fputs( CURSORS, stderr) )
#define		swhere(x,y)	( *(x) = g_spx, *(y) = g_spy )
#define		uwhere(x,y)	( (g_status & USER ) ?0: g_chtusr( g_spx, g_spy ), *(x) = g_px, *(y) = g_py )
#define		sxputsr(x,y,s)	sxputsa( g_spx + (x), g_spy + (y), s )
#define		uxputsr(x,y,s)	uxputsa( g_spx + (x), g_spy + (y), s )
#define		syputsr(x,y,s)	syputsa( g_spx + (x), g_spy + (y), s )
#define		uyputsr(x,y,s)	uyputsa( g_spx + (x), g_spy + (y), s )
