


POST(8)             UNIX Programmer's Manual              POST(8)



NAME
     post - deliver a message

SYNOPSIS
     /usr/misc/mh/lib/post [-alias aliasfile]
          [-filter filterfile] [-nofilter] [-format] [-noformat]
          [-msgid] [-nomsgid] [-verbose] [-noverbose] [-watch]
          [-nowatch] [-width columns] file [-help]
9     _P_o_s_t is the program called by _s_e_n_d (1) to deliver the mes-
     sage in _f_i_l_e to local and remote users.  In fact, all of the
     functions attributed to _s_e_n_d on its manual page are per-
     formed by _p_o_s_t, with _s_e_n_d acting as a relatively simple
     preprocessor.  Thus, it is _p_o_s_t which parses the various
     header fields, appends From: and Date: lines, and interacts
     with the _S_e_n_d_M_a_i_l transport system.  _P_o_s_t will not normally
     be called directly by the user.

     _P_o_s_t searches the "To:", "cc:", "Bcc:", "Fcc:", and
     "Resent-xxx:" header lines of the specified message for des-
     tination addresses, checks these addresses for validity, and
     formats them so as to conform to ARPAnet Internet Message
     Format protocol, unless the `-noformat' flag is set.  This
     will normally cause "@_l_o_c_a_l-_s_i_t_e" to be appended to each
     local destination address, as well as any local return
     addresses. The `-width columns' switch can be used to indi-
     cate the preferred length of the header components that con-
     tain addresses.

     If a "Bcc:" field is encountered, its addresses will be used
     for delivery, and the "Bcc:" field will be removed from the
     message sent to sighted recipients.  The blind recipients
     will receive an entirely new message with a minimal set of
     headers.  Included in the body of the message will be a copy
     of the message sent to the sighted recipients.  If
     `-filter filterfile' is specified, then this copy is fil-
     tered (re-formatted) prior to being sent to the blind reci-
     pients.

     The `-alias aliasfile' switch can be used to specify a file
     that post should take aliases from.  More than one file can
     be specified, each being preceded with `-alias'.  In any
     event, the primary alias file is read first.

     The `-msgid' switch indicates that a "Message-ID:" or
     "Resent-Message-ID:" field should be added to the header.

     The `-verbose' switch indicates that the user should be
     informed of each step of the posting/filing process.

     The `-watch' switch indicates that the user would like to
     watch the transport system's handling of the message (e.g.,
     local and "fast" delivery).



Printed 1/10/87                MH                               1


9


POST(8)             UNIX Programmer's Manual              POST(8)



     _P_o_s_t consults the envariable $SIGNATURE to determine the
     sender's personal name in constructing the "From:" line of
     the message.

FILES
     /usr/misc/mh/lib/mtstailor         tailor file
     /usr/misc/mh/refile                Program to process Fcc:s
     /usr/misc/mh/lib/mhl               Program to process Bcc:s
     /usr/misc/mh/lib/MailAliases       Primary alias file

PROFILE COMPONENTS
     _p_o_s_t does NOT consult the user's .mh_profile

SEE ALSO
     _S_t_a_n_d_a_r_d _f_o_r _t_h_e _F_o_r_m_a_t _o_f _A_R_P_A _I_n_t_e_r_n_e_t _T_e_x_t _M_e_s_s_a_g_e_s (aka
     RFC-822),
     mhmail(1), send(1), mh-mail(5), mh-alias(5)

DEFAULTS
     `-alias /usr/misc/mh/lib/MailAliases'
     `-format'
     `-nomsgid'
     `-noverbose'
     `-width 72'
     `-nofilter'

CONTEXT
     None

BUGS
     "Reply-To:" fields are allowed to have groups in them
     according to the 822 specification, but _p_o_s_t won't let you
     use them.






















Printed 1/10/87                MH                               2



