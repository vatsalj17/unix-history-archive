


SHOW(1)             UNIX Programmer's Manual              SHOW(1)



NAME
     show - show (list) messages

SYNOPSIS
     show [+folder] [msgs] [-draft] [-header] [-noheader]
          [-showproc program] [-noshowproc]
          [switches for _s_h_o_w_p_r_o_c] [-help]
9     _S_h_o_w lists each of the specified messages to the standard
     output (typically, the terminal).  Typically, the messages
     are listed exactly as they are, with no reformatting.  A
     program named by the _s_h_o_w_p_r_o_c profile component is invoked
     to do the listing, and any switches not recognized by _s_h_o_w
     are passed along to that program.  The default program is
     known as _m_o_r_e (1).  To override the default and the _s_h_o_w_p_r_o_c
     profile component, use the `-showproc program' switch.  For
     example, `-show pr' will cause the _p_r (1) program to list
     the messages.  The _M_H command _m_h_l can be used as a _s_h_o_w_p_r_o_c
     to show messages in a more uniform format.  Normally, this
     program is specified as the _s_h_o_w_p_r_o_c is the user's
     .mh_profile.  See _m_h_l (1) for the details.  If the
     `-noshowproc' option is specified, `/bin/cat' is used
     instead of _s_h_o_w_p_r_o_c.

     The `-header' switch tells _s_h_o_w to display a one-line
     description of the message being shown.  This description
     includes the folder and the message number.

     If no `msgs' are specified, the current message is used.  If
     more than one message is specified, _m_o_r_e will prompt for a
     <RETURN> prior to listing each message.  _m_o_r_e will list each
     message, a page at a time.  When the end of page is reached,
     _m_o_r_e will ring the bell and wait for a <SPACE> or <RETURN>.
     If a <RETURN> is entered, _m_o_r_e will print the next line,
     whereas <SPACE> will print the next screenful.  To exit
     _m_o_r_e, type "q".

     If the standard output is not a terminal, no queries are
     made, and each file is listed with a one-line header and two
     lines of separation.

     "show -draft" will list the file <mh-dir>/draft if it
     exists.

     If the profile entry "Unseen-Sequence" is present and
     non-empty, then _s_h_o_w will remove each of the messages shown
     from each sequence named by the profile entry.  This is
     similar to the "Previous-Sequence" profile entry supported
     by all _M_H commands which take `msgs' or `msg' arguments.

FILES
     $HOME/.mh_profile                  The user profile
9


Printed 1/10/87                MH                               1






SHOW(1)             UNIX Programmer's Manual              SHOW(1)



PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Current-Folder:      To find the default current folder
     Unseen-Sequence:     To name sequences denoting unseen messages
     showproc:            Program to show messages

SEE ALSO
     mhl(1), more(1), next(1), pick(1), prev(1), scan(1)

DEFAULTS
     `+folder' defaults to the current folder
     `msgs' defaults to cur
     `-format'
     `-header'

CONTEXT
     If a folder is given, it will become the current folder.
     The last message shown will become the current message.

BUGS
     The `-header' switch doesn't work when `msgs' expands to
     more than one message.  If the _s_h_o_w_p_r_o_c is _m_h_l, then is
     problem can be circumvented by referencing the "messagename"
     field in the _m_h_l format file.

     _S_h_o_w updates the user's context before showing the message.
     Hence if _s_h_o_w will mark messages as seen prior to the user
     actually seeing them.  This is generally not a problem,
     unless the user relies on the "unseen" messages mechanism,
     and interrupts _s_h_o_w while it is showing "unseen" messages.

     If _s_h_o_w_p_r_o_c is _m_h_l, then _s_h_o_w uses a built-in _m_h_l: it does
     not actually run the _m_h_l program.  Hence, if you define your
     own _s_h_o_w_p_r_o_c, don't call it _m_h_l since _s_h_o_w won't run it.

     If _m_o_r_e (1) is your showproc (the default), then avoid run-
     ning _s_h_o_w in the background with only its standard output
     piped to another process, as in

          show | imprint &

     Due to a bug in _m_o_r_e, show will go into a "tty input" state.
     To avoid this problem, re-direct _s_h_o_w's diagnostic output as
     well.  For users of _c_s_h:

          show |& imprint &

     For users of _s_h:

          show 2>&1 | imprint &





Printed 1/10/87                MH                               2



