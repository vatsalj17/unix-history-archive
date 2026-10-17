/* $Header: /ker/io.386/RCS/nkb.c,v 2.5 93/08/19 10:39:17 nigel Exp Locker: nigel $ */
/*
 * Keyboard driver, no virtual consoles, loadable tables.
 *
 * $Log:	nkb.c,v $
 * Revision 2.5  93/08/19  10:39:17  nigel
 * r83 ioctl (), corefile, new headers
 * 
 * Revision 2.4  93/08/19  04:03:03  nigel
 * Nigel's R83
 */

#include <sys/errno.h>
#include <sys/stat.h>
#include <signal.h>
#include <stddef.h>

#include <sys/coherent.h>
#include <sys/uproc.h>
#include <sys/con.h>
#include <sys/tty.h>
#include <sys/seg.h>
#include <sys/sched.h>
#include <sys/kb.h>
#include <sys/devices.h>
#include <sys/silo.h>
#include <sys/vtkd.h>

#if	DEBUG
#define	KBDEBUG(x)	printf(x)	/* debugging output */
#define	KBDEBUG2(x, y)	printf(x, y)	/* debugging output */
#define	KBDEBUG3(x, y, z)	printf(x, y, z)	/* debugging output */
#else
#define	KBDEBUG(x)			/* no output */
#define	KBDEBUG2(x, y)			/* no output */
#define	KBDEBUG3(x, y, z)			/* no output */
#endif

/*
 * values for kbstate
 */
#define	KB_IDLE		0		/* nothing going on right now */
#define	KB_SINGLE	1		/* sent a single byte cmd to the kbd */
#define	KB_DOUBLE_1	2		/* sent 1st byte of 2-byte cmd to kbd */
#define	KB_DOUBLE_2	3		/* sent 2nd byte of 2-byte cmd to kbd */

/*
 * patchable params for non-standard keyboards
 */
int	KBDATA = 0x60;			/* Keyboard data */
int	KBCTRL = 0x61;			/* Keyboard control */
int	KBSTS_CMD = 0x64;		/* Keyboard status / command */
int	KBFLAG = 0x80;			/* Keyboard reset flag */
int	KBBOOT = 1;			/* 0: disallow reboot from keyboard */
int	KBTIMEOUT = 10000;		/* shouldn't need this much */
int	KBCMDBYTE = 0x05;		/* no translation */

/*
 * KBSTATUS bits
 */
#define	STS_OBUF_FULL	0x01		/* kbd output buffer full */
#define	STS_IBUF_FULL	0x02		/* kbd input buffer full */
#define	STS_SYSTEM	0x04
#define	STS_CMD_DATA	0x08		/* 1: command or status */
#define	STS_INHIBIT	0x10		/* 0: keyboard inhibited */
#define	STS_AUX_OBUF_FULL	0x20
#define	STS_TIMEOUT	0x40		/* general timeout */
#define	STS_PAR_ERR	0x80		/* parity error */

/*
 * The following are magic commands which read from or write to the
 * controller command byte. These get output to the KBSTS_CMD port.
 */
#define	C_READ_CMD	0x20		/* read controller command byte */
#define	C_WRITE_CMD	0x60		/* write controller command byte */
#define	C_TRANSLATE	0x40		/* translate enable bit in cmd byte */

/*
 * Globals:
 * The 286 keyboard mapping table is too large to fit into kernel data space,
 * so we need to allocate a segment to it.  386 is easy.
 * The function keys tend to be small and tend to change substantially
 * more often than the mapping table, so we keep them in the kernel data space.
 */
static	unsigned shift;			/* state of all shift / lock keys */
static	unsigned char	** funkeyp = 0;	/* ptr to array of func. keys ptrs */
static	FNKEY	* fnkeys = 0;		/* pointer to structure of values */
static	unsigned fklength;		/* length of function key data */
static	unsigned prev_cmd;		/* previous command sent to KBD */
static	unsigned cmd2;			/* 2nd byte of command to KBD */
static	unsigned sh_index;		/* shift / lock state index */
#ifdef _I386
static	KBTBL	kb [MAX_KEYS];		/* keyboard table */
#else
static	SEG	* kbsegp;		/* keyboard table segment */
#endif

/*
 * State variables.
 */
int		islock;			/* Keyboard locked flag */
int		isbusy;			/* Raw input conversion busy */
static	char	table_loaded;		/* true == keyboard table resident */
static	char	fk_loaded;		/* true == function keys resident */
static	int	kbstate = KB_IDLE;	/* current keyboard state */
static	int	xlate = 1;		/* scan code translation flag */

#define	ESCAPE_CHAR	'\x1B'
#define	ESCAPE_STRING	"\x1B"

static int X11led;


/*
 * Functions.
 */

int		mmstart ();
int		mmwrite ();
void		mmwatch ();


/*
 * Terminal structure.
 */

TTY	istty = {
	{0}, {0}, 0, mmstart, NULL, 0, 0
};

static silo_t in_silo;

/*
 * Load entry point.
 */

void
isload ()
{
	kbstate = KB_IDLE;
	table_loaded = 0;		/* no keyboard table yet */
	fk_loaded = 0;			/* no Fn keys yet */

	/*
	 * Enable mmwatch () invocation every second.
	 */

	drvl [KB_MAJOR].d_time = 1;

	/*
	 * Initiailize video display.
	 */

	mmstart (& istty);

	fklength = 0;
	KBDEBUG ("Exiting kbload ()\n");
}


/*
 * Unload entry point.
 */

void
isuload ()
{
	if (kbstate != KB_IDLE)
		printf ("kb: keyboard busy during unload\n");
}


/*
 * ship a single byte command to the keyboard
 */

void
kb_cmd (cmd)
unsigned cmd;
{
	int timeout;
	int s;

	s = sphi ();
	KBDEBUG2(" kb_cmd (%x)", cmd);
	while (kbstate != KB_IDLE) {
		/* The nkb driver is waiting for a command to complete.  */
		x_sleep (& kbstate, pritty, slpriSigLjmp, "nkbcmd");
	}

	kbstate = KB_SINGLE;
	timeout = KBTIMEOUT;
	while (--timeout > 0 && (inb (KBSTS_CMD) & STS_IBUF_FULL))
		;
	if (! timeout)
		printf ("kb: command timeout\n");
	else {
		outb (KBDATA, cmd);
		while (kbstate != KB_IDLE) {
			/* The nkb driver is still waiting for a command to complete. */
			x_sleep (& kbstate, pritty, slpriSigLjmp, "nkbcmd...");
		}
	}
	spl (s);
}


/*
 * ship a two byte command to the keyboard
 */

void
kb_cmd2(cmd, arg)
unsigned cmd, arg;
{
	int timeout;
	int s;

	s = sphi ();
	KBDEBUG3(" kb_cmd2(%x, %x)", cmd, arg);
	while (kbstate != KB_IDLE) {
		/*
		 * The nkb driver is waiting for a
		 * 2 byte command to complete.
		 */
		x_sleep (& kbstate, pritty, slpriSigLjmp, "nkbcmd2...");
	}
	kbstate = KB_DOUBLE_1;
	cmd2 = arg;
	prev_cmd = cmd;
	timeout = KBTIMEOUT;
	while (--timeout > 0 && (inb (KBSTS_CMD) & STS_IBUF_FULL))
		;
	if (! timeout)
		printf ("kb: command timeout\n");
	else {
		outb (KBDATA, cmd);
		while (kbstate != KB_IDLE) {
			/*
			 * The nkb driver is still waiting for a
			 * 2 byte command to complete.
			 */
			x_sleep (& kbstate, pritty, slpriSigLjmp, "nkbcmd2...");
		}
	}
	spl (s);
}


/*
 * update the keyboard status LEDS.
 * we chose the shift / lock key positions so this would be easy.
 * this flavor of routine is called while processing a system call on
 * behalf of the user.
 */

void
updleds ()
{
 	if (! xlate)
		kb_cmd2(K_LED_CMD, X11led);
 	else
		kb_cmd2(K_LED_CMD, (shift >> 1) & 0x7);
}


/*
 * same as above, but callable from interrupt routines and other places
 * which cannot sleep () waiting for the state machine to go idle.
 */

void
updleds2()
{
	int timeout;
	int s;

	timeout = KBTIMEOUT;
	s = sphi ();
	while (--timeout > 0 && (inb (KBSTS_CMD) & STS_IBUF_FULL))
		;
	kbstate = KB_DOUBLE_1;
	
	if (! xlate)
		cmd2 = X11led;
	else
		cmd2 = (shift >> 1) & 0x7;

	prev_cmd = K_LED_CMD;
	outb (KBDATA, K_LED_CMD);
	spl (s);
}


/*
 * Open routine.
 */

void
isopen (dev, mode)
dev_t dev;
unsigned int mode;
{
	int s;

	KBDEBUG (" kbopen ()");
	if (minor (dev) != 0) {
		set_user_error (ENXIO);
		return;
	}
	if ((istty.t_flags & T_EXCL) != 0 && ! super ()) {
		set_user_error (ENODEV);
		return;
	}
	ttsetgrp (& istty, dev, mode);

	s = sphi ();
	if (istty.t_open ++ == 0) {
		istty.t_flags = T_CARR;	/* indicate "carrier" */
		ttopen (& istty);
	}
	spl (s);
#if 0
	updleds ();			/* update keyboard status LEDS */
#endif
}


/*
 * Close a tty.
 */

void
isclose (dev)
dev_t dev;
{
	int s;

	s = sphi ();
	if (-- istty.t_open == 0) {
		ttclose (& istty);
	}
	spl (s);
}


/*
 * Read routine.
 */

void
isread (dev, iop)
dev_t dev;
IO * iop;
{
	ttread (& istty, iop, 0);
	if (istty.t_oq.cq_cc)
		mmtime (& istty);
}


#define TIMER_CTL    0x43                     /* Timer control */
#define TIMER_CNT    0x42                     /* Timer counter */
#define SPEAKER_CTL  0x61                     /* Speaker control */

static TIM tp;

void
resetkb (action)
int action;
{
	if (action == 1) {
		timeout (& tp, 20, resetkb, 2);
		outb (KBCTRL, 0xCC);		/* Clock high */
	}

	if (action == 2) {
		int i = inb (KBDATA);
		outb (KBCTRL, 0xCC);		/* Clear keyboard */
		outb (KBCTRL, 0x4D);		/* Enable keyboard */
	}
}


/*
 * Set the in-core keyboard mapping table.
 * The table is sorted by scan code prior to calling ioctl ().
 * All unused table entries (holes in the scan code map) have
 * a zero for the k_key field.
 * This makes key lookup at interrupt time fast by using the scan code
 * as an index into the table.
 */

void
issettable (vec)
char	* vec;
{
	unsigned i;
	int s;
	int timeout;
	static	KBTBL	this_key;	/* current key from kbd table */
	unsigned int cmd_byte;

	KBDEBUG (" TIOCSETKBT");
	kb_cmd2 (K_SCANCODE_CMD, 3);		/* select set 3 */
	kb_cmd (K_ALL_TMB_CMD);			/* default: TMB for all keys */

	for (i = 0; i < MAX_KEYS; ++ i) {
		ukcopy (vec, & this_key, sizeof (this_key));

		kb [i] = this_key;		/* store away */
		vec += sizeof (this_key);

		if (this_key.k_key != i && this_key.k_key != 0) {
			printf ("kb: incorrect or unsorted table entry %d\n", i);
			set_user_error (EINVAL);
			return;
		}

		if (this_key.k_key != i)
			continue;		/* no key */

		switch (this_key.k_flags & TMODE) {
		case T:				/* typematic */
			kb_cmd2 (K_KEY_T_CMD, i);
			break;

		case M:				/* make only */
			kb_cmd2 (K_KEY_M_CMD, i);
			break;

		case MB:			/* make / break */
			kb_cmd2 (K_KEY_MB_CMD, i);
			break;

		case TMB:			/* typematic make / break */
			break;			/* this is the default */

		default:
			printf ("kb: bad key mode\n");
		}
	}

	updleds ();
	kb_cmd2 (K_SCANCODE_CMD, 3);		/* select set 3 */
	kb_cmd (K_ENABLE_CMD);			/* start scanning */

	/*
	 * The following code disables translation from the on-board
	 * keyboard / aux controller. Without disabling translation, the
	 * received scan codes still look like code set 1 codes even
	 * though we put the keyboard controller in scan code set 3.
	 * Yes, this is progress....
	 */

#if 0
	while (inb (KBSTS_CMD) & STS_IBUF_FULL)
		;
	outb (KBSTS_CMD, C_READ_CMD);		/* read controller cmd byte */
	while (!(inb (KBSTS_CMD) & STS_OBUF_FULL))
		;
	cmd_byte = inb (KBDATA);
	KBDEBUG2(" cmd_byte =%x", cmd_byte);
#endif
	timeout = KBTIMEOUT;

	s = sphi ();
	while ((inb (KBSTS_CMD) & STS_IBUF_FULL) != 0 && -- timeout > 0)
		/* DO NOTHING */ ;

	outb (KBSTS_CMD, C_WRITE_CMD);		/* write controller cmd byte */
	for (timeout = 50 ; -- timeout > 0;)
		/* DO NOTHING */ ;

	timeout = KBTIMEOUT;
	while ((inb (KBSTS_CMD) & STS_IBUF_FULL) != 0 && -- timeout > 0)
		/* DO NOTHING */ ;

	outb (KBDATA, KBCMDBYTE);		 /* turn off translation */	

	timeout = KBTIMEOUT;
	while ((inb (KBSTS_CMD) & STS_IBUF_FULL) != 0 && --timeout > 0)
		/* DO NOTHING */ ;
	spl (s);

#if DEBUG
	kb_cmd2 (K_SCANCODE_CMD, 0);		/* query s.c. mode */
#endif
	++ table_loaded;
}


/*
 * Get the in-core keyboard mapping table and pass it to the user.
 */

void
isgettable (vec)
char	* vec;
{
	KBDEBUG (" TIOCGETKBT");
	kucopy (kb, vec, sizeof (kb));
}


/*
 * Set and receive the function keys.
 */

void
isfunction (c, v)
int c;
FNKEY * v;
{
	unsigned char * cp;
	unsigned i;
	unsigned char	numkeys = 0;

	if (c == TIOCGETF) {
		KBDEBUG (" TIOCGETF");
		if (! fk_loaded)
			set_user_error (EINVAL);
		else
			kucopy (fnkeys, v, fklength);	/* copy ours to user */
	} else { /* TIOCSETF */
		/*
		 * If we had a previous function key arena, free it up.
		 * Since we don't know how large the function key arena will
		 * be, we must size it in the user data space prior to
		 * (re)kalloc ()'ing it. This is ugly, but a helluva lot better
		 * than the old driver which used a hard coded limit of 150!
		 */
		KBDEBUG (" TIOCSETF");
		fk_loaded = 0;

		if (fnkeys != NULL)
			kfree (fnkeys);		/* free old arena */

		if (funkeyp != NULL)
			kfree (funkeyp);	/* free old ptr array */
		ukcopy (& v->k_nfkeys, & numkeys, sizeof (numkeys));
		fklength = sizeof (FNKEY);

		cp = (char *) (v + 1);

		for (i = 0; i < numkeys; i ++) {
			do {
				++ fklength;
			} while (getubd (cp ++) != DELIM);
		}

		fnkeys = (FNKEY *) kalloc (fklength);
		funkeyp = (unsigned char **) kalloc (numkeys * sizeof (char *));
		if (fnkeys == NULL || funkeyp == NULL) {
			if (fnkeys != NULL) {
				kfree (fnkeys);
				fnkeys = 0;
			}
			if (funkeyp != NULL) {
				kfree (funkeyp);
				funkeyp = 0;
			}
			set_user_error (ENOMEM);
			return;
		}

		cp = (char *) (fnkeys + 1);		/* point to Fn ... */
		v = (FNKEY *) (v + 1);			/* ... key arena */

		for (i = 0; i < numkeys; i ++) {

			funkeyp [i] = cp;		/* save pointer */
			while ((* cp ++ = getubd (v ++)) != DELIM)
				/* DO NOTHING */ ;
		}

		fnkeys->k_nfkeys = numkeys;
		fk_loaded = 1;
	}
}


/*
 * Ioctl routine.
 * nb: archaic TIOCSHIFT and TIOCCSHIFT no longer needed / supported.
 */

void
isioctl (dev, com, vec)
dev_t dev;
struct sgttyb * vec;
{
	int s;

	switch (com) {
#define KDDEBUG 0
#if KDDEBUG
	case KDMEMDISP: {
		struct kd_memloc * mem;
		unsigned char ub, pb;
		mem = vec;
		pxcopy ( mem->physaddr, & pb, 1, SEL_386_KD);
		ub = getubd (mem->vaddr);
		printf ("User's byte %x (%x), Physical byte %x, Addresses %x %x\n",
			mem->ioflg, ub, pb, mem->vaddr, mem->physaddr);
		break;
	}
#endif
	case KDMAPDISP: {
	       struct kd_memloc * mem;
	       mem = (struct kd_memloc *) vec;
	       mapPhysUser (mem->vaddr, mem->physaddr, mem->length);
	}

	case KDDISABIO:
		__kddisabio();
		break;

	case KDENABIO:
		__kdenabio();
		break;

	case KDADDIO:
		__kdaddio((unsigned short)vec);
		break;

	case KDDELIO:
		__kddelio((unsigned short)vec);
		break;

	case KIOCSOUND: {
		if (vec) {
			outb (TIMER_CTL, 0xB6); 
			outb (TIMER_CNT, (int)vec & 0xFF);
			outb (TIMER_CNT, (int)vec >> 8);
			/* Turn speaker on */
			outb (SPEAKER_CTL, inb (SPEAKER_CTL) | 03);
		} else 
			outb (SPEAKER_CTL, inb (SPEAKER_CTL) & ~ 03 );
		break;
	}

	case KDSKBMODE: {
		static int vtB4X11;
#if	0
		outb (KBCTRL, 0x0C);		   /* Clock low */
		timeout (& tp, 3, resetkb, 1);	   /* wait about 20-30ms */
#endif
		if (xlate > (int) vec) {	/* Turning translation off */
			kb_cmd2(K_SCANCODE_CMD, 1);	 /* set 1 for X */

#if 0
			/* deactivate virtual terminal */
			vtB4X11 = vtactive;
			vtdeactivate (vtdata [vtactive], vtdata [vtcount]);
			vtactivate (vtdata [vtcount]);
			vtactive = vtcount;
#endif
		} else if (xlate < (int) vec) {	/* turning translation on */
			kb_cmd2(K_SCANCODE_CMD, 3);	 /* set 3 for COH */
#if 0
			/* reactivate virtual terminal */
			vtactivate (vtdata [vtB4X11]);
			vtactive = vtB4X11;
#endif
		}
		xlate = (int) vec;
#if	0
		kb_cmd (K_ALL_TMB_CMD);	/* default: TMB for all keys */
#endif
		break;
	}

	case KDGKBSTATE: 
		* (char *) vec = 0;
		if (shift & ((1 << lshift) | ( 1 << rshift)))
			* (char *) vec |= M_KDGKBSTATE_SHIFT;
		if (shift & ((1 << lctrl) | (1 << rctrl)))
			* (char *) vec |= M_KDGKBSTATE_CTRL;
		if (shift & ((1 << lalt) | (1 << ralt)))
			* (char *) vec |= M_KDGKBSTATE_ALT;
		break;
     
	case KDSETLED: {
	       X11led = (int) vec;
	       updleds ();
	       break;		  
	}

	case KDGETLED:
		* (char *) vec = 0;
		if (shift & (1 << num))
			* (char *) vec |= M_KDGETLED_NUMLOCK;
		if (shift & (1 << caps))
			* (char *) vec |= M_KDGETLED_CAPLOCK;
		if (shift & (1 << scroll))
			* (char *) vec |= M_KDGETLED_SCRLOCK;
		break;

	case TIOCSETF:
	case TIOCGETF:
		isfunction (com, (char *)vec);
		break;

	case TIOCSETKBT:
		issettable (vec);
		break;

	case TIOCGETKBT:
		isgettable (vec);
		break;

	case KIOCINFO:
	        * (int *) vec = 0x6664;
	        break;

	default:				/* pass to TTY driver */
		s = sphi ();
		ttioctl (& istty, com, vec);
		spl (s);
		break;
	}
}


/*
 * Poll routine.
 */

int
ispoll (dev, ev, msec)
dev_t dev;
int ev;
int msec;
{
	return ttpoll (& istty, ev, msec);
}


/**
 *
 * void
 * isin (c)	-- append character to raw input silo
 * char c;
 */

static void
isin (c)
int c;
{
	int cache_it = 1;
	TTY * tp = & istty;
	void ttstart ();

	/*
	 * If using software incoming flow control, process and
	 * discard t_stopc and t_startc.
	 */
	if (_IS_IXON_MODE (tp)) {
#if _I386
		if (_IS_START_CHAR (tp, c) ||
		    (_IS_IXANY_MODE (tp) && (tp->t_flags & T_STOP) != 0)) {
			tp->t_flags &= ~ (T_STOP | T_XSTOP);
			ttstart (tp);
			cache_it = 0;
		} else if (_IS_STOP_CHAR (tp, c)) {
			if ((tp->t_flags & T_STOP) == 0)
				tp->t_flags |= (T_STOP | T_XSTOP);
			cache_it = 0;
		}
#else
		if (_IS_STOP_CHAR (tp, c)) {
			if ((tp->t_flags & T_STOP) == 0)
				tp->t_flags |= T_STOP;
			cache_it = 0;
		}
		if (I_S_START_CHAR (tp, c)) {
			tp->t_flags &= ~ T_STOP;
			ttstart (tp);
			cache_it = 0;
		}
#endif
	}

	/*
	 * Cache received character.
	 */
	if (cache_it) {
		in_silo.si_buf [in_silo.si_ix] = c;

		if (++ in_silo.si_ix >= sizeof (in_silo.si_buf))
			in_silo.si_ix = 0;
	}
}


/*
 * Process a key given its scan code and direction.
 * 
 * In this table driven version of the keyboard driver, we trade off the
 * code complexity associated with all the black magic that used to be
 * performed on a per-key basis with the increased memory requirements
 * associated with the table driven approach.
 */

void
process_key (key, up)
unsigned key;
int	 up;
{
	unsigned char * cp;
	KBTBL	key_vals;			/* table values for this key */
	unsigned val;
	unsigned char flags;

	KBDEBUG3(" proc (%x %s)", key, (up ? "up" : "down"));
	if (! table_loaded)
		return;				/* throw away key */

	key_vals = kb [key];

	if (key_vals.k_key != key)		/* empty entry */
		return;
	flags = key_vals.k_flags;

	if (flags & S) {			/* some shift / lock key ? */
		switch (key_vals.k_val [BASE]) {
		case caps:
		case num:
			if (! up) {
				shift ^= (1 << key_vals.k_val [BASE]);
				updleds2 ();
			}
			break;

		case scroll:
			if (! up) {
				shift ^= (1 << key_vals.k_val [BASE]);
				updleds2 ();
				if (! _IS_RAW_INPUT_MODE (& istty)) {
					if (istty.t_flags & T_STOP)
						isin (istty.t_tchars.t_startc);
					else
						isin (istty.t_tchars.t_stopc);
				}
			}
			break;

		default:
			if (up)
				shift &= ~(1 << key_vals.k_val [BASE]);
			else
				shift |= (1 << key_vals.k_val [BASE]);
			break;
		}

		/*
		 * Calculate the shift index based upon the state of
		 * the shift and lock keys.
		 */

		sh_index = BASE;		/* default condition */
		if (shift & (1 << altgr))
			sh_index = ALT_GR;
		else {
			if (shift & ((1 << lalt)|(1 << ralt)))
				sh_index |= ALT;
			if (shift & ((1 << lctrl)|(1 << rctrl)))
				sh_index |= CTRL;
			if (shift & ((1 << lshift)|(1 << rshift)))
				sh_index |= SHIFT;
		}
	} /* if (flags & S) */

	/*
	 * If the tty is not open or the key has no value in the current
	 * shift state, the key is just tossed away.
	 */

	if (up || ! istty.t_open || key_vals.k_val [sh_index] == none)
		return;

	if (((flags & C) != 0 && (shift & (1 << caps)) != 0) ||
	    ((flags & N) != 0 && (shift & (1 << num)) != 0))
		val = key_vals.k_val [sh_index ^ SHIFT];
	else
		val = key_vals.k_val [sh_index];

	/*
	 * Check for function key or special key implemented as
	 * a function key (reboot == f0, tab and back-tab, etc).
	 */

	if (flags & F) {
#if	0
		if (val == 0 && ! up && KBBOOT)
			boot ();
#endif
		if (! fk_loaded || val >= fnkeys->k_nfkeys)
			return;
		if ((cp = funkeyp [val]) == NULL) /* has a value? */
			return;
		while (* cp != DELIM)
			isin (* cp ++);		/* queue up Fn key value */
		return;
	}

	/*
	 * Normal key processing.
	 */
	isin (val);		 /* send the char */
}


/**
 *
 * void
 * isbatch ()	-- raw input conversion routine
 *
 *	Action:	Enable the video display.
 *		Canonize the raw input silo.
 *
 *	Notes:	isbatch () was scheduled as a deferred process by nkbintr ().
 */

static void
isbatch (tp)
TTY * tp;
{
	int c;
	static int lastc;

	/*
	 * Ensure video display is enabled.
	 */
	mm_von ();
	isbusy = 0;

	/*
	 * Process all cached characters.
	 */

	while (in_silo.si_ix != in_silo.si_ox) {
		/*
		 * Get next cached char.
		 */
		c = in_silo.si_buf [in_silo.si_ox];

		if (in_silo.si_ox >= sizeof (in_silo.si_buf) - 1)
			in_silo.si_ox = 0;
		else
			in_silo.si_ox ++;

		if (islock == 0 || _IS_INTERRUPT_CHAR (tp, c) ||
		    _IS_QUIT_CHAR (tp, c)) {
			ttin (tp, c);
		} else if ((c == 'b') && lastc == ESCAPE_CHAR) {
			islock = 0;
			ttin (tp, lastc);
			ttin (tp, c);
		} else if ((c == 'c') && lastc == ESCAPE_CHAR) {
			ttin (tp, lastc);
			ttin (tp, c);
		} else
			putchar ('\a');
		lastc = c;
	}
}


/*
 * Receive interrupt.
 */

void
nkbintr ()
{
	unsigned c;
	unsigned r;
	static	char keyup;

	/*
	 * Schedule raw input handler if not already active.
	 */

	if (! isbusy) {
		defer (isbatch, & istty);
		isbusy = 1;
	}

	/*
	 * Pull character from the data
	 * port. Pulse the KBFLAG in the control
	 * port to reset the data buffer.
	 */

	r = inb (KBDATA) & 0xFF;
	c = inb (KBCTRL);
	outb (KBCTRL, c | KBFLAG);
	outb (KBCTRL, c);

	/*
	 * check returned value from keyboard to see if it's a command
	 * or status back to us. If not, it we assume that it's a key code.
	 */

	KBDEBUG2 (" intr (%x)", r);

 	if (! xlate) switch (r) {

 	case K_BAT_BAD:
 		printf ("kb: keyboard BAT failed\n");
 		break;
 	case K_RESEND:
 		KBDEBUG ("\nkb: request to resend command\n");
 		outb (KBDATA, prev_cmd);
 		break;
 	case K_OVERRUN_23:
 		printf ("kb: keyboard buffer overrun\n");
 		break;
 	case K_ACK:
 		/*
 		 * we received an ACKnowledgement from the keyboard.
 		 * advance the state machine and continue.
 		 */
 		KBDEBUG (" ACK ");

 		switch (kbstate) {
 		case KB_IDLE:			/* shouldn't happen */
#if 0
 			printf ("nkb: ACK while idle\n");
#endif
 			break;

 		case KB_SINGLE:			/* done with 1-byte command */
 		case KB_DOUBLE_2:		/* done w / 2nd of 2-byte cmd */
 			kbstate = KB_IDLE;
 			wakeup (& kbstate);
 			break;

 		case KB_DOUBLE_1:
 			kbstate = KB_DOUBLE_2;
 			outb (KBDATA, cmd2);
 			break;

 		default:
 			printf ("kb: bad kbstate %d\n", kbstate);
 			break;
 		}
 		break;
 	default:
          	isin (r);
              	break;
 	} else switch (r) {

	case K_BREAK:
		keyup = 1;			/* key going up */
		break;

	case K_ECHO_R:
	case K_BAT_OK:
		break;				/* very nice, but ignored */

	case K_BAT_BAD:
		printf ("kb: keyboard BAT failed\n");
		break;

	case K_RESEND:
		KBDEBUG ("\nkb: request to resend command\n");
		outb (KBDATA, prev_cmd);
		break;

	case K_OVERRUN_23:
		printf ("kb: keyboard buffer overrun\n");
		break;

	case K_ACK:
		/*
		 * we received an ACKnowledgement from the keyboard.
		 * advance the state machine and continue.
		 */
		KBDEBUG (" ACK");
		switch (kbstate) {

		case KB_IDLE:			/* shouldn't happen */
#if 0
			printf ("kb: ACK while keyboard idle\n");
#endif
			break;

		case KB_SINGLE:			/* done with 1-byte command */
		case KB_DOUBLE_2:		/* done w / 2nd of 2-byte cmd */
			kbstate = KB_IDLE;
			wakeup (& kbstate);
			break;

		case KB_DOUBLE_1:
			kbstate = KB_DOUBLE_2;
			outb (KBDATA, cmd2);
			break;

		default:
			printf ("kb: bad kbstate %d\n", kbstate);
			break;
		}
		break;

	default:
		process_key (r, keyup);
		keyup = 0;
	}
}


/**
 *
 * void
 * ismmfunc (c)	-- process keyboard related output escape sequences
 * char c;
 */

void
ismmfunc (c)
int c;
{
	switch (c) {
	case 't':	/* Enter numlock */
		shift |= (1 << num);
		updleds ();			/* update LED status */
		break;

	case 'u':	/* Leave numlock */
		shift &= ~ (1 << num);
		updleds ();			/* update LED status */
		break;

	case '=':			/* Enter alternate keypad -- ignored */
	case '>':			/* Exit alternate keypad -- ignored */
		break;

	case 'c':	/* Reset terminal */
		islock = 0;
		break;
	}
}


/*
 * unlock the scroll in case an interrupt character is received
 */

void
kbunscroll ()
{
	shift &= ~(1 << scroll);
	updleds ();
}


/*
 * Configuration table.
 */

CON nkbcon ={
	DFCHR | DFPOL,			/* Flags */
	KB_MAJOR,			/* Major index */
	isopen,				/* Open */
	isclose,			/* Close */
	NULL,				/* Block */
	isread,				/* Read */
	mmwrite,			/* Write */
	isioctl,			/* Ioctl */
	NULL,				/* Powerfail */
	mmwatch,			/* Timeout */
	isload,				/* Load */
	isuload,			/* Unload */
	ispoll				/* Poll */
};
