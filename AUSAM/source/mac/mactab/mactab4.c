#include	"../mac/mac.h"
#include	"mactab.h"
#include	"mactab.x"


plit()
{
	while (getlin())  {
		getsym();
		if (sym != LBL)  {
			error("label expected", 0);
			continue;
			}

		/*
		 *   Copy into literal table
		 */

		copy(clabel, literals[nlit]);

		nlit++;
		if (nlit >= NLIT)
			break;
		}

	head.h_literals = nlit;
	return;
}
