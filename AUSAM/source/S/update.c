/*
 *	update for "disk" integrity and "lnode" checkpointing.
 */

#include	<param.h>
#include	<passwd.h>
#include	<lnode.h>

#define	CHECKPOINTTIME	(15*60)	/* minutes between lnode checkpoints */
#define	SYNCTIME	(5*60)	/* maximum minutes between syncs */
#define	SYNCFACTOR	32	/* multiply time for last sync
					   by this to get sleep time
					   also min sleep time      */

char chkptf[] "/etc/lnodes.chkpt";

terminate()
{
	signal( 14 , 1);		/* no further signals please */
	clktim(0);
	unlink( chkptf );
	sync();
	exit();
}

checkpoint()
{
	struct lnode lnodes[ MAXUSERS ];
	register n;

	signal( 15 , checkpoint );	/* catch real-time limit */
	clktim( CHECKPOINTTIME );
	if( creat( chkptf , 0600 ) == 0 ) {
		if( n = limits( &lnodes , L_ALLLIM ) )
			write( 0 , &lnodes , n * sizeof lnodes[0] );
		close(0);
	}
}

main()
{
	long time(), syntim;
	register tim;

	signal(14,terminate);	/* clean up and exit when system shutsdown */
	close(0); close(1); close(2);

	checkpoint();
	for(;;) {
		syntim = time();
		sync();
		tim = time() - syntim;
		if( tim ) {
			tim = SYNCFACTOR * tim;
			if( tim > SYNCTIME ) tim = SYNCTIME;
		} else tim = SYNCFACTOR;
		sleep( tim );
	}
}
