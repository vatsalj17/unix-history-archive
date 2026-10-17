


MSGCHK(1)           UNIX Programmer's Manual            MSGCHK(1)



NAME
     msgchk - check for messages

SYNOPSIS
     msgchk [users ...] [-host host] [-user user] [-rpop]
          [-norpop] [-help]
9     The _m_s_g_c_h_k program checks all known mail drops for mail
     waiting for you to receive.  For those drops which have mail
     for you, _m_s_g_c_h_k will indicate if it believes that you have
     seen the mail in question before.

     If the local host is configured as a POP client, or if the
     `-host host' switch is given, _m_s_g_c_h_k will query the POP ser-
     vice host as to the status of mail waiting.  The
     `-user user' switch may be given to specify the name of the
     POP subscriber you wish to check mail for on the POP service
     host.  The `-rpop' switch uses the UNIX _r_P_O_P (authentication
     done via trusted connections).  In contrast, the `-norpop'
     switch uses the ARPA _P_O_P (in which case _m_s_g_c_h_k will prompt
     for a password).

FILES
     $HOME/.mh_profile                  The user profile
     /usr/misc/mh/lib/mtstailor         tailor file
     /usr/spool/mail/$USER              Location of mail drop

PROFILE COMPONENTS
     None

SEE ALSO
     _P_o_s_t _O_f_f_i_c_e _P_r_o_t_o_c_o_l (_r_e_v_i_s_e_d) (aka RFC-819 with revisions),
     inc(1)

DEFAULTS
     `user' defaults to the current user
     `-rpop'

CONTEXT
     None












9


Printed 1/10/87                MH                               1



