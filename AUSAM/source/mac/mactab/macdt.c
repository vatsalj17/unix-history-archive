#include	"../mac/mac.h"


/*
 *   MACDT:
 *
 *   MAC temporary file decoder.  This routine is useful
 *   for decoding and symbolically printing the contents
 *   of MAC temp. files, left around by using the '-u'
 *   option on the MAC control statement.
 *
 */


				/* intercode struct */
struct	it	i;


main(argc, argv)
char	*argv[ ];
{
	register int fd;
	register int n;

	n = 0;			/* line counter */

	if (argc < 2)
		fd = open("mactemp", 0);
	else
		fd = open(argv[1], 0);

	if (fd < 0)  {
		printf("can't open temp\n");
		exit(1);
		}

	while (read(fd, &i, IT))  {

		printf("%3d: ", ++n);
		printf("%8x  ", i.i_flags);
		if (i.i_label == ERR)
			printf("       ");
		else
			printf("$%-5d ", i.i_label);
		printf("%5d ", i.i_op);
		printf("(%3d) ", i.i_loc);
		printf("%1x %2d %2d %2d ",
			i.i_selc[0], i.i_selc[1],
			i.i_selc[2], i.i_selc[3]);
		printf(":%s\n", i.i_opr);
		}

	return;
}
