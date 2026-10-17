.TH 3270 1 "22 March 1985"
.SH NAME
3270 \- IBM 3277/3278 terminal emulator
.br
mset \- set terminal keyboard mapping
.SH SYNOPSIS
.B 3270
host
.br
.B mset
[ name ]
.SH DESCRIPTION
.I 3270
makes an ordinary ASCII terminal appear to be an IBM 3278 terminal and
allows full-screen interaction with any VM/CMS system over a TCP/IP connection
using the Telnet protocol as implemented by the WISCNET software for VM/CMS.
The
.I 3270
program uses
.IR curses (3)
to redraw the the screen according to the 3270 protocol,
and allows escape sequences to represent special function keys present on
3270 terminals but not on most ASCII terminals.
The program may be viewed as a special-purpose version of
.IR telnet (1)
which understands 3270 terminals, and it is similar to
.IR tn3270 (1)
from Berkeley, but it was written from scratch and does not share any
code with either of these programs.
.SH ENVIRONMENT
The specifcation of escape screen-drawing sequences is taken
from the
.I TERM
and
.I TERMCAP
variables as explained in
.IR termcap (3X).
Similarly, the specification of keyboard escape sequences is taken
from the
.I TERM
and
.I MAP3270
environment variables,
allowing the user to customize input escape seqences according to
personal taste or to accomodate special keys on a terminal that
transmit escape sequences.
In more detail, the algorithm is as follows:
If
.I MAP3270
is defined and the value does not start with `/', the value is taken
to be a definition, as defined below under PROFILE.
Otherwise, a database of descriptions is searched for an item matching
the value of the
.I TERM
environment variable.
The database is the file named by the
.I MAP3270
variable, if it is defined and begins with `/';
otherwise is a default database in an installation-dependent location,
normally
.IR /usr/local/3270profile .
If the named terminal is not found in the database, the database is searched
again for the special name `unknown'.
If that name is not found, a built-in terminal definition is used
(see below).
.PP
The program
.I mset
can be used to search for a definition (as described above) and put it
into the environment variable
.I MAP3270
thus speeding startup of
.IR 3270 .
The following command does the trick for
.IR csh (1)
(sorry, no support for 
.IR sh (1)).
.IP
set noglob; eval `mset`
.PP
.I 3270
determines the number of lines on the display from the `li'
termcap capability
(or from the terminal driver on 4.3 BSD systems) and attempts to
emulate the biggest model of 3270 terminal possible (24, 32, or 43 lines).
.SH PROFILE
The format of `profile' file is the same as `map' files used by 
the Berkeley tn3270 program:
.PP
Everything inside a `string' (see below) is left `as is'
(but newlines inside strings are not allowed).
Otherwise, whitespace is ignored and everything from a `#' up to end-of-line
is ignored.
After this space-and-comment stripping, the format of a file is
.nf

    (name ("|" name)* definition)+

where `definition' is
    "{" (action "=" string ( "|" string )* ";")* "}"

.fi
where `name' is a terminal name (sequence of alphanumerics), `action' is
one of the action names described below, and `string' is similar
in format to a C string:
.nf

    "'" ( "^" char
        | "\e" ("E" | "n" | "r" | "b" | "t" | "f" | "v" | "'")
        | "\e" octdigit [ octdigit [octdigit]]
        | otherchar
        )* "'"

.fi
The meanings of the escapes are
.nf
    "^" char = (char=='?' ? '\e177' : (c & 037))
        (NB: even if 'char' is "'")
    "\eE" = escape (^[, \e033)
    "\en" = newline (^J, \e012)
    "\et" = horizontal tab (^I, \e011)
    "\er" = return (^M, \e015)
    "\eb" = backspace (^H, \e010)
    "\ef" = formfeed (^L, \e014)
    "\ev" = vertical tab (^K, \e013)
    (the last three are not recognized by tn3270)
    "\e" num = character with value 'num', where num is a one-, two-, or
        three-digit octal number (this is also an extension over tn3270)
    In all other cases
    "\e" char = char
        (including the cases char="\e" and char="'")
.fi

It is illegal for any string to be prefix (or equal to) another in
any one definition.
.SH ACTIONS
The following actions names are recognized.
Note that not all actions are currently implemented.
An escape sequence that corresponds to an unimplemented action causes
the terminal bell to ring.
.PP
The following correspond to keys on the 3270 keyboard:
.nf
	enter,
	pfk1, pfk2, ..., pfk24  (`PF' keys)
	clear
	pa1, pa2, pa3 
	nl (`newline' or `elbow' key)
	left, down, up, right (arrow keys)
	insrt (toggles insert mode)
	delete (delete character)
	eeof (erase to end-of-field)
	tab
	btab (back-tab)

.fi
The following are currently all the same:
They cause the screen to be redrawn
.nf

	reset, master_reset, reshow

The following are currently unimplemented

	home, einp, dp, fm, cursel, flinp, escape,
	settab, clrtab, setmrg, sethom,
	coltab, colbak, indent, undent
.fi
.PP
A sample entry (and the one used if no definition can be found in the
database) is as follows:
.nf
unknown {
	enter='^m'; clear='^a'; nl='^p'; tab='^i'; btab='^u';
	left='^h'; right='^l'; up='^k'; down='^j';
	delete='^x'; eeof='^f'; insrt='^w';
	pfk1='\eE1'; pfk2='\eE2'; pfk3='\eE3'; pfk4='\eE4';
	pfk5='\eE5'; pfk6='\eE6'; pfk7='\eE7'; pfk8='\eE8';
	pfk9='\eE9'; pfk10='\eE0'; pfk11='\eE-'; pfk12='\eE=';
	pfk13='\eEq'; pfk14='\eEw'; pfk15='\eEe'; pfk16='\eEr';
	pfk17='\eEt'; pfk18='\eEy'; pfk19='\eEu'; pfk20='\eEi';
	pfk21='\eEo'; pfk22='\eEp'; pfk23='\eE['; pfk24='\eE]';
	pa1='\eEa'; pa2='\eEs'; pa3='\eEd'; reshow='^v';
}
.fi
.SH FILES
/etc/termcap
.br
/usr/local/3270profile
.SH AUTHOR
Marvin Solomon
.SH SEE ALSO
telnet(1), termcap(3x), termcap(5), tn3270(5), curses(3x)
.SH BUGS
Not all 3270 functions are supported, nor all tn3270 features, nor
all telnet commands.
.PP
.I 3270
cannot be used as a replacement for
.IR telnet .
.PP
The program runs in
.I cbreak
mode, so some control charcters (such as stop, start, and interrupt)
are caught by the terminal driver, and cannot be used in escape sequences.
