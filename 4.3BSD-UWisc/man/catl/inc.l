


INC(1)              UNIX Programmer's Manual               INC(1)



NAME
     inc - incorporate new mail

SYNOPSIS
     inc [+folder] [-audit audit-file] [-noaudit] [-changecur]
          [-nochangecur] [-form formatfile] [-format string]
          [-file name] [-silent] [-nosilent] [-truncate]
          [-notruncate] [-width columns] [-host host]
          [-user user] [-pack file] [-nopack] [-rpop] [-norpop]
          [-help]
9     _I_n_c incorporates mail from the user's incoming mail drop
     into an _M_H folder.  If `+folder' isn't specified, the folder
     named "inbox" in the user's _M_H directory will be used.  The
     new messages being incorporated are assigned numbers start-
     ing with the next highest number in the folder.  If the
     specified (or default) folder doesn't exist, the user will
     be queried prior to its creation.  As the messages are pro-
     cessed, a _s_c_a_n listing of the new mail is produced.

     If the user's profile contains a "Msg-Protect: nnn" entry,
     it will be used as the protection on the newly created mes-
     sages, otherwise the _M_H default of 0644 will be used.  Dur-
     ing all operations on messages, this initially assigned pro-
     tection will be preserved for each message, so _c_h_m_o_d(1) may
     be used to set a protection on an individual message, and
     its protection will be preserved thereafter.

     If the switch `-audit audit-file' is specified (usually as a
     default switch in the profile), then _i_n_c will append a
     header line and a line per message to the end of the speci-
     fied audit-file with the format:

          inc date
               <scan line for first message>
               <scan line for second message>
                         <etc.>

     This is useful for keeping track of volume and source of
     incoming mail.  Eventually, _r_e_p_l, _f_o_r_w, _c_o_m_p, and _d_i_s_t may
     also produce audits to this (or another) file, perhaps with
     "Message-Id:" information to keep an exact correspondence
     history.  "Audit-file" will be in the user's MH directory
     unless a full path is specified.

     _I_n_c will incorporate even improperly formatted messages into
     the user's MH folder, inserting a blank line prior to the
     offending component and printing a comment identifying the
     bad message.

     In all cases, the user's mail drop will be zeroed, unless
     the `-notruncate' switch is given.
9


Printed 1/10/87                MH                               1






INC(1)              UNIX Programmer's Manual               INC(1)



     If the profile entry "Unseen-Sequence" is present and
     non-empty, then _i_n_c will add each of the newly incorporated
     messages to each sequence named by the profile entry.  This
     is similar to the "Previous-Sequence" profile entry sup-
     ported by all _M_H commands which take `msgs' or `msg' argu-
     ments.  Note that _i_n_c will not zero each sequence prior to
     adding messages.

     The interpretation of the `-form formatfile',
     `-format string', and `-width columns' switches is the same
     as in _s_c_a_n (1).

     By using the `-file name' switch, one can direct _i_n_c to
     incorporate messages from a file other than the user's mail-
     drop.  Note that the name file will NOT be zeroed, unless
     the `-truncate' switch is given.

     If the envariable $MAILDROP is set, then _i_n_c uses it as the
     location of the user's maildrop instead of the default (the
     `-file name' switch still overrides this, however).  If this
     envariable is not set, then _i_n_c will consult the profile
     entry "MailDrop" for this information.  If the value found
     is not absolute, then it is interpreted relative to the
     user's _M_H directory.  If the value is not found, then _i_n_c
     will look in the standard system location for the user's
     maildrop.

     The `-silent' switch directs _i_n_c to be quiet and not ask any
     questions at all.  This is useful for putting _i_n_c in the
     background and going on to other things.

     If the local host is configured as a POP client, or if the
     `-host host' switch is given, then _i_n_c will query the POP
     service host as to the status of mail waiting.  The
     `-user user' switch may be given to specify the name of the
     POP subscriber you wish to check mail for on the POP service
     host.  The `-rpop' switch uses the UNIX _r_P_O_P (authentication
     done via trusted connections).  In contrast, the `-norpop'
     switch uses the ARPA _P_O_P (in which case _i_n_c will prompt for
     a password).

     If _i_n_c uses POP, then the `-pack file' switch is considered.
     If given, then _i_n_c simply uses the POP to _p_a_c_k_f (1) the
     user's maildrop from the POP service host to the named file.
     This switch is provided for those users who prefer to use
     _m_s_h to read their maildrops.

FILES
     $HOME/.mh_profile                  The user profile
     /usr/misc/mh/lib/mtstailor         tailor file
     /usr/spool/mail/$USER              Location of mail drop




Printed 1/10/87                MH                               2






INC(1)              UNIX Programmer's Manual               INC(1)



PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Alternate-Mailboxes: To determine the user's mailboxes
     Folder-Protect:      To set mode when creating a new folder
     Msg-Protect:         To set mode when creating a new message and audit-file
     Unseen-Sequence:     To name sequences denoting unseen messages

SEE ALSO
     _P_o_s_t _O_f_f_i_c_e _P_r_o_t_o_c_o_l (_r_e_v_i_s_e_d) (aka RFC-819 with revisions),
     mhmail(1), scan(1), mh-mail(5), post(8)

DEFAULTS
     `+folder' defaults to "inbox"
     `-noaudit'
     `-changecur'
     `-format' defaulted as described above
     `-nosilent'
     `-truncate' if `-file name' not given, `-notruncate' otherwise
     `-width' defaulted to the width of the terminal
     `-nopack'
     `-rpop'

CONTEXT
     The folder into which messages are being incorporated will
     become the current folder.  The first message incorporated
     will become the current message, unless the `-nochangecur'
     option is specified.  This leaves the context ready for a
     _s_h_o_w of the first new message.

BUGS
     The argument to the `-format' switch must be interpreted as
     a single token by the shell that invokes _i_n_c.  Therefore,
     one must usually place the argument to this switch inside
     double-quotes.





















Printed 1/10/87                MH                               3



