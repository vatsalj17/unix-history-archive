/*
 * sroff driving tables
 * Messenger 10
 */
#include "../sdef.h"

/*
 * Only the first PFONT physical fonts (phys = 0,1,...,PFONT-1)
 * can be proportional and their character widths are in fontab[phys].
 * If a value for phys is >= PFONT and has PROPOR or'ed in,
 * a nonexistent fontab will be consulted for widths, with amusing
 * results.
 * Special characters (values greater than 0177) are in fontabS.
 * The special characters assume that the scientific font is on position F
 * (phys = 6) and the greek/math font is on position G (phys = 7).
 * All values are in units (mils, since INCH is 1000).
 * fonts, ulfont, fontab and fontab1 are all overlaid by the -F option.
 */

struct fonts {		/* info for all fonts to be mounted */
	char lab;	/* name */
	char phys;	/* sanders logical font #, |= PROPOR if proportional */
	int Char;	/* character width if nonproportional */
	int Em;		/* character size, value of 1m */
			/* (point size) x (1000/72)    */
	char mount[10];	/* sanders font number */
} fonts[NFONTS] = {
	{'R', 0,	100, 139, "X5011010"},	/* 10-pitch (Messenger 10)*/
	{'I', 0,	83,  139, ""},
	{'B', 0,	83,  139, ""},		/* emboldened Messenger 12 */
	{'S', 6,	100, 139, "S5480210"},	/* Sci-Pi */
	{ 0,  3,	100, 139, ""},		/* unused */
	{'M', 4,	83,  139, "X5010608"},	/* 12-pitch (Messenger 12)*/
	{ 0,  5,	100, 139, ""},		/* unused */
	{'G', 7,	100, 139, "S5480310"}	/* Greek */
};

int ulfont = -1;
int bdtab[NFONTS] = {		/* amount of emboldening */
	0,
	0,
	7,
	0,
	0,
	0,
	0,
	0
};
int smnt = 3;			/* font 'S' is in fonts[smnt] */
int sbold = -1;

int fontab[PFONT][0200-040] = {
{
			/* widths for font 0 PROPOR */
#include "Nonpropor"
},

{
			/* character widths for font 1 PROPOR */
	0,	/*unused*/
},

{
			/* font 2 PROPOR */
	0,	/*unused*/
}
};

int fontabS[MAXCHAR-0200] = {

#include "Special"
};
