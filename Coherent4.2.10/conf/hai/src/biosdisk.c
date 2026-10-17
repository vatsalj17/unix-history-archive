#include <kernel/typed.h>
#include <sys/hdioctl.h>
#include <sys/haiscsi.h>
#include <sys/scsiwork.h>

/*
 * First, what to do with disks not supported by the BIOS.
 * Answer:  support them with a default geometry. This mimics the
 * Adaptec controller's logic which is simplest so far. In the future,
 * may need to set it up so that the Seagate/Future Domain geometry is 
 * supported.
 */

#define BLK_PER_GIG	2097152	/* One gigabyte in blocks. */
#define SMDRV_HDS	64	/* Small drive # heads */
#define SMDRV_SPT	32	/* Small drive # sectors */
#define LGDRV_HDS	255	/* Large drive # heads */
#define LGDRV_SPT	63	/* Large drive # sectors */

extern _drv_parm_t	_sd_drv_parm[];

/*
 * loadbiosparms()
 *
 * Load the bios parameters for a scsi drive into the drive parms table.
 */

#if __USE_PROTO__
void loadbiosparms(int biosnum, int s_id)
#else
void 
loadbiosparms(biosnum, s_id)
int  	biosnum;
int 	s_id;
#endif
{
    FIFO		*ffp;
    typed_space 	*tp;
    extern typed_space	boot_gift;
    
    /*
     * Attempt to load the parameters for the drive from the bootgift.
     * Recent "advances" in disk technology force this on us although
     * it probably should have been done this way from the word go.
     * Piggy left us the parameters in the T_BIOS_DISK entries in the boot
     * gift.
     */
      
    if (biosnum >= 0 &&
	biosnum <= 1 &&
	(ffp = fifo_open (&boot_gift, 0)) != F_NULL) {

	/*
	 * Look through the entire boot gift for our entry.
	 */

	for ( ; (tp = fifo_read(ffp)) != 0; ) {
	    BIOS_DISK * bdp = (BIOS_DISK *)tp->ts_data;

	    /*
	     * If we found a disk drive and it's ours load the parameters.
	     */

	    if (tp->ts_type == T_BIOS_DISK && bdp->dp_drive == biosnum) {
		_sd_drv_parm[s_id].ncyl = bdp->dp_cylinders;
		_sd_drv_parm[s_id].nhead = bdp->dp_heads;
		_sd_drv_parm[s_id].nspt = bdp->dp_sectors;

		break;
	    } /* if */
	} /* for */
	fifo_close(ffp);
    } /* if */
} /* loadbiosparms() */

/*
 *  haihdgeta()     --  Get disk driver paramters.
 *
 *  This function is provided to support the HDGETA/HDSETA I/O Controls
 *  so you don't need the old Adaptec SCSI driver to set up the partition
 *  table on initial setup.  There is a catch-22 with this.  You need
 *  to know the drive's geometry in order to set up the partition table
 *  but cannot get the drive's geometry until you have set up the partition
 *  table.  We solve this by using the drive's size and then dividing
 *  down based upon SDS_HDS heads and SDS_SPT sectors per track.  Every
 *  host adapter will want to do this differently.  The Adaptec solution
 *  is the best that I've seen so far. (It allows you to use Huge (1.0
 *  GB) disks under DOS without trouble.
 */

#if __USE_PROTO__
void haihdgeta(int target, hdparm_t *hdp, unsigned int diskcap)
#else
void
haihdgeta(target, hdp, diskcap)
int 		target;
hdparm_t	*hdp;
unsigned int	diskcap;
#endif
{
	memset(hdp, 0, sizeof(hdparm_t));
	*((unsigned short *) hdp->rwccp) = 0xffff;
	*((unsigned short *) hdp->wpcc) = 0xffff;
	if (_sd_drv_parm[target]. ncyl != 0) {
		*((unsigned short *) hdp->ncyl) = _sd_drv_parm[target].ncyl;
		*((unsigned short *) hdp->landc) = _sd_drv_parm[target].ncyl;
		
		hdp->nhead = _sd_drv_parm[target].nhead;
		if (hdp->nhead > 8)
			hdp->ctrl |= 8;
		hdp->nspt = _sd_drv_parm[target].nspt;
	} else {
		if (diskcap < BLK_PER_GIG) {
			_sd_drv_parm[target].nhead = SMDRV_HDS;
			_sd_drv_parm[target].nspt = SMDRV_SPT;
		} else {
			_sd_drv_parm[target].nhead = LGDRV_HDS;
			_sd_drv_parm[target].nspt = LGDRV_SPT;
		}
		_sd_drv_parm[target].ncyl = diskcap / (_sd_drv_parm[target].nhead * _sd_drv_parm[target].nspt);
		
		*((unsigned short *) hdp->ncyl) = _sd_drv_parm[target].ncyl;
		*((unsigned short *) hdp->landc) = _sd_drv_parm[target].ncyl;
		
		hdp->nhead = _sd_drv_parm[target].nhead;
		if (hdp->nhead > 8)
			hdp->ctrl |= 8;
		hdp->nspt = _sd_drv_parm[target].nspt;
	}
}		/* haihdgeta() */

/***********************************************************************
 *  haihdseta()     --  Set disk parameters in accordance with HDSETA
 *                      I/O Control.
 *
 *  Set the disk paramters accordingly for a request from the fdisk
 *  program.  This call really doesn't do anything on the adaptec or
 *  in the SCSI driver in general because SCSI Disks use Logical Block
 *  addressing rather than Cylinder/Head/Track addressing found with
 *  less intelligent Disk drive types.  What this will do is allow
 *  the fdisk program to work so a user can format his disk and install
 *  Coherent on it (A Good Thing).  This boils down to a fancy way to
 *  patch SDS_HDS and SDS_SPT.
 */

#if __USE_PROTO__
void haihdseta(int target, hdparm_t *hdp)
#else
void
haihdseta(target, hdp)
int 		target;
hdparm_t	*hdp;
#endif
{
	_sd_drv_parm[target].ncyl =  *((unsigned short *) hdp->ncyl);
	_sd_drv_parm[target].nhead = hdp->nhead;
	_sd_drv_parm[target].nspt =  hdp->nspt;
}	/* haihdseta() */

/* End of file */


