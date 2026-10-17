/* LINTLIBRARY */
/*
 * mayget.c
 *
 * Sun MC68000 ROM monitor Emulator Trap package
 *
 * char emt_mayget()	-- returns character read from console keyboard
			   if one is there, else -1 (nonblocking getchar)
 */

#include <sunemt.h>

char emt_mayget()
{

/* the proper way
	return(
	    emt_call(EMT_MAYGET)
	);

the improper way, will do till EMT_MAYGET is installed */
	if (*(short *)0x400000 == 0x5375)  /* if SMI */
	    return emt_call(EMT_MAYGET);
	else if (!linereadyrx(0)) return -1;
	else return lineget(0)&127;
}
