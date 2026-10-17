


DP(8)               UNIX Programmer's Manual                DP(8)



NAME
     dp - parse dates 822-style

SYNOPSIS
     /usr/misc/mh/lib/dp [-form formatfile] [-format string]
          [-width columns] dates ...  [-help]
9     _D_p is a program that parses dates according to the ARPA
     Internet standard.  It also understands many non-standard
     formats, such as those produced by TOPS-20 sites and some
     UNIX sites using _c_t_i_m_e (3).  It is useful for seeing how _M_H
     will interpret a date.

     The _d_p program treats each argument as a single date, and
     prints the date out in the official 822-format.  Hence, it
     is usually best to enclose each argument in double-quotes
     for the shell.

     To override the output format used by _d_p, the
     `-format string' or `-format file' switches are used.  This
     permits individual fields of the address to be extracted
     with ease.  The string is simply a format stringand thefile
     is simply a format file.  See _m_h-_f_o_r_m_a_t (5) for the details.

     Here is the default format string used by _d_p:

          %<(nodate{text})error:
     %{text}%|%(putstr(pretty{text}))%>

     which says that if an error was detected, print the error, a
     `:', and the date in error.  Otherwise, output the
     822-proper format of the date.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     None

SEE ALSO
     ap(8)
     _S_t_a_n_d_a_r_d _f_o_r _t_h_e _F_o_r_m_a_t _o_f _A_R_P_A _I_n_t_e_r_n_e_t _T_e_x_t _M_e_s_s_a_g_e_s (aka
     RFC-822)

DEFAULTS
     `-format' default as described above
     `-width' default to the width of the terminal

CONTEXT
     None


9


Printed 1/10/87                MH                               1






DP(8)               UNIX Programmer's Manual                DP(8)



BUGS
     The argument to the `-format' switch must be interpreted as
     a single token by the shell that invokes _d_p.  Therefore, one
     must usually place the argument to this switch inside
     double-quotes.


















































Printed 1/10/87                MH                               2



