


WHATNOW(1)          UNIX Programmer's Manual           WHATNOW(1)



NAME
     whatnow - prompting front-end for send

SYNOPSIS
     whatnow [-draftfolder +folder] [-draftmessage msg]
          [-nodraftfolder] [-editor editor] [-noedit]
          [-prompt string] [file] [-help]
9     _W_h_a_t_n_o_w is the default program that queries the user about
     the disposition of a composed draft.  It is normally invoked
     by one of _c_o_m_p, _d_i_s_t, _f_o_r_w, or _r_e_p_l after the initial edit.

     When started, the editor is started on the draft (unless
     `-noedit' is given, in which case the initial edit is
     suppressed).  Then, _w_h_a_t_n_o_w repetitively prompts the user
     with "What now?" and awaits a response.  The valid responses
     are
     display         to list the message being distributed/replied-to on
                     the terminal
     edit            to re-edit using the same editor that was used on the
                     preceding round unless a profile entry
                     "<lasteditor>-next: <editor>" names an alternate editor
     edit <editor>   to invoke <editor> for further editing
     list            to list the draft on the terminal
     push            to send the message in the background
     quit            to terminate the session and preserve the draft
     quit -delete    to terminate, then delete the draft
     refile +folder  to refile the draft into the given folder
     send            to send the message
     send -watch     to cause the delivery process to be monitored
     whom            to list the addresses that the message will go to
     whom -check     to list the addresses and verify that they are
                     acceptable to the transport service

     For the edit response, any valid switch to the editor is
     valid.  Similarly, for the send and whom responses, any
     valid switch to _s_e_n_d (1) and _w_h_o_m (1) commands, respec-
     tively, are valid.  For the push response, any valid switch
     to _s_e_n_d (1) is valid (as this merely invokes _s_e_n_d with the
     `-push' option).  For the _r_e_f_i_l_e response, any valid switch
     to the _f_i_l_e_p_r_o_c is valid.  For the display and list
     responses, any valid argument to the _l_p_r_o_c is valid.  If any
     non-switch arguments are present, then the pathname of the
     draft will be excluded from the argument list given to the
     _l_p_r_o_c (this is useful for listing another _M_H message).

     See _m_h-_p_r_o_f_i_l_e (5) for further information about how editors
     are used by MH.  It also discusses how complex envariables
     can be used to direct _w_h_a_t_n_o_w's actions.

     The `-prompt string' switch sets the prompting string for
     _w_h_a_t_n_o_w.
9


Printed 1/10/87                MH                               1






WHATNOW(1)          UNIX Programmer's Manual           WHATNOW(1)



     The `-draftfolder +folder' and `-draftmessage msg' switches
     invoke the _M_H draft folder facility.  This is an advanced
     (and highly useful) feature.  Consult the Advanced Features
     section of the _M_H manual for more information.

FILES
     $HOME/.mh_profile                  The user profile
     <mh-dir>/draft                     The draft file

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Draft-Folder:        To find the default draft-folder
     Editor:              To override the default editor
     <lasteditor>-next:   To name an editor to be used after exit from <lasteditor>
     fileproc:            Program to refile the message
     lproc:               Program to list the contents of a message
     sendproc:            Program to use to send the message
     whomproc:            Program to determine who a message would go to

SEE ALSO
     send(1), whom(1)

DEFAULTS
     `-prompt "What Now? "'

CONTEXT
     None

BUGS
     The argument to the `-prompt' switch must be interpreted as
     a single token by the shell that invokes _w_h_a_t_n_o_w.  There-
     fore, one must usually place the argument to this switch
     inside double-quotes.

     If _s_e_n_d_p_r_o_c is _s_e_n_d, then _w_h_a_t_n_o_w uses a built-in _s_e_n_d, it
     does not actually run the _s_e_n_d program.  Hence, if you
     define your own _s_e_n_d_p_r_o_c, don't call it _s_e_n_d since _w_h_a_t_n_o_w
     won't run it.

















Printed 1/10/87                MH                               2



