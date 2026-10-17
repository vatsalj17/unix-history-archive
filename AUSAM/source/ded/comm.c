#include "ded.h"
#include "match.h"
#include "char.h"

extern tdiag(); /* see do_rwrite */

struct COMMANDS
 { char *str;
    int (*func)();
    int params;
 };

extern char *get_range();
extern char one_letter();
extern
	do_quote(),
	do_ampersand(),
	do_slash(),
	do_semicolon(),
	do_percent(),
	do_unix(),
	do_at(),
	do_prime(),
	do_mark(),
	do_equals(),
	do_break(),
	do_copy(),
	do_copy(),
	do_linedelete(),
	do_dbug(),
	do_eof(),
	do_first(),
	do_file(),
	do_first(),
	do_last(),
	do_last(),
	do_move(),
	do_mark(),
	do_move(),
	do_exit(),
	do_printscreen(),
	do_quit(),
	do_rquit(),
	do_rwrite(),
	do_tab(),
	do_tof(),
	do_unbug(),
	do_write(),
	do_change();
/* 0 means no parameters,
 * 1 means parameters, leading spaces insignificant
 * 2 means parameters, leading spaces must be left alone
 */
struct COMMANDS  commands[]
 { "\"",       do_quote,       0,
    "&",        do_ampersand,   0,
    "/",        do_slash,       2,
    ";",        do_semicolon,   2,
    "%",        do_percent,     0,
    "!",        do_unix,        1,   /* see file unix.c */
    "@",       do_at,          0,
    "'",        do_prime,       1,
    ".",        do_mark,        1,
    "=",        do_equals,      1,
    "b",        do_break,       1,
    "c",        do_copy,        1,
    "copy",     do_copy,        1,
    "d",        do_linedelete,  1,
    "dbug",     do_dbug,        1,
    "eof",      do_eof,         0,
    "f",        do_first,       1,
    "file",     do_file,        0,
    "first",    do_first,       1,
    "l",        do_last,        1,
    "last",     do_last,        1,
    "m",        do_move,        1,
    "mark",     do_mark,        1,
    "move",     do_move,        1,
    "ok",       do_exit,        0,
    "p",        do_printscreen, 0,
    "q",        do_quit,        0,
    "reallyquit",
		do_rquit,     0,
    "reallywrite",
		do_rwrite,   0,
    "t",        do_tab,         1,
    "tof",      do_tof,         0,
    "unbug",    do_unbug,       1,
    "w",        do_write,       0,
    "x",        do_change,      1,
    0,
 };

do_command()
 { char line[ENOUGH];
    struct COMMANDS *com;
    char str[ENOUGH];
    register char *sp, *lp;
    register int lnum;

    /* find command string */
    diag_clear();
    copyrow(EDITROW,1,line);
    lp=line;
    sp = str;

    /* remember current position */
    store_c(&virt_c, (mode==INMODE ? &in_c: &edit_c));

    /* pick up command */
    while (*lp==' ') lp++;
    if (*lp==0)
      tdiag("?? no command");
    else
    if (alpha(*lp))
      while (alpha(*lp)) *sp++ = lcase(*lp++);
    else
    if (digit(*lp))
     { lnum = 0;
	do lnum = lnum*10+(*lp++ - '0'); while (digit(*lp));
	lnum--; /* I number from zero */
	if (*lp != 0) tdiag("!! command follows number !!");
	else
	if (lnum<0 || lnum>maxl) tdiag("!! no such line !!");
	else
	 { blobit(); disp_range(lnum, lnum, true); }
     }
    else
      /* if no alpha character, accept first character */
      *sp++ = *lp++;
    *sp=0;



    /* find procedure indexed by command and call it */
    com = commands;
    while (com->str != 0)
      if (streq(str,com->str))
	{  if (com->params || *lp==0)
	     { blobit();
		/* find first character of command */
		if (com->params==1) while(*lp==' ') lp++;
		(*(com->func))(lp);
		diag_clear();
		return(true); /* procedure will call tdiag or fdiag on error */
	     }
	    else fdiag("?? no parameters allowed ??");
	}
      else com++;

    tdiag("?? unrecognisable command");
 }

/* fill in missing argument from memory */
char *fillin(arg, old_arg, message)
char *arg, *old_arg, *message;
 { register char *a, *oa;
    register int i;

    if (arg==0 || *arg==0)
     { if (*old_arg==0) tdiag("?? %s ??", message);
	else
	 { diag_clear();
	    redraw(EDITROW, lastcol(EDITROW)+1, old_arg);
	    blobit();
	    return(old_arg);
	 }
     }
    else
     { a = arg; oa = old_arg;
	while (*oa++ = *a++);
	return(arg);
     }
  }

do_exit()
 { leave(mainloop,true); }

do_eof()
 { if (topl+LASTINROW<maxl) set_topl(maxl);
    else complain();
    i_mode(LASTINROW,lastcol(LASTINROW)+1);
 }

do_tof()
 { if (topl==0) complain();
    else set_topl(0);
    i_mode(0,0);
 }

do_linedelete()
 { del_row(in_c.row, (mode == INMODE ? &virt_c : &in_c));
    return(true);
 }

/* procedures to create ranges */
do_first(str)
char *str;
 { register char c, rc;

    c = one_letter(str); rc = c-'a';
    b_range[rc] = topl+in_c.row;
    if (e_range[rc] < b_range[rc]) e_range[rc] = b_range[rc];

    tell_range(c);
 }

do_last(str)
char *str;
 { register char c, rc;

    c = one_letter(str); rc = c-'a';
    e_range[rc] = topl+in_c.row;
    if (b_range[rc]<0 || e_range[rc] < b_range[rc]) b_range[rc] = e_range[rc];

    tell_range(c);
 }

do_mark(str)
char *str;
 { register char c, rc;

    c = one_letter(str); rc = c-'a';
    b_range[rc] = e_range[rc] = topl+in_c.row;

    tell_range(c);
 }

/* routines to move blocks of lines about */

do_move(str)
char *str;
 { shift_range(str, true); }

do_copy(str)
 { shift_range(str, false); }

shift_range(str, moving)
char *str;
int moving;
 { int begin, end, d1, d2, destination;
    int after;
    int size;

    str = get_range(str, &begin, &end);
    str = get_range(str, &d1, &d2);

    if (*str=='+') { after = true; str++; }
    else
    if (*str=='-') { after = false; str++; }
    else after = true; /* default */

    destination = (after ? d2+1 : d1);

    if (*str!=0) fdiag("!! only two arguments allowed !!");
    else
    if (begin>end) fdiag("!! invalid source range (%d to %d)", begin+1, end+1);
    else
    if ( (moving && begin<=destination && destination<=end) ||
	 (begin<destination && destination<=end) )
      fdiag("!! destination (%d) is within source (%d to %d) !!",
		    destination+1, begin+1, end+1);

    size = end-begin+1;

    savescreen(); saveedit();

    while (begin<=end)
      if (moving)
       { moveline(begin, destination);
	  if (destination>end)
	    /* begin stays the same (next line of source moves down one)
	     * end moves down one
	     * destination stays same (first line of destination moves down)
	     */
	    end--;
	  else
	    /* (destination is before begin)
	     * begin moves up one (next line of source moves down)
	     * destination moves up one
	     * end stays the same
	     */
	   { destination++; begin++; }
       }
      else
       { copyline(begin, destination);
	  if (destination>end)
	    /* begin moves up one, to next line of source
	     * end stays the same
	     * destination moves up one
	     */
	   { begin++; destination++; }
	  else
	    /* (destination is before begin)
	     * destination moves up one
	     * begin moves up two (destination has pushed it)
	     * end moves up one
	     */
	   { destination++; begin =+ 2; end++; }
       }

    /* if the line was moved after, display start of range! */
    topl = enclose_range(destination-size, destination-1, after);
    adj_maxl();
    refreshscreen();
    disp_range(destination-size, destination-1, after);
 }

/* get a single range argument from the command string */
char *get_range(str, a_first, a_second)
char *str;
int *a_first, *a_second;
 { register char c, rc;

    while (*str==' ') str++;
    if ((c = *str++)==0)
      fdiag("!! range argument missing !!");
    else
    if (c>='a' && c<='z') /* letter range */
     { rc = c-'a';
	if (b_range[rc]<0 || e_range[rc]>maxl || b_range[rc]>e_range[rc])
	  fdiag("!! no such range as %c !!", c);
	else
	 { *a_first = b_range[rc];
	    *a_second = e_range[rc];
	 }
     }
    else
    if (c=='.')
      *a_first = *a_second = topl+in_c.row;
    else
      fdiag("!! range must be letter or dot !!");

    /* eat the stuff after the argument */
    while (*str==' ') str++;
    if (*str==',') str++;
    return(str);
 }

/* display size of range */
tell_range(c)
char c;
 { register char rc;

    if (c=='.')
      tdiag("(current line is %d)", topl+in_c.row+1);
    else
    if (c<'a' || c>'z')
      editerror("invalid range character %o", c);
    else
     { rc = c-'a';
	if (b_range[rc]==e_range[rc])
	  tdiag("('%c' labels line %d)", c, b_range[rc]+1);
	else
	  tdiag("('%c' labels lines %d to %d)",
			c, b_range[rc]+1, e_range[rc]+1);
     }
 }

/* display a range on screen */
do_prime(str)
char *str;
 { int begin, end;
    register char c;
    int before;

    str = get_range(str, &begin, &end);

    if ((c = *str)=='-' || c==0) before = true;
    else
    if (c=='+') before = false;
    else
      fdiag("!! range must be single lower case letter !!");

    if (c!=0 && str[1]!=0 )
      fdiag("!! character follows %c !!",c);
    else
      disp_range(begin, end, before);
 }

/* give information about ranges */
do_equals(str)
char *str;
 { if (str[1]!=0)
      fdiag("!! range must be single character !!");
    else
      tell_range(*str=='.' ? *str : one_letter(str));
 }

/* read a single range character argument */
char one_letter(str)
char *str;
 { register char c;

    if ((c = *str) == 0 || c<'a' || c>'z' || str[1] != 0)
      tdiag("!! single lower-case letter required !!");

    return(c);
 }

/* routines to change to next or previous screenful */

do_quote()
 { if (topl+LASTINROW < maxl) set_topl(topl+NINROWS);
    else complain();
    i_mode(0,0);
 }

do_ampersand()
 { if (topl > 0) set_topl(topl-NINROWS);
    else complain();
    i_mode(0,0);
 }

do_quit()
 { savescreen();
    if (tmp_changed) tdiag("!! file has changed - use reallyquit to exit");
    else do_rquit();
 }

do_rquit()
 { leave(mainloop, false); }

char old_search[ENOUGH]; /* should contain zero first time */

do_slash(str)
char *str;
 { str = fillin(str, old_search, "no string specified");
    return(do_find(str, '/', FORWARD));
 }

do_semicolon(str)
char *str;
 { str = fillin(str, old_search, "no string specified");
    return(do_find(str, ';', BACKWARD));
  }

do_find(str, c, direction)
char *str;
char c;
int direction;
 { struct RE re[RESIZE];
    char *sp;

    if (*str==0) fdiag("?? no string specified ??");

    sp = build_re(re, str, c);
    if (*sp==0)
     { if ( find(re, direction, topl+in_c.row, in_c.col) )
	 { i_display(ms_line);
	    complain(); /* bleep */
	    i_mode(ms_line-topl, ms_col);
	 }
	else tdiag("!! can't find it !!");
     }
    else
     fdiag("!! string doesn't end with %c !!", c);
 }

char old_pat[ENOUGH]; /* I hope this contains zero to start with */

do_change(str)
char *str;
 { struct RE re[RESIZE];
    char *sp;
    char fin_ch;

    str = fillin(str, old_pat, "no pattern");
    if ((fin_ch = *str) != '/' && fin_ch != ';') fdiag("!! no pattern !!");

    sp = build_re(re,str+1,fin_ch);

    if (*sp==0) fdiag("!! no second string !!");

    return( change(re,sp,(fin_ch=='/' ? FORWARD : BACKWARD), fin_ch,
			topl+in_c.row,in_c.col) );
 }

do_dbug(str)
char *str;
 { if (*str==c_SPACE) str++;

    if (str[1] != '\0') fdiag("!! too many letters !!");
    else
    if (set_dbug(*str)) return(true);
    else fdiag("!! invalid debug option %c", *str);
 }

do_unbug(str)
char *str;
 { if (*str==c_SPACE) str++;

    if (str[1] != 0) fdiag("!! too many letters !!");
    else
    if (unset_dbug(*str)) return(true);
    else fdiag("!! invalid debug option %c", *str);
 }

do_percent()
 { int fline;

    fline = topl+in_c.row;
    set_topl(fline-NINROWS/2);
    i_mode(fline-topl,in_c.col);
 }

do_at()
 { return(do_unix(0)); } /* activates a newsh */

do_printscreen()
 { savescreen(); setupscreen(); return(true); }

do_tab(str)
char *str;
 { return(option(str,'t',&tbout)); }

do_break(str)
char *str;
 { return(option(str,'b',&txin)); }

do_write()
 { savescreen();
    if (!tmp_changed)
      tdiag("!! file hasn't changed - use reallywrite to force output");
    else
      do_rwrite();
 }

do_rwrite()
 { savefile(tdiag, true); complain();
    i_mode(in_c.row, in_c.col);
 }

option(str, com, var)
char *str, com;
int *var;
 { if (*str==0) fdiag("?? no option follows %c ??", com);
    else
    if (*str=='+' && str[1]==0) *var = true;
    else
    if (*str=='-' && str[1]==0) *var = false;
    else
      fdiag("?? unrecognisable option after %c ??", com);

    return(true);
 }

i_mode(srow,scol)
int srow,scol;
 { diag_clear();
    if (mode==INMODE)
     { position(srow,scol); leave(do_command,true); }
    else
     { set_c(srow,scol,&in_c); etoi(); }
 }

/* put a selected line on-screen */
i_display(fline)
int fline;
 { if (fline<topl+2 || topl+LASTINROW-2<fline)
      set_topl(fline-NINROWS/2);
 }

fdiag(str,c1,c2,c3,c4,c5)
char *str;
int c1,c2,c3,c4,c5;
 { diag(str,c1,c2,c3,c4,c5);
    complain();
    leave(do_command, false);
 }

tdiag(str,c1,c2,c3,c4,c5)
char *str;
int c1,c2,c3,c4,c5;
 { diag(str,c1,c2,c3,c4,c5);
    complain();
    leave(do_command, true);
 }

/* tell the user which file it is */
do_file()
 { tdiag(filename); }

/* show a range on the screen */
disp_range(begin, end, before)
int begin, end, before;
 { int newtop;

    newtop = enclose_range(begin, end, before);
    set_topl(newtop);
    i_mode((before ? begin-topl: end-topl), 0);
 }

int enclose_range(begin, end, before)
int begin, end, before;
 { register int size, res;

    if ((size = end-begin+1) <= NINROWS)
      res = (begin<topl+2 || end>topl+LASTINROW-2 ?
		begin-(NINROWS-size)/2 : topl);
    else
    if (before)
      res = (begin<topl+2 || begin>topl+NINROWS/2 ?
		begin-NINROWS/2 : topl);
    else
      res = (end<topl+2 || end>topl+LASTINROW-2 ?
		end-NINROWS/2 : topl);

    return(res<0 ? 0 : res+LASTINROW>=maxl ? maxl-(LASTINROW) : res);
 }
