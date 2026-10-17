


RMF(1)              UNIX Programmer's Manual               RMF(1)



NAME
     rmf - remove folder

SYNOPSIS
     rmf [+folder] [-interactive] [-nointeractive] [-help]
9     _R_m_f removes all of the messages (files) within the specified
     (or default) folder, and then removes the folder (directory)
     itself.  If there are any files within the folder which are
     not a part of _M_H, they will _n_o_t be removed, and an error
     will be produced.  If the folder is given explicitly or the
     `-nointeractive' option is given, then the folder will be
     removed without confirmation.  Otherwise, the user will be
     asked for confirmation.  If _r_m_f can't find the current
     folder, for some reason, the folder to be removed defaults
     to `+inbox' with confirmation.

     _R_m_f irreversibly deletes messages that don't have other
     links, so use it with caution.

     If the folder being removed is a subfolder, the parent
     folder will become the new current folder, and _r_m_f will pro-
     duce a message telling the user this has happened.  This
     provides an easy mechanism for selecting a set of messages,
     operating on the list, then removing the list and returning
     to the current folder from which the list was extracted.

     _R_m_f of a read-only folder will delete the private sequence
     and cur information (i.e., "atr-_s_e_q-_f_o_l_d_e_r" entries) from
     the profile without affecting the folder itself.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Current-Folder:      To find the default current folder

SEE ALSO
     rmm(1)

DEFAULTS
     `+folder' defaults to the current folder, usually with confirmation
     `-interactive' if +folder' not given, `-nointeractive' otherwise

CONTEXT
     _R_m_f will set the current folder to the parent folder if a
     subfolder is removed; or if the current folder is removed,
     it will make "inbox" current.  Otherwise, it doesn't change
     the current folder or message.


9


Printed 1/10/87                MH                               1






RMF(1)              UNIX Programmer's Manual               RMF(1)



BUGS
     Although intuitively one would suspect that _r_m_f works recur-
     sively, it does not.  Hence if you have a sub-folder within
     a folder, in order to _r_m_f the parent, you must first _r_m_f
     each of the children.


















































Printed 1/10/87                MH                               2



