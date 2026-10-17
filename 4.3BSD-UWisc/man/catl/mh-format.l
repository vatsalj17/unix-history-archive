


MH-FORMAT(5)        UNIX Programmer's Manual         MH-FORMAT(5)



NAME
     mh-format - format file for MH message system

SYNOPSIS
     some _M_H commands
9     Several _M_H commands utilize either a _f_o_r_m_a_t string or a _f_o_r_-
     _m_a_t file during their execution.  For example, _s_c_a_n (1) uses
     a format string which directs it how to generate the scan
     listing for each message; _r_e_p_l (1) uses a format file which
     directs it how to generate the reply to a message, and so
     on.

     Format strings are designed to be efficiently parsed by _M_H
     since they represent an integral part of _M_H.  This means
     that novice, casual, or even advanced users of _M_H should
     deal with them.  It suffices to have your local _M_H expert
     actually write new format commands or modify existing ones.
     This manual section explains how to do just that.

     A format string is similar to a _p_r_i_n_t_f (3) string, but uses
     multi-letter `%'-escapes.  When specifying a string, the
     usual C backslash characters are honored: `\b', `\f', `\n',
     `\r', and `\t'.  Continuation lines in format files end with
     `\' followed by the newline character.

     The interpretation model is based on a simple machine with
     two registers, _n_u_m and _s_t_r.  The former contains an integer
     value, the latter a string value.  When an escape is pro-
     cessed, if it requires an argument, it reads the current
     value of either _n_u_m or _s_t_r; and, if it returns a value, it
     writes either _n_u_m or _s_t_r.

     Escapes are of three types: _c_o_m_p_o_n_e_n_t_s, _f_u_n_c_t_i_o_n_s, and, _c_o_n_-
     _t_r_o_l.  A component escape is specified as `%{name}', and is
     created for each header found in the message being pro-
     cessed.  For example `%{date} refers to the "Date:" field of
     the appropriate message.  A component escape is always
     string valued.

     A control escape is one of: `%<escape', `%|', and `%>',
     which correspond to if-then-else constructs: if `escape' is
     non-zero (for integer-valued escapes), or non-empty (for
     string-valued escapes), then everything up to `%|' or `%>'
     (whichever comes first) is interpreted; otherwise, then skip
     to `%|' or `%>' (whichever comes first) and start interpret-
     ing again.

     A function escape is specified as `%(name)', and is stati-
     cally defined.  Here is the list:
     _e_s_c_a_p_e      _a_r_g_u_m_e_n_t  _r_e_t_u_r_n_s  _i_n_t_e_r_p_r_e_t_a_t_i_o_n
     nonzero     integer   integer  _n_u_m is non-zero
     zero        integer   integer  _n_u_m is zero



Printed 1/10/87                MH                               1


9


MH-FORMAT(5)        UNIX Programmer's Manual         MH-FORMAT(5)



     eq          integer   integer  _n_u_m == width
     ne          integer   integer  _n_u_m != width
     gt          integer   integer  width > _n_u_m
     null        string    integer  _s_t_r is empty
     nonnull     string    integer  _s_t_r is non-empty
     putstr      string             print _s_t_r
     putstrf     string             print _s_t_r in the specified width
                                    (e.g., %20(putstrf{subject})
     putnum      integer            print _n_u_m
     putnumf     integer            print _n_u_m in the specified width
                                    (e.g., %4(putnumf(msg))
     msg                   integer  message number
     cur                   integer  message is current
     size                  integer  size of message
     strlen      string    integer  length of _s_t_r
     me                    string   the user's mailbox
     plus                  integer  add width to _n_u_m
     minus                 integer  subtract _n_u_m from width
     charleft              integer  space left in output buffer
     timenow               integer  seconds since the UNIX epoch

     When _s_t_r is a date, these escapes are useful:
     _e_s_c_a_p_e      _a_r_g_u_m_e_n_t  _r_e_t_u_r_n_s  _i_n_t_e_r_p_r_e_t_a_t_i_o_n
     sec         string    integer  seconds of the minute
     min         string    integer  minutes of the day
     hour        string    integer  hours of the day (24 hour clock)
     mday        string    integer  day of the month
     mon         string    integer  month of the year
     wday        string    integer  day of the week (Sunday=0)
     year        string    integer  year of the century
     yday        string    integer  day of the year
     dst         string    integer  daylight savings in effect
     zone        string    integer  timezone
     sday        string    integer  day of the week known
                                    1 for explicit in date
                                    0 for implicit (_M_H figured it out)
                                    -1 for unknown (_M_H couldn't figure it out)
     clock       string    integer  seconds since the UNIX epoch
     rclock      string    integer  seconds prior to current time
     month       string    string   month of the year
     lmonth      string    string   month of the year (long form)
     tzone       string    string   timezone
     day         string    string   day of the week
     weekday     string    string   day of the week (long)
     tws         string    string   official 822 rendering of the date
     pretty      string    string   a more user-friendly rendering
     nodate      string             date wasn't parseable

     When _s_t_r is an address, these escapes are useful:
     _e_s_c_a_p_e      _a_r_g_u_m_e_n_t  _r_e_t_u_r_n_s  _i_n_t_e_r_p_r_e_t_a_t_i_o_n
     pers        string    string   the personal name of the address
     mbox        string    string   the local part of the address



Printed 1/10/87                MH                               2






MH-FORMAT(5)        UNIX Programmer's Manual         MH-FORMAT(5)



     host        string    string   the domain part of the address
     path        string    string   the route part of the address
     type        string    integer  the type of host
                                    -1 for uucp
                                    0 for local
                                    1 for network
                                    2 for unknown
     nohost      string    integer  no host was present in the address
     ingrp       string    integer  the address appeared inside a group
     gname       string    string   name of the group (present for first
                                    address only)
     note        string    string   commentary text
     proper      string    string   official 822 rendering of the address
     friendly    string    string   a more user-friendly rendering
     mymbox      string             the address refers to the user's mailbox
     formataddr  string             print _s_t_r in an address list

     With all this in mind, here's the default format string for
     _s_c_a_n.  It's been divided into several pieces for readabil-
     ity.  The first part is:

          %4(putnumf(msg))%<(cur)+%| %>%<{replied}-%| %>

     which says that the message number should be printed in four
     digits, if the message is the current message then a `+'
     else a space should be printed, and if a "Replied:" field is
     present then a `-' else a space should be printed.  Next:

          %02(putnumf(mon{date}))/%02(putnumf(mday{date}))

     the hours and minutes are printed in two digits (zero
     filled).  Next,

          %<{date} %|*>

     If no "Date:" field was present, then a `*' is printed, oth-
     erwise a space.  Next,

          %<(mymbox{from})To:%14(putstrf(friendly{to}))

     if the message is from me, print `To:' followed by a
     "user-friendly" rendering of the first address in the "To:"
     field.  Continuing,

          %|%17(putstrf(friendly{from}))%>

     if the message isn't from me, then the print the "From:"
     address is printed.  And finally,

          %{subject}%<{body}<<%{body}%>

     the subject and initial body are printed.



Printed 1/10/87                MH                               3






MH-FORMAT(5)        UNIX Programmer's Manual         MH-FORMAT(5)



     Although this seems complicated, in point of fact, this
     method is flexible enough to extract individual fields and
     print them in any format the user desires.

     If the `-form formatfile' switch is given, _s_c_a_n will treat
     each line in the named file as a format string and act
     accordingly.  This lets the user employ canned scan listing
     formats.  Take a look at the three files
     /usr/misc/mh/lib/scan.time, /usr/misc/mh/lib/scan.size, and
     /usr/misc/mh/lib/scan.timely.

FILES
     None

PROFILE COMPONENTS
     None

SEE ALSO
     ap(8), dp(8)

DEFAULTS
     None

CONTEXT
     None

BUGS
     On hosts where _M_H was configured with the BERK option,
     address parsing is not enabled.


























Printed 1/10/87                MH                               4



