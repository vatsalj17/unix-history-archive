/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1994 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */
/* $Header: /ker/io.386/RCS/msg386.c,v 2.4 93/08/19 04:03:01 nigel Exp Locker: nigel $ */
/*
 * System V Compatible Messaging
 *
 * $Log:	msg386.c,v $
 * Revision 2.4  93/08/19  04:03:01  nigel
 * Nigel's R83
 */

#include <kernel/_timers.h>
#include <sys/coherent.h>
#include <kernel/trace.h>
#include <sys/sched.h>
#include <sys/types.h>
#include <sys/uproc.h>
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/con.h>
#include <sys/seg.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#define	__MSG_READ	__IRUGO
#define	__MSG_WRITE	__IWUGO


/*
 * Global Message Parameters. We want them to be patchable.
 */

extern	unsigned NMSQID;	/* maximum number of message queues */
extern	unsigned NMSQB;		/* default maximum queue size in bytes */
extern	unsigned NMSG;		/* maximum number of messages per queue */
extern	unsigned NMSC;		/* message text size */

#ifdef	TRACER
int	dballoc = 0;	/* For debug only */
int	dbfree = 0;
#endif

/* Message Information */
struct msqid_ds *msqs = NULL;	/* Array of message queues */
__DUMB_GATE	*msg_gate;	/* Message gates */
char		**msg_map;	/* Memory map */

#define	__LOCK_MESSAGE_QUEUE(n,where) \
		(__GATE_LOCK (msg_gate [n], "lock : msgq " where))
#define	__UNLOCK_MESSAGE_QUEUE(n) \
		(__GATE_UNLOCK (msg_gate [n]))


/* 
 * Msgctl - Message Control Operations.
 */

umsgctl(qid, cmd, buf)
int qid;
int cmd;
struct msqid_ds *buf;
{
	struct msqid_ds * qp;	/* message queues */
	struct msg    *	mp;	/* single message queue */
	unsigned short	n;	/* temporary variable */
	struct msqid_ds	temp;	/* copy of user buf */

	/* Validate qid */
	if (qid < 0) {
		set_user_error (EINVAL);
		return -1;
	}
	qp = msqs + qid % NMSQID;

	/* Validate queue existence.*/
	if ((qp->msg_perm.seq != qid ||
	    (qp->msg_perm.mode & __IPC_ALLOC) == 0) &&
	    (cmd != IPC_STAT || ! super ())) {
		set_user_error (EINVAL);
		return -1;
	}

	switch (cmd) {
	case IPC_STAT:
		/* Validate access authority. */
		if (ipc_lack_perm (& qp->msg_perm, __MSG_READ))
			break;

		 /* Copy queue info to user buffer */
		kucopy (qp, buf, sizeof (struct msqid_ds));
		break;

	case IPC_SET:
		/* Validate modify authority. */
		if (ipc_cred_match (& qp->msg_perm) != _CRED_OWNER) {
			set_user_error (EPERM);
			break;
		}

		/*
		 * Get desired queue size.
		 */

		if (ukcopy (buf, & temp, sizeof (temp)) != sizeof (temp))
			break;

		/*
		 * Only super-user can increase queue size.
		 */

		if (temp.msg_qbytes > qp->msg_qbytes && ! super ()) {
			set_user_error (EPERM);
			break;
		}

		/*
		 * Set queue parameters.
		 */

		qp->msg_perm.uid = temp.msg_perm.uid;
		qp->msg_perm.gid = temp.msg_perm.gid;
		qp->msg_perm.mode = (qp->msg_perm.mode & ~ __IRWXUGO) |
					(temp.msg_perm.mode & __IRWXUGO);

		/* We may want to change the max size of a single message too.
		 * It is not obvious how to do it. There is no
		 * description in SVID. So it is possible that at some point
		 * the size of the single message happens to be greater than
		 * the size of message queue ;-(
		 */

		qp->msg_qbytes = NMSQB = temp.msg_qbytes;
		break;

	case IPC_RMID:
		 /* Validate removal authority. */
		/* Validate modify authority. */
		if (ipc_cred_match (& qp->msg_perm) != _CRED_OWNER) {
			set_user_error (EPERM);
			break;
		}

		/* Free all messages on the queue being removed. */
		while (mp = qp->msg_first) {
			qp->msg_first = mp->msg_next;
			T_MSGQ(0x01, dballoc -= sizeof (struct msg));
			msgfree (mp);
		}
		T_MSGQ(0x01, printf("F%d", dballoc));

		/* Reset queue parameters. */
		qp->msg_last = NULL;
		qp->msg_qnum = 0;
		qp->msg_cbytes = 0;
		qp->msg_perm.mode = 0;

		/* Set last change time */
		qp->msg_ctime = posix_current_time ();

		/* We have to pick up a new unique sequence number.
		 * There is a "wrap around bug". But, it is BCS.
		 */
		qp->msg_perm.seq += (unsigned short) 50;
		break;

	default:
		set_user_error (EINVAL);
	}

	if (get_user_error ())
		return -1;

	return 0;
}


/*
 * Msgget - Get set of messages
 */

umsgget (mykey, msgflg)
key_t mykey;
int msgflg;
{
	struct msqid_ds * qp;		
	struct msqid_ds * freeidp = NULL;	
	
	/* Init message queues on the first msgget */
	if (msqs == NULL)
		if (msginit ()) {
			set_user_error (ENOSPC);
			return -1;
		}

	/* Search for desired message queue [also for first free queue]. */
	for (qp = msqs; qp < msqs + NMSQID; qp++) {
		/* Look for an older free queue */
		if ((qp->msg_perm.mode & __IPC_ALLOC) == 0) {
			if (freeidp == NULL ||
			    freeidp->msg_ctime > qp->msg_ctime)
				freeidp = qp;
			continue;
		}
		if (mykey == IPC_PRIVATE) {	/* creat a new queue */
#if	0
			/*
			 * NIGEL: This looks bogus to me... why should a
			 * request for a private queue fail? My feeling is
			 * that the IPC_EXCL should be ignored.
			 */

			if ((msgflg & IPC_EXCL) != 0 &&	/* unique new queue */
			    mykey == qp->msg_perm.key) {
				set_user_error (EEXIST);/* We cannot creat */
				return -1;	   /* exclusive queue */
			}
#endif
			continue;
		}

		if (qp->msg_perm.key != mykey) 	
			continue;

		if ((msgflg & (IPC_CREAT | IPC_EXCL)) ==
		    (IPC_CREAT | IPC_EXCL)) {
			set_user_error (EEXIST);/* We cannot creat */
			return -1;		/* exclusive queue */
		}

		if (ipc_lack_perm (& qp->msg_perm, msgflg))
			return -1;
		return qp->msg_perm.seq;
	}

	/* Creat a new queue */
	if ((msgflg & IPC_CREAT) == 0) {
		set_user_error (ENOENT);
		return -1;
	}
	if ((qp = freeidp) == NULL) {
		set_user_error (ENOSPC);
		return -1;
	}

	/* Set new queue */
	ipc_perm_init (& qp->msg_perm, msgflg);
	qp->msg_ctime = posix_current_time ();
	qp->msg_qnum = qp->msg_lspid = qp->msg_lrpid = qp->msg_stime 
		= qp->msg_rtime = 0;
	qp->msg_perm.key = mykey;

	return qp->msg_perm.seq;
}


/*
 * Allocate space for the message queues and gates.
 * Initialize message queue headers
 * Return -1 on error.
 */

msginit ()
{
	int		i;
	struct msqid_ds	*qp;
	
	T_MSGQ(0x01, printf("A%d",dballoc += sizeof(struct msqid_ds) * NMSQID));

	/* Allocate space for message headers */
	if ((msqs = (struct msqid_ds *) kalloc (sizeof (struct msqid_ds)
						* NMSQID)) == NULL)
		return -1;

	T_MSGQ(0x01, printf("A%d", dballoc += sizeof(__DUMB_GATE) * NMSQID));

	/* Allocate space for message gates */
	if ((msg_gate = (__DUMB_GATE *) kalloc (sizeof (__DUMB_GATE) *
						NMSQID)) == NULL) {
		kfree (msqs);
		msqs = NULL;
		return -1;
	}

	T_MSGQ(0x01, printf("A%d", dballoc += sizeof(char *) * NMSQID * NMSG));

	/* Allocate space for the message map */
	if ((msg_map = kalloc (sizeof(char *) * NMSQID * NMSG)) == NULL) {
		kfree (msqs);
		msqs = NULL;
		kfree (msg_gate);
		return -1;
	}

	/* Clear gate and map areas */
	memset (msg_map, sizeof (char *) * NMSQID * NMSG, 0);

	for (i = 0 ; i < NMSQID ; i ++)
		__GATE_INIT (msg_gate [i], "message queue", 0);

	/* Set initial queue values */

	for (qp = msqs; qp < msqs + NMSQID; qp++) {
		qp->msg_first  = NULL;	/* First and last pointers to 	*/
		qp->msg_last   = NULL;	/* message queue	      	*/
		qp->msg_cbytes = 0;	/* Number of bytes in queue	*/
		qp->msg_qnum   = 0;	/* Number of messages in queue	*/
		qp->msg_qbytes = NMSQB;	/* Max size of a queue		*/
		qp->msg_lspid  = 0;	/* Pid of last msgsnd		*/
		qp->msg_lrpid  = 0;	/* Pid of last msgrcv		*/
		qp->msg_stime  = 0;	/* Last msgsnd time		*/
		qp->msg_rtime  = 0;	/* Last msgrcv time		*/
		qp->msg_ctime  = posix_current_time (); /* Last change time */
		qp->msg_perm.seq = qp - msqs;
		qp->msg_perm.mode = 0;
	}
	return 0;
}


/* 
 * Remove the message.	Clear message text and header. Reset values in 
 * message map.
 */

msgfree(mp)
struct msg	*mp;	/* Message header to be remove */
{
	kfree (msg_map [mp->msg_spot]);
	msg_map [mp->msg_spot] = NULL;
	kfree (mp);
	T_MSGQ(0x01, printf("F%d", dballoc-=(mp->msg_ts + sizeof(struct msg))));
}


/*
 * Send a Message
 */

umsgsnd(qid, bufp, msgsz, msgflg)
int 		qid;	/* queue id */
long	     *	bufp;	/* message buffer */
int 		msgsz;	/* message size */
int		msgflg;	/* flags */
{
	struct msqid_ds	*qp;	/* message queue */
	struct msg    *	mp;	/* message struct */ 
	struct msg    *	tmp;
	int		q_num;	/* number of a queue */
	int		i_spot;	/* # of empty entry in map */

	/* Validate queue identifier. */
	for (qp = msqs; qp < msqs + NMSQID; qp++) 
		if (qp->msg_perm.seq == qid) 	/* found */
			break;

	q_num = qp - msqs;		
	/* qid is not a valid qid identifier */
	if (q_num >= NMSQID) {
		set_user_error (EINVAL);
		return -1;
	}

	if (ipc_lack_perm (& qp->msg_perm, __MSG_WRITE))
		return -1;

	/* Check if message has a valid message type and size.
	 * The comparison with NMSQB was done because user could
	 * reduce this value.
	 */

	if (* bufp < 1 || msgsz < 0 || msgsz > NMSC || msgsz > NMSQB) {
		set_user_error (EINVAL);
		return -1;
	}
	
	/* Now we have a valid message. Check if we can send it. */

	__LOCK_MESSAGE_QUEUE (q_num, "umsgsend () #1");

	while (qp->msg_qnum >= NMSG || 
	       qp->msg_qbytes < (qp->msg_cbytes + msgsz)) {
		if (msgflg & IPC_NOWAIT) {
			set_user_error (EAGAIN);
			__UNLOCK_MESSAGE_QUEUE (q_num);
			return -1;
		}
		/* We have to wait here */
		qp->msg_perm.mode |= MSG_WWAIT;

		__UNLOCK_MESSAGE_QUEUE (q_num);

		if (x_sleep (qp, pritty, slpriSigCatch, "umsgsnd")
		    == PROCESS_SIGNALLED) {
			/* Abort if a signal was received */
			set_user_error (EINTR);
			return -1;
		}

		/* Abort if the message queue was removed. */
		if (qid != qp->msg_perm.seq) {
			set_user_error (EINVAL);
			return -1;
		}

		__LOCK_MESSAGE_QUEUE (q_num, "umsgsend () #2");
	}

	/* Find empty entry in message map */
	for (i_spot = 0; i_spot < NMSQID * NMSG ; i_spot ++)
		if (msg_map [i_spot] == NULL)
			break;
	/* It cannot happen when we do not have empty entry in map,
	 * but let check it.
	 */

	if (i_spot >= NMSQID * NMSG) {
		set_user_error (ENOSPC);
		return -1;
	}

	T_MSGQ(0x01, printf("A%d", dballoc += sizeof(struct msg)));

	/* Get space for the message header */
	if ((mp = kalloc (sizeof (struct msg))) == NULL) {

		__UNLOCK_MESSAGE_QUEUE (q_num);
		set_user_error (ENOSPC);
		return -1;
	}

	T_MSGQ(0x01, printf("A%d", dballoc += msgsz));

	/* Alloc space for the message text */
	if ((msg_map [i_spot] = kalloc (msgsz)) == NULL) {
		kfree (mp);
		__UNLOCK_MESSAGE_QUEUE (q_num);
		set_user_error (ENOSPC);
		return -1;
	}

	mp->msg_next = NULL;
	mp->msg_ts = msgsz;
	/* The map address is a number of msg_map array element */
	mp->msg_spot = i_spot;

	/* Transfer the message type and text.*/
	if (ukcopy (bufp, & mp->msg_type, sizeof (mp->msg_type)) !=
	    sizeof (mp->msg_type))
		set_user_error (EFAULT);
	else if (ukcopy (bufp + 1, msg_map [i_spot], msgsz) != msgsz)
		set_user_error (EFAULT);

	if (get_user_error ()) {
		msgfree (mp);
		__UNLOCK_MESSAGE_QUEUE (q_num);
		return -1;
	}

	/* Move the message to the desired queue. */
	if (qp->msg_first == NULL) /* This is the first message per queue */
		qp->msg_first = qp->msg_last = mp;
	else {	/* There are messages */
		/* Find last message in gueue */
		for (tmp = qp->msg_first; ; tmp = tmp->msg_next)
			if (tmp->msg_next == NULL)
				break;
		qp->msg_last = tmp->msg_next = mp;
	}
	mp->msg_next = NULL;

	/* Update queue statistics. */
	qp->msg_cbytes += msgsz;
	qp->msg_qnum ++;
	qp->msg_lspid = SELF->p_pid;
	qp->msg_stime = posix_current_time ();

	/* Unlock queue and wake processes waiting to receive. */

	__UNLOCK_MESSAGE_QUEUE (q_num);

	if (qp->msg_perm.mode & MSG_RWAIT) {
		qp->msg_perm.mode &= ~MSG_RWAIT;
		wakeup(qp);
	}
	return 0;
}


/*
 * Receive a Message
 */

umsgrcv (qid, bufp, msgsz, msgtyp, msgflg)
int		qid;	/* Message queue id */
long	      *	bufp;	/* Message buffer */
int		msgsz;	/* Message text size */
long		msgtyp;	/* Message type	*/
int 		msgflg;	/* Message flag	*/
{
	struct msqid_ds	*qp;	/* queue headers */
	struct msg    *	mp;	/* message headers */
	struct msg    *	prev;
	int		q_num;	/* queue number */

	/* Validate queue identifier. */
	if (qid < 0 || msqs == NULL) {
		set_user_error (EINVAL);
		return -1;
	}
	q_num = qid % NMSQID;
	qp = msqs + q_num;

	/* Validate queue existence.*/
	if (qp->msg_perm.seq != qid || (qp->msg_perm.mode & __IPC_ALLOC) == 0) {
		set_user_error (EINVAL);
		return -1;
	}

	/* Check permissions */

	if (ipc_lack_perm (& qp->msg_perm, __MSG_READ))
		return -1;

	/* Wait for message */
	__LOCK_MESSAGE_QUEUE (q_num, "umsgrcv () #1");

	for (;;) {
		prev = NULL;
		mp = qp->msg_first;
		/* Find mesg of type <= abs(msgtyp) */
		if (msgtyp < 0) {
			struct msg *qmin;	/* Message with lowest mtype */
			struct msg *xprev;	/* Previous message */
			
			qmin = NULL;
			xprev = prev;
			msgtyp = -msgtyp;

			for (; mp != NULL; prev = mp, mp = mp->msg_next) {
				if (mp->msg_type > msgtyp)
					continue;
				if (qmin == NULL 
					   || mp->msg_type < qmin->msg_type) {
					xprev = prev;
					qmin = mp;
				}
			}
			prev = xprev;
			mp  = qmin;
			msgtyp = -msgtyp;
		} else if (msgtyp > 0) { /* Find message of type == msgtyp */
			while (mp != NULL && mp->msg_type != msgtyp) {
				prev = mp;
				mp = mp->msg_next;
			}
		} else 	/* Else take first message */
			mp = qp->msg_first;

		if (mp != NULL)	/* Found */
			break;
	
		/* Can't wait to receive mesg */
		if ((msgflg & IPC_NOWAIT) != 0) {
			set_user_error (ENOMSG);
			__UNLOCK_MESSAGE_QUEUE (q_num);
			return -1;
		}

		/* We can go sleep now */
		qp->msg_perm.mode |= MSG_RWAIT;

		__UNLOCK_MESSAGE_QUEUE (q_num);

		if (x_sleep (qp, pritty, slpriSigCatch, "umsgrcv")
		    == PROCESS_SIGNALLED) {
			set_user_error (EINTR);
			return -1;
		}

		/* Not same q anymore */
		if (qid != qp->msg_perm.seq) {
			set_user_error (EINVAL);
			return -1;
		}

		__LOCK_MESSAGE_QUEUE (q_num, "umsgrcv () #2");
	}

	/* Ensure entire message can be transferred, or MSG_NOERROR asserted.*/
	if (msgsz < mp->msg_ts && (msgflg & MSG_NOERROR) == 0) {
		__UNLOCK_MESSAGE_QUEUE (q_num);
		set_user_error (E2BIG);
		return -1;
	}

	/* Transfer message data */
	if (msgsz > mp->msg_ts)
		msgsz = mp->msg_ts;

	if (kucopy (& mp->msg_type, bufp, sizeof (mp->msg_type)) !=
	    sizeof (mp->msg_type))
		set_user_error (EFAULT);
	else if (kucopy (msg_map [mp->msg_spot], bufp + 1, msgsz) != msgsz)
		set_user_error (EFAULT);

	/* Abort if address fault occurred during transfer. */
	if (get_user_error ()) {
		__UNLOCK_MESSAGE_QUEUE (q_num);
		return -1;
	}

	/* Remove message from queue */
	if (prev != NULL)
		prev->msg_next = mp->msg_next;
	else
		qp->msg_first = mp->msg_next;

	if (qp->msg_last == mp)
		qp->msg_last = prev;


	/* Update queue statistics */
	qp->msg_cbytes -= mp->msg_ts;
	qp->msg_qnum --;
	qp->msg_lrpid = SELF->p_pid;
	qp->msg_rtime = posix_current_time ();

	/* free message */
	msgfree(mp);

	__UNLOCK_MESSAGE_QUEUE (q_num);

	/* Wakeup processes waiting to send. */

	if (qp->msg_perm.mode & MSG_WWAIT) {
		qp->msg_perm.mode &= ~ MSG_WWAIT;
		wakeup (qp);
	}
	return msgsz;
}


/*
 * Dispatch the top-level system call; under iBCS2, all the msg... () calls
 * are funneled through a single kernel entry point.
 */

umsgsys(func, arg1, arg2, arg3, arg4, arg5)
{
	switch (func) {
	case 0: return umsgget (arg1, arg2);
	case 1: return umsgctl (arg1, arg2, arg3);
	case 2: return umsgrcv (arg1, arg2, arg3, arg4, arg5);
	case 3:	return umsgsnd (arg1, arg2, arg3, arg4);
	default:set_user_error (EINVAL);
	}
}
