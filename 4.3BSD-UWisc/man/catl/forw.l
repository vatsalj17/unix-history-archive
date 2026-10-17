


FORW(1)             UNIX Programmer's Manual              FORW(1)



NAME
     forw - forward messages

SYNOPSIS
     forw [+folder] [msgs] [-annotate] [-noannotate]
          [-draftfolder +folder] [-draftmessage msg]
          [-nodraftfolder] [-editor editor] [-noedit]
          [-filter filterfile] [-form formfile] [-format]
          [-noformat] [-inplace] [-noinplace]
          [-whatnowproc program] [-nowhatnowproc] [-help]

     forw [+folder] [msgs] [-digest list] [-issue number]
          [-volume number] [other switches for _f_o_r_w] [-help]
9     _F_o_r_w may be used to prepare a message containing other mes-
     sages.  It constructs the new message from the components
     file or `-form formfile' (see _c_o_m_p ), with a body composed
     of the message(s) to be forwarded.  An editor is invoked as
     in _c_o_m_p, and after editing is complete, the user is prompted
     before the message is sent.

     The default message form contains the following elements:

          To:
          cc:
          Subject:
          --------

     If the file named "forwcomps" exists in the user's MH direc-
     tory, it will be used instead of this form.  In either case,
     the file specified by `-form formfile' will be used if
     given.

     If the draft already exists, _f_o_r_w will ask you as to the
     disposition of the draft.  A reply of quit will abort _f_o_r_w,
     leaving the draft intact; replace will replace the existing
     draft with a blank skeleton; and list will display the
     draft.

     If the `-annotate' switch is given, each message being for-
     warded will be annotated with the lines

          Forwarded: date
          Forwarded: addrs

     where each address list contains as many lines as required.
     This annotation will be done only if the message is sent
     directly from _f_o_r_w.  If the message is not sent immediately
     from _f_o_r_w, "comp -use" may be used to re-edit and send the
     constructed message, but the annotations won't take place.
     The '-inplace' switch causes annotation to be done in place
     in order to preserve links to the annotated message.
9


Printed 1/10/87                MH                               1






FORW(1)             UNIX Programmer's Manual              FORW(1)



     See _c_o_m_p (1) for a description of the `-editor' and
     `-noedit' switches.

     Although _f_o_r_w uses the `-form formfile' switch to direct it
     how to construct the beginning of the draft, the
     `-filter filterfile', `-format', and `-noformat' switches
     direct _f_o_r_w as to how each forwarded message should be for-
     matted in the body of the draft.  If `-noformat' is speci-
     fied, then each forwarded message is output exactly as it
     appears.  If `-format' or `-filter filterfile' is specified,
     then each forwarded message is filtered (re-formatted) prior
     to being output to the body of the draft.  The filter file
     for _f_o_r_w should be a standard form file for _m_h_l, as _f_o_r_w
     will invoke _m_h_l to format the forwarded messages.  The
     default message filter (what you get with `-format') is:

          width=80,overflowtext=,overflowoffset=10
          leftadjust,compress,compwidth=9
          Date:formatfield="%<(nodate{text})%{text}%|%(tws{text})%>"
          From:
          To:
          cc:
          Subject:
          :
          body:nocomponent,overflowoffset=0,noleftadjust,nocompress

     If the file named "mhl.forward" exists in the user's MH
     directory, it will be used instead of this form.  In either
     case, the file specified by `-filter filterfile' will be
     used if given.  To summarize: `-noformat' will reproduce
     each forwarded message exactly, `-format' will use _m_h_l and a
     default filterfile, "mhl.forward", to format each forwarded
     message, and `-filter filterfile' will use the named filter-
     file to format each forwarded message with _m_h_l.

     Each forwarded message is separated with an encapsulation
     delimiter so that when received, the message is suitable for
     bursting by _b_u_r_s_t (1).

     For users of _p_r_o_m_p_t_e_r (1), by specifying prompter's `-
     prepend' switch in the .mh_profile file, any commentary text
     is entered before the forwarded messages.  (A major win!)

     The `-draftfolder +folder' and `-draftmessage msg' switches
     invoke the _M_H draft folder facility.  This is an advanced
     (and highly useful) feature.  Consult the Advanced Features
     section of the _M_H manual for more information.

     Upon exiting from the editor, _f_o_r_w will invoke the _w_h_a_t_n_o_w
     program.  See _w_h_a_t_n_o_w (1) for a discussion of available
     options.  The invocation of this program can be inhibited by
     using the `-nowhatnowproc' switch.  (In truth of fact, it is



Printed 1/10/87                MH                               2






FORW(1)             UNIX Programmer's Manual              FORW(1)



     the _w_h_a_t_n_o_w program which starts the initial edit.  Hence,
     `-nowhatnowproc' will prevent any edit from occurring.)

     The `-digest list', `-issue number', and `-volume number'
     switches implement a digest facility for _M_H.  See the _M_H
     user's manual for more information.

FILES
     /usr/misc/mh/lib/forwcomps         The message skeleton
     or <mh-dir>/forwcomps              Rather than the standard skeleton
     /usr/misc/mh/lib/digestcomps       The message skeleton if `-digest' is given
     or <mh-dir>/digestcomps            Rather than the standard skeleton
     /usr/misc/mh/lib/mhl.forward       The message filter
     or <mh-dir>/mhl.forward            Rather than the standard filter
     $HOME/.mh_profile                  The user profile
     <mh-dir>/draft                     The draft file

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Current-Folder:      To find the default current folder
     Draft-Folder:        To find the default draft-folder
     Editor:              To override the default editor
     Msg-Protect:         To set mode when creating a new message (draft)
     fileproc:            Program to refile the message
     mhlproc:             Program to filter messages being forwarded
     whatnowproc:         Program to ask the "What now?" questions

SEE ALSO
     _P_r_o_p_o_s_e_d _S_t_a_n_d_a_r_d _f_o_r _M_e_s_s_a_g_e _E_n_c_a_p_s_u_l_a_t_i_o_n (aka RFC-934),
     comp(1), dist(1), repl(1), send(1), whatnow(1)

DEFAULTS
     `+folder' defaults to the current folder
     `msgs' defaults to cur
     `-noannotate'
     `-nodraftfolder'
     `-noformat'
     `-noinplace'

CONTEXT
     If a folder is given, it will become the current folder.
     The first message forwarded will become the current message.

BUGS
     If _w_h_a_t_n_o_w_p_r_o_c is _w_h_a_t_n_o_w, then _f_o_r_w uses a built-in _w_h_a_t_-
     _n_o_w, it does not actually run the _w_h_a_t_n_o_w program.  Hence,
     if you define your own _w_h_a_t_n_o_w_p_r_o_c, don't call it _w_h_a_t_n_o_w
     since _f_o_r_w won't run it.

     When _f_o_r_w is told to annotate the messages it forwards, it
     doesn't actually annotate them until the draft is success-
     fully sent.  If from the _w_h_a_t_n_o_w_p_r_o_c, you _p_u_s_h instead of



Printed 1/10/87                MH                               3






FORW(1)             UNIX Programmer's Manual              FORW(1)



     _s_e_n_d, it's possible to confuse _f_o_r_w by re-ordering the file
     (e.g., by using `folder -pack') before the message is suc-
     cessfully sent.  _D_i_s_t and _r_e_p_l don't have this problem.




















































Printed 1/10/87                MH                               4



