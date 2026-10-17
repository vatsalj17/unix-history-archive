/* LINTLIBRARY */
/*
 * mayput.c
 *
 * Sun MC68000 ROM monitor Emulator Trap package
 *
 * int emt_mayput(c)	-- prints character c on Console if possible
 *			   otherwise returns -1
 */

#include <sunemt.h>

int emt_mayput(c)
char c;
{

/* the proper way
	return(
	    emt_call(EMT_MAYPUT,c)
	);

a bad way, will do till EMT_MAYPUT is installed in all proms */
	if (*(short *)0x400000 == 0x5375)  /* if SMI */
	    return emt_call(EMT_MAYPUT,c);
	else if (!linereadytx(0)) return -1;
	else {
		lineput(0,c);
		return 0;
	}
}

