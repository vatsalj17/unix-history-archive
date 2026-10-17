


PICK(1)             UNIX Programmer's Manual              PICK(1)



NAME
     pick - select messages by content

SYNOPSIS
     pick [+folder] [msgs] [-and ...] [-or ...] [-not ...]
          [-lbrace ... -rbrace] [--component pattern]
          [-after date] [-before date] [-datefield field]
          [-sequence name ...] [-public] [-nopublic] [-zero]
          [-nozero] [-list] [-nolist] [-help]

     typically:
          scan `pick -from jones`
          pick -to holloway -sequence select
          show `pick -before friday`
9     _P_i_c_k searches messages within a folder for the specified
     contents, and then identifies those messages.  Two types of
     search primitives are available: pattern matching and date
     constraint operations.

     A modified _g_r_e_p(1) is used to perform the matching, so the
     full regular expression (see _e_d(1)) facility is available
     within `pattern'.  With `-search', `pattern' is used
     directly, and with the others, the grep pattern constructed
     is:

          "component[ \t]*:.*pattern"

     This means that the pattern specified for a `-search' will
     be found everywhere in the message, including the header and
     the body, while the other pattern matching requests are lim-
     ited to the single specified component.  The expression

          `--component pattern'

     is a shorthand for specifying

          `-search "component[ \t]*:.*pattern" '

     It is used to pick a component which is not one of "To:",
     "cc:", "Date:", "From:", or "Subject:".  An example is
     `pick --reply-to pooh'.

     Pattern matching is performed on a per-line basis.  Within
     the header of the message, each component is treated as one
     long line, but in the body, each line is separate.
     Lower-case letters in the search pattern will match either
     lower or upper case in the message, while upper case will
     match only upper case.

     Independent of any pattern matching operations requested,
     the switches `-after date' or `-before date' may also be
     used to introduce date/time contraints on all of the



Printed 1/10/87                MH                               1


9


PICK(1)             UNIX Programmer's Manual              PICK(1)



     messages.  By default, the "Date:" field is consulted, but
     if another date yielding field (such as "BB-Posted:" or
     "Delivery-Date:") should be used, the `-datefield field'
     switch may be used.  _P_i_c_k will actually parse the date
     fields in each of the messages specified in `msgs' (unlike
     the `-date' switch described above which does a pattern
     matching operation), and compare them to the date/time
     specified by use of the `-after' and `-before' switches.  If
     `-after' is given, then only those messages whose "Date:"
     field value is chronologically after the date specified will
     be considered.  The `-before' switch specifies the compli-
     mentary action.

     Both the `-after' and `-before' switches take legal
     822-style date specifications as arguments.  _P_i_c_k will
     default certain missing fields so that the entire date need
     not be specified.  These fields are (in order of default-
     ing): timezone, time and timezone, date, date and timezone.
     All defaults are taken from the current date, time, and
     timezone.  In addition to 822-style dates, _p_i_c_k will also
     recognize any of the days of the week ("sunday", "monday",
     and so on), and the special dates "today", "yesterday", and
     "tomorrow".  All days of the week are judged to refer to a
     day in the past (e.g., telling _p_i_c_k "saturday" on a "tues-
     day" means "last saturday" not "this saturday").  Finally,
     in addition to these special specifications, _p_i_c_k will also
     honor a specification of the form "-dd", which means "dd
     days ago".

     _P_i_c_k supports complex boolean operations on the searching
     primitives with the `-and', `-or', `-not', and
     `-lbrace ... -rbrace' switches.  For example,

          pick -after yesterday -and -lbrace -from freida -or -from fear -rbrace

     identifies messages recently sent by "frieda" or "fear".

     The matching primitives take precedence over the `-not'
     switch, which in turn takes precedence over `-and' which in
     turn takes precedence over `-or'.  To override the default
     precedence, the `-lbrace' and `-rbrace' switches are pro-
     vided, which act just like opening and closing parentheses
     in logical expressions.

     Once the search has been performed, if the `-list' switch is
     given, the message numbers of the selected messages are
     written to the standard output separated by newlines.  This
     is _e_x_t_r_e_m_e_l_y useful for quickly generating arguments for
     other _M_H programs by using the "backquoting" syntax of the
     shell.  For example, the command

          scan `pick +todo -after "31 Mar 83 0123 PST"`



Printed 1/10/87                MH                               2






PICK(1)             UNIX Programmer's Manual              PICK(1)



     says to _s_c_a_n those messages in the indicated folder which
     meet the appropriate criterion.  Note that since _p_i_c_k 's
     context changes are written out prior to _s_c_a_n 's invocation,
     you need not give the folder argument to _s_c_a_n as well.

     Regardless of the operation of the `-list' switch, the
     `-sequence name' switch may be given once for each sequence
     the user wishes to define.  For each sequence named, that
     sequence will be defined to mean exactly those messages
     selected by _p_i_c_k.  For example,

          pick -from frated -seq fred

     defines a new message sequence for the current folder called
     "fred" which contains exactly those messages that were
     selected.

     Note that whenever _p_i_c_k processes a `-sequence name' switch,
     it sets `-nolist'.

     By default, _p_i_c_k will zero the sequence before adding it.
     This action can be disabled with the `-nozero' switch, which
     means that the messages selected by _p_i_c_k will be added to
     the sequence, if it already exists, and any messages already
     a part of that sequence will remain so.

     The `-public' and `-nopublic' switches are used by _p_i_c_k in
     the same way _m_a_r_k uses them.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Current-Folder:      To find the default current folder

SEE ALSO
     mark(1)

DEFAULTS
     `+folder' defaults to the current folder
     `msgs' defaults to all
     `-datefield date'
     `-nopublic' if the folder is read-only, `-public' otherwise
     `-zero'
     `-list' is the default if no `-sequence', `-nolist' otherwise

CONTEXT
     If a folder is given, it will become the current folder.

HISTORY
     In previous versions of _M_H, the _p_i_c_k command would _s_h_o_w,



Printed 1/10/87                MH                               3






PICK(1)             UNIX Programmer's Manual              PICK(1)



     _s_c_a_n, or _r_e_f_i_l_e the selected messages.  This was rather
     "inverted logic" from the UNIX point of view, so _p_i_c_k was
     changed to define sequences and output those sequences.
     Hence, _p_i_c_k can be used to generate the arguments for all
     other _M_H commands, instead of giving _p_i_c_k endless switches
     for invoking those commands itself.

     Also, previous versions of _p_i_c_k balked if you didn't specify
     a search string or a date/time constraint.  The current ver-
     sion does not, and merely matches the messages you specify.
     This lets you type something like:

          show `pick last:20 -seq fear`

     instead of typing

          mark -add -nozero -seq fear last:20
          show fear

     Finally, timezones used to be ignored when comparing dates:
     they aren't any more.

BUGS
     The argument to the `-after' and `-before' switches must be
     interpreted as a single token by the shell that invokes
     _p_i_c_k.  Therefore, one must usually place the argument to
     this switch inside double-quotes.  Furthermore, any occu-
     rance of `-datefield' must occur prior to the `-after' or
     `-before' switch it applies to.

     If _p_i_c_k is used in a back-quoted operation, such as

          scan `pick -from jones`

     and _p_i_c_k fails (e.g., no messages are from "jones"), then
     the shell will still run the outer command (e.g., "scan").
     Since no messages were matched, _p_i_c_k produced no output, and
     the argument given to the outer command as a result of
     backquoting _p_i_c_k is empty.  In the case of _M_H programs, the
     outer command now acts as if the default `msg' or `msgs'
     should be used (e.g., "all" in the case of _s_c_a_n ).  To
     prevent this unexpected behavior, if `-list' was given, and
     if its standard output is not a tty, then _p_i_c_k outputs the
     illegal message number "0" when it fails.  This lets the
     outer command fail gracefully as well.










Printed 1/10/87                MH                               4



