/*************************************************************************
 * Function:   unsigned short cyrix_detect(void)
 * Returns:    0xFFFF -- non-Cyrix CPU detected
 *             0xFFFE -- Cyrix OEM CPU detected (no cache startup)
 *             id#    -- Cyrix Upgrade CPU detected (cache startup possible)
 *
 * This code is based on code provided by Cyrix.  For current code,
 * contact Cyrix.
 */

#define CR2_MASK 0x04		/* LockNW */
#define CR3_MASK 0x02		/* NMIEN */

#define _KERNEL	1

#include <sys/cyrix.h>
#include <sys/coherent.h>

#if __USE_PROTO__
unsigned short cyrix_detect(void)
#else
unsigned short
cyrix_detect()
#endif
{
	unsigned char orgc2, newc2, orgc3, newc3;
	int cr2_rw = 0, cr3_rw = 0;
	unsigned short type = 0;
	unsigned short ret;
	
	/*
	 * See if it is a Cyrix CPU, and if so what kind.
	 */
	if(iscyrix()) {
		/*
		 * Test Cyrix C2 register read/writeable.
		 */

		orgc2 = cx_r(0xC2);	/* Get current c2 value */
		newc2 = orgc2 | CR2_MASK;	/* Set test bits */
		cx_w(0xC2, newc2);	/* Write test value to c2 */
		cx_r(0xC0);		/* Dummy read to change bus */


		/* See if only mask bits were set */
		if((cx_r(0xC2) & CR2_MASK) == CR2_MASK) {	

			newc2 = orgc2 & ~CR2_MASK;	/* Clear test bits */
			cx_w(0xC2, newc2);	/* Write cleared mask */
			cx_r(0xC0);		/* Dummy read to change bus */


			/* See if the mask bits were cleared */
			if((cx_r(0xC2) & CR2_MASK) == 0)
				cr2_rw = 1;
		}

		/* Return c2 to original value */
		cx_w(0xC2, orgc2);



		/*
		 * Test Cyrix c3 register read/writeable.
		 */
		orgc3 = cx_r(0xC3);	/* Get current c3 value */
		newc3 = orgc3 | CR3_MASK;	/* Set NMI_ENABLE */
		cx_w(0xC3, newc3);	/* Write test value to c3 */

		/* See if only the mask bits were set. */
		if((cx_r(0xC3) & CR3_MASK) == CR3_MASK) {

			newc3 = orgc3 & ~CR3_MASK;	/* Clear NMI_ENABLE */
			cx_w(0xC3, newc3);	/* Write cleared mask to c3 */

			/* See if mask bits were cleared. */	
			if((cx_r(0xC3) & CR3_MASK) == 0)
				cr3_rw = 1;
		}


		cx_w(0xC3, orgc3);

		/*
		 * Only the DRx2 and SRx2 have a cache we can enable,
		 * so, those are the only ones we care about.
		 */
		if((cr2_rw && cr3_rw) || (!cr2_rw && cr3_rw)) {
			type = cx_r(0xFE);		/* IDIR0 */
			type += cx_r(0xFF) << 8;	/* IDIR1 */
		}

		/*
		 * If we know it's not one of the above, get out.
		 * It must be either a Cx486S_a or a Cx486_pr.
		 */
		if(type == 0)
			ret = CYRIX_OEM;

		switch(type & 0xFF) {
			case (CYRIX_SRX):	/* We only care about these */
			case (CYRIX_DRX):
			case (CYRIX_SRX2):
			case (CYRIX_DRX2):
				ret = type;
				break;

			default:
				ret = CYRIX_OEM;
				break;
		}
	} else	/* Non-Cyrix */
		ret = CYRIX_NOT;

	return ret;
}

#define CX_NC0 1
#define CX_NC1 2
#define CX_FLUSH 16
#define CX_RPL 1


/* Turn the cache on a Cyrix user-upgrade chip */
#if __USE_PROTO__
void
cyrix_cache_on(void)
#else
void
cyrix_cache_on()
#endif
{
	unsigned char tmp_reg;
	int s;

	/* Lock out interrupts */
	s = sphi();

	/* Invalidate cache and turn off caching */
	cx_invalidate_cache();

	/* Turn on caching control regs */
	tmp_reg = cx_r(0xC0);
	tmp_reg |= (CX_NC0 | CX_NC1 | CX_FLUSH);
	cx_w(0xC0, tmp_reg);

	tmp_reg = cx_r(0xC1);
	tmp_reg &= ~CX_RPL;
	cx_w(0xC1, tmp_reg);

	/* Zero out the non-cacheable regions. */
	cx_w(0xC6, 0x00);
	cx_w(0xC5, 0x00);
	cx_w(0xC6, 0x00);

	/* 
	 * Cyrix doesn't clear these in their example.
	 * They depend on the fact
	 * that hardware will have them zero'ed out as part
	 * of the initialization during reset.
	 */
#if 0
	cx_w(0xC8, 0x00);
	cx_w(0xC9, 0x00);
	cx_w(0xCB, 0x00);
	cx_w(0xCC, 0x00);
	cx_w(0xCE, 0x00);
	cx_w(0xCF, 0x00);
#endif

	/* Re-enable caching */
	cx_start_cache();

	/* Interrupts restored */
	spl(s);
}
