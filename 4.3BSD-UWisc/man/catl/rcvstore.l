


RCVSTORE(1)         UNIX Programmer's Manual          RCVSTORE(1)



NAME
     rcvstore - incorporate new mail asynchronously

SYNOPSIS
     /usr/misc/mh/lib/rcvstore [+folder] [-create] [-nocreate]
          [-sequence name ...] [-public] [-nopublic] [-zero]
          [-nozero] [-help]
9     _R_c_v_s_t_o_r_e incorporates a message from the standard input into
     an _M_H folder.  If `+folder' isn't specified, the folder
     named "inbox" in the user's _M_H directory will be used
     instead.  The new message being incorporated is assigned the
     next highest number in the folder.  If the specified (or
     default) folder doesn't exist, then it will be created if
     the `-create' option is specified, otherwise _r_c_v_s_t_o_r_e will
     exit.

     If the user's profile contains a "Msg-Protect: nnn" entry,
     it will be used as the protection on the newly created mes-
     sages, otherwise the _M_H default of 0644 will be used.  Dur-
     ing all operations on messages, this initially assigned pro-
     tection will be preserved for each message, so _c_h_m_o_d(1) may
     be used to set a protection on an individual message, and
     its protection will be preserved thereafter.

     _R_c_v_s_t_o_r_e will incorporate anything except zero length mes-
     sages into the user's MH folder.

     If the profile entry "Unseen-Sequence" is present and
     non-empty, then _r_c_v_s_t_o_r_e will add the newly incorporated
     message to each sequence named by the profile entry.  This
     is similar to the "Previous-Sequence" profile entry sup-
     ported by all _M_H commands which take `msgs' or `msg' argu-
     ments.  Note that _r_c_v_s_t_o_r_e will not zero each sequence prior
     to adding messages.

     Furthermore, the incoming messages may be added to user-
     defined sequences as they arrive by appropriate use of the
     `-sequence' option.  As with _p_i_c_k, use of the `-zero' and
     `-nozero' switches can also be used to zero old sequences or
     not.  Similarly, use of the `-public' and `-nopublic
     switches may be used to force additions to public and
     private sequences.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Folder-Protect:      To set mode when creating a new folder
     Msg-Protect:         To set mode when creating a new message
     Unseen-Sequence:     To name sequences denoting unseen messages
9


Printed 1/10/87                MH                               1






RCVSTORE(1)         UNIX Programmer's Manual          RCVSTORE(1)



SEE ALSO
     inc(1), pick(1), mh-mail(5)

DEFAULTS
     `+folder' defaults to "inbox"
     `-create'
     `-nopublic' if the folder is read-only, `-public' otherwise
     `-nozero'

CONTEXT
     No context changes will be attempted, with the exception of
     sequence manipulation.











































Printed 1/10/87                MH                               2



