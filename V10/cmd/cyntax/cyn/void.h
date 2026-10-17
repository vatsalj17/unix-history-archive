#define isvoid(t)	(((t)->x_what==xt_basetype)&&(t->x_value.i&VOID))
#define isvoidp(t)	(((t)->x_what==xt_ptrto)&&isvoid((t)->x_left))
