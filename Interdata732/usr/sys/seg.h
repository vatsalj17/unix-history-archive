/*
 * INTERDATA SOFTWARE USER SEGMENTATION REGISTERS
 */

extern int	*uisa;
extern int	*kisa;

/*
 * masks for selecting parts of segmentation register
 */

#define	SEGMASK	03777400	/* segment start address */
#define	SEGEP	0200	/* execute protect */
#define	SEGIP	0100	/* write-interrupt protect */
#define	SEGWP	040	/* write protect */
#define	SEGPRES	020	/* present */

/*
 * structure used to address
 * a sequence of integers.
 */
struct
{
	int	r[];
};
extern int	*ka6;		/* pointer to seg reg for u segment */
