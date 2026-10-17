#define INCH 240
/*
Carousel 300 with Electronic Format Control
nroff driving tables
width and code tables
*/

struct {
	int bset;
	int breset;
	int Hor;
	int Vert;
	int Newline;
	int Char;
	int Em;
	int Halfline;
	int Adj;
	char *twinit;
	char *twrest;
	char *twnl;
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
	char *codetab[256-32];
	int zzz;
	} t {
/*bset*/	0,
/*breset*/	0,
/*Hor*/		INCH/10,
/*Vert*/	INCH/12,
/*Newline*/	INCH/6,
/*Char*/	INCH/10,
/*Em*/		INCH/10,
/*Halfline*/	INCH/12,
/*Adj*/		INCH/10,
/*twinit*/	"",
/*twrest*/	"",
/*twnl*/	"\r\033D\033D",		/*** Note: changed from "\n" because of
					 *** hardware peculiarity of Carousel
					 *** which only affects half-line
					 *** motions
					 ***/
/*hlr*/		"\033d",
/*hlf*/		"\033D",
/*flr*/		"\0337",
/*bdon*/	"",
/*bdoff*/	"",
/*ploton*/	" \033P",	/* set horiz pitch to 0 */
/*plotoff*/	"\033p\b",	/* horiz pitch to .1 inch */
/*up*/		"\033a",
/*down*/	"\033A",
/*right*/	"\033Q \033P",	/* horiz pitch .01 */
/*left*/	"\033Q\b\033P",
/*codetab*/
#include "code.300"
