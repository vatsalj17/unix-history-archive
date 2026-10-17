#include	"../mac/mac.h"
#include	"mactab.h"
#include	"mactab.x"


/*
 *   Machine header section.
 *
 *   Contains a set necessary data items describing
 *   the target machine's architecture.
 */
phed()
{
	register char *r;
	register char cc;
	register int ndc;
	register int i;

	ndc = 0;			/* number of dc?'s */
	head.h_page = 60;		/* def. page size */

	while (getlin())  {

		getsym();
		if (sym != LBL)  {
			error("symbolic needed", 0);
			continue;
			}

		if (compar("dc", clabel))  {
			if (ndc >= 4)  {
				error("only 4 dc's allowed", 0);
				continue;
				}

			getsym();
			if (sym != LBL && sym != LIT)  {
				error("dc char missing", 0);
				continue;
				}

			head.dctype[ndc].f_class = clabel[0];

			while (*p == ' ' || *p == '\t')
				p++;

			head.dctype[ndc].f_len = fmtscan(&head.dctype[ndc]);

			ndc++;
			continue;
			}

		if (compar("defmt", clabel))  {
			head.dctype[4].f_class = '\0';
			while (*p == ' ' || *p == '\t')
			p++;
			head.dctype[4].f_len = fmtscan(&head.dctype[4]);
			continue;
			}

		if (compar("mac", clabel))  {
			getsym();
			if (sym != STR)  {
				error("string needed", 0);
				continue;
				}

			copy(clabel, head.h_mac);
			continue;
			}

		if (compar("page", clabel))  {
			getsym();
			if (sym != CON)  {
				error("page length required");
				continue;
				}

			head.h_page = mem;
			continue;
			}

		if (compar("byte", clabel))  {
			getsym();
			if (sym != CON || mem <= 0)  {
				error("funny byte width %d", mem);
				continue;
				}

			head.h_bu_len = mem;
			continue;
			}

		if (compar("word", clabel))  {
			getsym();
			if (sym != CON || mem <= 0)  {
				error("funny word width %d", mem);
				continue;
				}

			head.h_w_len = mem;
			continue;
			}

		if (compar("ii", clabel))  {
			getsym();
			if (sym != CON)  {
				error("ill. instr value needed", 0);
				continue;
				}

			head.h_ii = mem;
			continue;
			}

		if (compar("pc", clabel))  {
			getsym();
			if (sym != LBL)  {
				error("label needed", 0);
				continue;
				}

			if (compar("pre", clabel))
				head.h_pc_post = FALSE;
			else
				if (compar("post", clabel))
					head.h_pc_post = TRUE;
				else
					error("pre/post pc only", 0);

			continue;
			}

		error("bad header descriptor %s", clabel);
		}

	return;
}
