/*
   Jonathan Payne at Lincoln-Sudbury Regional High School 5-25-83

   jove_case.c

   contains case case-region and case-word functions.  */

#include "jove.h"

DOTsave(buf)
BUFLOC *buf;
{
	buf->p_line = curline;
	buf->p_char = curchar;
}

CapWord()
{
	int num = exp;

	exp = 1;	/* So all the commands are done once */

	while (num--) {
		to_word(1);	/* Go to the beginning of the next word. */
		if (eobp())
			break;
		upper(&linebuf[curchar]);	/* Cap this word. */
		SetModified(curbuf);	/* Or else lsave won't do anything */
		makedirty(curline);
		curchar++;
		while (!eolp() && isword(linebuf[curchar])) {
			lower(&linebuf[curchar]);
			curchar++;
		}
	}
}

case_word(up)
{
	BUFLOC bp;

	DOTsave(&bp);
	ForWord();	/* Go to end of the region */
	case_reg(bp.p_line, bp.p_char, curline, curchar, up);
}

upper(c)
register char	*c;
{
	if (*c >= 'a' && *c <= 'z')
		*c -= 040;
}

lower(c)
register char	*c;
{
	if (*c >= 'A' && *c <= 'Z')
		*c += 040;
}

case_reg(line1, char1, line2, char2, up)
LINE	*line1,
	*line2;
{
	char lbuf[LBSIZE];

	SetModified(curbuf);
	fixorder(&line1, &char1, &line2, &char2);
	lsave();
	ignore(getline(line1->l_dline, lbuf));
	for (;;) {
		if (line1 == line2 && char1 == char2)
			break;
		if (lbuf[char1] == '\0') {
			char1 = 0;
			line1->l_dline = putline(lbuf);
			makedirty(line1);
			if (lastp(line1))
				break;
			line1 = line1->l_next;
			ignore(getline(line1->l_dline, lbuf));
			continue;
		}
		if (up)
			upper(&lbuf[char1]);
		else
			lower(&lbuf[char1]);
		char1++;
	}
	line1->l_dline = putline(lbuf);
	makedirty(line1);
	getDOT();
}

CasRegLower()
{
	CaseReg(0);
}

CasRegUpper()
{
	CaseReg(1);
}

CaseReg(up)
{
	register MARK	*mp = CurMark();

	case_reg(curline, curchar, mp->m_line, mp->m_char, up);
}
