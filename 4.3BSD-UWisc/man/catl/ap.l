


AP(8)               UNIX Programmer's Manual                AP(8)



NAME
     ap - parse addresses 822-style

SYNOPSIS
     /usr/misc/mh/lib/ap [-form formatfile] [-format string]
          [-normalize] [-nonormalize] [-width columns] addrs ...
          [-help]
9     _A_p is a program that parses addresses according to the ARPA
     Internet standard.  It also understands many non-standard
     formats.  It is useful for seeing how _M_H will interpret an
     address.

     The _a_p program treats each argument as one or more
     addresses, and prints those addresses out in the official
     822-format.  Hence, it is usually best to enclose each argu-
     ment in double-quotes for the shell.

     To override the output format used by _a_p, the
     `-format string' or `-format file' switches are used.  This
     permits individual fields of the address to be extracted
     with ease.  The string is simply a format stringand thefile
     is simply a format file.  See _m_h-_f_o_r_m_a_t (5) for the details.

     In addition to the standard escapes, _s_c_a_n also recognizes
     the following additional escape:
     _e_s_c_a_p_e  _s_u_b_s_t_i_t_u_t_i_o_n
     error   a diagnostic if the parse failed

     If the `-normalize' switch is given, _a_p will try to track
     down the official hostname of the address.

     Here is the default format string used by _a_p:

          %<{error}%{error}: %{text}%|%(putstr(proper{text}))%>

     which says that if an error was detected, print the error, a
     `:', and the address in error.  Otherwise, output the
     822-proper format of the address.

FILES
     $HOME/.mh_profile                  The user profile
     /usr/misc/mh/lib/mtstailor         tailor file

PROFILE COMPONENTS
     None

SEE ALSO
     dp(8),
     _S_t_a_n_d_a_r_d _f_o_r _t_h_e _F_o_r_m_a_t _o_f _A_R_P_A _I_n_t_e_r_n_e_t _T_e_x_t _M_e_s_s_a_g_e_s (aka
     RFC-822)

9


Printed 1/10/87                MH                               1






AP(8)               UNIX Programmer's Manual                AP(8)



DEFAULTS
     `-format' defaults as described above
     `-normalize'
     `-width' defaults to the width of the terminal

CONTEXT
     None

BUGS
     The argument to the `-format' switch must be interpreted as
     a single token by the shell that invokes _a_p.  Therefore, one
     must usually place the argument to this switch inside
     double-quotes.

     On hosts where _M_H was configured with the BERK option,
     address parsing is not enabled.







































Printed 1/10/87                MH                               2



