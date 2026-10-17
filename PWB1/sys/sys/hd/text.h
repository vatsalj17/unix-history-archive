/*
 * Text structure.
 * One allocated per pure procedure on swap device.
 * Manipulated by text.c
 */
struct text
{
	int	x_daddr;	/* disk address of segment */
	int	x_caddr;	/* core address, if loaded */
	int	x_size;		/* size (*64) */
	int	*x_iptr;	/* inode of prototype */
	char	x_count;	/* reference count */
	char	x_ccount;	/* number of loaded references */
	char	x_flag;		/* traced, written flags */
};

extern struct text text[];

#define	XWRIT	02		/* Text written into, must swap out */
#define	XLOAD	04		/* Currently being read from file */
#define	XLOCK	010		/* Being swapped in or out */
#define	XWANT	020		/* Wanted for swapping */
