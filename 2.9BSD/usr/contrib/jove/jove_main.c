/*
   Jonathan Payne at Lincoln-Sudbury Regional High School 4-19-83

   jove_main.c

   Contains the main loop, initializations, getch routine... */

#include "jove.h"

#include "termcap.h"

#include <sys/ioctl.h>
#include <signal.h>
#include <sgtty.h>

#ifdef TIOCSLTC
struct ltchars	ls1,
		ls2;
#endif

struct tchars	tc1,
		tc2;

int	errormsg;

int	iniargc;
char	**iniargv;

extern char	*tfname;

finish(code)
{
	int	Crashit = code && (code != LOGOEXIT);

#ifdef LSRHS_KLUDGERY
	if (Crashit)
		setdump(1);
#endif
	if (code == SIGINT) {
		char	c;

#ifndef SIGTSTP			/* Job stopping in other words */
		ignorf(signal(code, finish));
#endif
		message("Quit? ");
		UpdateMesg();
		ignore(read(0, &c, 1));
		message("");
		if ((c & 0377) != 'y') {
			redisplay();
			return;
		}
	}
	if (Crashit) {
		if (!Crashing) {
			putstr("Writing modified JOVE buffers...");
			Crashing++;
			exp_p = 0;
			WtModBuf();
		} else
			putstr("Complete lossage!");
	}
	ttyset(0);
	if (VE)
		putpad(VE, 1);
	Placur(LI - 1, 0);
	putpad(CE, 1);
	flusho();
#ifdef LSRHS_KLUDGERY
	if (VT)
		deal_with_scroll();
#endif
	ignore(unlink(tfname));
	if (Crashit)
		abort();
	exit(code);
}

#define NTOBUF	20		/* Should never get that far ahead */
static char	smbuf[NTOBUF],
		*bp = smbuf;
static int	nchars = 0;

Ungetc(c)
int	c;
{
	if (c == EOF || nchars >= NTOBUF)
		return EOF;
	*--bp = c;
	nchars++;
	return c;
}

getchar()
{
	if (nchars == 0) {
		if ((nchars = read(Input, smbuf, sizeof smbuf)) == 0) {
			if (Input)
				return EOF;
			finish(0);
		}
		bp = smbuf;
		InputPending = nchars > 1;
	}
	nchars--;
	return *bp++;
}

/* Returns non-zero if a character waiting */

charp()
{
	if (Input)
		return 0;
	if (nchars)		/* Quick check */
		return 1;

	else {
#ifdef FIONREAD
		long	c;

		if (ioctl(0, FIONREAD, (char *) &c) == -1)
#else
		int	c;

		if (ioctl(0, TIOCEMPTY, (char *) &c) == -1)
#endif
			c = 0;
		return (c > 0);
	}
}

ResetTerm()
{
	if (IS)
		putpad(IS, 1);
	if (VS)
		putpad(VS, 1);
	ttyset(1);
}

UnsetTerm()
{
	ttyset(0);
	if (VE)
		putpad(VE, 1);
	Placur(LI - 1, 0);
	outchar('\n');
	flusho();
}

PauseJove()
{
	UnsetTerm();
#ifdef SIGTSTP
	ignore(kill(0, SIGTSTP));
#else
	Suspend();
#endif
	ResetTerm();
	ClAndRedraw();
}

#ifndef SIGTSTP
Suspend()
{
	char	*shell,
		*getenv();
	int	pid;

	switch (pid = fork()) {
	case -1:
		complain("Fork failed");

	case 0:
		signal(SIGTERM, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		execl(shell ? shell : "/bin/csh", "ed_shell", 0);
		message("Execl failed");
		_exit(1);

	default:
		signal(SIGQUIT, SIG_IGN);
		while (wait(0) != pid)
			;
		signal(SIGQUIT, finish);
	}
}
#endif

int	OKXonXoff = 0;		/* ^S and ^Q initially DON'T work */

ReInitTTY()
{
	ttyset(0);	/* Back to original settings */
	ttinit();
}

ttinit()
{
#ifdef TIOCSLTC
	ioctl(0, TIOCGLTC, (char *) &ls1);
	ls2 = ls1;
	ls2.t_suspc = (char) -1;
	ls2.t_dsuspc = (char) -1;
	ls2.t_flushc = (char) -1;
	ls2.t_lnextc = (char) -1;
#endif

	/* Change interupt and quit. */
	ioctl(0, TIOCGETC, (char *) &tc1);
	tc2 = tc1;
	tc2.t_intrc = '\035';
	tc2.t_quitc = (char) -1;
	if (OKXonXoff) {
		tc2.t_stopc = (char) -1;
		tc2.t_startc = (char) -1;
	}
	ttyset(1);

	/* Go into cbreak, and -echo and -CRMOD */

#ifdef SIGTSTP			/* New signal mechanism */
	ignorf(sigset(SIGHUP, finish));
	ignorf(sigset(SIGINT, finish));
	ignorf(sigset(SIGQUIT, SIG_IGN));
	ignorf(sigset(SIGBUS, finish));
	ignorf(sigset(SIGSEGV, finish));
	ignorf(sigset(SIGPIPE, finish));
	ignorf(sigset(SIGTERM, SIG_IGN));
#else
	ignorf(signal(SIGHUP, finish));
	ignorf(signal(SIGINT, finish));
	ignorf(signal(SIGQUIT, SIG_IGN));
	ignorf(signal(SIGBUS, finish));
	ignorf(signal(SIGSEGV, finish));
	ignorf(signal(SIGPIPE, finish));
	ignorf(signal(SIGTERM, SIG_IGN));
#endif
}

ttyset(n)
{
	struct sgttyb	tty;

	if (ioctl(0, TIOCGETP, (char *) &tty) == -1) {
		putstr("No terminal");
		exit(1);
	}
	if (n) {
		tty.sg_flags &= ~(ECHO | CRMOD);
		tty.sg_flags |= CBREAK;
	} else {
		tty.sg_flags |= (ECHO | CRMOD);
		tty.sg_flags &= ~CBREAK;
	}
	if (ioctl(0, TIOCSETN, (char *) &tty) == -1) {
		putstr("cbreak?");
		exit(1);
	}
	ioctl(0, TIOCSETC, n == 0 ? (char *) &tc1 : (char *) &tc2);
#ifdef TIOCSLTC
	ioctl(0, TIOCSLTC, n == 0 ? (char *) &ls1 : (char *) &ls2);
#endif
}

#ifdef LSRHS_KLUDGERY
deal_with_scroll()
{
	char	*pp;

	pp = getenv("SCROLL");
	if (!pp || !strcmp(pp, "smooth"))
		putstr("\033[?4h");	/* Put in smooth scroll. */
}
#endif

/* NOSTRICT */

char *
emalloc(size)
{
	char	*ptr;

	if (ptr = malloc((unsigned) size))
		return ptr;
	GCchunks();
	if (ptr = malloc((unsigned) size))
		return ptr;
	error("out of memory");
	/* NOTREACHED */
}

dispatch(c)
register int	c;
{
	struct function	*fp;

	fp = mainmap[c];
	if (fp == 0) {
		rbell();
		exp = 1;
		exp_p = errormsg = 0;
		message("");
		return;
	}
	ExecFunc(fp, 0
);
}

int	LastKeyStruck;

getch()
{
	int	c;

	if (stackp >= 0 && macstack[stackp]->Flags & EXECUTE)
		c = MacGetc();
	else {
		redisplay();
		if ((c = getchar()) == EOF)
			finish(SIGHUP);
		c &= 0177;
		if (KeyMacro.Flags & DEFINE)
			MacPutc(c);
	}
	LastKeyStruck = c;
	return c;
}

parse(argc, argv)
char	*argv[];
{
	BUFFER	*firstbuf = 0;
	char	c;

	message("Jonathan's Own Version of Emacs");

	*argv = (char *) 0;
	argvp = argv + 1;

	while (argc > 1) {
		if (argv[1][0] == '-') {
			if (argv[1][1] == 't') {
				++argv;
				--argc;
				exp_p = 1;
				find_tag(argv[1]);
				if (!firstbuf)
					firstbuf = curbuf;
			}
		} else if (argv[1][0] == '+' &&
					(c = argv[1][1]) >= '0' && c <= '9') {
			++argv;
			--argc;
			SetBuf(do_find(curwind, argv[1]));
			if (!firstbuf)
				firstbuf = curbuf;
			SetLine(next_line(curline, atoi(&argv[0][1]) - 1));
		} else {
			SetBuf(do_find(curwind, argv[1]));
			if (!firstbuf)
				firstbuf = curbuf;
		}

		++argv;
		--argc;
	}

	if (firstbuf)
		SetBuf(firstbuf);
}

copy_n(f, t, n)
register int	*f,
		*t,
		n;
{
	while (n--)
		*f++ = *t++;
}

#ifdef lint
Ignore(a)
	char *a;
{

	a = a;
}

Ignorf(a)
	int (*a)();
{

	a = a;
}
#endif

/* VARARGS1 */

error(fmt, args)
char	*fmt;
{
	if (fmt) {
		format(mesgbuf, fmt, &args);
		UpdMesg++;
	}
	rbell();
	longjmp(mainjmp, ERROR);
}

/* VARARGS1 */

complain(fmt, args)
char	*fmt;
{
	if (fmt) {
		format(mesgbuf, fmt, &args);
		UpdMesg++;
	}
	rbell();	/* Always ring the bell now */
	longjmp(mainjmp, COMPLAIN);
}

/* VARARGS1 */

confirm(fmt, args)
char	*fmt;
{
	char *yorn;

	format(mesgbuf, fmt, &args);
	yorn = ask((char *)0, mesgbuf);
	if (*yorn != 'Y' && *yorn != 'y')
		longjmp(mainjmp, COMPLAIN);
}

#ifndef PROFILE
exit(status)
{
	flusho();
	_exit(status);
}
#endif

Recurse()
{
	RecDepth++;
	DoKeys(0);
	RecDepth--;
}

read_ch()
{
	int	c;

	if ((c = peekc) != -1) {
		peekc = -1;
		return c;
	}
	return getch();
}

DoKeys(first)
{
	int	c;
	jmp_buf	savejmp;

	copynchar((char *) savejmp, (char *) mainjmp, sizeof savejmp);

	switch (setjmp(mainjmp)) {
	case 0:
		if (first)
			parse(iniargc, iniargv);
		break;

	case QUIT:
		copynchar((char *) mainjmp, (char *) savejmp, sizeof mainjmp);
		return;

	case ERROR:
		getDOT();	/* God knows what state linebuf was in */

	case COMPLAIN:
		IOclose();
		Getchar = getch;
		if (Input) {
			ignore(close(Input));
			Input = 0;	/* Terminal has control now */
		}
		errormsg++;
		FixMacros();
		Asking = 0;		/* Not anymore we ain't */
		redisplay();
		break;
	}

	this_cmd = last_cmd = 0;

	for (;;) {
		exp = 1;
		exp_p = 0;
		last_cmd = this_cmd;
cont:
		this_cmd = 0;
		c = read_ch();
		if (c == -1)
			continue;
	 	dispatch(c);
		if (this_cmd == ARG_CMD)
			goto cont;
	}
}

int	Crashing = 0;

main(argc, argv)
char	*argv[];
{
	extern char searchbuf[];
	char	*home;

	peekc = -1;
	killptr = errormsg = 0;
	curbuf = world = (BUFFER *) 0;

	iniargc = argc;
	iniargv = argv;

	searchbuf[0] = '\0';
	InputPending = 0;
	Asking = 0;
	Crashing = 0;
	Input = 0;		/* Terminal? */
	RecDepth = 0;		/* Top level */
	Getchar = getch;

	if (setjmp(mainjmp)) {
		printf("Pre-error: \"%s\"; tell Jon Payne\n", mesgbuf);
		finish(0);
	}

	getTERM();
	InitCM();
	CanScroll = ((AL && DL) || VT);
	settout();
	make_scr();	/* Do this before making zero */
	tmpinit();	/* Init temp file */
	MacInit();	/* Initialize Macros */
	InitFuncs();	/* Initialize functions and variables */
	InitBindings();	/* Everyday EMACS commands */
	winit();	/* Initialize window */

	noflags(origflags);
	curbuf = do_select(curwind, Mainbuf);

	if (home = getenv("HOME"))
		ignore(joverc(sprint("%s/.joverc", home)));
	ttinit();	/* Initialize terminal (after ~/.joverc) */

	putpad(CL, 1);
	if (IS)
		putpad(IS, 1);
	if (VS)
		putpad(VS, 1);

	copy_n(origflags, curbuf->b_flags, NFLAGS);
	/* All new buffers will have these flags when created. */
	RedrawDisplay();	/* Start the redisplay process */
	DoKeys(1);
	finish(0);
}
