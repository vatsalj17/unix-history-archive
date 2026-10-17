/*
 * ftxqueue.c
 *
 * Queue services for FDC-tape single block transfer requests.
 *
 */

/*
 * ----------------------------------------------------------------------
 * Includes.
 */
#include	<sys/coherent.h>
#include	<sys/cmn_err.h>

#include	<sys/ft.h>
#include	<sys/ftxreq.h>

#define FTX_REQ_QLEN	60

/*
 * A block transfer request is for a real transfer (read/write) or
 * to skip a block that has been entered into the Bad Block Table (BBT).
 */

struct FtxQueue {
	int		head;
	int		tail;
	int		count;
	struct FtxReq	buf [FTX_REQ_QLEN];
};

static struct FtxQueue ftxq;

/************************************************************************
 * ftxqinit
 *
 * Initialize with no requests active, all request records free.
 ***********************************************************************/

#if	__USE_PROTO__
void ftxQinit (void)
#else
void
ftxQinit ()
#endif
{
	ftxq.count = 0;
	ftxq.head = 0;
	ftxq.tail = 0;
}

/************************************************************************
 * ftxReqPut
 *
 * Put a request record onto the queue.
 ***********************************************************************/

#if	__USE_PROTO__
void ftxReqPut (struct FtxReq * reqp)
#else
void
ftxReqPut (reqp)
struct FtxReq * reqp;
#endif
{
	int		s;
	
	s = sphi ();
	if (ftxq.count < FTX_REQ_QLEN) {
		ftxq.buf [ftxq.tail] = * reqp;
		ftxq.count ++;
		ftxq.tail ++;
		if (ftxq.tail >= FTX_REQ_QLEN)
			ftxq.tail = 0;
#if FT_TRACE
	} else {
		cmn_err (CE_WARN, "ftxReqPut : queue overflow");
#endif
	}

	spl (s);
}

/************************************************************************
 * ftReqGet
 *
 * Take a request record off the queue - return its pointer or NULL.
 ***********************************************************************/

#if	__USE_PROTO__
struct FtxReq * ftxReqGet (void)
#else
struct FtxReq *
ftxReqGet ()
#endif
{
	struct FtxReq	* retval;
	int		s;

	s = sphi ();
	if (ftxq.count > 0) {
		retval = & ftxq.buf [ftxq.head];
		ftxq.count --;
		ftxq.head ++;
		if (ftxq.head >= FTX_REQ_QLEN)
			ftxq.head = 0;
	} else
		retval = NULL;
	spl (s);
	return retval;
}

/************************************************************************
 * ftReqSize
 *
 * Return the number of items on the queue.
 ***********************************************************************/

#if	__USE_PROTO__
int ftxReqSize (void)
#else
int
ftxReqSize ()
#endif
{
	return ftxq.count;
}

/************************************************************************
 * ftReqFreeSize
 *
 * Return the number free slots in the queue.
 ***********************************************************************/

#if	__USE_PROTO__
int ftxReqFreeSize (void)
#else
int
ftxReqFreeSize ()
#endif
{
	return FTX_REQ_QLEN - ftxq.count;
}
