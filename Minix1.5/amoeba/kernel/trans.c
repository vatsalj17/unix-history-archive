/****************************************************************************/
/*									    */
/* (c) Copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands */
/*									    */
/*    This product is part of the  Amoeba  distributed operating system.    */
/*									    */
/*    Permission to use, sell, duplicate or disclose this software must be  */
/* obtained in writing.  Requests for such permissions may be sent to	    */
/*									    */
/*									    */
/*		Dr. Andrew S. Tanenbaum					    */
/*		Dept. of Mathematics and Computer Science		    */
/*		Vrije Universiteit					    */
/*		Postbus 7161						    */
/*		1007 MC Amsterdam					    */
/*		The Netherlands						    */
/*									    */
/****************************************************************************/

#define NDEBUG
#define TRANS

/*
 * This module implements the transaction mechanism. The transaction
 * calls are:
 *	getreq(header, buffer, count);
 *	putrep(header, buffer, count);
 *	trans(hdr1, buf1, cnt1, hdr2, buf2, cnt2);
 *
 * ``Getreq'' is called by servers that want to wait for a request.
 * ``Putrep'' is called by servers that what to send a reply to a client.
 * ``Trans'' is called by clients that want to send a request to a server.
 * Requests are addressed by the ``h_port'' field in the header structure.
 * Replies are automatically sent back to the corresponding client. Between
 * a ``getreq'' and a ``putrep'' the server may not call ``getreq''. All
 * the three calls are blocking.
 *
#ifndef NONET
 * The following network interface routines must be defined externally:
 *
 *	puthead(dest, source, ident, seq, type, size);
 *	append(data, size, send);
 *	pickoff(data, size);
 *
 * ``Puthead'' is called first when a packet has to be sent. Among other
 * things the destination and the size are specified. If this size is zero,
 * the packet must be sent immediately.
 * ``Append'' is called when more data must be appended to a packet. The
 * ``send'' parameter is set when the packet can be sent.
 * ``Pickoff'' is called when a packet has arrived. The specified number
 * of bytes must copied to the specified data buffer.
 *
 * When a packet arrives, the local routine ``handle'' must be called:
 *	handle(dest, source, ident, seq, type, size, hdr);
 * ``Hdr'' contains the first HEADERSIZE data bytes. With a call to
 * ``getall'' this buffer is enlarged to contain the full packet.
#endif NONET
 */

#include "kernel.h"
#include "amoeba.h"
#undef IDLE
#include "byteorder.h"
#include "exception.h"
#include "global.h"
#include "task.h"
#include "internet.h"
#include "amstat.h"
#include "assert.h"

#define	send		amsend
#define	received	amreceived
extern struct task	task[];

#define FIRSTSIZE	(PACKETSIZE - HEADERSIZE)

#define MAXCNT		30000		/* maximum buffer size */

#define taskptr(to)	(&task[tasknum(to)])

extern port NULLPORT;

#ifndef NONET
#ifndef NOCLIENT
extern unshort maxcrash;
#endif NOCLIENT
extern unshort retranstime, crashtime, clientcrash;
extern unshort maxretrans, mincrash;
#endif

address local;
extern long ticker;
extern address portlookup();
extern phys_bytes umap();

#ifdef STATISTICS
struct amstat amstat;
#endif
#ifdef BUFFERED

/* Some simple routines to test ``BUFFERED transactions''
 */

PRIVATE char bufs[10][BUFSIZE];
PRIVATE char used[11];

#define NOBUF	((buffer) 0)

buffer allocbuf(){
  register buffer b;

  for (b = 1; b <= 10; b++)
	if (!used[b]) {
		used[b] = 1;
		return(b);
	}
#ifndef NDEBUG
  printf("out of buffers\n");
#endif
  return(NOBUF);
}

freebuf(b)
buffer b;
{
  assert(used[b]);
  used[b] = 0;
}

putbuf(b, addr, size)
buffer b;
vir_bytes addr;
unshort size;
{
  register phys_bytes pb;

  assert(used[b]);
  if ((pb = umap(curtask, addr, (vir_bytes) size)) == 0)
	return(0);
  phys_copy((phys_bytes) bufs[b - 1], pb, (phys_bytes) size);
  return(1);
}

getbuf(b, addr, size)
buffer b;
vir_bytes addr;
unshort size;
{
  register phys_bytes pb;

  assert(used[b]);
  if ((pb = umap(curtask, addr, (vir_bytes) size)) == 0)
	return(0);
  phys_copy(pb, (phys_bytes) bufs[b - 1], (phys_bytes) size);
  return(1);
}

#endif BUFFERED

/* return where the ``location'' can be found:
 *	DONTKNOW:	the location is unknown;
 *	LOCAL:		it's on this machine;
 *	GLOBAL:		it's probably somewhere on the net.
 */
area(location)
register address location;
{
  if (location == SOMEWHERE)
	return(DONTKNOW);
  if (siteaddr(location) == local)
	return(LOCAL);
#ifdef NONET
  assert(0);
  /*NOTREACHED*/
#else
  return(GLOBAL);
#endif
}

/* (re)start getreq
 */
static getreqstart(t)
register struct task *t;
{
  t->ts_state = WAITBUF;
  t->ts_seq = 0;
  t->ts_offset = 0;
  portinstall(&t->ts_rhdr->h_port, t->ts_addr, WAIT);
}

#ifndef NONET	/*--------------------------------------------------*/

static headerfromnet(hdr)
header *hdr;
{
  dec_s_be(&hdr->h_command);
  dec_s_be(&hdr->h_size);
  dec_l_be(&hdr->h_offset);
  dec_s_be(&hdr->h_extra);
}

static headertonet(hdr)
header *hdr;
{
  enc_s_be(&hdr->h_command);
  enc_s_be(&hdr->h_size);
  enc_l_be(&hdr->h_offset);
  enc_s_be(&hdr->h_extra);
}

/* A locate message has arrived. This routine handles it by looking the
 * ports up in the cache, and if it finds some there that are local it
 * sends a reply back with those ports.
 */

static locreq(ph, ptab)         /* handle locate request */
register struct pktheader *ph;
register char *ptab;
{
  getall();
  portask(pktfrom(ph), (port *) ptab, ph->ph_size/PORTSIZE);
  netenable();
}

/* called from portcache.c */
hereport(asker, ptab, nports)
address asker;
port *ptab;
unsigned nports;
{

  nports *= PORTSIZE;
  puthead(asker, local, 0, 0, HERE, nports);
  append((phys_bytes) ptab, nports, SEND);
}

#ifndef NOCLIENT

/* called from portcache.c */
whereport(ptab, nports)
port *ptab;
unsigned nports;
{
  nports *= PORTSIZE;
  puthead(BROADCAST, local, 0, 0, LOCATE, nports);
  append((phys_bytes) ptab, nports, SEND);
}

/* start getrep
 */
static getrepstart(t)
register struct task *t;
{
  t->ts_flags &= ~PUTREQ;
  t->ts_state = WAITBUF;
  t->ts_seq = 0;
  t->ts_offset = 0;
  t->ts_flags |= GETREP;
  t->ts_timer = crashtime;
  t->ts_count = maxcrash;
}

/* A reply on a locate message has arrived. Store the ports in the cache.
 */
static here(ph, ptab)	/* handle locate reply */
register struct pktheader *ph;
register char *ptab;
{
  register port *p;
  register unshort from = pktfrom(ph);

  getall();
  p = ABSPTR(port *, ptab + ph->ph_size);
  while ((char *) --p >= ptab)
	portinstall(p, from, NOWAIT);
  netenable();
}

/* After I've enquired about the health of a server, the processor answered
 * that it's fine. Thank goodness. But keep asking.
 */
static alive(ph)
register struct pktheader *ph;
{
  register struct task *t = &task[ph->ph_dsttask & 0xFF];

  netenable();
  if (t->ts_server == pktfrom(ph) && t->ts_svident == ph->ph_ident &&
						(t->ts_flags & GETREP)) {
	t->ts_timer = crashtime;
	t->ts_count = maxcrash;
	t->ts_signal = 0;
  }
}

/* After I've enquired about the health of a server, the processor answered
 * that it's dead. Too bad. Notify client.
 */
static dead(ph)
register struct pktheader *ph;
{
  register struct task *t = &task[ph->ph_dsttask & 0xFF];

  netenable();
  if (t->ts_server == pktfrom(ph) && t->ts_svident == ph->ph_ident &&
		(t->ts_state == WAITBUF || t->ts_state == RECEIVING ||
		 t->ts_state == SENDING) &&
		(t->ts_flags & (PUTREQ | GETREP))) {
	t->ts_timer = 0;
	t->ts_state = FAILED;
	wakeup((event_t) &t->ts_state);
  }
}
#endif NOCLIENT

/* Someone enquires how a server doing. Inform him. A signal may be sent
 * along with the enquiry. Handle that.
 */
static enquiry(ph)
register struct pktheader *ph;
{
  register unshort from = pktfrom(ph), to = pktto(ph);
  register struct task *t = &task[ph->ph_dsttask & 0xFF];

  netenable();
  if (t->ts_client == from && t->ts_clident == ph->ph_ident) {
  	if (t->ts_flags & SERVING) {
		t->ts_cltim = clientcrash;
		puthead(from, to, ph->ph_ident, 0, ALIVE, 0);
		if (ph->ph_signal != 0)
			putsig(t, (unshort) (ph->ph_signal & 0xFF));

	}
  }
  else
	puthead(from, to, ph->ph_ident, 0, DEAD, 0);
}

/* Send a fragment of a packet. If it's the first, insert the header.
 */
static sendfragment(t, what)
struct task *t;
unshort what;
{
  register address to;
  register short size = t->ts_xcnt - t->ts_offset;
  register phys_bytes xbuf;

#ifndef NOCLIENT
  if (t->ts_flags & PUTREQ) {
	to = t->ts_server;
	what |= REQUEST;
  }
  else
#endif
  {
	to = t->ts_client;
	what |= REPLY;
  }
  if (t->ts_seq == 0) {		/* first fragment--append header */
	if (size <= FIRSTSIZE)		/* first and last */
		what |= LAST;
	else {				/* first but not last */
		size = (size + HEADERSIZE - 1) % PACKETSIZE - HEADERSIZE + 1;
		if (size < 0)
			size = 0;
	}
	puthead(to, t->ts_addr, t->ts_ident, t->ts_seq, (char) what,
						(unshort) size + HEADERSIZE);
	headertonet(t->ts_xhdr);
	append((phys_bytes) t->ts_xhdr, HEADERSIZE, size == 0 ? SEND : NOSEND);
	headerfromnet(t->ts_xhdr);
  }
  else {			/* not first */
	if (size <= PACKETSIZE)		/* last but not first */
		what |= LAST;
	else				/* not first and not last */
		size = PACKETSIZE;
	puthead(to, t->ts_addr, t->ts_ident, t->ts_seq, (char) what,
							(unshort) size);
  }
/*
** a change from original trans code because on an ibmpc kernel virtual
** addresses are different from kernel physical.  therefore we must
** replace call to append with am_doappend().
*/
  if (size != 0)
	if ((xbuf = umap(t, (vir_bytes) (t->ts_xbuf + t->ts_offset),
						(vir_bytes) size)) == 0)
		return(0);
	else
		am_doappend(xbuf, (unshort) size, SEND);
  return(1);
}

/* Send a message. Call sendfragment to send the first fragment. When an
 * acknowledgement arrives, the interrupt routine handling it will send
 * the next fragment, if necessary.
 */
static send(){
  register struct task *c = curtask;
  register unshort what = 0;

  c->ts_state = SENDING;
#ifdef NOCLIENT
  c->ts_ident = c->ts_clident;
#else
  c->ts_ident = c->ts_flags & PUTREQ ? c->ts_svident : c->ts_clident;
#endif
  c->ts_seq = 0;
  c->ts_count = maxretrans;
  do {
	if (!sendfragment(c, what)) {
		c->ts_state = MEMFAULT;
		return;
	}
	if (c->ts_state == SENDING) {
		c->ts_timer = retranstime;
		if (sleep((event_t) &c->ts_state)) {
			c->ts_timer = 0;
			c->ts_state = ABORT;
			return;
		}
	}
	if (c->ts_state == ACKED) {
		c->ts_state = SENDING;
		what = 0;
	}
	else /* if (c->ts_state == SENDING) */
		what = RETRANS;
  } while (c->ts_state == SENDING);
}

/* An acknowledgement for a fragment arrived. If it wasn't the last fragment,
 * send the next. Else if it was a putrep, restart the task. Else it was the
 * putreq part of a transaction, start up the getrep part.
 */
static gotack(ph)
register struct pktheader *ph;
{
  register unshort to = pktto(ph), from = pktfrom(ph);
  register struct task *t = &task[ph->ph_dsttask & 0xFF];

  netenable();
  if (t->ts_state == SENDING && t->ts_ident == ph->ph_ident &&
						t->ts_seq == ph->ph_seq) {
	if (ph->ph_seq == 0) {		/* first fragment */
		register short size = t->ts_xcnt - t->ts_offset;
		
#ifndef NOCLIENT
		if (t->ts_flags & PUTREQ)
			t->ts_server = from;
		else
#endif
		if (t->ts_client != from)
			return;
		if (size > 0)
			size = (size + HEADERSIZE - 1) % PACKETSIZE
							- HEADERSIZE + 1;
		if (size > 0)
			t->ts_offset += size;
	}
	else
		t->ts_offset += PACKETSIZE;
	t->ts_timer = 0;
	if (t->ts_offset >= t->ts_xcnt)		/* ack for last fragment */
#ifndef NOCLIENT
		if (t->ts_flags & PUTREQ) {	/* in a transaction */
			getrepstart(t);
			if (t->ts_signal != 0)
				puthead(from, to, ph->ph_ident, t->ts_signal,
								ENQUIRY, 0);
		}
		else
#endif
		{				/* putrep done */
			assert(t->ts_flags & PUTREP);
			t->ts_state = DONE;
			wakeup((event_t) &t->ts_state);
		}
	else {
		t->ts_seq++;
#ifdef BUFFERED
		t->ts_timer = 0;
		t->ts_count = maxretrans;
		t->ts_state = ACKED;
		wakeup((event_t) &t->ts_state);
#else
		if (sendfragment(t, 0)) {
			t->ts_timer = retranstime;
			t->ts_count = maxretrans;
		}
		else {
			t->ts_timer = 0;
			t->ts_state = MEMFAULT;
			wakeup((event_t) &t->ts_state);
		}
#endif BUFFERED
	}
  }
}

#ifndef NOCLIENT
/* A nak has arrived. Obviously the server was not at the assumed address.
 * Wake up the task, to do a new locate.
 */
static gotnak(ph)
register struct pktheader *ph;
{
  register struct task *t = &task[ph->ph_dsttask & 0xFF];

  netenable();
  if (t->ts_state == SENDING && t->ts_ident == ph->ph_ident && t->ts_seq == 0
	  && (t->ts_flags & PUTREQ) && t->ts_server == (ph->ph_srcnode & 0xFF)) {
	t->ts_timer = 0;
	t->ts_state = NACKED;
	wakeup((event_t) &t->ts_state);
  }
}
#endif NOCLIENT

/* A fragment has arrived. Get the data and see if more fragments are expected.
 */
static received(t, what, size, hdr)
struct task *t;
char what, *hdr;
unshort size;
{
  register unshort cnt = t->ts_rcnt - t->ts_offset, n;
  register phys_bytes rbuf;

  if (cnt > size)
	cnt = size;
  if (cnt != 0) {
	rbuf = umap(t, (vir_bytes) (t->ts_rbuf+t->ts_offset), (vir_bytes) cnt);
	if (rbuf == 0) {
		netenable();
		t->ts_timer = 0;
		t->ts_state = MEMFAULT;
		wakeup((event_t) &t->ts_state);
		return;
	}
	t->ts_offset += cnt;
	if (t->ts_seq != 0) {	/* copy the ``header'' */
		n = cnt < HEADERSIZE ? cnt : HEADERSIZE;
/* kernel virtual != kernel physical under minix */
		am_phys_copy((vir_bytes)hdr, rbuf, (phys_bytes) n);
		rbuf += n;
		cnt -= n;
	}
	if (cnt != 0)
		pickoff(rbuf, cnt);
  }
  netenable();
  if (what & LAST) {
	t->ts_timer = 0;
	t->ts_state = DONE;
#ifndef BUFFERED
	wakeup((event_t) &t->ts_state);
#endif
  }
  else {
	t->ts_seq++;
	t->ts_timer = crashtime;
	t->ts_count = mincrash;
	t->ts_state = RECEIVING;
  }
}

#ifdef BUFFERED

/* A network fragment is available. Wake up the waiting task.
 */
static gotbuffer(t, from, ident, what, size)
register struct task *t;
address from;
char ident, what;
unshort size;
{
  assert(t->ts_state == WAITBUF || t->ts_state == RECEIVING);
  t->ts_sender = from;
  t->ts_ident = ident;
  t->ts_buffer = NETBUF;
  t->ts_bufcnt = size;
  t->ts_what = what;
  t->ts_timer = 0;	/* shouldn't be necessary, but can't hurt */
  wakeup((event_t) &t->ts_state);
}

#endif BUFFERED

/* See if someone is handling the request for `from.'
 */
static struct task *find(from, ident)
address from;
char ident;
{
  register struct task *t;

  for (t = task; t < uppertask; t++)
	if ((t->ts_flags & (GETREQ | SERVING | PUTREP)) &&
				t->ts_client == from && t->ts_clident == ident)
		return(t);
  return(NILTASK);
}

/* A request packet has arrived. Find out which task this packet should go to.
 * Send an acknowledgement if it can't do any harm.
 */
static gotrequest(ph, hdr)
register struct pktheader *ph;
header *hdr;
{
  register struct task *t;
  register unsigned /* unshort */ from = pktfrom(ph), to, size = ph->ph_size;

  if (ph->ph_seq == 0)	/* only the first fragment is really interesting */
	if ((ph->ph_type & RETRANS) &&
				(t = find(from, ph->ph_ident)) != NILTASK) {
		netenable();		/* ack got lost, send it again */
		puthead(from, t->ts_addr, ph->ph_ident, ph->ph_seq, ACK, 0);
	}
	else {
		register address location;

		location = portlookup(&hdr->h_port, NOWAIT, DELETE);
		if (location == NOWHERE || siteaddr(location) != local) {
			netenable();
			puthead(from, pktto(ph), ph->ph_ident, 0, NAK, 0);
			return;
		}
		size -= HEADERSIZE;
		t = taskptr(location);
		t->ts_client = from;
		t->ts_clident = ph->ph_ident;
		*t->ts_rhdr = *hdr;
		headerfromnet(t->ts_rhdr);
#ifdef BUFFERED
		gotbuffer(t, from, ph->ph_ident, ph->ph_type, size);
#else
		if ((ph->ph_type & (LAST | RETRANS)) != LAST)
			puthead(from, location, ph->ph_ident, 0, ACK, 0);
		received(t, ph->ph_type, size, (char *) 0);
#endif BUFFERED
	}
  else {	/* seq != 0 */
	to = pktto(ph);
	t = &task[ph->ph_dsttask & 0xFF];
	if (t->ts_state != RECEIVING || ph->ph_seq != t->ts_seq) {
		netenable();
		puthead(from, to, ph->ph_ident, ph->ph_seq, ACK, 0);
	}
	else {
#ifdef BUFFERED
		t->ts_savehdr = (char *) hdr;
		gotbuffer(t, from, ph->ph_ident, ph->ph_type, size);
#else
		puthead(from, to, ph->ph_ident, ph->ph_seq, ACK, 0);
		received(t, ph->ph_type, size, (char *) hdr);
#endif BUFFERED
	}
  }
}

#ifndef NOCLIENT
/* A reply packet has arrived. Send an acknowledgement if it can't do any
 * harm.
 */
static gotreply(ph, hdr)
register struct pktheader *ph;
header *hdr;
{
  register unshort from = pktfrom(ph), to = pktto(ph), size = ph->ph_size;
  register struct task *t = &task[ph->ph_dsttask & 0xFF];

  if (ph->ph_ident != t->ts_svident || ph->ph_seq != t->ts_seq)
	t = NILTASK;
  else if ((t->ts_flags & GETREP) == 0)
	if (t->ts_flags & PUTREQ) {	/* ack for request got lost */
		compare(t->ts_ident, ==, ph->ph_ident);
		getrepstart(t);		/* start the getrep */
		t->ts_signal = 0;
	}
	else
		t = NILTASK;
  if (t != NILTASK) {
	if (ph->ph_seq == 0) {
		*t->ts_rhdr = *hdr;
		headerfromnet(t->ts_rhdr);
		size -= HEADERSIZE;
	}
	else if (t->ts_state != RECEIVING)
		t = NILTASK;
  }
  if (t == NILTASK) {
	netenable();
	puthead(from, to, ph->ph_ident, ph->ph_seq, ACK, 0);
  }
  else {
#ifdef BUFFERED
	t->ts_savehdr = (char *) hdr;
	gotbuffer(t, from, ph->ph_ident, ph->ph_type, size);
#else
	puthead(from, to, ph->ph_ident, ph->ph_seq, ACK, 0);
	received(t, ph->ph_type, size, (char *) hdr);
#endif BUFFERED
  }
}
#endif NOCLIENT

/* A packet has arrived. Call an appropiate routine, after checking some
 * things.
 */
pkthandle(ph, hdr)
register struct pktheader *ph;
register char *hdr;
{
  if (ph->ph_dsttask < ntask && ph->ph_size <= PACKETSIZE)
	switch (ph->ph_type & TYPE) {
  	case LOCATE:
		if (ph->ph_size == 0 || ph->ph_ident != 0 || ph->ph_seq != 0)
			break;
		if ((ph->ph_size % PORTSIZE) != 0)
			break;
		locreq(ph, hdr);
		return(1);
	case REQUEST:
		if (ph->ph_seq == 0 && ph->ph_size < HEADERSIZE)
			break;
		gotrequest(ph, ABSPTR(header *, hdr));
		return(1);
	case ACK:
		if (ph->ph_size != 0)
			break;
		gotack(ph);
		return(1);
	case ENQUIRY:
		if (ph->ph_size != 0)
			break;
		enquiry(ph);
		return(1);
#ifndef NOCLIENT
	case HERE:
		if (ph->ph_size == 0 || ph->ph_ident != 0 || ph->ph_seq != 0)
			break;
		if ((ph->ph_size % PORTSIZE) != 0)
			break;
		here(ph, hdr);
		return(1);
	case REPLY:
		if (ph->ph_seq == 0 && ph->ph_size < HEADERSIZE)
			break;
		gotreply(ph, ABSPTR(header *, hdr));
		return(1);
	case NAK:
		if (ph->ph_size != 0 || ph->ph_seq != 0)
			break;
		gotnak(ph);
		return(1);
	case ALIVE:
		if (ph->ph_size != 0 || ph->ph_seq != 0)
			break;
		alive(ph);
		return(1);
	case DEAD:
		if (ph->ph_size != 0 || ph->ph_seq != 0)
			break;
		dead(ph);
		return(1);
#endif
	case 0:
		return(0);
  }
  return(0);
}

#ifdef notdef  /* don't need this for minix */
handle(to, from, ident, seq, what, size, hdr)	/* compatibility */
address to, from;
char ident, seq, what;
unshort size;
char *hdr;
{
  struct pktheader ph;

  ph.ph_dstnode = siteaddr(to);
  ph.ph_srcnode = siteaddr(from);
  ph.ph_dsttask = tasknum(to);
  ph.ph_srctask = tasknum(from);
  ph.ph_ident = ident;
  ph.ph_seq = seq;
  ph.ph_size = size;
  ph.ph_type = what;
  return pkthandle(&ph, hdr);
}

#endif

/* A timer has gone off too many times. See what went wrong. Restart the task.
 */
static failed(t)
register struct task *t;
{
  assert(t->ts_flags & (GETREQ | PUTREP | GETREP | PUTREQ));
#ifndef NDEBUG
  if (t->ts_flags & (GETREQ | PUTREP))
	printf("%x: client %x failed (%d)\n", t - task,
					t->ts_client, t->ts_state);
  if (t->ts_flags & (GETREP | PUTREQ))
	printf("%x: server %x failed (%d)\n", t - task,
					t->ts_server, t->ts_state);
#endif
#ifdef STATISTICS
  if (t->ts_flags & (GETREQ | PUTREP))
	amstat.ams_clfail++;
  if (t->ts_flags & (GETREP | PUTREQ))
	amstat.ams_svfail++;
#endif
  switch (t->ts_state) {
  case SENDING:		/* Message didn't arrive */
	t->ts_state = FAILED;
	assert(t->ts_flags & (PUTREQ | PUTREP));
	break;
  case WAITBUF:		/* server site has crashed */
	assert(t->ts_flags & GETREP);
  case RECEIVING:
#ifndef NOCLIENT
	if (t->ts_flags & GETREP)
		t->ts_state = FAILED;
	else
#endif
	{
		getreqstart(t);	/* client failed, restart getreq */
		return;
	}
	break;
  default:
	assert(0);
  }
  wakeup((event_t) &t->ts_state);
}

/* A timer went off. See what is wrong.
 */
static again(t)
register struct task *t;
{
  switch (t->ts_state) {
  case SENDING:			/* retransmit */
#ifdef STATISTICS
  if (t->ts_flags & (GETREQ | PUTREP))
	amstat.ams_rxcl++;
  if (t->ts_flags & (GETREP | PUTREQ))
	amstat.ams_rxsv++;
#endif
#ifdef BUFFERED
	wakeup((event_t) &t->ts_state);
#else
	if (!sendfragment(t, RETRANS))
		assert(0);
	t->ts_timer = retranstime;
#endif
	break;
  case WAITBUF:			/* Check if the server is still alive */
	assert(t->ts_flags & GETREP);
  case RECEIVING:
#ifndef NOCLIENT
	if (t->ts_flags & GETREP)  /* See if the other side is still there */
		puthead(t->ts_server, t->ts_addr, t->ts_svident,
						t->ts_signal, ENQUIRY, 0);
#endif
	t->ts_timer = retranstime;
	break;
  default:
	assert(0);
  }
}

#endif NONET	/*--------------------------------------------------*/

/* First check all the timers. If any went off call the appropiate routine.
 * Then see if there are ports to locate.
 */
netsweep(){
  register struct task *t;

  for (t = task; t < uppertask; t++) {
	if (t->ts_timer != 0 && --t->ts_timer == 0)	/* timer expired */
#ifndef NOCLIENT
		if (t->ts_flags & LOCATING)
			portquit(&t->ts_xhdr->h_port, t);
#endif
#ifndef NONET
#ifndef NOCLIENT
		else
#endif
		{
			compare(t->ts_count, !=, 0);
			if (--t->ts_count == 0)		/* serious */
				failed(t);
			else				/* try again */
				again(t);
			break;
		}
	if (t->ts_cltim != 0 && --t->ts_cltim == 0) {	/* client crashed */
#ifdef STATISTICS
		amstat.ams_clcrash++;
#endif
		putsig(t, CRASH);
	}
#endif NONET
  }
}

#ifdef BUFFERED

/*  Data has arrived. Get it. If there's more, wait for it.
 */
static recvbuf(){
  register struct task *c = curtask, *t;

  c->ts_state = WAITBUF;
  for (;;) {
#ifndef NONET
	if (c->ts_buffer == NETBUF) {		/* something from the net */
		c->ts_buffer = NOBUF;
		puthead(c->ts_sender, c->ts_addr, c->ts_ident, c->ts_seq,
								ACK, 0);
		received(c, (char) c->ts_what, c->ts_bufcnt, c->ts_savehdr);
		if (c->ts_state != RECEIVING)
			return;
	}
	else
#endif NONET
	if (c->ts_buffer != NOBUF && !putbuf(c->ts_buffer, (vir_bytes)
				(c->ts_rbuf + c->ts_offset), c->ts_bufcnt)) {
#ifndef NDEBUG
		printf("%x: bad rbuf (received from %x)\n",
						c->ts_addr, t->ts_addr);
#endif
		c->ts_state = MEMFAULT;		/* receiver fails */
		freebuf(c->ts_buffer);
		c->ts_buffer = NOBUF;
		if (c->ts_sender != NOWHERE) {
			t = taskptr(c->ts_sender);
			t->ts_state = FAILED;
			wakeup((event_t) &t->ts_state);
		}
		return;
	}
	else {		/* local copy done */
		c->ts_offset += c->ts_bufcnt;
		if (c->ts_bufcnt < BUFSIZE) {	/* last buffer */
			if (c->ts_buffer != NOBUF) {
				freebuf(c->ts_buffer);
				c->ts_buffer = NOBUF;
			}
			c->ts_state = DONE;
			return;
		}
		else {			/* more buffers expected */
			compare(c->ts_sender, !=, NOWHERE);
			c->ts_state = RECEIVING;
			t = taskptr(c->ts_sender);
			compare(t->ts_state, ==, SENDING);
			assert(t->ts_flags & (PUTREQ | PUTREP));
			wakeup((event_t) &t->ts_state);
		}
	}
	if (sleep((event_t) &c->ts_state)) {		/* wait for rest */
		portremove(&c->ts_rhdr->h_port, c->ts_addr);
		if (c->ts_buffer != NOBUF) {
			freebuf(c->ts_buffer);
			c->ts_buffer = NOBUF;
		}
		c->ts_state = ABORT;
		return;
	}
  }
}

#endif BUFFERED

/* The transaction is local. This routine does the sending.
 */
static sendbuf(t)
register struct task *t;
{
  register struct task *c = curtask;
  register unshort cnt = t->ts_rcnt < c->ts_xcnt ? t->ts_rcnt : c->ts_xcnt;
#ifdef BUFFERED
  register unshort size;
  buffer allocbuf();

  c->ts_state = SENDING;
  t->ts_sender = c->ts_addr;
  if (cnt == 0)
	t->ts_buffer = NOBUF;
  else if ((t->ts_buffer = allocbuf()) == NOBUF) {
	c->ts_xcnt = FAIL;
	c->ts_state = FAILED;
	if (!(t->ts_flags & GETREQ)) {
		t->ts_state = FAILED;	/* trans fails */
		wakeup((event_t) &t->ts_state);
	}
	return;
  }
  do {
	if ((size = cnt) != 0) {
		if (size > BUFSIZE)
			size = BUFSIZE;
		if (!getbuf(t->ts_buffer, (vir_bytes) (c->ts_xbuf +
						c->ts_offset), size)) {
#ifndef NDEBUG
			printf("%x: bad xbuf (sending to %x)\n",
						c->ts_addr, t->ts_addr);
#endif
			freebuf(t->ts_buffer);
			t->ts_buffer = NOBUF;
			c->ts_xcnt = FAIL;		/* for putrep */
			c->ts_state = MEMFAULT;
			if (!(t->ts_flags & GETREQ)) {
				t->ts_state = FAILED;	/* trans fails */
				wakeup((event_t) &t->ts_state);
			}
			break;
		}
	}
	else if (t->ts_buffer != NOBUF) {
		freebuf(t->ts_buffer);
		t->ts_buffer = NOBUF;
	}
	t->ts_bufcnt = size;
	assert(t->ts_state == WAITBUF || t->ts_state == RECEIVING);
	wakeup((event_t) &t->ts_state);
	if (size != BUFSIZE) {		/* last buffer */
		t->ts_sender = NOWHERE;
		c->ts_state = DONE;
		break;
	}
	cnt -= size;
	c->ts_offset += size;
	if (sleep((event_t) &c->ts_state))
		c->ts_state = ABORT;
  } while (c->ts_state == SENDING);
#else
  compare(t->ts_state, ==, WAITBUF);
  if (cnt != 0) {
	register phys_bytes rbuf, xbuf;

	if ((rbuf = umap(t, t->ts_rbuf, (vir_bytes) cnt)) == 0) {
#ifndef NDEBUG
		printf("bad rbuf (%X,%X)\n", t->ts_rbuf, (vir_bytes) cnt);
#endif
		c->ts_state = FAILED;
		c->ts_xcnt = FAIL;
		t->ts_state = MEMFAULT;
		wakeup((event_t) &t->ts_state);
		return;
	}
	if ((xbuf = umap(c, c->ts_xbuf, (vir_bytes) cnt)) == 0) {
#ifndef NDEBUG
		printf("bad xbuf (%X,%X)\n", c->ts_xbuf, (vir_bytes) cnt);
#endif
		c->ts_state = MEMFAULT;
		c->ts_xcnt = FAIL;
		if (t->ts_flags & GETREQ)
			getreqstart(t);	/* client failed, restart getreq */
		else {
			t->ts_state = FAILED;
			wakeup((event_t) &t->ts_state);
		}
		return;
	}
	phys_copy(xbuf, rbuf, (phys_bytes) cnt);
  }
  t->ts_offset = cnt;
  t->ts_state = DONE;
  wakeup((event_t) &t->ts_state);
  c->ts_state = DONE;
#endif BUFFERED
}

#ifndef NOCLIENT
/* The transaction is local. Send the request to the server.
 */
static recvrequest(){
  register address to;
  register struct task *c = curtask, *t;

  if ((to = portlookup(&c->ts_xhdr->h_port, NOWAIT, DELETE)) == NOWHERE)
	return(0);
#ifndef NONET
  if (siteaddr(to) != local)
	return(0);
#endif
  t = taskptr(to);
  c->ts_server = to;
  t->ts_client = c->ts_addr;
  t->ts_clident = c->ts_svident;
  *t->ts_rhdr = *c->ts_xhdr;
  sendbuf(t);
  return(c->ts_xcnt != FAIL);
}
#endif

/* A task calls this routine when it wants to be blocked awaiting a request.
 * It specifies the header containing the port to wait for, a buffer where
 * the data must go and the size of this buffer. Getreq returns the size of
 * the request when one arrives.
 */
unshort getreq(hdr, buf, cnt)
header *hdr;
bufptr buf;
unshort cnt;
{
  register struct task *c = curtask;

  if (c->ts_flags != 0 || cnt > MAXCNT)
	return(FAIL);
  if (NullPort(&hdr->h_port))
	return(FAIL);
#ifdef STATISTICS
  amstat.ams_getreq++;
#endif
  c->ts_rhdr = hdr;
  c->ts_rbuf = buf;
  c->ts_rcnt = cnt;
  c->ts_flags |= GETREQ;
  getreqstart(c);
  if (sleep((event_t) &c->ts_state)) {
	portremove(&hdr->h_port, c->ts_addr);
	c->ts_state = ABORT;
  }
#ifdef BUFFERED
  if (c->ts_state == WAITBUF)
	recvbuf();
#endif
  c->ts_flags &= ~GETREQ;
  switch (c->ts_state) {
  case DONE:
	c->ts_flags |= SERVING;
#ifndef NONET
	if (area(c->ts_client) != LOCAL)
		c->ts_cltim = clientcrash;
#endif NONET
	cnt = c->ts_offset;
	break;
  case ABORT:
	cnt = ABORTED;
	break;
  case MEMFAULT:
	cnt = BADADDRESS;
	break;
  default:
	assert(0);
  }
  c->ts_state = IDLE;
  return(cnt);
}

/* A task wants to send a reply to its client. Putrep returns the size of
 * the reply.
 */
unshort putrep(hdr, buf, cnt)
header *hdr;
bufptr buf;
unshort cnt;
{
  register struct task *c = curtask;

  if (c->ts_flags != SERVING)
	return(FAIL);
  c->ts_flags &= ~SERVING;
  if (cnt > MAXCNT)
	return(FAIL);
#ifdef STATISTICS
  amstat.ams_putrep++;
#endif
  c->ts_cltim = 0;
  c->ts_xhdr = hdr;
  c->ts_xbuf = buf;
  c->ts_xcnt = cnt;
  c->ts_offset = 0;
  c->ts_flags |= PUTREP;
#ifndef NONET
  if (siteaddr(c->ts_client) != local)
	send();
  else
#endif
  {		/* local transaction */
	register struct task *t = taskptr(c->ts_client);

	if (t->ts_server == c->ts_addr) {
		*t->ts_rhdr = *hdr;
		sendbuf(t);
	}
  }
  c->ts_flags &= ~PUTREP;
  if (c->ts_state == MEMFAULT)
	cnt = BADADDRESS;
  c->ts_state = IDLE;
  return(cnt);
}

#ifndef NOCLIENT
/* Somebody wants to contact a server, and wait for a reply. The port this
 * server should listen to is specified in the first header. The reply
 * comes in the second. Trans returns the size of the reply, or FAIL if
 * a transaction fails after the server has been located.
 */
unshort trans(hdr1, buf1, cnt1, hdr2, buf2, cnt2)
header *hdr1, *hdr2;
bufptr buf1, buf2;
unshort cnt1, cnt2;
{
  register struct task *c = curtask;

  if ((c->ts_flags & ~SERVING) || cnt1 > MAXCNT || cnt2 > MAXCNT)
	return(FAIL);
  if (NullPort(&hdr1->h_port))
	return(FAIL);
#ifdef STATISTICS
  amstat.ams_trans++;
#endif
  for (;;) {
	c->ts_state = IDLE;
	c->ts_xhdr = hdr1; c->ts_xbuf = buf1; c->ts_xcnt = cnt1;
	c->ts_rhdr = hdr2; c->ts_rbuf = buf2; c->ts_rcnt = cnt2;
	c->ts_signal = 0;
	if (!PortCmp(&c->ts_portcache, &hdr1->h_port)) {
		c->ts_flags |= LOCATING;
		c->ts_timer = c->ts_maxloc;
		c->ts_totloc -= ticker;
		c->ts_server = portlookup(&hdr1->h_port, WAIT, LOOK);
		c->ts_totloc += ticker;
		c->ts_timer = 0;
		c->ts_flags &= ~LOCATING;
		switch (c->ts_server) {
		case NOWHERE:		/* server not found */
			c->ts_portcache = NULLPORT;
			return(c->ts_signal == 0 ? NOTFOUND : ABORTED);
		case SOMEWHERE:
			c->ts_portcache = NULLPORT;
			return(FAIL);
		}
		c->ts_portcache = hdr1->h_port;
	}
#ifdef notdef
	else
		c->ts_server = siteaddr(c->ts_server);
#endif
	c->ts_svident++;
	c->ts_offset = 0;
	c->ts_flags |= PUTREQ;
#ifndef NONET
	if (siteaddr(c->ts_server) != local) {
#ifdef STATISTICS
		amstat.ams_remtrans++;
#endif
		c->ts_totsvr -= ticker;
		send();
		c->ts_flags &= ~PUTREQ;
#ifdef BUFFERED
		if (c->ts_state == WAITBUF)
			recvbuf();	/* await the reply */
#endif
		c->ts_totsvr += ticker;
		if (c->ts_state == NACKED || c->ts_state == FAILED) {
			portremove(&hdr1->h_port, siteaddr(c->ts_server));
			c->ts_portcache = NULLPORT;
		}
		if (c->ts_state != NACKED)
			break;
#ifdef STATISTICS
		amstat.ams_naks++;
#endif
		c->ts_portcache = NULLPORT;
	}
	else
#endif NONET
	if (recvrequest()) {	/* local transaction */
#ifdef STATISTICS
		amstat.ams_loctrans++;
#endif
		c->ts_flags &= ~PUTREQ;
		c->ts_flags |= GETREP;
		c->ts_totsvr -= ticker;
		c->ts_offset = 0;
		c->ts_state = WAITBUF;
		if (c->ts_signal != 0) {
			putsig(taskptr(c->ts_server),
					(unshort) (c->ts_signal & 0xFF));
			c->ts_signal = 0;
		}
		if (sleep((event_t) &c->ts_state))
			c->ts_state = ABORT;
#ifdef BUFFERED
		else
			recvbuf();
#endif
		c->ts_totsvr += ticker;
		break;
	}
	else {		/* too bad, try again */
		c->ts_flags &= ~PUTREQ;
		if (c->ts_state == MEMFAULT)
			break;
	}
	c->ts_portcache = NULLPORT;
	if (c->ts_signal != 0) {
		c->ts_state = ABORT;
		break;
	}
  }
  c->ts_signal = 0;
  c->ts_flags &= ~(PUTREQ | GETREP);
  if (c->ts_state == DONE) {
	c->ts_state = IDLE;
	return c->ts_offset;
  }
#ifndef NDEBUG
  printf("trans failed with %x (state = %d; command = %d; port ",
			c->ts_server, c->ts_state, c->ts_xhdr->h_command);
  prport(&c->ts_xhdr->h_port);
  printf(")\n");
#endif
  switch (c->ts_state) {
  case FAILED:
  case ABORT:
	cnt2 = FAIL;
	break;
  case MEMFAULT:
	cnt2 = BADADDRESS;
	break;
  default:
	assert(0);
  }
  c->ts_state = IDLE;
  return(cnt2);
}
#endif NOCLIENT

/* If doing a transaction, send a signal to the server. For a remote server,
 * the signal is sent along with enquiries. If it's still locating a server,
 * abort that.  If it isn't doing a transaction, but blocked in a getreq,
 * abort that.
 */
sendsig(t, signal)
register struct task *t;
char signal;
{
#ifndef NOCLIENT
  if (t->ts_flags & (LOCATING | PUTREQ | GETREP))
	t->ts_signal = signal;
  if (t->ts_flags & LOCATING)
	portquit(&t->ts_xhdr->h_port, t);
  else if (t->ts_state == WAITBUF)
	if (t->ts_flags & GETREQ) {
		portremove(&t->ts_rhdr->h_port, t->ts_addr);
		t->ts_state = ABORT;
		wakeup((event_t) &t->ts_state);
	}
	else if (signal != 0 && (t->ts_flags & GETREP))
#ifndef NONET
		if (area(t->ts_server) != LOCAL)
			puthead(t->ts_server, t->ts_addr, t->ts_svident,
							signal, ENQUIRY, 0);
		else
#endif NONET
		{
			putsig(taskptr(t->ts_server),
						(unshort) (signal & 0xFF));
			t->ts_signal = 0;
		}
#endif NOCLIENT
}

#ifndef NOCLIENT
/* Abort anything task s is doing. If the task is serving somebody, notify
 * it that the server has failed.
 */
destroy(s)
register struct task *s;
{
  register struct task *t;

  sendsig(s, (char) CRASH);
  if (s->ts_flags & SERVING)
#ifndef NONET
	if (area(s->ts_client) != LOCAL) {
		puthead(s->ts_client, s->ts_addr, s->ts_clident, 0, DEAD, 0);
		s->ts_cltim = 0;
	}
	else
#endif
	{
#ifndef NDEBUG
		printf("%x destroyed, %x victim\n", s->ts_addr, s->ts_client);
#endif
		t = taskptr(s->ts_client);
		if (t->ts_state == WAITBUF) {
			assert(t->ts_flags & GETREP);
			t->ts_timer = 0;
			t->ts_state = FAILED;
			wakeup((event_t) &t->ts_state);
		}
	}
  s->ts_timer = 0;
  if (s->ts_flags & (LOCATING|GETREQ|GETREP|PUTREQ|PUTREP)) {
	s->ts_state = ABORT;
	wakeup((event_t) &s->ts_state);
  }
  else {
	s->ts_state = IDLE;
	s->ts_flags = 0;
  }
  s->ts_server = s->ts_client = 0;
  s->ts_portcache = NULLPORT;
}

/* Clean up the mess.
 */
cleanup(){
  register struct task *c = curtask;

  compare(c->ts_state, ==, IDLE);
  destroy(c);
}
#endif


/* Limit the maximum locate time & service time. 0 is infinite.
 */
unshort timeout(maxloc)
unshort maxloc;
{
  unshort oldloc = curtask->ts_maxloc;

  curtask->ts_maxloc = maxloc;
  return(oldloc);
}

#ifndef NDEBUG
transdump(){
  register struct task *t;
  static char *states[] = {
	"IDLE", "SENDING", "DONE", "ACKED", "NACKED", "FAILED",
	"WAITBUF", "RECEIVING", "ABORT", "MEMFAULT"
  };
  static struct ftab {
	unshort flag;
	char *name;
  } ftab[] = {
	{ GETREQ,	"GETREQ"  }, { SERVING,	 "SERVING" },
	{ PUTREP,	"PUTREP"  }, { LOCATING, "LOCATE"  },
	{ PUTREQ,	"PUTREQ"  }, { GETREP,	 "GETREP"  },
  };
  register struct ftab *p;

  printf("\nTK STATE    CTM TIM CNT  CLT  SRV CLI SVI SEQ SIG FLAGS\n");
  for (t = task; t < uppertask; t++) {
	if (t->ts_state == IDLE && t->ts_flags == 0) {
		compare(t->ts_cltim, ==, 0);
		compare(t->ts_timer, ==, 0);
		compare(t->ts_signal, ==, 0);
		continue;
	}
	printf("%2d %9s%3d %3d %3d %4x %4x %3d %3d %3d %3d", t - task,
		states[t->ts_state], t->ts_cltim, t->ts_timer, t->ts_count,
		t->ts_client, t->ts_server, t->ts_clident & 0xFF,
		t->ts_svident & 0xFF, t->ts_seq & 0xFF, t->ts_signal & 0xFF);
	for (p = ftab; p < &ftab[sizeoftable(ftab)]; p++)
		if (t->ts_flags & p->flag)
			printf(" %s", p->name);
	if (t->ts_flags & (GETREQ | LOCATING | GETREP)) {
		printf("  '");
		prport(t->ts_flags & GETREQ ? &t->ts_rhdr->h_port
							: &t->ts_xhdr->h_port);
		printf("'");
	}
	printf("\n");
  }
}
#endif NDEBUG

trinit(){
  curtask->ts_addr = ((curtask - task) << 8 | local);
}

/* Get the site address.
 */
transinit(){
#ifdef NONET
  local = 1;
#else
  extern address interinit();

  local = siteaddr(interinit());
/*
  netenable();
*/
#endif NONET
}

amdump()
{
#ifdef STATISTICS
  printf("\nAmoeba statistics:\n");
  printf("clfail  %7D ", amstat.ams_clfail);
  printf("svfail  %7D ", amstat.ams_svfail);
  printf("clcrash %7D ", amstat.ams_clcrash);
  printf("rxcl    %7D ", amstat.ams_rxcl);
  printf("rxsv    %7D\n",amstat.ams_rxsv);
  printf("trans   %7D ", amstat.ams_trans);
  printf("local   %7D ", amstat.ams_loctrans);
  printf("remote  %7D ", amstat.ams_remtrans);
  printf("getreq  %7D ", amstat.ams_getreq);
  printf("putrep  %7D\n",amstat.ams_putrep);
  printf("naks    %7D\n",amstat.ams_naks);
#endif
}
