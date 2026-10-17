/*
 * Text structure.
 * One allocated per pure
 * procedure on swap device.
 * Manipulated by text.c
 */
struct text
{
#ifdef SHARED_DATA | LOWER_TEXT_SWAPS
	int	x_flag;		/* flags for locking etc */
#endif
	int	x_daddr;	/* disk address of segment */
	int	x_caddr;	/* core address, if loaded */
	int	x_size;		/* size (*64) */
	int	*x_iptr;	/* inode of prototype */
	char	x_count;	/* reference count */
	char	x_ccount;	/* number of loaded references */
#ifdef SHARED_DATA
	int	*x_spid;	/* shared data identification */
				/* must be same type as 'x_iptr' */
#endif
} text[NTEXT];


	/*
	 * flag codes
	 */

#define TXTBUSY	1
#define TXTWANT	2
