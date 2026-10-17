


REPL(1)             UNIX Programmer's Manual              REPL(1)



NAME
     repl - reply to a message

SYNOPSIS
     repl [+folder] [msg] [-annotate] [-noannotate]
          [-cc all/to/cc/me] [-nocc all/to/cc/me]
          [-draftfolder +folder] [-draftmessage msg]
          [-nodraftfolder] [-editor editor] [-noedit]
          [-fcc +folder] [-filter filterfile] [-form formfile]
          [-format] [-noformat] [-inplace] [-noinplace] [-query]
          [-noquery] [-width columns] [-whatnowproc program]
          [-nowhatnowproc] [-help]
9     _R_e_p_l aids a user in producing a reply to an existing mes-
     sage.  _R_e_p_l uses a reply template to guide its actions when
     constructing the message draft of the reply.  In its sim-
     plest form (with no arguments), it will set up a
     message-form skeleton in reply to the current message in the
     current folder, and invoke the whatnow shell.  The default
     reply template will direct _r_e_p_l to construct the composed
     message as follows:

          To: <Reply-To> or <From>
          cc: <cc>, <To>, and yourself
          Subject: Re: <Subject>
          In-reply-to: Your message of <Date>.
                       <Message-Id>

     where field names enclosed in angle brackets (< >) indicate
     the contents of the named field from the message to which
     the reply is being made.  The `-cc type' switch takes an
     argument which specifies who gets placed on the "cc:" list
     of the reply.  The `-query' switch modifies the action of
     `-cc type' switch by interactively asking you if each
     address that normally would be placed in the "To:" and "cc:"
     list should actually be sent a copy.  (This is useful for
     special-purpose replies.) Note that the position of the
     `-cc' and `-nocc' switches, like all other switches which
     take a positive and negative form, is important.

     If the file named "replcomps" exists in the user's MH direc-
     tory, it will be used instead of the default form.  In
     either case, the file specified by `-form formfile' will be
     used if given.

     If the draft already exists, _r_e_p_l will ask you as to the
     disposition of the draft.  A reply of quit will abort _r_e_p_l,
     leaving the draft intact; replace will replace the existing
     draft with a blank skeleton; and list will display the
     draft.

     See _c_o_m_p (1) for a description of the `-editor' and
     `-noedit' switches.  Note that while in the editor, the



Printed 1/10/87                MH                               1


9


REPL(1)             UNIX Programmer's Manual              REPL(1)



     message being replied to is available through a link named
     "@" (assuming the default _w_h_a_t_n_o_w_p_r_o_c ).  In addition, the
     actual pathname of the message is stored in the envariable
     $editalt, and the pathname of the folder containing the mes-
     sage is stored in the envariable $mhfolder.

     Although _r_e_p_l uses the `-form formfile' switch to direct it
     how to construct the beginning of the draft, the
     `-filter filterfile' switch directs _r_e_p_l as to how the mes-
     sage being replied-to should be formatted in the body of the
     draft.  If `-filter' is not specified, then the message
     being replied-to is not included in the body of the draft.
     If `-filter filterfile' is specified, then the message being
     replied-to is filtered (re-formatted) prior to being output
     to the body of the draft.  The filter file for _r_e_p_l should
     be a standard form file for _m_h_l, as _r_e_p_l will invoke _m_h_l to
     format the message being replied-to.  There is no default
     message filter (`-filter' must be followed by a file name).
     A filter file that is commonly used is:

          :
          body:nocomponent,compwidth=9,offset=9

     which says to output a blank line and then the body of the
     message being replied-to, indented by one tab-stop.

     If the `-annotate' switch is given, the message being
     replied-to will be annotated with the lines

          Replied: date
          Replied: addrs

     where the address list contains one line for each addressee.
     The annotation will be done only if the message is sent
     directly from _r_e_p_l.  If the message is not sent immediately
     from _r_e_p_l, "comp -use" may be used to re-edit and send the
     constructed message, but the annotations won't take place.
     The `-inplace' switch causes annotation to be done in place
     in order to preserve links to the annotated message.

     With the `-format' switch one can indicate if Internet-style
     formatting should be used (or not be used with `-noformat').
     If present (the default), then lines beginning with the
     fields "To:", "cc:", and "Bcc:" will be standardized and
     have duplicate addresses removed.  In addition, the
     `-width columns' switch will guide _r_e_p_l's formatting of
     these fields.

     The `-fcc +folder' switch can be used to automatically
     specify a folder to receive Fcc:s.  More than one folder,
     each preceeded by `-fcc' can be named.




Printed 1/10/87                MH                               2






REPL(1)             UNIX Programmer's Manual              REPL(1)



     A reply template is simply a format file.  See _m_h-_f_o_r_m_a_t (5)
     for the details.

     In addition to the standard escapes, _r_e_p_l also recognizes
     the following additional escape:
     _e_s_c_a_p_e  _s_u_b_s_t_i_t_u_t_i_o_n
     fcc     any `-fcc folder' switches

     The `-draftfolder +folder' and `-draftmessage msg' switches
     invoke the _M_H draft folder facility.  This is an advanced
     (and highly useful) feature.  Consult the Advanced Features
     section of the _M_H manual for more information.

     Upon exiting from the editor, _r_e_p_l will invoke the _w_h_a_t_n_o_w
     program.  See _w_h_a_t_n_o_w (1) for a discussion of available
     options.  The invocation of this program can be inhibited by
     using the `-nowhatnowproc' switch.  (In truth of fact, it is
     the _w_h_a_t_n_o_w program which starts the initial edit.  Hence,
     `-nowhatnowproc' will prevent any edit from occurring.)

FILES
     /usr/misc/mh/lib/replcomps         The reply template
     or <mh-dir>/replcomps              Rather than the standard template
     $HOME/.mh_profile                  The user profile
     <mh-dir>/draft                     The draft file

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Alternate-Mailboxes: To determine the user's mailboxes
     Current-Folder:      To find the default current folder
     Draft-Folder:        To find the default draft-folder
     Editor:              To override the default editor
     Msg-Protect:         To set mode when creating a new message (draft)
     fileproc:            Program to refile the message
     mhlproc:             Program to filter message being replied-to
     whatnowproc:         Program to ask the "What now?" questions

SEE ALSO
     comp(1), dist(1), forw(1), send(1), whatnow(1), mh-format(5)

DEFAULTS
     `+folder' defaults to the current folder
     `msg' defaults to cur
     `-nocc all' at ATHENA sites, `-cc all' otherwise
     `-format'
     `-noannotate'
     `-nodraftfolder'
     `-noinplace'
     `-noquery'
     `-width 72'





Printed 1/10/87                MH                               3






REPL(1)             UNIX Programmer's Manual              REPL(1)



CONTEXT
     If a folder is given, it will become the current folder.
     The message replied-to will become the current message.

BUGS
     If any addresses occur in the reply template, addresses in
     the template that do not contain hosts are defaulted
     incorrectly.  Instead of using the localhost for the
     default, _r_e_p_l uses the sender's host.  Moral of the story:
     if you're going to include addresses in a reply template,
     include the host portion of the address.

     If _w_h_a_t_n_o_w_p_r_o_c is _w_h_a_t_n_o_w, then _r_e_p_l uses a built-in _w_h_a_t_-
     _n_o_w, it does not actually run the _w_h_a_t_n_o_w program.  Hence,
     if you define your own _w_h_a_t_n_o_w_p_r_o_c, don't call it _w_h_a_t_n_o_w
     since _r_e_p_l won't run it.

     If your current working directory is not writable, the link
     named "@" is not available.




































Printed 1/10/87                MH                               4



