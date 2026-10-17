/* $Header: /ker/coh.386/RCS/fifo.c,v 2.5 93/10/29 00:55:09 nigel Exp Locker: nigel $ */
/*
 * $Log:	fifo.c,v $
 * Revision 2.5  93/10/29  00:55:09  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.4  93/08/19  03:26:26  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Revision 2.3  93/07/26  14:59:42  nigel
 * Nigel's R80
 * 
 * Revision 1.3  93/04/14  10:06:27  root
 * r75
 * 
 * Revision 1.2  92/01/06  11:59:11  hal
 * Compile with cc.mwc.
 */

#include <kernel/typed.h>


/*
 * Arguments are passed into the kernel through boot_gift.
 * If you start getting "Not enough room for all arguments." messages
 * at boot time, just increase the BG_LEN  to whatever you need.
 * This structure is EXACTLY BG_LEN bytes long.
 */

TYPED_SPACE(boot_gift, BG_LEN, T_FIFO_SIC);

/*
 * fifo_open()
 *
 * Open a typed space as a fifo.
 *
 * Takes a typed_space that is already allocated, and a mode.    The type of
 * the typed space must be a FIFO.  Only T_FIFO_SIC has been implemented
 * (static, in-core fifo).
 *
 * The mode indicates whether to open for reading or writing.
 *  mode == 0 means read only.
 *  mode == 1 means write only.
 *  Other values are illegal.
 *
 * Returns a pointer to an initialized FIFO structure.  FIFO structures are
 * allocated from a pre-allocated array.  Returns F_NULL if it can't open
 * the fifo.
 */
FIFO *
fifo_open(fifo_space, mode)
typed_space *fifo_space;
int mode;
{
	/* ff_table is a table of FIFO structures which can be allocated on
	 * demand.  It is functionally similiar to the file descriptor table
	 * in the kernel.
	 */
	static FIFO ff_table[NFIFOS];
	static int inited = 0;	/* Has ff_table been initialized?  */

	int i;		/* A handy counter.  */
	FIFO *the_fifo;	/* The fifo we are going to allocate.  */

	/* Initialize ff_table the first time we get called.  */
	if (! inited) {
		for (i = 0; i < NFIFOS; ++i) {
			ff_table[i].f_space = F_NULL;
			ff_table[i].f_flags = 0;
		}
		inited = 1;
	}

	/* Check the type of the space we were passed.  */
	switch (fifo_space->ts_type) {

	case T_FIFO:	/* Overly general type, assuming SIC.  */
		fifo_space->ts_type = T_FIFO_SIC;
		break;

	case T_FIFO_SIC:	/* Static In-core Fifo.  */
		break;

	case T_FIFO_DIC:	/* Dynamic In-core Fifo (can grow).  */
		return F_NULL;	/* Unimplemented.  */

	case T_FIFO_SP:	/* Static Permanent Fifo (fixed size file). */
		return F_NULL;	/* Unimplemented.  */

	case T_FIFO_DP:	/* Dynamic Permanent Fifo (ordinary file).  */
		return F_NULL;	/* Unimplemented.  */

	default:
		return F_NULL;	/* Illegal type encountered.  */
	}

	/* ASSERTION: fifo_space is a valid and implemented FIFO.  */

	/* Find the first free FIFO structure.  */

	/* This should be re-implemented using a malloc-based scheme.
	 * At the moment, the tertiary boot libraries do not include a
	 * malloc.
	 */
	for (i = 0 ; i < NFIFOS && ff_table[i].f_flags != 0 ; i ++)
		/* DO NOTHING */ ;

	if (NFIFOS == i)
		return F_NULL;	/* No more free fifo structs.  */

	the_fifo = ff_table + i;

	/* ASSERTION: the_fifo points at a FIFO we can take.  */

	/* Initialize the FIFO struct.  */
	the_fifo->f_space = fifo_space;
	the_fifo->f_offset = fifo_space->ts_data;

	/* Initilize the flags.  */
	switch(mode) {
	case 0:	/* read */
		the_fifo->f_flags |= F_READ;
		break;
	case 1:	/* write */
		the_fifo->f_flags |= F_WRITE;
		break;
	default:
		return F_NULL;	/* Illegal mode flag.  */
	}
		
	return the_fifo;
}

/*
 * fifo_close()
 *
 * Finish with using a typed space as a fifo.
 * Free up FIFO structure associated with a typed space.
 * Returns 0 if ffp was not open, 1 otherwise.
 */
int
fifo_close(ffp)
FIFO *ffp;
{
	if (0 == ffp->f_flags)
		return 0;	/* This ffp is not open.  */

	ffp->f_space = F_NULL;
	ffp->f_offset = 0;
	ffp->f_flags = 0;

	return 1;
}


/*
 * fifo_read()
 *
 * Read a typed space from a fifo.
 * Return a pointer to the next typed space in the fifo ffp.  Returns
 * NULL on end of fifo.
 *
 * This read assumes that ffp->f_space has type T_FIFO_SIC.
 */

typed_space *
fifo_read(ffp)
register FIFO *ffp;
{
	typed_space *retval;

	/* Read MUST be set.  */
	if (F_READ != F_READ & ffp->f_flags ) {
		printf(" fifo_read: READ not set ");
		return 0;  /* This ffp is not open for reading.  */
	}

	/* From here to the end of fifo_read is really fifo_read_sic().  */


	/* Space of size 0 marks EOFIFO.  */
	if (ffp->f_offset->ts_size == 0) {
		printf(" fifo_read: space of size 0 ");
		retval = 0;
	} else {
		/* Return the next space.  */
		retval = ffp->f_offset;
		/* Advance to the next space.  */
		(char *) ffp->f_offset += ffp->f_offset->ts_size;
	}

	return retval;
}
