/*
   Jonathan Payne at Lincoln-Sudbury Regional High School 5/25/83
  
   Incremental search here...  */

#include "jove.h"

#define CharCom(a, b) (IsFlagSet(globflags, CASEIND) ? (Upper(a) == Upper(b)) : (a == b))

#define FOUND	1
#define GOBACK	2

static char	IncBuf[100],
		*incp = 0;
int	FirstInc = 0;
jmp_buf	incjmp;

IncFSearch()
{
	IncSearch(1);
}

IncRSearch()
{
	IncSearch(-1);
}

extern char	searchbuf[];

IncSearch(direction)
{
	BUFLOC	bp;

	DOTsave(&bp);
	switch (setjmp(incjmp)) {
	case GOBACK:
		SetDot(&bp);
		break;

	case 0:
		IncBuf[0] = 0;
		incp = IncBuf;
		FirstInc = 1;
		SetMark();
		isearch(direction, 0, 0);
		break;

	case FOUND:
	default:
		break;
	}
	message(IncBuf);	/* Show the search string */
	setsearch(IncBuf);	/* This is now the global string */
}

isearch(direction, failing, ctls)
{
	char	c;
	BUFLOC	bp,
		*newdot;
	int	nextfail = failing;	/* Kind of a temp variable */

	DOTsave(&bp);
	for (;;) {
		s_mess("%s%sI-search: %s", failing ? "Failing " : "",
				direction < 0 ? "reverse-" : "", IncBuf);
		c = (*Getchar)();
		switch (c) {
		case '\177':
		case CTL(H):
			if (!ctls)	/* If we didn't repeated the command */
				if (incp > IncBuf)
					*--incp = 0;
			return;

		case CTL([):
okayleave:
			longjmp(incjmp, FOUND);

		case CTL(\\):
			c = CTL(S);
		case CTL(R):
		case CTL(S):
			if (FirstInc && incp) {	/* We have been here before */
				FirstInc = 0;
				strcpy(IncBuf, searchbuf);
				incp = IncBuf + strlen(IncBuf);
				failing = 0;
			}
			/* If we are not failing, OR we are failing BUT we
			   are changing direction, then allow another
			   search */

			if (!failing || ((direction == -1 && c == CTL(S)) ||
					(direction == 1 && c == CTL(R)))) {
				newdot = dosearch(IncBuf, c == CTL(R) ? -1 : 1, 0);
				if (newdot) {
					SetDot(newdot);
					nextfail = 0;
				} else
					nextfail = 1;
				SetMark();
				isearch(c == CTL(R) ? -1 : 1, nextfail, 1);
				nextfail = failing;
			} else
				rbell();
			break;

		case CTL(G):
			longjmp(incjmp, GOBACK);
			/* Easy way out! */

		case CTL(Q):
		case CTL(^):
			c = (*Getchar)() | 0200;	/* Tricky! */
		default:
			if (c & 0200)
				c &= 0177;
			else {	/* Check for legality */
				if (c < ' ' && c != '\t') {
					peekc = c;
					goto okayleave;
				}
			}
			FirstInc = 0;	/* Not first time anymore! */
			*incp++ = c;	/* Always put in string */
			*incp = 0;
			if (!failing) {
				if (direction > 0 && CharCom(linebuf[curchar], c))
					ForChar();
				else if (direction < 0 && CharCom(linebuf[curchar + strlen(IncBuf) - 1], c))
					;
				else if (!failing) {
					newdot = dosearch(IncBuf, direction, 0);
					if (newdot == 0)
						nextfail++;
					else
						SetDot(newdot);
				}
			}
			isearch(direction, nextfail, 0);
			nextfail = failing;
			/* Reset this to what it was, because if we
			 * are here again, then we must have deleted
			 * back.
			 */
		}
		SetDot(&bp);
	}
}
