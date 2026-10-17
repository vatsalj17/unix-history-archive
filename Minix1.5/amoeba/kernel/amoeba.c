/*
**	Minix /dev/amoeba driver
**
**	This file contains two kernel servers: amint_task and amoeba_task.
**	amoeba_task implements transactions for user tasks and amint_task
**	handles asynchronous events such as timeouts, incoming ethernet
**	packets and interrupts.
**
**	An amoeba_task is permanently assigned to a process until a transaction
**	is complete.  If you do a getreq then the kernel task remains
**	allocated until after the putrep or the server dies.
**	There is a limit of one operation at a time except in the case that a
**	getreq is followed by a trans.
**
**	The value of curtask is only correct if there is non-preemptive
**	scheduling of kernel tasks.  It is reset by am_sleep when it returns
**	which keeps it pointing to the correct place.
**
**	Lines marked HACK are of doubtful portability but produce efficient
**	code.
*/

#define NDEBUG

#include "kernel.h"
#include "minix/com.h"
#include "minix/callnr.h"
#include "signal.h"
#include "proc.h"
#include "protect.h"

#include "amoeba.h"
#undef umap
#include "amparam.h"
#include "global.h"
#define	MPX
#define	TRANS
#include "task.h"
#include "assert.h"
#include "internet.h"
#include "etherformat.h"
#include "byteorder.h"

/* amoeba task table - can't alloc memory in minix kernel */
PUBLIC struct task	am_task[AM_NTASKS];

#if !NONET

#define	ETH_HDRS	(sizeof (Framehdr))
#define	HSZ		(ETH_HDRS + HEADERSIZE) /* watchout for alignment */
#define	FAKESITENO	0xff			/* to bluff trans.c */
#define	MAPENTRIES	127
/* two hacks for speed */
#define	EANULL(a)	NullPort((port *) (a))			/* HACK! */
#define	EACMP(a, b)	PortCmp((port *) (a), (port *) (b))	/* HACK! */

PRIVATE Etherpacket	Packet;	/* the latest arrived amoeba ethernet packet */
PRIVATE phys_bytes	Bufaddr;/*physical address of Packet */

PRIVATE phys_bytes	Inptr;	/* used by pickoff() & getall() to copy data */
PRIVATE	unsigned	Insiz;	/* total size of received packet */
PRIVATE phys_bytes	Outptr;	/* pointer to pos currently building packet */
PRIVATE	unsigned	Outsiz;	/* size of currently building packet */
PRIVATE phys_bytes	Xmtbuf;	/* Pointer to current ethernet write buffer */
PRIVATE	Eth_addr	Myaddr;	/* ether address of this host */
PRIVATE Eth_addr	Gwaddr;	/* ether address of pronet gateway */
PUBLIC  vir_bytes	eplus_seg; /* segment containing Etherplus buffer */

/* broadcast address for ethernet. see next comment. */
PRIVATE	Eth_addr	Broadcastaddr = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
/* the 128th map entry is to hold the broadcast address.  can you say HACK? */
PRIVATE Eth_addr	Eamap[MAPENTRIES+1];

#endif


/*===========================================================================*
 *                            amoeba_task                                    *
 *===========================================================================*/
PUBLIC void
amoeba_task()
{
    void	am_reply();
    message	mess;
    int		mytask;

    mytask = AMOEBA_CLASS - proc_number(proc_ptr);
    curtask = &am_task[mytask];  /* make me the current amoeba task */
    am_init();
    while (TRUE)
    {
	receive(ANY, &mess);
	curtask = &am_task[mytask];  /* make me the current amoeba task */
	switch (mess.AM_OP)
	{
	case AM_TRANS:
	    do_trans(&mess);
	    break;
	case AM_GETREQ:
	    do_getreq(&mess);
	    break;
	case AM_PUTREP:
	    do_putrep(&mess);
	    break;
	default:
	    if (mess.m_source >= 0)
		am_reply(TASK_REPLY, mess.m_source, mess.AM_PROC_NR, 1, EINVAL);
	    break;
	} /* end switch */
    } /* end while */
}


/*===========================================================================*
 *                                am_init                                    *
 *===========================================================================*/
PRIVATE int
am_init()
{
/* non-pre-emptive scheduling is assumed here for initialisation! */
    static int Initialised;

    if (Initialised == 0)
    {		    /* set up the ethernet driver and init the tables */
	Initialised++;

	uppertask = &am_task[AM_NTASKS];
	ntask = AM_NTASKS;
#if !NONET
	eplus_seg = protected_mode ? EPLUS_SELECTOR 
				   : physb_to_hclick(EPLUS_BASE);
	net_init();
#endif
	transinit();
	portinit();
    }
    trinit();
    curtask->mx_flags = RUNNABLE;
}


/*===========================================================================*
 *                             amint_task                                    *
 *===========================================================================*/
PUBLIC void
amint_task()
{
  phys_bytes	umap();
  message	mess;
  struct task * t;

#if !NONET
  Bufaddr = umap(proc_addr(AMINT_CLASS), D, (vir_bytes)&Packet, (vir_bytes)HSZ);
#endif

  set_timer();	/* start the netsweep timer */
  while (TRUE){
	receive(ANY, &mess);
	switch (mess.m_type){
#if !NONET
	case HARD_INT:		/* an ethernet packet arrived */
	    recvintr();
	    enable_irq(ETHER_IRQ);/* reenable ether ints */
	    break;
#endif
	case AM_TIMEOUT:	/* run transaction sweepers every 0.1 secs */
	    netsweep();
	    portsweep();
	    set_timer();	/* reset the timeout */
	    break;
	case AM_PUTSIG:		/* user typed a del or a quit or a kill */
	    sendsig(&am_task[mess.AM_COUNT], 1);
	    break;
	case AM_TASK_DIED:	/* a user task died while doing an operation */
	    t = &am_task[mess.AM_COUNT];
	    if (t->mx_active)	/* if transaction record is still valid */
	    {
		destroy(t);	/* then destroy it */
		t->mx_proc_nr = 0;
		t->mx_active = 0;
		t->mx_flags = 0;
	    }
	    break;
	default:
	    break;
	}
    }
}
/*===========================================================================*
 *                             got_packet                                    *
 *===========================================================================*/
PUBLIC
got_packet()
{
/* Called by dp8390_int() when receive interrupt for arriving packet. */
  interrupt(AMINT_CLASS);
}

/*===========================================================================*
 *                             set_timer                                     *
 *===========================================================================*/
PRIVATE
set_timer()
{
    message	mess;
    int		am_runsweep();

    mess.m_type = SET_ALARM;
    mess.CLOCK_PROC_NR = AMINT_CLASS;
    mess.DELTA_TICKS = HZ/10;		/* every 0.1 seconds ! */
    mess.FUNC_TO_CALL = (void (*)())am_runsweep;
    sendrec(CLOCK, &mess);
}


/*===========================================================================*
 *                            am_runsweep                                    *
 *===========================================================================*/
PRIVATE
am_runsweep()
{
    message	mess;

    mess.m_type = AM_TIMEOUT;
    send(AMINT_CLASS, &mess);
}


/*===========================================================================*
 *                               do_trans                                    *
 *===========================================================================*/
PRIVATE
do_trans(m_ptr)
message *	m_ptr;
{
    header *	am_starttask();
    void	am_endtask();
    unshort	trans();
    void	am_reply();

    unshort	ret;
    header *	hdr;
    Trpar	param;	/* parameter block for transaction */

#define	req	param.tp_par[0]
#define	rep	param.tp_par[1]

/* copy in parameter block */
    if (get_param(m_ptr, &param) == 0)
    {
	am_reply(TASK_REPLY, m_ptr->m_source, m_ptr->AM_PROC_NR, 1, EFAULT);
	return;
    }
/* copy header in */
    hdr = am_starttask(m_ptr);
    if (get_header(m_ptr, req.p_hdr, hdr) == 0)
    {
	am_endtask();
	am_reply(TASK_REPLY, m_ptr->m_source, m_ptr->AM_PROC_NR, 1, EFAULT);
	return;
    }
/* reply to FS to suspend luser task */
    am_reply(TASK_REPLY, m_ptr->m_source, m_ptr->AM_PROC_NR, 0, SUSPEND);
/* start locate timer */
    (void)am_timeout(param.tp_maxloc);
/* call trans */
    ret = trans(hdr, req.p_buf, req.p_cnt, hdr, rep.p_buf, rep.p_cnt);
/* copy header to luser task (trans already copied the data) */
    if ((short)ret >= 0 && put_header(m_ptr->AM_PROC_NR, hdr, rep.p_hdr) == 0)
    {
	am_endtask();
	am_reply(AM_REVIVE, m_ptr->m_source, m_ptr->AM_PROC_NR, 1, EFAULT);
	return;
    }
    am_endtask();
/* revive luser task */
    am_reply(AM_REVIVE, m_ptr->m_source, m_ptr->AM_PROC_NR, 1, (int)ret);
}


/*===========================================================================*
 *                              do_getreq                                    *
 *===========================================================================*/
PRIVATE
do_getreq(m_ptr)
message *	m_ptr;
{
    unshort	getreq();
    header *	am_starttask();
    void	am_endtask();
    void	am_reply();

    Trpar	param;
    unshort	ret;
    header *	hdr;
    int		free;

/* copy parameter block for getreq */
    if (get_param(m_ptr, &param) == 0)
    {
	am_reply(TASK_REPLY, m_ptr->m_source, m_ptr->AM_PROC_NR, 1, EFAULT);
	return;
    }
/* copy header in */
    hdr = am_starttask(m_ptr);
    if (get_header(m_ptr, param.tp_par[0].p_hdr, hdr) == 0)
    {
	am_endtask();
	am_reply(TASK_REPLY, m_ptr->m_source, m_ptr->AM_PROC_NR, 1, EFAULT);
	return;
    }
/* reply to FS to suspend luser task */
    am_reply(TASK_REPLY, m_ptr->m_source, m_ptr->AM_PROC_NR, 0, SUSPEND);
/* call getreq */
    ret = getreq(hdr, param.tp_par[0].p_buf, param.tp_par[0].p_cnt);
    free = 0;
    if ((short)ret < 0)	/* getreq failed */
    {
	free = 1;
	am_endtask();
    }
    else    /* copy header to luser task (getreq already copied the data) */
	if (put_header(m_ptr->AM_PROC_NR, hdr, param.tp_par[0].p_hdr) == 0)
	{
	    free = 1;
	    ret = EFAULT;
	    am_endtask();
	}
/* restart luser task but don't free the kernel task! */
    am_reply(AM_REVIVE, m_ptr->m_source, m_ptr->AM_PROC_NR, free, (int)ret);
}
dump_param(p)
Trpar *p;
{
}
dump_header(h)
header *h;
{
  int i;

  printf("header\n");
  printf("port: ");
  for (i = 0; i < PORTSIZE; i++)
	printf(" %x ", h->h_port._portbytes[i]);
  printf("\n");
  printf("h_command = %u, h_offset = %d, h_size = %d\n",
	h->h_command, h->h_offset, h->h_size);
}

/*===========================================================================*
 *                              do_putrep                                    *
 *===========================================================================*/
PRIVATE
do_putrep(m_ptr)
message *	m_ptr;
{
    unshort	putrep();
    void	am_reply();

    Trpar	param;
    header *	hdr;
    unshort	ret;

/* copy in parameter block */
    if (get_param(m_ptr, &param) == 0)
    {
	am_reply(TASK_REPLY, m_ptr->m_source, m_ptr->AM_PROC_NR, 1, EFAULT);
	return;
    }
/* make sure that there was a getrequest */
    if (!curtask->mx_active || curtask->mx_proc_nr != m_ptr->AM_PROC_NR)
    {
	am_reply(TASK_REPLY, m_ptr->m_source, m_ptr->AM_PROC_NR, 1, (int)FAIL);
	return;
    }
/* copy in header */
    hdr = &curtask->mx_hdr;
    if (get_header(m_ptr, param.tp_par[0].p_hdr, hdr) == 0)
    {
	am_endtask();
	am_reply(TASK_REPLY, m_ptr->m_source, m_ptr->AM_PROC_NR, 1, EFAULT);
	return;
    }
/* tell FS to suspend luser task */
    am_reply(TASK_REPLY, m_ptr->m_source, m_ptr->AM_PROC_NR, 0, SUSPEND);
/* send reply */
    ret = putrep(hdr, param.tp_par[0].p_buf, param.tp_par[0].p_cnt);
    am_endtask();
/* restart luser task */
    am_reply(AM_REVIVE, m_ptr->m_source, m_ptr->AM_PROC_NR, 1, (int)ret);
}


/*===========================================================================*
 *                              do_arrive                                    *
 *===========================================================================*/
PRIVATE
do_arrive(addr)
  phys_bytes addr;
{
#if !NONET
/*
**	we already know it is an amoeba packet, otherwise we wouldn't get here.
**	copy in the ethernet header, the internet header and the amoeba header
**	and then call packet handle.  getall and pickoff do the rest!
*/
    phys_copy(addr, Bufaddr, (long)HSZ);

/* fix amoeba size field - NB. the following is a macro */
    dec_s_le(&Packet.ep_fr.f_ah.ph_size);
/* if packethandle succeeds it calls netenable itself! */
    if (check(&Packet))	/* then give it to the transaction layer */
    {
	Inptr = addr + HSZ;	/* pointer to the data after headers */
	Insiz = Packet.ep_fr.f_ah.ph_size;
	Packet.ep_fr.f_ah.ph_size -= ETH_HDRS;
	Packet.ep_fr.f_ah.ph_dstnode = FAKESITENO;
	if (!pkthandle(&Packet.ep_fr.f_ah, Packet.ep_data))
	    am_netenable();
    }
    else
	am_netenable();
#endif
}


/*===========================================================================*
 *                               am_reply                                    *
 *===========================================================================*/
PRIVATE void
am_reply(code, replyee, proc_nr, free_it, status)
int	code;			/* TASK_REPLY or revive */
int	replyee;		/* destination address for the reply */
int	proc_nr;		/* to whom the reply should go */
int	free_it;		/* if a getreq, then don't free this task */
int	status;			/* reply code */
{
/* send reply to process doing a trans, getreq or putrep or anything else */
    message	a_mess;

    a_mess.m_type = AM_SYSCALL;
    a_mess.AM_OP = code;
    a_mess.AM_FREE_IT = (long)free_it;
    a_mess.AM_PROC_NR = proc_nr;
    a_mess.AM_STATUS = status;
    send(replyee, &a_mess);
}


/*===========================================================================*
 *                           am_starttask                                    *
 *===========================================================================*/
PRIVATE header *
am_starttask(m_ptr)
message *	m_ptr;
{
/* check to see if already doing a transaction for this user */
    if (curtask->mx_active && curtask->mx_proc_nr == m_ptr->AM_PROC_NR)
    {
	assert(curtask->mx_flags & RUNNABLE);
	assert(!(curtask->mx_flags & NESTED));
	curtask->mx_flags |= NESTED;
    }
    else
    {
	curtask->mx_proc_nr = m_ptr->AM_PROC_NR;
	curtask->mx_active = 1;
	curtask->mx_flags = RUNNABLE;
    }
    return &curtask->mx_hdr;
}


/*===========================================================================*
 *                             am_endtask                                    *
 *===========================================================================*/
PRIVATE void
am_endtask()
{
    if (curtask->mx_flags & NESTED)
	curtask->mx_flags &= ~NESTED;
    else
    {
	am_cleanup();
	curtask->mx_flags = 0;
	curtask->mx_active = 0;
    }
}


/*===========================================================================*
 *                             get_param                                     *
 *===========================================================================*/
PRIVATE
get_param(m_ptr, param)
message *	m_ptr;
Trpar *		param;
{
    phys_bytes	umap();
    phys_bytes	src;	/* physical address of parameter block */
    phys_bytes  dst;	/* kernel buffer */

/* copy parameter block for trans */
    if (m_ptr->AM_COUNT != (int)sizeof (Trpar) ||
	(src = umap(proc_addr(m_ptr->AM_PROC_NR), D,
		(vir_bytes)m_ptr->AM_ADDRESS, (vir_bytes)sizeof (Trpar))) == 0)
	return 0;
    dst = umap(proc_ptr, D, (vir_bytes)param,
					(vir_bytes)sizeof (Trpar));
    phys_copy(src, dst, (long)sizeof (Trpar));
    return 1;
}


/*===========================================================================*
 *                             get_header                                    *
 *===========================================================================*/
PRIVATE
get_header(m_ptr, h_src, h_dest)
message *	m_ptr;
header *	h_src;
header *	h_dest;
{
/* get amoeba header from user space */
    phys_bytes	umap();
    phys_bytes	src;	/* user's header */
    phys_bytes	dst;	/* kernel buffer for header */

    if ((src = umap(proc_addr(m_ptr->AM_PROC_NR), D, (vir_bytes)h_src,
					(vir_bytes)sizeof (header))) == 0)
	return 0;
    dst = umap(proc_ptr, D, (vir_bytes)h_dest,
					(vir_bytes)sizeof (header));
    phys_copy(src, dst, (long)sizeof (header));
    return 1;
}


/*===========================================================================*
 *                             put_header                                    *
 *===========================================================================*/
PRIVATE
put_header(proc_nr, h_src, h_dest)
int		proc_nr;
header *	h_src;
header *	h_dest;
{
/* write an amoeba header into user space */
    phys_bytes	umap();
    phys_bytes	src;
    phys_bytes	dst;

    if ((dst = umap(proc_addr(proc_nr), D, (vir_bytes)h_dest,
					(vir_bytes)sizeof (header))) == 0)
	return 0;
    src = umap(proc_ptr, D, (vir_bytes)h_src,
					(vir_bytes)sizeof (header));
    phys_copy(src, dst, (long)sizeof (header));
    return 1;
}


/*
**	routines which are needed by trans.c
*/

/*===========================================================================*
 *                                 am_umap                                   *
 *===========================================================================*/
PUBLIC phys_bytes
am_umap(a, b, c)
struct task * a;
vir_bytes b;
vir_bytes c;
{
/* the umap in trans.c needs to be converted to minix umap */
    phys_bytes	umap();

    return umap(proc_addr(a->mx_proc_nr), D, b, c);
}


/*===========================================================================*
 *                                am_psig                                    *
 *===========================================================================*/
PUBLIC
am_psig(t, sig)
struct task *	t;
unshort		sig;
{
/* should propagate if between a g & p and remember sig! */
    sendsig(t, (char)sig);	/* propagate signal to servers */
    cause_sig(t->mx_proc_nr, SIGAMOEBA);
}


/*
** sleep and wakeup don't fit into the amoeba model too well.
** the following are hacks and don't give true sleep and wakeup
** semantics.  they also do not take account of interrupts but seem to work.
*/

/*===========================================================================*
 *                               am_sleep                                    *
 *===========================================================================*/
am_sleep(addr)
event_t	addr;
{
    message	mess;
    struct task *	c;

    c = curtask;
    receive(ANY, &mess);
    if (mess.AM_ADDRESS != addr)
	printf("am_sleep: woken badly %x %x\n", mess.ADDRESS, addr);
    curtask = c;
    return 0;
}


/*===========================================================================*
 *                               am_wakeup                                   *
 *===========================================================================*/
am_wakeup(addr)
event_t addr;
{
    message mess;
    int tasknr;

    mess.AM_ADDRESS = addr;	
    tasknr = ((struct task *)addr - am_task); /* HACK */
    if (am_task[tasknr].mx_active)  /* don't wake it up if it is dead! */
	send(AMOEBA_CLASS - tasknr, &mess);
}


#ifndef NDEBUG

#define PRINTABLE(c) (((c) >= ' ' && (c) <= '~') ? (c) : '?')


/*===========================================================================*
 *                                  prport                                   *
 *===========================================================================*/
PUBLIC
prport(p)
port * p;
{
    int i;

    for (i = 0; i < PORTSIZE; i++)
		printf("%c", PRINTABLE(p->_portbytes[i]));
}

#endif


#if !NONET

/*===========================================================================*
 *                              interinit                                    *
 *===========================================================================*/
PUBLIC address
interinit()
{
    return 0xFF;
}


/*===========================================================================*
 *                                  check                                    *
 *===========================================================================*/
PRIVATE
check(p)
Etherpacket *	p;
{
/* make sure that an ethernet packet is a valid amoeba packet */
    if (p->ep_fr.f_ah.ph_srcnode == 0)	/* from an ethernet host */
    {
	if ((p->ep_fr.f_ah.ph_srcnode = ealookup(&p->ep_fr.f_srcaddr)) == 0)
	{
	    printf("ethernet mapping table overflow\n");
	    return 0;
	}
    }
    else	/* was the packet from ProNet? */
#ifdef PRONET
    {
	if (p->ep_fr.f_ah.ph_srcnode & ETHERBITS)
	    return 0;
    /* a packet from the pronet gateway */
	if (EANULL(&Gwaddr))
	{
	    Gwaddr = p->ep_fr.f_srcaddr;
	    pr_addr("Gateway to pronet at", &Gwaddr);
	}
	else
	    if (!EACMP(&Gwaddr, &p->ep_fr.f_srcaddr))
		pr_addr("Second gateway claims to be at", &p->ep_fr.f_srcaddr);
    }
#else
	return 0;
#endif PRONET
    return 1;
}


/*===========================================================================*
 *                                pr_addr                                    *
 *===========================================================================*/
PRIVATE
pr_addr(s, p)
char *		s;
Eth_addr *	p;
{
/* print an ethernet address */
    printf("%s %x:%x:%x:%x:%x:%x\n", s,
	p->e[0] & 0xff,
	p->e[1] & 0xff,
	p->e[2] & 0xff,
	p->e[3] & 0xff,
	p->e[4] & 0xff,
	p->e[5] & 0xff);
}


/*===========================================================================*
 *                             am_puthead                                    *
 *===========================================================================*/
PUBLIC
am_puthead(dst, src, ident, seq, type, size)
address	dst;
address	src;
char	ident;
char	seq;
char	type;
unshort	size;
{
    phys_bytes	umap();
    phys_bytes	eth_getbuf();

    unshort	totalsize;
    char	dstnode;
    Framehdr	fh;
    phys_bytes	phd;

    totalsize = size + sizeof (Framehdr);
    compare(totalsize, <=, 1514);
    fh.f_ah.ph_dstnode = dstnode = lobyte(dst);
    if ((dstnode & ETHERBITS) == 0)
    {
	assert(!EANULL(&Gwaddr));
	fh.f_dstaddr = Gwaddr;
    }
    else	/* broadcast is also handled here! */
	fh.f_dstaddr = Eamap[dstnode & 0x7f];
    fh.f_srcaddr = Myaddr;
    fh.f_proto = AMOEBAPROTO;
    enc_s_be(&fh.f_proto);

    fh.f_ah.ph_srcnode = 0;
    fh.f_ah.ph_dsttask = hibyte(dst);
    fh.f_ah.ph_srctask = hibyte(src);
    fh.f_ah.ph_ident = ident;
    fh.f_ah.ph_seq = seq;
    fh.f_ah.ph_type = type;
    fh.f_ah.ph_flags = 0;
    fh.f_ah.ph_size = totalsize;
    enc_s_le(&fh.f_ah.ph_size);
    if ((Xmtbuf = eth_getbuf()) != 0)
    {
	phd = umap(proc_ptr, D, (vir_bytes)&fh, (vir_bytes)sizeof (Framehdr));
	phys_copy(phd, Xmtbuf, (long)sizeof (Framehdr));
	if (size == 0){
	    eth_write(Xmtbuf, 60);
	}
	else
	{
	    Outsiz = sizeof (Framehdr);
	    Outptr = Xmtbuf + sizeof (Framehdr);
	}
    }
    else
	Outptr = 0;
}


/*===========================================================================*
 *                                am_gall                                    *
 *===========================================================================*/
PUBLIC
am_gall()	/* getall in trans.c */
{
/*
**  copy in any bytes not already copied into packet! We've already copied
**  the first HSZ bytes!
**  Bufaddr points to the local buffer Packet and Inptr points to the current
**  position in the buffer on the ethernet card.
*/
    long size;

    if ((size = (long)Insiz - HSZ) > 0)
	phys_copy(Inptr, Bufaddr+HSZ, size);
}


/*===========================================================================*
 *                             am_do_append                                  *
 *===========================================================================*/
PUBLIC
am_doappend(data, size, dosend)
phys_bytes	data;
unshort		size;
int		dosend;
{
/* add more data to current ethernet output packet */
    if (Outptr == 0)	/* previous puthead failed */
	return;
    phys_copy(data, Outptr, (long)size);
    Outptr += size;
    Outsiz += size;
    if (dosend)
    {
	if (Outsiz < 60)
	    Outsiz = 60;
	eth_write(Xmtbuf, (int)Outsiz);
    }
}


/*===========================================================================*
 *                             am_pickoff                                    *
 *===========================================================================*/
PUBLIC
am_pickoff(data, size)
phys_bytes	data;
unsigned	size;
{
    phys_copy(Inptr, data, (long)size);
    Inptr += size;
}


/*===========================================================================*
 *                             am_append                                     *
 *===========================================================================*/
PUBLIC
am_append(data, size, dosend)
phys_bytes	data;	/* not really a phys_bytes! really a vir_bytes */
unshort		size;
int		dosend;
{
    phys_bytes	paddr;
    phys_bytes	umap();

    paddr = umap(proc_ptr, D, (vir_bytes)data, (vir_bytes)size);
    am_doappend(paddr, size, dosend);
}


/*===========================================================================*
 *                             am_phys_copy                                  *
 *===========================================================================*/
PUBLIC
am_phys_copy(s, d, size)
vir_bytes	s;
phys_bytes	d;
phys_bytes	size;
{
/*
** the phys_copy in trans.c needs a little help in places since kernel
** virtual address need to be umapped under minix
*/
    phys_bytes	umap();
    phys_bytes	ps;

    ps = umap(proc_ptr, D, (vir_bytes)s, (vir_bytes)size);
    phys_copy(ps, d, size);
}


/*===========================================================================*
 *                               ealookup                                    *
 *===========================================================================*/
PRIVATE
ealookup(addr)
Eth_addr *	addr;
{
    int		index;
    int		i;
    Eth_addr *	mep;

    index = addr->e[5] & 0x7f;	/* hash it */
    if (index >= MAPENTRIES)
	index = 0;
    i = index;
    do
    {
	mep = &Eamap[i];
	if (EACMP(addr, mep))
	    return i | ETHERBITS;
	if (EANULL(mep))
	{
	    *mep = *addr;
	    return i | ETHERBITS;
	}
	if (++i >= MAPENTRIES)
	    i = 0;
    } while (i != index);
    return 0;
}


/*
** the following routines provide the interface to the ethernet driver
*/


/*===========================================================================*
 *                                net_init                                   *
 *===========================================================================*/
PUBLIC
net_init()
{
    int pkt_arr();  /* called by ethernet driver when packet arrives */
    int pkt_sent(); /* called by ethernet driver when packet was sent */
    int eth_init(); /* initialise ethernet driver */

    Eamap[MAPENTRIES] = Broadcastaddr;
    epl_init();
    etheraddr(&Myaddr);
    pr_addr("Etheraddr:", &Myaddr);
    eth_init(&Myaddr, pkt_arr, pkt_sent);
    enable_irq(ETHER_IRQ);
}


/*
** some special hack-defines because physical addresses are stored in a long
** and virtual addresses are not
*/

#define	PROTO_OFFSET	((int) &((Etherpacket *) 0)->ep_fr.f_proto)

PRIVATE phys_bytes Paddr;

/*===========================================================================*
 *                                 pkt_arr                                   *
 *===========================================================================*/
PRIVATE
pkt_arr(addr, count)
phys_bytes addr;
int count;
{
/*
** This routine is called when an ethernet interrupt occurs.
** It must select appropriate amoeba task to give the packet to.
** NB: we are only interested in amoeba packets!
*/
    short	getbint();
    short	protocol;

    protocol = getbint(addr + PROTO_OFFSET);

    Paddr = addr;

#ifdef ALTAMOEBAPROTO
    if (protocol == AMOEBAPROTO || protocol == ALTAMOEBAPROTO)
#else
    if (protocol == AMOEBAPROTO)
#endif
    {
	do_arrive(addr);
    }
    else{	/* not an amoeba packet, so give it back */
	eth_release(addr);
    }
}


/*===========================================================================*
 *                                 pkt_sent                                  *
 *===========================================================================*/
/*ARGSUSED*/
PRIVATE
pkt_sent (addr)
phys_bytes addr;
{
/*
** This is never called. The ethernet driver busy waits!  It is here for
** compatibility with the ethernet driver
*/
}


/*===========================================================================*
 *                             am_netenable                                  *
 *===========================================================================*/
PUBLIC
am_netenable()	/* release the last received message */
{
	eth_release(Paddr);
}

#endif NONET
