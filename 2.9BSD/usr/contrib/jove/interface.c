/* Jonathan Payne at Lincoln-Sudbury Regional High School 5-25-83

   interface.c

   contains the procedures to HELP the user by creating buffers with
   information in them, or temporarily writing over the user's text. */


#include "termcap.h"
#include "jove.h"

static char	*BufToUse;	/* Buffer to pop to if we are using buffers */
static WINDOW	*LastW;		/* Save old window here so we can return */
static BUFFER	*LastB;		/* Buffer that used to be in LastW (in case it
				   isn't when we're done. */

static int	Gobble,
		Wrapped,
		StartNo,
		LineNo;		/* Line number we have reached (if we are NOT
				   using buffers.  This way is MUCH easier since
				   all we have to do is zero out the oimage line
				   pointer and let redisplay() notice the change
				   and fix it. */

static int	WhichKind;	/* Buffers or screen? */

int	UseBuffers = 0;		/* Don't create buffers by default. It may
				   be useful sometimes to making listings */

#define WITHSCREEN	1
#define WITHBUFFER	2

/* Tell With Buffers sets everything up so that we can clean up after
   ourselves when we are told to. */

TellWBuffers(bname, clobber)
char	*bname;
{
	WhichKind = WITHBUFFER;		/* So we know how to clean up */
	BufToUse = bname;
	LastW = curwind;
	LastB = curbuf;
	pop_wind(bname, clobber);	/* This creates the window and
					   makes it the current window. */
}

/* Tell With Screen.  If gobble is non-zero we DON'T ungetc characters as
   they are typed  e.g. --more-- or at the end of the list. */

TellWScreen(gobble)
{
	WhichKind = WITHSCREEN;
	StartNo = LineNo = FLine(curwind);	/* Much easier, see what I mean! */
	Wrapped = 0;
	Gobble = gobble;
}

/* DoTell ... don't keep the user in suspense!
   
   Takes a string as an argument and displays it correctly, i.e. if we are
   using buffers simply insert the string into the buffer adding a newline.
   Otherwise we swrite the line and change oimage */

DoTell(string)
char	*string;
{
	if (WhichKind == WITHBUFFER) {
		exp = 1;
		ins_str(string);
		LineInsert();
		return OKAY;
	}

	if (LineNo == StartNo + curwind->w_height - 1) {
		int	c;

		Wrapped++;		/* We wrapped ... see StopTelling */
		LineNo = StartNo;
		message("--more--");
		UpdateMesg();
		switch (c = getchar()) {
		case ' ':
			break;

		case CTL(G):
		case '\177':
			if (!Gobble)
				ignore(Ungetc(c));
			return ABORT;

		default:
			if (Gobble == 0)
				ignore(Ungetc(c));
			return STOP;
		}
		message("");
		UpdateMesg();
	}
	i_set(LineNo, 0);
	ignore(swrite(string));
	cl_eol();
	oimage[LineNo].Line = (LINE *) -1;
	LineNo++;
	flusho();
	return OKAY;
}

StopTelling()
{
	if (WhichKind == WITHBUFFER) {
		NotModified();
		SetWind(LastW);
		LastW = 0;
	} else {
		int	c;

		ignore(DoTell("----------"));
		c = getchar();
		if (c != ' ' && Gobble == 0)
			ignore(Ungetc(c));
	}
}

