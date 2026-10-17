


DIST(1)             UNIX Programmer's Manual              DIST(1)



NAME
     dist - redistribute a message to additional addresses

SYNOPSIS
     dist [+folder] [msg] [-annotate] [-noannotate]
          [-draftfolder +folder] [-draftmessage msg]
          [-nodraftfolder] [-editor editor] [-noedit]
          [-form formfile] [-inplace] [-noinplace]
          [-whatnowproc program] [-nowhatnowproc] [-help]
9     _D_i_s_t is similar to _f_o_r_w.  It prepares the specified message
     for redistribution to addresses that (presumably) are not on
     the original address list.

     The default message form contains the following elements:

          Resent-To:
          Resent-cc:

     If the file named "distcomps" exists in the user's MH direc-
     tory, it will be used instead of this form.  In either case,
     the file specified by `-form formfile' will be used if
     given.  The form used will be prepended to the message being
     resent.

     If the draft already exists, _d_i_s_t will ask you as to the
     disposition of the draft.  A reply of quit will abort _d_i_s_t,
     leaving the draft intact; replace will replace the existing
     draft with a blank skeleton; and list will display the
     draft.

     Only those addresses in "Resent-To:", "Resent-cc:", and
     "Resent-Bcc:" will be sent.  Also, a "Resent-Fcc: folder"
     will be honored (see _s_e_n_d (1)).  Note that with _d_i_s_t, the
     draft should contain only "Resent-xxx:" fields and no body.
     The headers and the body of the original message are copied
     to the draft when the message is sent.  Use care in con-
     structing the headers for the redistribution.

     If the `-annotate' switch is given, the  message being dis-
     tributed will be annotated with the lines:

          Resent: date
          Resent: addrs

     where each address list contains as many lines as required.
     This annotation will be done only if the message is sent
     directly from _d_i_s_t.  If the message is not sent immediately
     from _d_i_s_t, "comp -use" may be used to re-edit and send the
     constructed message, but the annotations won't take place.
     The '-inplace' switch causes annotation to be done in place
     in order to preserve links to the annotated message.
9


Printed 1/10/87                MH                               1






DIST(1)             UNIX Programmer's Manual              DIST(1)



     See _c_o_m_p (1) for a description of the `-editor' and
     `-noedit' switches.  Note that while in the editor, the mes-
     sage being resent is available through a link named "@"
     (assuming the default _w_h_a_t_n_o_w_p_r_o_c ).  In addition, the
     actual pathname of the message is stored in the envariable
     $editalt, and the pathname of the folder containing the mes-
     sage is stored in the envariable $mhfolder.

     The `-draftfolder +folder' and `-draftmessage msg' switches
     invoke the _M_H draft folder facility.  This is an advanced
     (and highly useful) feature.  Consult the Advanced Features
     section of the _M_H manual for more information.

     Upon exiting from the editor, _d_i_s_t will invoke the _w_h_a_t_n_o_w
     program.  See _w_h_a_t_n_o_w (1) for a discussion of available
     options.  The invocation of this program can be inhibited by
     using the `-nowhatnowproc' switch.  (In truth of fact, it is
     the _w_h_a_t_n_o_w program which starts the initial edit.  Hence,
     `-nowhatnowproc' will prevent any edit from occurring.)

FILES
     /usr/misc/mh/lib/distcomps         The message skeleton
     or <mh-dir>/distcomps              Rather than the standard skeleton
     $HOME/.mh_profile                  The user profile
     <mh-dir>/draft                     The draft file

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Current-Folder:      To find the default current folder
     Draft-Folder:        To find the default draft-folder
     Editor:              To override the default editor
     fileproc:            Program to refile the message
     whatnowproc:         Program to ask the "What now?" questions

SEE ALSO
     comp(1), forw(1), repl(1), send(1), whatnow(1)

DEFAULTS
     `+folder' defaults to the current folder
     `msg' defaults to cur
     `-noannotate'
     `-nodraftfolder'
     `-noinplace'

CONTEXT
     If a folder is given, it will become the current folder.
     The message distributed will become the current message.

HISTORY
     _D_i_s_t originally used headers of the form "Distribute-xxx:"
     instead of "Resent-xxx:".  In order to conform with the ARPA
     Internet standard, RFC-822, the "Resent-xxx:" form is now



Printed 1/10/87                MH                               2






DIST(1)             UNIX Programmer's Manual              DIST(1)



     used.  _D_i_s_t will recognize "Distribute-xxx:" type headers
     and automatically convert them to "Resent-xxx:".

BUGS
     _D_i_s_t does not _r_i_g_o_r_o_u_s_l_y check the message being distributed
     for adherence to the transport standard, but _p_o_s_t called by
     _s_e_n_d does.  The _p_o_s_t program will balk (and rightly so) at
     poorly formatted messages, and _d_i_s_t won't correct things for
     you.

     If _w_h_a_t_n_o_w_p_r_o_c is _w_h_a_t_n_o_w, then _d_i_s_t uses a built-in _w_h_a_t_-
     _n_o_w, it does not actually run the _w_h_a_t_n_o_w program.  Hence,
     if you define your own _w_h_a_t_n_o_w_p_r_o_c, don't call it _w_h_a_t_n_o_w
     since _d_i_s_t won't run it.

     If your current working directory is not writable, the link
     named "@" is not available.






































Printed 1/10/87                MH                               3



