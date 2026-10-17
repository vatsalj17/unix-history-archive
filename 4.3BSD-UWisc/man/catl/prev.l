


PREV(1)             UNIX Programmer's Manual              PREV(1)



NAME
     prev - show the previous message

SYNOPSIS
     prev [+folder] [-header] [-noheader] [-showproc program]
          [-noshowproc] [-switches for _s_h_o_w_p_r_o_c] [-help]
9     _P_r_e_v performs a _s_h_o_w on the previous message in the speci-
     fied (or current) folder.  Like _s_h_o_w, it passes any switches
     on to the program named by _s_h_o_w_p_r_o_c, which is called to list
     the message.  This command is almost exactly equivalent to
     "show prev".  Consult the manual entry for _s_h_o_w (1) for all
     the details.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Current-Folder:      To find the default current folder
     showproc:            Program to show the message

SEE ALSO
     show(1), next(1)

DEFAULTS
     `+folder' defaults to the current folder
     `-format'
     `-header'

CONTEXT
     If a folder is specified, it will become the current folder.
     The message that is shown (i.e., the previous message in
     sequence) will become the current message.

BUGS
     _p_r_e_v is really a link to the _s_h_o_w program.  As a result, if
     you make a link to _p_r_e_v and that link is not called _p_r_e_v,
     your link will act like _s_h_o_w instead.  To circumvent this,
     add a profile-entry for the link to your _M_H profile and add
     the argument _p_r_e_v to the entry.











9


Printed 1/10/87                MH                               1



