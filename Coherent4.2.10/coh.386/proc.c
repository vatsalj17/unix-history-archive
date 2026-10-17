/* $Header: /ker/coh.386/RCS/proc.c,v 2.6 93/10/29 00:55:30 nigel Exp Locker: nigel $ */
/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 3.x, 4.x
 *	Copyright (c) 1982, 1993.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * Process handling and scheduling.
 */

#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1
#define	_SYSV3		1

#include <sys/coherent.h>
#include <common/_wait.h>
#include <sys/types.h>

#include <kernel/proc_lib.h>
#include <kernel/sig_lib.h>
#include <sys/cmn_err.h>
#include <sys/ksynch.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <stddef.h>
#include <signal.h>
#include <limits.h>

#define	_KERNEL		1

#include <kernel/_timers.h>
#include <kernel/ddi_cpu.h>
#include <kernel/trace.h>
#include <kernel/reg.h>
#include <sys/uproc.h>
#include <sys/proc.h>
#include <sys/mmu.h>
#include <sys/acct.h>
#include <sys/inode.h>
#include <sys/ptrace.h>
#include <coh/proc.h>

/*
 * Number of entries in sleep/wakeup queue.
 */

#define	NHPLINK	97U
#define CLEAR_SIGN_BIT(foo)	((foo) & ~0x80000000)


/*
 * Sleep/wakeup queues.
 */

struct plink {
	PROC	      *	p_lforw;	/* Working forward pointer */
	PROC	      *	p_lback;	/* Working backward pointer */
};

__LOCAL__ struct plink	linkq [NHPLINK];	/* Sleep/wakeup hash queue */


#if	_USE_IDLE_PROCESS
/*
 * Pointer to idle process. For now, we still allocate a PROC *, but it
 * exists only to satisfy the stupid circular-list scheme for process
 * queueing; in actual fact, this process is never ever run.
 */

PROC	      *	iprocp;

#endif


/*
 * Function to hash a wakeup channel. To disappear ASAP along with the whole
 * uniprocessor sleep mechanism.
 */

#define hash(e)	((unsigned)(e) % NHPLINK)


/*
 * Initialization.
 * Set up the hash table queues.
 */

void
pcsinit ()
{
	struct plink * lp;

	/*
	 * Explicitly initialize everything in the first process. We use the
	 * kernel initialize routine with SELF set to NULL so that the first
	 * process gets started out with default values for fields that are
	 * normally inherited.
	 */

	procq.p_nforw = & procq;
	procq.p_nback = & procq;
	procq.p_lforw = & procq;
	procq.p_lback = & procq;

	/* Segments are initialized in mchinit () and eveinit (). */
	/* procq is static, so p_shmsr [] initializes to nulls.	*/

	(void) new_process_init (& procq);

	for (lp = linkq ; lp < linkq + NHPLINK ; lp ++) {
		lp->p_lforw = lp;
		lp->p_lback = lp;
	}

	/*
	 * After we have set things up, we can have a current process.
	 */

	SELF = & procq;
}


/*
 * Initiate a process.
 */

static PROC *
process ()
{
	PROC * pp1;
	PROC * pp;

static	pid_t		next_pid;

	if ((pp = new_process_init (NULL)) == NULL)
		return NULL;

	__GLOBAL_LOCK_PROCESS_TABLE ("process ()");

next:
	/*
	 * Pick the next process id.
	 */

	if (++ next_pid >= PID_MAX)
		next_pid = SYSPID_MAX + 1;
	pp->p_pid = next_pid;


	/*
	 * Make sure that process id is not in use.
	 */

	pp1 = & procq;
	while ((pp1 = pp1->p_nforw) != & procq) {
		if (pp1->p_pid < pp->p_pid)
			break;
		if (pp1->p_pid == pp->p_pid)
			goto next;
	}

	/*
	 * We've got a valid pid, so let's put this process into
	 * the process table.
	 */

	pp->p_nback = pp1->p_nback;
	pp1->p_nback->p_nforw = pp;
	pp->p_nforw = pp1;
	pp1->p_nback = pp;

	__GLOBAL_UNLOCK_PROCESS_TABLE ();
	return pp;
}

/*
 * Remove a process from the next queue and release and space.
 */

void
relproc (pp)
PROC * pp;
{
	SEG * sp;

	/*
	 * Child process still has a user-area.
	 */

	if ((sp = pp->p_segl [SIUSERP].sr_segp) != NULL) {

		/*
		 * Detach user-area from child process.
		 */
		pp->p_segl [SIUSERP].sr_segp = NULL;

		/*
		 * Child process is swapped out.
		 */
		if (pp->p_flags & PFSWAP)
			sp->s_lrefc ++;

		/*
		 * Release child's user-area.
		 */
		sfree (sp);
	}

	/*
	 * Remove process from doubly-linked list of all processes.
	 * Release space allocated for proc structure.
	 */

	__GLOBAL_LOCK_PROCESS_TABLE ("relproc ()");

	pp->p_nback->p_nforw = pp->p_nforw;
	pp->p_nforw->p_nback = pp->p_nback;

	__GLOBAL_UNLOCK_PROCESS_TABLE ();

	proc_destroy (pp);
}


/*
 * Create a clone of ourselves.
 *	N.B. - consave (& mcon) returns twice; anything not initialized
 *	in automatic storage before the call to segadup () will not be
 *	initialized when the second return from consave () commences.
 */

int
pfork ()
{
	PROC * cpp;
	int s;
	struct __mcon	mcon;

	if ((cpp = process ()) == NULL) {
		SET_U_ERROR (EAGAIN, "no more process table entries");
		return -1;
	}

	s = sphi ();

	/*
	 * Create an initial register context for the child process; the most
	 * obvious way to do this is to clone our context.
	 */

	if (consave (cpp->p_sysconp = & mcon) != 0) {
		/*
		 * Child process.
		 */

		/*
		 * WARNING!!! Parent returns first from consave() call
		 * above.  The lines below are executed only after the
		 * first context switch to the new child process.
		 *
		 * There is a brief interval between these times during
		 * which the process queue is fair game, e.g. for /dev/ps.
		 * So, be sure that changes here do not really belong
		 * in process() or something it calls.
		 */
		spl (s);
		u.u_btime = timer.t_time;
		u.u_flag = AFORK;

#if	UPROC_VERSION
		u.u_version = UPROC_VERSION;
#endif /* UPROC_VERSION */
		sproto (0);
		segload ();
		return 0;
	}

	spl (s);

	if (segadup (cpp) == 0) {
		SET_U_ERROR (EAGAIN, "can not duplicate segments");
		relproc (cpp);
		return -1;
	}

	shmDup (cpp);	/* copy shared memory info & update ref counts */

	if (u.u_rdir)
		u.u_rdir->i_refc ++;
	if (u.u_cdir)
		u.u_cdir->i_refc ++;
	fd_dup_all ();

	s = sphi ();
	process_set_runnable (cpp);
	spl (s);

	return cpp->p_pid;
}

PROC *
get_next_process()
{
  PROC *cur_proc;
  PROC *ret_proc;
  int max_prior;

  /*
   * This assumes that the process releasing context is queued 
   * at the **end** of the queue.
   */

  ret_proc = cur_proc = iprocp->p_lback;

  /* See if system is idle. */
  if (cur_proc == iprocp)
    return cur_proc;

  /* Determine max priority by adding priority with anti-starvation. */
  max_prior = cur_proc->p_schedPri + cur_proc->p_foodstamp;

  /* Select the process with the **highest** priority value */
  while (cur_proc != iprocp && cur_proc != NULL) {
    long cur_con;

    cur_con = CLEAR_SIGN_BIT(cur_proc->p_schedPri + cur_proc->p_foodstamp);
    
    if (cur_con > max_prior) {
      max_prior = cur_con;
      ret_proc = cur_proc;
    } else { /* Fend off starvation */
      if (cur_proc != SELF) {
	cur_proc->p_foodstamp++;
	cur_proc->p_foodstamp = CLEAR_SIGN_BIT(cur_proc->p_foodstamp);
      }
    }

    cur_proc = cur_proc->p_lback;
  }

  ASSERT(ret_proc != NULL);

  /* Starvation is not a concern for him who eats. */
  ret_proc->p_foodstamp = 0;
  return ret_proc;
}
   

/*
 * Select next process for execution, working backward from iprocp.
 * If it is not the idle process, delete it from the run queue.
 * If it is not the current process, consave the current process and
 * conrest the selected process.
 */

void
dispatch ()
{
  PROC *pp;
  int s;
  int old_quantum;
  
  LOCK_DISPATCH();
  
  s = sphi();

  while ((pp = get_next_process()) == iprocp) {
    pl_t		prev_pl;
    
    /*
     * The system is idle. The logical thing to do now would be to
     * go to ring 0 and halt. Instead, we'll enable interrupts and
     * wait for the process to change.
     */
    
    __MARK_CPU_IDLE();
    prev_pl = splbase();
    splo();
    
    while (iprocp->p_lback == iprocp)
      check_clock();
    
    /*
     * Re-enable interrupts, and restore normal programming.
     */
    
    (void) sphi();
    (void) splx(prev_pl);
    __CLEAR_CPU_IDLE();
  }
  
  pp->p_lforw->p_lback = pp->p_lback;
  pp->p_lback->p_lforw = pp->p_lforw;
  
  spl(s);
  
  old_quantum = quantum;

  if (pp != iprocp) {
    int newpri;
    int divisor;
    int new_usage;

    /*
     * The actual equation used to compute the priority is this:
     * p(t) = a * (DEFAULT_QUANTUM / (DEFAULT_QUANTUM - old_quantum))
     *        + (1 - a) * p(t - 1)
     *
     * Currently, a == 0.5 and the equation is optimized for this.
     * For p(0), look in ~/coh.386/lib/proc_init.c
     */
    divisor = (DEFAULT_QUANTUM == old_quantum) ? 1 :
      (DEFAULT_QUANTUM - old_quantum);
    
    new_usage = (DEFAULT_QUANTUM * SCHED_SCALE) / divisor;
    newpri = (new_usage + SELF->p_schedPri) >> 1;
    SELF->p_schedPri = CLEAR_SIGN_BIT(newpri);

#if 0
    printf("(%d) - 0x%x\n", SELF->p_pid, SELF->p_schedPri);
#endif

  }
  quantum = DEFAULT_QUANTUM;
  disflag = 0;
  
  if (pp != SELF) {
    struct __mcon	syscon;
    
    /*
     * Consave () returns twice.
     * 1st time is after our context is saved in syscon,
     *	whereupon we should restore other proc's context.
     * 2nd time is after our context is restored by another proc.
     * Conrest () forces a context switch to a new process.
     */
    
    s = sphi ();
    if (consave (SELF->p_sysconp = & syscon) == 0) {
      SELF = pp;
      conrest (* SELF->p_segl [SIUSERP].sr_segp->s_vmem,
	       SELF->p_sysconp);
    }
    
    if (SELF->p_pid) {	/* idle process is special! */
      ndpConRest ();
      segload ();
    }
    
    SELF->p_sysconp = NULL;
    spl (s);
  }
}


/*
 * Die.
 */

void
pexit (s)
int	s;
{
	PROC * pp1;
	SEG  * sp;
	int n;
	PROC	      *	parent = NULL;

	T_PIGGY (0x1, printf ("%s:pexit (%x)", SELF->p_comm, s));

	ndpEndProc ();

	/*
	 * Cancel alarm and poll timers [if any].
	 */
	timeout (& SELF->p_alrmtim, 0, NULL, 0);
	timeout (& SELF->p_polltim, 0, NULL, 0);

	/*
	 * Write out accounting directory and close all files associated with
	 * this process.
	 */
	setacct ();
	if (u.u_rdir)
		ldetach (u.u_rdir);
	if (u.u_cdir)
		ldetach (u.u_cdir);

	/*
	 * Block all signals before entering forced close sequence.
	 */

	___SIGSET_SET (SELF->p_signal_mask, -1UL);

	fd_close_all ();

	/*
	 * Free all segments in reverse order, except for user-area.
	 */

	for (n = NUSEG ; -- n > 0 ;) {
		if ((sp = SELF->p_segl [n].sr_segp) != NULL) {
			SELF->p_segl [n].sr_segp = NULL;
			sfree (sp);
		}
	}

	/* Detach remaining shared memory segments. */
	shmAllDt ();

	/* Adjust (undo) all semaphores. */
	semAllAdjust (SELF);

	/*
	 * Wakeup our parent.  If we have any children, init will become the
	 * new parent.  If there are any children we are tracing who are
	 * waiting for us, we wake them up.
	 */

	pp1 = & procq;

	/* pp1 runs through the list of all processes */
	while ((pp1 = pp1->p_nforw) != & procq) {

		/* if pp1 points to parent of the current process...*/
		if (pp1->p_pid == SELF->p_ppid) {
			parent = pp1;	/* Remember our parent.  */
			if (ASLEEP (pp1) && pp1->p_event == (char *) pp1)
				wakeup ((char *) pp1);
		}

		/* if pp1 points to child of the current process...*/
		if (pp1->p_ppid == SELF->p_pid) {
			pp1->p_ppid = 1;

			if (pp1->p_state == PSDEAD)
				wakeup ((char *) eprocp);

			if (pp1->p_flags & PFTRAC)
				ptrace_cleanup ();
		}
	}

	/*
	 * Mark us as dead and give up the processor forever.
	 */
	SELF->p_exit = s;
	SELF->p_state = PSDEAD;

	/*
	 * If this is a process group leader, inform all members of the group
	 * of the recent death with a HUP signal.
	 */

	if (SELF->p_group == SELF->p_pid)
		ukill (- SELF->p_pid, SIGHUP);

	/*
	 * If the parent is ignoring SIGCLD, 
	 * remove the zombie right away.
	 */

	if (parent == NULL)
		panic ("%d (@ %x), status %d has no parent!\n", SELF->p_pid,
		       SELF, s);

	if ((proc_signal_misc (parent) & __SF_NOCLDWAIT) != 0) {
		/*
		 * The parent has requested that no zombie processes be
		 * created out of childen.
		 */

		parent->p_cutime += SELF->p_utime + SELF->p_cutime;
		parent->p_cstime += SELF->p_stime + SELF->p_cstime;
		relproc (SELF);
	} else {
		/*
		 * Send parent a notification of our demise.
		 */
		__siginfo_t	sigchld;

		sigchld.__si_signo = SIGCHLD;
		sigchld.__si_errno = 0;
		if (__WIFEXITED (s)) {
			sigchld.__si_code = __CLD_EXITED;
			sigchld.__si_status = __WEXITSTATUS (s);
		} else {
			sigchld.__si_code = __WCOREDUMP (s) ? __CLD_DUMPED :
							      __CLD_KILLED;
			sigchld.__si_status = __WTERMSIG (s);
		}
		sigchld.__si_pid = SELF->p_pid;
		proc_send_signal (parent, & sigchld);
	}

	dispatch ();
}


/*
 * x_sleep ()
 *
 * Surrender CPU while awaiting some event or resource.
 *
 * Arguments:
 *	event:		key value; so wakeup () can find this sleep
 *	schedPri:	prilo/primed/prihi/pritape/pritty/pridisk/prinet
 *			just copied into proc struct for scheduler to use.
 *	sleepPri:	slpriNoSig	- signals may not interrupt sleep
 *			slpriSigLjmp	- signals cause longjmp (EINTR)
 *			slpriSigCatch	- signals are caught
 *	reason:		up to 10 chars of text for ps command "event"
 *
 * Return values:
 *	PROCESS_NORMAL_WAKE	wakeup received
 *	PROCESS_SIGNALLED	signal (other than SIGSTOP/SIGCONT) received
 *	PROCESS_CONTINUED	SIGSTOP / SIGCONT (unimplemented now)
 *
 * If longjmp occurs, won't return from x_sleep!
 */

#if __USE_PROTO__
__sleep_t x_sleep (char * event, int schedPri, int sleepPri, char * reason)
#else
__sleep_t
x_sleep (event, schedPri, sleepPri, reason)
char * event;
int schedPri;
int sleepPri;
char * reason;
#endif
{
  PROC *bp;
  PROC *fp;
  int s;
  
  /*
   * Before we queue ourselves on the event list, test for the presence
   * of signals if the users wants to know about them. Older versions of
   * the kernel tested after queueing, which meant that they left
   * the lists in an invalid state.
   */
  
  s = sphi();
  
  if (sleepPri != slpriNoSig && curr_signal_pending() != 0) {
    spl(s);
    
    if (sleepPri == slpriSigCatch)
      return PROCESS_SIGNALLED;
    
    /* Do longjmp to beginning of system call. */
    T_HAL (8, printf ("[%d]Ljmps ", SELF->p_pid));
    
    (void) sphi();
    envrest(u.u_sigenvp);
  }
  
  SELF->p_sleep = reason;
  
  /*
   * Get ready to go to sleep and do so.
   */
  
  SELF->p_state = sleepPri == slpriNoSig ? PSSLEEP : PSSLSIG;

  /* Scale priority by 2^n and mask off sign bit */
  SELF->p_foodstamp += (schedPri << 3) + 7;
  SELF->p_foodstamp = CLEAR_SIGN_BIT(SELF->p_foodstamp);

  SELF->p_event = event;
  fp = linkq + hash(event);
  bp = fp->p_lback;
  SELF->p_lforw = fp;
  fp->p_lback = SELF;
  SELF->p_lback = bp;
  bp->p_lforw = SELF;
  spl(s);
  
  /*
   * From here on we count on someone else dequeuing us.
   */
  
  dispatch ();
  SELF->p_sleep = NULL;
  
  if (sleepPri == slpriNoSig || curr_signal_pending () == 0)
    return PROCESS_NORMAL_WAKE;
  
  /* The process has been interrupted from sleep by a signal. */
  
  if (sleepPri == slpriSigCatch)
    return PROCESS_SIGNALLED;
  
  /* Do longjmp to beginning of system call. */
  T_HAL (8, printf ("[%d]Ljmps ", SELF->p_pid));
  (void) sphi ();
  envrest (u.u_sigenvp);
}


/*
 * Defer function to wake up all processes sleeping on the event `e'.
 */

void
wakeup (event)
char * event;
{
#if	0
	void dwakeup ();

#ifdef TRACER
	/*
	 * In diagnostic kernel, keep return addr on queue as well.
	 */
	int * r = (int *) (& e);
	defer0 (dwakeup, e, * (r - 1));
#else
	defer (dwakeup, e);
#endif
}

/*
 * Wake up all processes sleeping on "event".
 */

void
dwakeup (event)
char * event;
{
#endif
	PROC * pp;
	PROC * pp1;
	int s;

	/*
	 * Identify event queue to check.
	 * Disable interrupts.
	 */

	pp1 = linkq + hash (event);
	pp = pp1;
	s = sphi ();


	/*
	 * Traverse doubly-linked circular event-queue.
	 */

	while ((pp = pp->p_lforw) != pp1) {
		/*
		 * Process is waiting on event 'event'.
		 */

		if (pp->p_event == event) {
			/*
			 * Remove process from event queue.
			 * Update process priority.
			 * Insert process into run queue.
			 */

			pp->p_lback->p_lforw = pp->p_lforw;
			pp->p_lforw->p_lback = pp->p_lback;
			process_set_runnable (pp);

			/*
			 * Enable interrupts.
			 * Restart search at start of event queue.
			 * Disable interrupts.
			 */

			spl (s);
			pp = pp1;
			s = sphi ();
		}
	}
	spl (s);
}


/*
 * Wait for the gate `g' to unlock, and then lock it.
 */

void
__GATE_LOCK (g, reason)
__DUMB_GATE_P	g;
__CONST__ char * reason;
{
	int s;

	ASSERT (g != NULL);

	s = sphi ();
	while (g->_lock [0]) {
#if	TRACER
		if (g->_lock [0] != 1)
			cmn_err (CE_PANIC, "Uninitialised gate");
		if (__GATE_LOCK_OWNER (g) == (char *) SELF) {
			cmn_err (CE_WARN, "Recursive lock attempt by pid %d"
				SELF->p_pid);
			backtrace (0);
			break;
		}
#endif
		g->_lock [1] = 1;
		x_sleep ((char *) g, primed, slpriNoSig, reason);
		/* Waiting for a gate to unlock.  */
	}
	g->_lock [0] = 1;
	g->_lastlock = (char *) SELF;

	if ((__GATE_FLAGS (g) & __GATE_COUNT) != 0)
		u.u_lock_cnt ++;
	spl (s);
}


/*
 * Unlock the gate `g'.
 */

void
__GATE_UNLOCK (g)
__DUMB_GATE_P	g;
{
	ASSERT (g != NULL);
	ASSERT (g->_lock [0] != 0);

	g->_lock [0] = 0;
	if (g->_lock [1]) {
		ASSERT (g->_lock [1] == 1);

		g->_lock [1] = 0;
		disflag = 1;
		wakeup ((char *) g);
	}

	if ((__GATE_FLAGS (g) & __GATE_COUNT) != 0) {
		ASSERT (u.u_lock_cnt != 0);

		if (g->_lastlock != (char *) & procq)
			u.u_lock_cnt --;
	}
}


/*
 * Mark the given gate as being "owned" by an interrupt context.
 */

void
__GATE_TO_INTERRUPT (g)
__DUMB_GATE_P	g;
{
	ASSERT (g != NULL);
	ASSERT (g->_lastlock != (char *) & procq);

	if ((__GATE_FLAGS (g) & __GATE_COUNT) != 0) {
		ASSERT (u.u_lock_cnt != 0);
		u.u_lock_cnt --;
	}

	g->_lastlock = (char *) & procq;
}
