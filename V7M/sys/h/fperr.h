/*
 * structure of the floating-point error register save/return
 */

struct	fperr {
	int	f_fec;	/* floating error code */
	caddr_t	f_fea;	/* floating error address */
};
