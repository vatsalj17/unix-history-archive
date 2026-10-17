#include	"../mac/mac.h"
#include	"mactab.h"
#include	"mactab.x"


pend()
{
	if (nerr)  {
		error("errors encountered: %d", nerr);
		exit(1);
		}

	write(fd, &head, HT);
	if (nlit)
		write(fd, &literals[0][0], nlit*8);
	write(fd, &format[0], nfmt * FD);
	write(fd, &opcode[0], nops * head.h_o_len);
	write(fd, &parse[0], head.h_p_len * TBL);
	if (nsym)
		write(fd, &symtab[0], ST * nsym);

	close(fd);
	exit(0);
}
