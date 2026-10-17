/*
   Jonathan Payne at Lincoln-Sudbury Regional High School 5-25-83

   jove_term.c

   Gets the termcap information and complains if there are not enough
   of the basic features on the particular terminal. */

#include "jove_tune.h"
#include <sgtty.h>

/* Termcap definitions */

char	*UP,	/* Scroll reverse, or up */
	*VT,	/* If on vt100 */
	*SO,	/* Start standout */
	*SE,	/* End standout */
	*CM,	/* The cursor motion string */
	*CL,	/* Clear screen */
	*CE,	/* Clear to end of line */
	*HO,	/* Home cursor */
	*AL,	/* Addline (insert line) */
	*DL,	/* Delete line */
	*IS,	/* Initial start */
	*VS,	/* Visual start */
	*VE,	/* Visual end */
	*IC,	/* Insert char	*/
	*DC,	/* Delete char	*/
	*IM,	/* Insert mode */
	*EI,	/* End insert mode */
	*LL,	/* Move to last line, first column of screen */
	*BC,	/* Back space */
	*SR,
	*VB;

int	LI,		/* Number of lines */
	CO,		/* Number of columns */
	TABS,		/* Whether we are in tabs mode */
	UpLen,		/* Length of the UP string */
	HomeLen,	/* Length of Home string */
	LowerLen;	/* Length of lower string */

int	BG;		/* Are we on a bitgraph? */

int ospeed;

char	tspace[128];

char **meas[] = {
	&VS, &VE, &IS, &AL, &DL, &VT, &SO, &SE,
	&CM, &CL, &CE, &HO, &UP, &BC, &IC, &IM,
	&DC, &EI, &LL, &SR, &VB, 0
};

gets(buf)
char	*buf;
{
	buf[read(0, buf, 12) - 1] = 0;
}	

char	*sprint();

TermError(str)
char	*str;
{
	char	*cp;

	cp = sprint("Termcap error: %s\n", str);
	if (write(1, cp, strlen(cp)));
	exit(1);
}

getTERM()
{
	char	*getenv();
	struct sgttyb tty;
	char	*ts="vsveisaldlvtsosecmclcehoupbcicimdceillsrvb";
	char	termbuf[13],
		*termname = 0,
		*termp = tspace,
		tbuff[1024];
	int	i;

	if (gtty(0, &tty))
		TermError("ioctl fails");
	TABS = !(tty.sg_flags & XTABS);
	ospeed = tty.sg_ospeed;

	termname = getenv("TERM");
	if (termname == 0) {
		putstr("Enter terminal name: ");
		gets(termbuf);
		if (termbuf[0] == 0)
			TermError("");

		termname = termbuf;
	}

	BG = strcmp(termname, "bg") == 0;	/* Kludge to help out bg scroll */

	if (tgetent(tbuff, termname) < 1)
		TermError("terminal type?");

	if ((CO = tgetnum("co")) == -1)
		TermError("columns?");

	if ((LI = tgetnum("li")) == -1)
		TermError("lines?");

	for (i = 0; meas[i]; i++) {
		*(meas[i]) = (char *)tgetstr(ts,&termp);
		ts += 2;
	}
/* You can decide whether you want this ... */
	if (!CE || !UP)
		TermError("Your terminal sucks!");
}
