


RMM(1)              UNIX Programmer's Manual               RMM(1)



NAME
     rmm - remove messages

SYNOPSIS
     rmm [+folder] [msgs] [-help]
9     _R_m_m removes the specified messages by renaming the message
     files with preceding commas.  Many sites consider files that
     start with a comma to be a temporary backup, and arrange for
     _c_r_o_n (8) to remove such files once a day.

     If the user has a profile component such as

          rmmproc:  /bin/rm

     then instead of simply renaming the message file, _r_m_m will
     call the named program to delete the file.  Note that at
     most installations, _c_r_o_n (8) is told to remove files that
     begin with a comma once a night.

     Some users of csh prefer the following:

          alias rmm 'refile +d'

     where folder +d is a folder for deleted messages, and

          alias mexp 'rm `mhpath +d all`'

     is used to "expunge" deleted messages.

     The current message is not changed by _r_m_m, so a _n_e_x_t  will
     advance to the next message in the folder as expected.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Current-Folder:      To find the default current folder
     rmmproc:             Program to delete the message

SEE ALSO
     rmf(1)

DEFAULTS
     `+folder' defaults to the current folder
     `msgs' defaults to cur

CONTEXT
     If a folder is given, it will become the current folder.


9


Printed 1/10/87                MH                               1



