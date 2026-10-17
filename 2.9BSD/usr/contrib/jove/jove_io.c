/*
   Jonathan Payne at Lincoln-Sudbury Regional High School 5-25-83
  
   Commands to read/write files/regions.  */

#include "jove.h"
#include "termcap.h"

#include <sys/types.h>
#include <sys/stat.h>

char	*WERROR	= "Write error";
long	count = 0;
int	nlines = 0;

char	iobuff[LBSIZE],
	*nextip;
int	ninbuf;

IOclose()
{
	if (io > 0)
		ignore(close(io)), io = -1;
	nextip = iobuff;
	ninbuf = 0;
}

/* This reads a line from the input file into buf. */

getfline(buf)
char	*buf;
{
	register int	c;
	register char	*lp,
			*fp;

	lp = buf;
	*lp = '\0';
	fp = nextip;
	do {
		if (--ninbuf < 0) {
			ninbuf = read(io, iobuff, LBSIZE) - 1;
			fp = iobuff;
			if (ninbuf < 0) {
				*lp = '\0';
				IOclose();
				return EOF;
			}
		}
		c = *fp++;
		if (c == '\0')
			continue;
		if (lp >= &buf[LBSIZE - 1]) {
			message("Line to long...");
			redisplay();
			IOclose();
			return EOF;
		}

		*lp++ = c;
		count++;
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	return 0;
}

/* Write the region from line1/char1 to line2/char2 to the open
   fildes `io'.  */

int	EndWNewline = 1;

putreg(line1, char1, line2, char2)
LINE	*line1,
	*line2;
{
	int n;
	register char *fp, *lp;
	register int	nib;

	count = nlines = 0;
	nib = 512;
	fp = iobuff;
	lsave();	/* Need this! */

	while (line1 != line2->l_next) {
		lp = getline(line1->l_dline, linebuf) + char1;
		if (line1 == line2)
			linebuf[char2] = '\0';
		for (;;) {
			if (--nib < 0) {
				n = fp - iobuff;
				if (write(io, iobuff, n) != n)
					goto werror;
				nib = 511;
				count += n;
				fp = iobuff;
			}
			if ((*fp++ = *lp++) == 0) {
				if (line1 != line2) {
					nlines++;
					fp[-1] = '\n';
				} else
					fp--;	/* Don't write the NULL!! */
				break;
			}
		}
		line1 = line1->l_next;
		char1 = 0;
	}
	n = fp - iobuff;

	if (write(io, iobuff, n) != n)
		goto werror;
	count += n;
	getDOT();		/* What ever was in linebuf */
	IOclose();
	return;
werror:
	error(WERROR);
	/* NOTREACHED */
}

char *
IOerr(err, file)
char	*err, *file;
{
	return sprint("Couldn't %s \"%s\"", err, file);
}

read_file(file)
char	*file;
{
	BUFLOC	save;

	setcmode();

	io = open(file, 0);
	if (io == -1) {
		curbuf->b_ino = -1;
		s_mess(IOerr("open", file));
		return;
	}
	DOTsave(&save);
	set_ino(curbuf);
	dofread(file);
	SetDot(&save);
	getDOT();
	IOclose();
}

FileMess(file, lines, chars)
char	*file;
long	chars;
{
	s_mess("\"%s\" %d lines %D characters", file, lines, chars);
}

dofread(file)
char	*file;
{
	char	end[LBSIZE];
	int	xeof = 0;

	lsave();
	nlines = 0;
	count = 0L;
	s_mess("\"%s\"", file);
	UpdateMesg();
	ignore(getline(curline->l_dline, end));
	strcpy(genbuf, end);
	strcpy(end, &end[curchar]);
	if ((xeof = getfline(linebuf)) == 0)
		linecopy(genbuf, curchar, linebuf);

	curline->l_dline = putline(genbuf);
	if (!xeof) do {
		xeof = getfline(linebuf);
		nlines++;
		curline = listput(curbuf, curline);
		curline->l_dline = putline(linebuf) | DIRTY;
	} while (!xeof);

	linecopy(linebuf, (curchar = strlen(linebuf)), end);
	curline->l_dline = putline(linebuf);
	FileMess(file, nlines, count);
}

bufname(bp)
BUFFER	*bp;
{
	char	tmp[100],
		tmp1[100],
		*cp;
	int	try = 1;

	if ((cp = bp->b_fname) == 0)
		complain("No file name");
		
	if (*cp == '.')
		++cp;
	strcpy(tmp, cp);
	cp = rindex(tmp, '/');
	if (cp)
		strncpy(tmp, &cp[1], sizeof tmp);

	strcpy(tmp1, tmp);
	while (buf_exists(tmp)) {
		ignore(sprintf(tmp, "%s<%d>", tmp1, try));
		try++;
	}
	setbname(bp, tmp);
}

SaveFile()
{
	if (IsModified(curbuf)) {
		SetUnmodified(curbuf);
		file_write(curbuf->b_fname, 0);
	} else
		s_mess("No changes need be written");
}

filemunge(fname)
char	*fname;
{
	struct stat	stbuf;

	if (stat(fname, &stbuf))
		return;
	if (stbuf.st_ino != curbuf->b_ino && (stbuf.st_mode & S_IFMT) != S_IFCHR) 
		confirm("\"%s\" already exist; are you sure? ", fname);
}

WrtReg()
{
	DoWriteReg(0);
}

AppReg()
{
	DoWriteReg(1);
}

DoWriteReg(app)
{
	char	fname[100];
	MARK	*mp = CurMark();

	/* Won't get here if there isn't a mark */
	strcpy(fname, ask((char *)0, FuncName()));

	if (!app)
		filemunge(fname);
	if (app) {
		io = open(fname, 1);	/* Writing */
		if (io == -1)
			io = creat(fname, 0644);
		else
			dolseek(io, 0L, 2);
	} else 
		io = creat(fname, 0644);
	if (io == -1)
		complain(IOerr("create", fname));

	s_mess("\"%s\"", fname);
	UpdateMesg();			/* Update mesg line */
	if (inorder(mp->m_line, mp->m_char, curline, curchar))
		putreg(mp->m_line, mp->m_char, curline, curchar);
	else
		putreg(curline, curchar, mp->m_line, mp->m_char);
	FileMess(fname, nlines, count);
	IOclose();
}

WriteFile()
{
	char	*fname,
		fnamebuf[100];

	fname = ask(curbuf->b_fname, FuncName());
	strncpy(fnamebuf, fname, sizeof fnamebuf);

	filemunge(fnamebuf);
	setfname(curbuf, fnamebuf);
	set_ino(curbuf);
	file_write(curbuf->b_fname, exp_p);
	SetUnmodified(curbuf);
}

file_write(fname, app)
char	*fname;
{
	if (fname == 0 || *fname == '\0')
		complain("I need a file name");

	io = -1;
	if (app) {
		io = open(fname, 1);	/* Writing */
		if (io == -1)
			io = creat(fname, 0644);
		else
			dolseek(io, 0L, 2);
	} else 
		io = creat(fname, 0644);
	if (io == -1)
		complain(IOerr("create", fname));

	s_mess("\"%s\"", fname);
	UpdateMesg();		/* Update mesg line */
	if (EndWNewline) {	/* Make sure file ends with a newline */
		BUFLOC	save;

		DOTsave(&save);
		Eof();
		if (length(curline))	/* Not a blank line */
			DoTimes(LineInsert, 1);	/* Make it blank */
		SetDot(&save);
	}		
	putreg(curbuf->b_zero, 0, curbuf->b_dol, length(curbuf->b_dol));
	FileMess(fname, nlines, count);
	IOclose();
}

initlist(bp)
BUFFER	*bp;
{
	lfreelist(bp->b_zero);
	ignore(listput(bp, (LINE *) 0));		/* First line in buffer */
	
	bp->b_dot->l_dline = putline("") | DIRTY;
	bp->b_char = 0;
	AllMarkSet(bp, bp->b_dot, 0);
	if (bp == curbuf)
		getDOT();
	initwinds(bp);
}

setcmode()
{
	int	len = curbuf->b_fname ? strlen(curbuf->b_fname) : 0;
	
	if (len < 2)
		return;
	if (curbuf->b_fname[len - 1] == 'c' && curbuf->b_fname[len - 2] == '.')
		OnFlag(globflags, CMODE);
}

ReadFile()
{
	char	*fname;

	if (IsModified(curbuf))
		confirm("%s modified, shall I read anyway? ", filename(curbuf));

	fname = ask(curbuf->b_fname, FuncName());
	SetUnmodified(curbuf);
	setfname(curbuf, fname);
	initlist(curbuf);
	read_file(curbuf->b_fname);
}

InsFile()
{
	char	*fname;

	fname = ask(curbuf->b_fname, FuncName());
	read_file(fname);
	SetModified(curbuf);
}

dolseek(fd, offset, whence)
long	offset;
{
	if (lseek(fd, offset, whence) == -1)
		complain("lseek failed");
}
