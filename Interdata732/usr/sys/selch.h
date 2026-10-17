/*
 * Information about a device waiting to use a selector channel
 */

struct selchq {
	int	(*sq_sstart)();		/* startup routine when selch is free */
	int	(*sq_sintr)();		/* selch interrupt routine */
	struct selchq	*sq_forw;	/* next device waiting for channel */
};

/*
 * Selector channel address & commands
 */
extern char	selchaddr[];

#define	STOP	0x48
#define	GO	0x50
#define	READ_GO	0x70
