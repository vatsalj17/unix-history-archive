


SCAN(1)             UNIX Programmer's Manual              SCAN(1)



NAME
     scan - produce a one line per message scan listing

SYNOPSIS
     scan [+folder] [msgs] [-clear] [-noclear] [-form formatfile]
          [-format string] [-header] [-noheader] [-width columns]
          [-help]
9     _S_c_a_n produces a one-line-per-message listing of the speci-
     fied messages.  Each _s_c_a_n line contains the message number
     (name), the date, the "From:" field, the "Subject" field,
     and, if room allows, some of the body of the message.  For
     example:

          15+  7/ 5  Dcrocker  nned  Last week I asked some of
          16 - 7/ 5  dcrocker  message id format  I recommend
          18   7/ 6  Obrien    Re: Exit status from mkdir
          19   7/ 7  Obrien    "scan" listing format in MH

     The `+' on message 15 indicates that it is the current mes-
     sage.  The `-' on message 16 indicates that it has been
     replied to, as indicated by a "Replied:" component produced
     by an `-annotate' switch to the _r_e_p_l command.

     If there is sufficient room left on the _s_c_a_n line after the
     subject, the line will be filled with text from the body,
     preceded by <<, and terminated by >> if the body is suffi-
     ciently short.  _S_c_a_n actually reads each of the specified
     messages and parses them to extract the desired fields.
     During parsing, appropriate error messages will be produced
     if there are format errors in any of the messages.

     The `-header' switch produces a header line prior to the
     _s_c_a_n listing.  Currently, the name of the folder and the
     current date and time are output (see the HISTORY section
     for more information).

     If the `-clear' switch is used and _s_c_a_n'_s output is directed
     to a terminal, then _s_c_a_n will consult the $TERM and $TERMCAP
     envariables to determine your terminal type in order to find
     out how to clear the screen prior to exiting.  If the
     `-clear' switch is used and _s_c_a_n'_s output is not directed to
     a terminal (e.g., a pipe or a file), then _s_c_a_n will send a
     formfeed prior to exiting.

     For example, the command:

          (scan -clear -header; show all -show pr -f) | lpr

     produces a scan listing of the current folder, followed by a
     formfeed, followed by a formatted listing of all messages in
     the folder, one per page.  Omitting `-show pr -f' will cause
     the messages to be concatenated, separated by a one-line



Printed 1/10/87                MH                               1


9


SCAN(1)             UNIX Programmer's Manual              SCAN(1)



     header and two blank lines.

     If _s_c_a_n encounters a message without a "Date:" field, rather
     than leaving that portion of the scan listing blank, the
     date is filled-in with the last write date of the message,
     and post-fixed with a `*'.  This is particularly handy for
     scanning a _d_r_a_f_t _f_o_l_d_e_r, as message drafts usually aren't
     allowed to have dates in them.

     To override the output format used by _s_c_a_n, the
     `-format string' or `-format file' switches are used.  This
     permits individual fields of the scan listing to be
     extracted with ease.  The string is simply a format string
     and the file is simply a format file.  See _m_h-_f_o_r_m_a_t (5) for
     the details.

     In addition to the standard escapes, _s_c_a_n also recognizes
     the following additional escape:
     _e_s_c_a_p_e  _s_u_b_s_t_i_t_u_t_i_o_n
     body    the (compressed) first part of the body

     On hosts where _M_H was configured with the BERK option, _s_c_a_n
     has two other switches: `-reverse', and `-noreverse'.  These
     make _s_c_a_n list the messages in reverse order.  In addition,
     _s_c_a_n will update the _M_H context prior to starting the list-
     ing, so interrupting a long _s_c_a_n listing preserves the new
     context.  _M_H purists hate both of these ideas.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Alternate-Mailboxes: To determine the user's mailboxes
     Current-Folder:      To find the default current folder

SEE ALSO
     inc(1), pick(1), show(1), mh-format(5)

DEFAULTS
     `+folder' defaults to the folder current
     `msgs' defaults to all
     `-format' defaulted as described above
     `-noheader'
     `-width' defaulted to the width of the terminal

CONTEXT
     If a folder is given, it will become the current folder.

HISTORY
     Prior to using the format string mechanism, `-header' used
     to generate a heading saying what each column in the listing



Printed 1/10/87                MH                               2






SCAN(1)             UNIX Programmer's Manual              SCAN(1)



     was.  Format strings prevent this from happening.

BUGS
     The argument to the `-format' switch must be interpreted as
     a single token by the shell that invokes _s_c_a_n.  Therefore,
     one must usually place the argument to this switch inside
     double-quotes.
















































Printed 1/10/87                MH                               3



