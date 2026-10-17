


SEND(1)             UNIX Programmer's Manual              SEND(1)



NAME
     send - send a message

SYNOPSIS
     send [-alias aliasfile] [-draft] [-draftfolder +folder]
          [-draftmessage msg] [-nodraftfolder]
          [-filter filterfile] [-nofilter] [-format] [-noformat]
          [-forward] [-noforward] [-msgid] [-nomsgid] [-push]
          [-nopush] [-verbose] [-noverbose] [-watch] [-nowatch]
          [-width columns] [file ...] [-help]
9     _S_e_n_d will cause each of the specified files to be delivered
     (via _p_o_s_t (8)) to each of the destinations in the "To:",
     "cc:", "Bcc:", and "Fcc:" fields of the message.  If _s_e_n_d is
     re-distributing a message, as invoked from _d_i_s_t, then the
     corresponding "Resent-xxx" fields are examined instead.

     If `-push' is specified, _s_e_n_d will detach itself from the
     user's terminal and perform its actions in the background.
     If _p_u_s_h 'd and the draft can't be sent, then the `-forward'
     switch says that draft should be forwarded with the failure
     notice sent to the user.  This differs from putting _s_e_n_d in
     the background because the output is trapped and analyzed by
     _M_H.

     If `-verbose' is specified, _s_e_n_d will indicate the interac-
     tions occurring with the transport system, prior to actual
     delivery.  If `-watch' is specified _s_e_n_d will monitor the
     delivery of local and network mail.  Hence, by specifying
     both switches, a large detail of information can be gathered
     about each step of the message's entry into the transport
     system.

     The `-draftfolder +folder' and `-draftmessage msg' switches
     invoke the _M_H draft folder facility.  This is an advanced
     (and highly useful) feature.  Consult the Advanced Features
     section of the _M_H manual for more information.

     _S_e_n_d with no _f_i_l_e argument will query whether the draft is
     the intended file, whereas `-draft' will suppress this ques-
     tion.  Once the transport system has successfully accepted
     custody of the message, the file will be renamed with a
     leading comma, which allows it to be retrieved until the
     next draft message is sent.  If there are errors in the for-
     matting of the message, _s_e_n_d will abort with a (hopefully)
     helpful error message.

     If a "Bcc:" field is encountered, its addresses will be used
     for delivery, and the "Bcc:" field will be removed from the
     message sent to sighted recipients.  The blind recipients
     will receive an entirely new message with a minimal set of
     headers.  Included in the body of the message will be a copy
     of the message sent to the sighted recipients.  If



Printed 1/10/87                MH                               1


9


SEND(1)             UNIX Programmer's Manual              SEND(1)



     `-filter filterfile' is specified, then this copy is fil-
     tered (re-formatted) prior to being sent to the blind reci-
     pients.

     Prior to sending the message, the fields "From: user@local",
     and "Date: now" will be appended to the headers in the mes-
     sage.  If the envariable $SIGNATURE is set, then its value
     is used as your personal name when constructing the "From:"
     line of the message.  If this envariable is not set, then
     _s_e_n_d will consult the profile entry "Signature" for this
     information.  On hosts where _M_H was configured with the UCI
     option, if $SIGNATURE is not set and the "Signature" profile
     entry is not present, then the file $HOME/.signature is con-
     sulted.  If `-msgid' is specified, then a "Message-ID:"
     field will also be added to the message.

     If _s_e_n_d is re-distributing a message (when invoked by
     _d_i_s_t ), then "Resent-" will be prepended to each of these
     fields: "From:", "Date:", and "Message-ID:".  If the message
     already contains a "From:" field, then a "Sender:
     user@local" field will be added as well.  (An already exist-
     ing "Sender:" field is an error!)

     By using the `-format' switch, each of the entries in the
     "To:" and "cc:" fields will be replaced with "standard" for-
     mat entries.  This standard format is designed to be usable
     by all of the message handlers on the various systems around
     the Internet.  If `-noformat' is given, then headers are
     output exactly as they appear in the message draft.

     If an "Fcc: folder" is encountered, the message will be
     copied to the specified folder for the sender in the format
     in which it will appear to any non-Bcc receivers of the mes-
     sage.  That is, it will have the appended fields and field
     reformatting.  The "Fcc:" fields will be removed from all
     outgoing copies of the message.

     By using the `-width columns' switch, the user can direct
     _s_e_n_d as to how long it should make header lines containing
     addresses.

     By using the `-alias aliasfile' switch, the user can direct
     _s_e_n_d to consult the named files for alias definitions (more
     than one file, each  preceded by `-alias', can be named).
     See _m_h-_a_l_i_a_s (5) for more information.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Draft-Folder:        To find the default draft-folder



Printed 1/10/87                MH                               2






SEND(1)             UNIX Programmer's Manual              SEND(1)



     Signature:           To determine the user's mail signature
     mailproc:            Program to post failure notices
     postproc:            Program to post the message

SEE ALSO
     comp(1), dist(1), forw(1), repl(1), mh-alias(5), post(8)

DEFAULTS
     `file' defaults to <mh-dir>/draft
     `-alias /usr/misc/mh/lib/MailAliases'
     `-nodraftfolder'
     `-nofilter'
     `-format'
     `-forward'
     `-nomsgid'
     `-nopush'
     `-noverbose'
     `-nowatch'
     `-width 72'

CONTEXT
     None

































Printed 1/10/87                MH                               3



