


COMP(1)             UNIX Programmer's Manual              COMP(1)



NAME
     comp - compose a message

SYNOPSIS
     comp [+folder] [msg] [-draftfolder +folder]
          [-draftmessage msg] [-nodraftfolder] [-editor editor]
          [-noedit] [-file file] [-form formfile] [-use] [-nouse]
          [-whatnowproc program] [-nowhatnowproc] [-help]
9     _C_o_m_p is used to create a new message to be mailed.  It
     copies a message form to the draft being composed and then
     invokes an editor on the draft (unless `-noedit' is given,
     in which case the initial edit is suppressed).

     The default message form contains the following elements:

          To:
          cc:
          Subject:
          --------

     If the file named "components" exists in the user's MH
     directory, it will be used instead of this form.  The file
     specified by `-form formfile' will be used if given.  You
     may also start _c_o_m_p using the contents of an existing mes-
     sage as the form.  If you supply either a `+folder' or `msg'
     argument, that message will be used as the form.  You may
     not supply both a `-form formfile' and a `+folder' or `msg'
     argument.  The line of dashes or a blank line must be left
     between the header and the body of the message for the mes-
     sage to be identified properly when it is sent (see
     _s_e_n_d (1)).  The switch `-use' directs _c_o_m_p to continue edit-
     ing an already started message.  That is, if a _c_o_m_p (or
     _d_i_s_t, _r_e_p_l, or _f_o_r_w ) is terminated without sending the
     draft, the draft can be edited again via "comp -use".

     If the draft already exists, _c_o_m_p will ask you as to the
     disposition of the draft.  A reply of quit will abort _c_o_m_p,
     leaving the draft intact; replace will replace the existing
     draft with the appropriate form; list will display the
     draft; use will use the draft for further composition; and
     refile +folder will file the draft in the given folder, and
     give you a new draft with the appropriate form.  (The
     `+folder' argument to refile is required.)

     The `-draftfolder +folder' and `-draftmessage msg' switches
     invoke the _M_H draft folder facility.  This is an advanced
     (and highly useful) feature.  Consult the Advanced Features
     section of the _M_H manual for more information.

     The `-file file' switch says to use the named file as the
     message draft.
9


Printed 1/10/87                MH                               1






COMP(1)             UNIX Programmer's Manual              COMP(1)



     Upon exiting from the editor, _c_o_m_p will invoke the _w_h_a_t_n_o_w
     program.  See _w_h_a_t_n_o_w (1) for a discussion of available
     options.  The invocation of this program can be inhibited by
     using the `-nowhatnowproc' switch.  (In truth of fact, it is
     the _w_h_a_t_n_o_w program which starts the initial edit.  Hence,
     `-nowhatnowproc' will prevent any edit from occurring.)

FILES
     /usr/misc/mh/lib/components        The message skeleton
     or <mh-dir>/components             Rather than the standard skeleton
     $HOME/.mh_profile                  The user profile
     <mh-dir>/draft                     The draft file

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Draft-Folder:        To find the default draft-folder
     Editor:              To override the default editor
     Msg-Protect:         To set mode when creating a new message (draft)
     fileproc:            Program to refile the message
     whatnowproc:         Program to ask the "What now?" questions

SEE ALSO
     dist(1), forw(1), repl(1), send(1), whatnow(1)

DEFAULTS
     `+folder' defaults to the current folder
     `msg' defaults to the current message
     `-nodraftfolder'
     `-nouse'

CONTEXT
     None

BUGS
     If _w_h_a_t_n_o_w_p_r_o_c is _w_h_a_t_n_o_w, then _c_o_m_p uses a built-in _w_h_a_t_-
     _n_o_w, it does not actually run the _w_h_a_t_n_o_w program.  Hence,
     if you define your own _w_h_a_t_n_o_w_p_r_o_c, don't call it _w_h_a_t_n_o_w
     since _c_o_m_p won't run it.

















Printed 1/10/87                MH                               2



