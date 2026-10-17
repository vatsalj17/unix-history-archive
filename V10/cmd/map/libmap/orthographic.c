#include "map.h"

int
Xorthographic(struct place *place, double *x, double *y)
{
	*x = - place->nlat.c * place->wlon.s;
	*y = - place->nlat.c * place->wlon.c;
	return(place->nlat.l<0.? 0 : 1);
}

proj
orthographic(void)
{
	return(Xorthographic);
}
