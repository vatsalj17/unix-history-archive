#include	"../mac/mac.h"
#include	"mactab.h"
#include	"mactab.x"


plab()
{
	register int i;

	for (i=0; i<NSYM; i++)  {
		if (!getlin())
			break;

		getsym();
		if (sym != LBL)  {
			error("pre-definition - label needed", 0);
			continue;
			}

		copy(clabel, &symtab[i].s_name[0]);

		getsym();
		if (sym != CON)  {
			error("defined label - value required",0);
			mem = 0;
			}

		symtab[i].s_value = mem;
		symtab[i].s_mode  = ABS | DEFN;
		}

	nsym = i;
	head.h_labels = i;

	return;
}
