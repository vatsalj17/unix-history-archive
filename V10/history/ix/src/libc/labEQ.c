#include <sys/label.h>

labEQ(x,y)
register struct label *x, *y;
{
	register int i;
	
	if(x == 0 || y == 0)
		return  0;
	if(x->lb_flag != y->lb_flag)
		return 0;
	if(x->lb_flag == L_BITS)
		for(i=0; i<LABSIZ; i++)
			if(x->lb_bits[i] != y->lb_bits[i])
				return 0;
	return 1;
}
