#include	"mac.h"
#include	"mac.x"


pass2()
{
	register int fl;		/* opcode flags */
	register int op;
	register int lc;
	register int i;

	eof = FALSE;

	while (!eof)  {

		/*
		 *   Get record from temp file
		 */
		i = read(unit, &intercode, IT);
		if (!i)
			/*  End of processing  */
			return(0);

		nline++;
		/*
		 *   Get flags (opcode or pseudo)
		 *	 op value, current location counter
		 *	 (for listings) and current assembled
		 *	 code pointer (for listings).
		 */
		fl = intercode.i_flags;
		op = intercode.i_op;
		lc = locn[lcntr].l_value;
		cp = locn[lcntr].l_next;



		if (fl & OP)  {

			/*
			 *   Process opcode assembly
			 */
			gocode(op);
			olist(lc);
			continue;
			}


		if (fl & PS)  {

			/*
			 *   Process pseudo op.
			 */
			gpcode(op);
			plist(lc);
			continue;
			}


		/*
		 *   May be a null line, just a comment line,
		 *   a label equate, (to pc) or a dud.
		 */

		tlist(lc);
		continue;


		}

	return;
}
