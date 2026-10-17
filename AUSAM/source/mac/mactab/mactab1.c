#include	"../mac/mac.h"
#include	"mactab.h"
#include	"mactab.x"

main(argc, argv)
char	*argv[ ];
{
	register int i,j;

	if (argc > 1)  {
		close(0);
		if (open(argv[1], 0) < 0)  {
			error("can't open %s", argv[1]);
			exit(1);
			}
		}

	if (argc > 2)  {
		fd = creat(argv[2], 0666);
		if (fd < 0)  {
			error("can't create %s", argv[2]);
			return;
			}
		}


	/*
	 *   The parser description is made up of several parts.
	 *
	 *	The first symbol seen must be a label, and if not,
	 *	it is a fatal error. the label's value determines
	 *	the type of section to follow. the section is
	 *	terminated by an eof (% in col 1).
	 */

	for (;;)  {			/* get all segs */

		getlin();
		getsym();		/* initial label */

		if (sym == EOF)  {
			pend();
			error("no end section", 0);
			return;
			}

		if (sym != LBL)  {
			error("section descriptor needed", 0);
			exit(1);
			}

		/*
		 *   Decode descriptor and call handler.
		 */

		for (i=0; lvec[i]; i++)
			if (compar(lvec[i], clabel))  {
				fn = avec[i];
				(*fn)();
				break;
				}

		if (!lvec[i])  {
			error("section %s not found", clabel);
			exit(1);
			}

		}

	return;
}
