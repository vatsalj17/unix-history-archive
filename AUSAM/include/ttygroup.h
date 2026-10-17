
/*	terminal group definitions	*/

struct
{
	char	  tchar;	/* this char identifies this terminal group */
	unsigned  tmask;	/* 16 bit mask for this group */
}
tty_groups[]
{
#ifdef	AGSM
	'a', 0000001,	/* room 422 */
	'b', 0000002,	/* room 436 - not qume */
	'c', 0000004,	/* the qume */
	'd', 0000010,	/* room 437 - ian J - andrew H */
	'e', 0000020,	/* 662-1666 dialup lines(4) */
	'f', 0000040,	/* 662-1073, 662-1047 dialup lines(4) */
	'g', 0000100,	/* console - tty8 */
	'h', 0000200,	/* library terminals */
	'i', 0000400,	/* not used */
	'j', 0001000,	/* LOG line from csu */
	'k', 0002000,	/* LOG line from elec */
	'l', 0004000,	/* pges */
	'm', 0010000,	/* psychology */
	'n', 0020000,	/* mining */
	'o', 0040000,	/* commerce */
	'p', 0100000,	/* basser */
#endif	AGSM
#ifdef	EECF
	'a', 0000001,	/* Rm 318A */
	'b', 0000002,	/* Rm 318C */
	'c', 0000004,	/* Rm 343C */
	'd', 0000010,	/* Rm 343B console & vt05 */
	'e', 0000020,	/* Rm 343D Digital systems lab */
	'f', 0000040,	/* Log line from 11/40 */
	'g', 0000100,	/* Log line from AGSM */
	'h', 0000200,	/* Rm 313-4-5 DCS laboratory */
	'i', 0000400,	/* GT40 */
	'j', 0001000,	/* Tektronics gear, Rm 343C */
	'k', 0002000,	/* Qume - when priority work to be done */
	'l', 0004000,	/* Spare */
	'm', 0010000,	/* Spare */
	'n', 0020000,	/* Spare */
	'o', 0040000,	/* Spare */
	'p', 0100000,	/* Spare */
#endif	EECF
	'x', 0177777,	/* The lot - any user can use it */
};

#define	NTTYGRPS	(sizeof tty_groups/sizeof tty_groups[0])
