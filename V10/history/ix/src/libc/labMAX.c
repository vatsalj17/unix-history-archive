#include <sys/label.h>
extern char *memset ();

struct label
labMAX(x, y)
register struct label *x, *y;
{
	register int i;
	struct label result;
	static struct labpriv nopriv;

	result.lb_priv = nopriv;
	if (x == 0 || y == 0)
		result.lb_flag = L_NO;
	else if (x->lb_flag == L_UNDEF || y->lb_flag == L_UNDEF)
		result.lb_flag = L_NO;
	else if (x->lb_flag == L_BITS && y->lb_flag == L_BITS) {
		result.lb_flag = L_BITS;
		for(i=0; i<LABSIZ; i++)
			result.lb_bits[i] = x->lb_bits[i] | y->lb_bits[i];
		return result;
	} else if (x->lb_flag == L_YES || y->lb_flag == L_NO)
		result.lb_flag = y->lb_flag;
	else	/* x->lb_flag == L_NO || y->lb_flag == L_YES */
		result.lb_flag = x->lb_flag;
	memset((char*)result.lb_bits, 0, LABSIZ);
	return result;
}
