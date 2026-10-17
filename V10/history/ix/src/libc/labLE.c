#include <sys/label.h>

labLE(x, y)
register struct label *x, *y;
{
	register i;
	if(x == 0 || y == 0)
		return 0;
	if (x->lb_flag == L_YES || y->lb_flag == L_YES)
		return 1;
	if (x->lb_flag == L_NO || y->lb_flag == L_NO)
		return 0;
	if (x->lb_flag == L_UNDEF || y->lb_flag == L_UNDEF)
		return 0;
	for(i=0; i<LABSIZ; i++)
		if(x->lb_bits[i] & ~ y->lb_bits[i])
			return 0;
	return 1;
}
