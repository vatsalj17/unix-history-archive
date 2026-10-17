


PACKF(1)            UNIX Programmer's Manual             PACKF(1)



NAME
     packf - compress a folder into a single file

SYNOPSIS
     packf [+folder] [msgs] [-file name] [-help]
9     _P_a_c_k_f takes messages from a folder and copies them to a sin-
     gle file.  Each message in the file is separated by four
     CTRL-A's and a newline.  Messages packed can be unpacked
     using _i_n_c.

     If the _n_a_m_e given to the `-file name' switch exists, then
     the messages specified will be appended to the end of the
     file, otherwise the file will be created and the messages
     appended.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Current-Folder:      To find the default current folder
     Msg-Protect:         To set mode when creating a new `file'

SEE ALSO
     inc(1)

DEFAULTS
     `+folder' defaults to the current folder
     `msgs' defaults to all
     `-file ./msgbox'

CONTEXT
     If a folder is given, it will become the current folder.
     The first message packed will become the current message.

















9


Printed 1/10/87                MH                               1



