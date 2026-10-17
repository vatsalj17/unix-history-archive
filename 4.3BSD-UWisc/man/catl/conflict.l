


CONFLICT(8)         UNIX Programmer's Manual          CONFLICT(8)



NAME
     conflict - search for alias/password conflicts

SYNOPSIS
     /usr/misc/mh/lib/conflict [-mail name] [-search directory]
          [aliasfiles...] [-help]
9     _C_o_n_f_l_i_c_t is a program that checks to see if the interface
     between _M_H and transport system is in good shape

     _C_o_n_f_l_i_c_t also checks for maildrops in /usr/spool/mail which
     do not belong to a valid user.  It assumes that no user name
     will start with `.', and thus ignores files in
     /usr/spool/mail which begin with `.'.  It also checks for
     entries in the _g_r_o_u_p (5) file which do not belong to a valid
     user, and for users who do not have a valid group number.
     In addition duplicate users and groups are noted.

     If the `-mail name' switch is used, then the results will be
     sent to the specified _n_a_m_e.  Otherwise, the results are sent
     to the standard output.

     The `-search directory' switch can be used to search direc-
     tories other than /usr/spool/mail and to report anomalies in
     those directories.  The `-search directory' switch can
     appear more than one time in an invocation to _c_o_n_f_l_i_c_t.

     _C_o_n_f_l_i_c_t should be run under  _c_r_o_n (8), or whenever system
     accounting takes place.

FILES
     /usr/misc/mh/lib/mtstailor         tailor file
     /etc/passwd                        List of users
     /etc/group                         List of groups
     /usr/misc/mh/mhmail                Program to send mail
     /usr/spool/mail/                   Directory of mail drop

PROFILE COMPONENTS
     None

SEE ALSO
     mh-alias(5)

DEFAULTS
     `aliasfiles' defaults to /usr/misc/mh/lib/MailAliases

CONTEXT
     None




9


Printed 1/10/87                MH                               1



