#

/*
 *	copyright 1975 ace systems inc.
 */

/*
 *	ta-11 cassette driver
 */

#include "../param.h"
#include "../conf.h"
#include "../buf.h"
#include "../user.h"

int	taopenl[2];

int tabc,tabcz;  char *taba,tabaz;  int tactv;  int taerr;  taerrcnt;

/*	symbolic equates for ta-11 	*/

#define	taddr	0177500
struct	{
		int tacs;		/* ta-11 control and status register */
		int tadb; };		/* ta-11 data buffer register */

#define	taperr (1<<15)	/* 14->9,3->1, indicate type
			   valid only when ready is set */
#define	blkchk	(1<<14)	/* crc error for read and sfb 
			   sets error for read */
#define	xot	(1<<13)		/* eot or bot on selected tape
			   sets error for all but rewind */
#define	wlock	(1<<12)	/* selected tape is write protected
			   write or wfg must be set */
#define	fgap	(1<<11)	/* tape mark encounted during read,sfb,srf,sff
			   sets error only on read, sfb */
#define	timerr	(1<<10)	/* missed character
                             sets error only on read, write */
#define	offline	((1<<9)|(1<<15)) /* selected cassette unavailable */

#define	treq	(1<<7)	/* data available in tadb during read
			   data wanted in tadb during write */
#define	ready	(1<<5)	/* unit ready to accept commands */
#define	ilbs	(1<<4)	/* initate last byte sequence */
#define	wfg	((1<<6)|(0<<1)|1)	/* write file gap */
#define	write	((1<<6)|(1<<1)|1)	/* write */
#define	read	((1<<6)|(2<<1)|1)	/* read */
#define	srf	((1<<6)|(3<<1)|1)	/* space reverse file */
#define	srb	((1<<6)|(4<<1)|1)	/* space reverse block */
#define	sff	((1<<6)|(5<<1)|1)	/* space forward file */
#define	sfb	((1<<6)|(6<<1)|1)	/* space forward block */
#define	rewind	((1<<6)|(7<<1)|1)	/* rewind */









taopen(dev,flag)
{
	register lr;

	if( (lr=dev.d_minor)>1 )  u.u_error=ENXIO;
	else { taddr->tacs =| lr<<8 ; 
		if( taopenl[lr] || ((taddr->tacs&offline)==offline) )
				u.u_error=ENXIO;
			else
				taopenl[lr]=1;
	}
}
taclose(dev,flag)
{
	register lr;
	lr = dev.d_minor<<8;
	if (flag) tastart(lr,wfg);
	tastart(lr,rewind);
	taopenl[lr>>8]=0;
}
tastart( lr , cmd )
{
	register struct buf *bp;
	if(u.u_count>512)  { u.u_error=ENXIO; return; }
	while(tactv) sleep(&tactv,1);
	taddr->tacs =| lr ;
	if( (!(taddr->tacs&ready))||((taddr->tacs&offline)==offline))
			{ u.u_error = ENXIO; return; }
	switch(cmd) {
	  case read:	bp = getblk(NODEV);
			tabaz=taba=bp->b_addr;
			tabcz=tabc= u.u_count;
			break;
	  case write:	bp = getblk(NODEV);
			tabaz=taba=bp->b_addr;
			tabcz=tabc= u.u_count;
			iomove(bp,0,u.u_count,B_WRITE);
	  case wfg:
	  case rewind:
			break;
	}
	tactv = cmd;	taerr = 0;	taerrcnt = 5;
	taddr->tacs=  lr | cmd;
	while(tactv) sleep(&tactv,1);	/* wait for i/o to complete */
	if((taerr==2)&&(cmd==read)) return;
	switch(cmd) {
	  case read:	if(tabc==u.u_count) return;
	  		iomove(bp,0,u.u_count-tabc,B_READ);
	  case write:	brelse(bp);
	  case wfg:	break;
	  case rewind:	return;
	}
	if(taerr) u.u_error = ENXIO;
}

taintr()
{

	if(!(tactv)) return;

	if(taddr->tacs&treq)
	  { if (!tabc--) { taddr->tacs =| ilbs; tabc=0; }
		else if (tactv==read)
			*taba++ =taddr->tadb;
		   else taddr->tadb= *taba++;
		return;	}

	if(taddr->tacs&taperr) {
	  if(!taerr) {
/*************
	   	deverror(bp,taddr->tacs);
*************/
	   	taerr = 1;
		if((taerrcnt--)>0) switch(tactv) { 
		case read:	if(taddr->tacs&(xot|fgap)) {
					taerr++; goto rti;
				}
		case write:	if(taddr->tacs&timerr) {
				taddr->tacs = (taddr->tacs & 0177740)|srb;
				return; 
				}
		}
	  }
	}
	   else if(taerr) {
		taddr->tacs = (taddr->tacs & 0177740) | tactv;
		taerr = 0;	taerrcnt = 5;
		taba = tabaz;	tabc = tabcz;
		return;
	   }
rti:	tactv = 0;
	wakeup(&tactv);
}
/*
 *	 i/o interface for  cassette.
 *	essentially provided so users can read/write caps-11 format
 *	 tapes.
 */

taread(dev)
{
	tastart(dev.d_minor<<8,read);
}

tawrite(dev)
{
	tastart(dev.d_minor<<8 , (u.u_count==1) ? wfg : write);
}
