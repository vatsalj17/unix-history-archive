#include	"../mac/mac.h"
#include	"mactab.h"
#include	"mactab.x"

extern	cmp();


pops()
{
	register char *op;
	register int i;
	register int len;
	register int class;

	getlin();
	getsym();
	if (sym != LBL && !compar("class", clabel))  {
		error("invalid symbol class", 0);
		exit(1);
		}

	getsym();
	if (sym != CON)  {
		error("bad conversion", 0);
		exit(1);
		}

	class = mem;
	len = 8 + INT + (2 * INT * class);
	opcode = sbrk(128 * len);
	if (opcode < 0)  {
		error("reduce table size", 0);
		exit(1);
		}
	head.h_o_len = len;		/* sizeof (struct st) */

	op = &opcode[0].o_name[0];
	while (getlin())  {
		getsym();
		if (sym != LBL)  {
			error("bad opcode %s", clabel);
			continue;
			}

		copy(&clabel[0], &op->o_name[0]);

		for (i=0; i<class; i++)  {
			getsym();
			if (sym != CON)  {
				error("format descriptor required");
				break;
				}
			op->o_code[i].o_format = mem;

			getsym();
			if (sym != CON)  {
				error("opcode value needed");
				break;
				}
			op->o_code[i].o_value = mem;
			}

		op =+ len;
		nops++;
		}

	head.h_ops = nops;

	/*
	 *   Sort opcodes for binary search.
	 */
	qsort(&opcode[0], nops, len, &cmp);
	return;
}
