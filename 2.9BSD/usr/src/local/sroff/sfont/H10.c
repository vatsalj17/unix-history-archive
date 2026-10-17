/*
 * sroff driving tables
 * H10 (Helvesan 10 and Helvesan 10 Italic)
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
	{'R', 0|PROPOR, 0,   139, "X5021210"},	/* Helvesan 10 */
	{'I', 1|PROPOR, 0,   139, "X5022110"},	/* Helvesan 10 Italics */
	{'B', 0|PROPOR, 0,   139, ""},		/* emboldened Helvesan 10 */
	{'S', 6,	100, 139, "S5480210"},	/* Sci-Pi */
	{'M', 4,	83,  139, "X5010608"},	/* 12-pitch (Messenger 12)*/
	{'F', 2|PROPOR, 0,   139, "X5020508"},	/* Helvesan 8 */
	{'L', 5,	168, 250, "X5030120"},	/* Presentation 6 */
	{'G', 7,	100, 139, "S5480310"}	/* Greek */
};

int ulfont = -1;
int bdtab[NFONTS] = {		/* amount of emboldening */
	0,
	0,
	6,
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
#include "Helvesan10"
},

{
			/* character widths for font 1 PROPOR */
#include "Helvesan10Ital"
},

{
			/* font 2 PROPOR */
#include "Helvesan8"
}
};

int fontabS[MAXCHAR-0200] = {

#include "Special"
};
