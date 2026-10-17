/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(message, @(#)message.h	1.1.1.2	(5/12/87));


/* message queue defines */
#define	IPC_CREAT	0001000
#define IPC_EXCL	0002000
#define	IPC_NOWAIT	0004000
#define IPC_RMID	0
#define	IPC_SET		1
#define IPC_STAT	2
#define IPC_PRIVATE	(long)0
#define MSG_NOERROR	010000
#define MAX_QBYTES	0020000
#define NO_SAVE		0000010
#define NO_COPY		0000020



typedef struct msgbuf { /* a message */
	long 	mtype;		/* message identifier */
	char	mtext[1];	/* text of the message */
} msgbuf;

typedef struct message_list { /* make a linked list of messages */
	msgbuf 	*msg;			/* the message */
	int	size;			/* size of the message */
	struct message_list *next;	/* link to the next message */
} message_list;


typedef struct msqid_ds { /* a message queue */
	struct Proc *cid;	/* process that created the queue */
	short 	msg_qnum;	/* number of messages in the queue */
	short	msg_qbytes;	/* number of bytes used by the queue */
	struct Proc *msg_lspid;	/* process id of the last process to send to the queue */
	struct Proc *msg_lrpid;	/* process id of the last process to rcv from the queue */
	unsigned long msg_stime;/* time of last send to the queue */
	unsigned long msg_rtime;/* time of the last rcv from the queue */
	unsigned long msg_ctime;/* time of the last change to this structure */
	message_list *msg_list;	/* linked list of messages in the queue */
	short	msg_curbytes;	/* current number of bytes used by the queue */
	short	state;		/* used to decide if to delete when the process dies */	
	long	name;		/* the name of the queue (key) */
	struct msqid_ds *next;  /* link to the next queue */
} msqid_ds;


long     msgget();               /* (key,msgflg) get a message from a queue */
void    init_msq();             /* (mp,name,msgflg) initialize a message queue */
int     msgctl();               /* (msqid,cmd,buf) stat, set, or remove a queue */
int     valid_msqid();          /* (msqid) is the given msqid valid? */
void    free_msq();             /* (msqid)free the memory used by a message queue */
int     msgsnd();               /* (msqid,msgp,msgsz,msgflg) send a message */
int     msgrcv();               /* (msqid,msgp,msgsz,msgtyp,msgflg) receive a message */
message_list *rcv_message();    /* (msqid,msgtyp) get the first message of the given type */


