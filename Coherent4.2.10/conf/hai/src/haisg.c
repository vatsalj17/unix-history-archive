#include <sys/coherent.h>
#include <sys/cmn_err.h>

#include <sys/buf.h>
#include <sys/mmu.h>
#include <sys/haiscsi.h>

#define LOCAL	static
#if defined(__GNUC__)
#define Register
#else
#define Register	register
#endif

#define min(a, b) (((a) < (b)) ? (a) : (b))

#if __USE_PROTO__
size_t ukbuftosgl(vaddr_t ukbuf,
		  size_t ukbufsize,
		  haisgsegm_p sglist,
		  size_t segcount)
#else
size_t
ukbuftosgl(ukbuf, ukbufsize, sglist, segcount)
vaddr_t ukbuf;
size_t ukbufsize;
haisgsegm_p sglist;
size_t segcount;
#endif
{
    haisgsegm_p sg_seg;
    paddr_t newsegstart;
    size_t newsegsize;

    if (!(sg_seg = sglist) || !segcount) {
	cmn_err(CE_WARN, "hai: ukbuftosgl() with empty segment list.");
	return NULL;
    } /* if */

    /*
     * Create the first segment in the list. Zero the size entry
     * and defer setting it correctly until the top of the loop.
     */

    sg_seg->sgs_segstart = vtop(ukbuf);
    sg_seg->sgs_segsize = 0;
    newsegsize = NBPC - (sg_seg->sgs_segstart & (NBPC - 1));
    if (newsegsize > ukbufsize)
	newsegsize = ukbufsize;

    for ( ; ; ) {
	
	/*
	 * Now adjust the size of the current entry and of the buffer
	 * if there isn't any more buffer left then we're done.
	 */

	sg_seg->sgs_segsize += newsegsize;
	ukbufsize -= newsegsize;
	if (ukbufsize == 0)
	    break;

	/*
	 * Adjust the pointer to the virtual buffer and figure out the
	 * page of physical memory that it points to. Also compute the
	 * size of the buffer within the page.
	 */

	ukbuf += newsegsize;
	newsegstart = vtop(ukbuf);
	newsegsize = min(ukbufsize, NBPC);

	/*
	 * If the page is discontiguous then we have to start a new
	 * segment. As before set the size of the new segment to 0,
	 * defering setting it correctly until the top of the loop.
	 */

	if (newsegstart != sg_seg->sgs_segstart + sg_seg->sgs_segsize) {
	    
	    /*
	     * Make sure that we don't use too many segments!
	     */

	    if (--segcount <= 0) {
		cmn_err(CE_WARN, "hai: out of segments creating sglist.");
		return 0;
	    }
	    
	    ++sg_seg;
	    sg_seg->sgs_segstart = newsegstart;
	    sg_seg->sgs_segsize = 0;
	} /* if */
    } /* for */

    return sg_seg - sglist + 1;
} /* ukbuftosgl() */

#if __USE_PROTO__
size_t sysgbuftosgl(paddr_t sysgbuf,
		    size_t sysgbufsize,
		    haisgsegm_p sglist,
		    size_t segcount)
#else
size_t
sysgbuftosgl(sysgbuf, sysgbufsize, sglist, segcount)
paddr_t sysgbuf;
size_t sysgbufsize;
haisgsegm_p sglist;
size_t segcount;
#endif
{
    haisgsegm_p sg_seg;
    paddr_t newsegstart;
    size_t newsegsize;

    if (!(sg_seg = sglist) || segcount == 0) {
	cmn_err(CE_WARN, "hai: sysgbuftosgl() with empty segment list.");
	return NULL;
    } /* if */

    /*
     * Create the first segment in the list. Zero the size entry
     * and defer setting it correctly until the top of the loop.
     */

    sg_seg->sgs_segstart = P2P(sysgbuf);
    sg_seg->sgs_segsize = 0;
    newsegsize = NBPC - (sg_seg->sgs_segstart & (NBPC - 1));
    if (newsegsize > sysgbufsize)
	newsegsize = sysgbufsize;

    for ( ; ; ) {
	
	/*
	 * Now adjust the size of the current entry and of the buffer
	 * if there isn't any more buffer left then we're done.
	 */

	sg_seg->sgs_segsize += newsegsize;
	sysgbufsize -= newsegsize;
	if (sysgbufsize == 0)
	    break;

	/*
	 * Adjust the pointer to the virtual buffer and figure out the
	 * page of physical memory that it points to. Also compute the
	 * size of the buffer within the page.
	 */

	sysgbuf += newsegsize;
	newsegstart = P2P(sysgbuf);
	newsegsize = min(sysgbufsize, NBPC);

	/*
	 * If the page is discontiguous then we have to start a new
	 * segment. As before set the size of the new segment to 0,
	 * defering setting it correctly until the top of the loop.
	 */

	if (newsegstart != sg_seg->sgs_segstart + sg_seg->sgs_segsize) {
	    
	    /*
	     * Make sure that we don't use too many segments!
	     */

	    if (--segcount <= 0) {
		cmn_err(CE_WARN, "hai: out of segments creating sglist.");
		return 0;
	    } /* if */
	    
	    ++sg_seg;
	    sg_seg->sgs_segstart = newsegstart;
	    sg_seg->sgs_segsize = 0;
	} /* if */
    } /* for */

    return sg_seg - sglist + 1;
} /* sysgbuftosgl() */

#if __USE_PROTO__
int addbuftosgl(paddr_t sysgaddr,
		size_t bufsize,
		haisgsegm_p seg,
		size_t segcount)
#else     
int 
addbuftosgl(sysgaddr, bufsize, seg, segcount)
paddr_t sysgaddr;
size_t bufsize;
haisgsegm_p seg;
size_t segcount;
#endif
{
    /*
     * Start by saving the end of the list for possible undo operation.
     */
    
    haisgsegm_p start_seg = seg;
    paddr_t undo_segstart = seg->sgs_segstart;
    size_t undo_segsize = seg->sgs_segsize;
    
    haisgsegm_p stop_seg = seg + segcount;
    size_t bytescopied;
    paddr_t physaddr;
    paddr_t segend;
    int retval;
    
    /*
     * Setup the first element of an empty scatter/gather list here.
     */

    if (seg->sgs_segstart == NULL) {
	seg->sgs_segstart = P2P(sysgaddr);
	seg->sgs_segsize = 0;
    } /* if */

    bytescopied = 0;
    while (bufsize > 0) {
	seg->sgs_segsize += bytescopied;
	sysgaddr += bytescopied;
	physaddr = P2P(sysgaddr);
	segend = seg->sgs_segstart + seg->sgs_segsize;

	if (physaddr != segend) {
	    if (++seg >= stop_seg) 
		break;

	    seg->sgs_segstart = physaddr;
	    seg->sgs_segsize = 0;
	} /* if */
	
	bytescopied = NBPC - ((unsigned) segend & (NBPC - 1));
	if (bytescopied > bufsize)
	    bytescopied = bufsize;
	
	seg->sgs_segsize += bytescopied;
	bufsize -= bytescopied;
    } /* while */
    
    if (bufsize > 0) {
	seg = start_seg;
        seg->sgs_segstart = undo_segstart;
	seg->sgs_segsize = undo_segsize;
	retval =  -1;
    } /* if */
    else
	retval = seg - start_seg;

    if (++seg < stop_seg) {
	seg->sgs_segstart = NULL;
	seg->sgs_segsize = 0;
    } /* if */

    return retval;
} /* addbuftosgl() */

#if __USE_PROTO__
int buftosglist(buf_t *bp, size_t bufcount, haisgsegm_p seg, size_t segcount)
#else
int buftosglist(bp, bufcount, seg, segcount)
buf_t *bp; 
size_t bufcount;
haisgsegm_p seg; 
size_t segcount;
#endif
{
    size_t index;
    size_t segsused;

    /*
     * Set up the initial segment to have the physical address of the first
     * buffer and 0 size. This way addbuftosgl() works properly.
     */
    
    seg->sgs_segstart = NULL;
    seg->sgs_segsize = 0;
    for (index = 0; index < segcount; ) {

	/*
	 * Add this segment to the scatter gather list. resid is the 
	 * number of bytes that could not be fit in if we run out of
	 * segments.
	 */
	
	segsused = addbuftosgl(bp->b_paddr + bp->b_count - bp->b_resid,
			       bp->b_resid, 
			       seg + index, 
			       segcount - index);
 
	if (segsused == -1)
	    break;
	else
	    index += segsused;
		
	bp = bp->b_actf;
	if (!bp || bp->b_resid != bp->b_count || --bufcount)
	    break;
    } /* for */

    return index;
} /* buftosglist() */

/* End of file */
