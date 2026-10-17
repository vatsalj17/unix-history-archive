/*
 * data structures for the MSCP disk class driver
 */

struct radisk {
	struct msportsw *port;
	short open;
	char flags;
	char rplret;
	long cmdcrf;		/* crf of pending non-xfer command */
	char cmdopc;		/* opcode too */
	struct buf *actf;	/* next buffer to send */
	struct buf *actl;	/* end of list of unsent buffers */
	struct ud_unit di;	/* size et al parameters */
};
