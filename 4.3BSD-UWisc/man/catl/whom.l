


WHOM(1)             UNIX Programmer's Manual              WHOM(1)



NAME
     whom - report to whom a message would go

SYNOPSIS
     whom [-alias aliasfile] [-check] [-nocheck] [-draft]
          [-draftfolder +folder] [-draftmessage msg]
          [-nodraftfolder] [file] [-help]
9     _W_h_o_m is used to expand the headers of a message into a set
     of addresses and optionally verify that those addresses are
     deliverable at that time (if `-check' is given).

     The `-draftfolder +folder' and `-draftmessage msg' switches
     invoke the _M_H draft folder facility.  This is an advanced
     (and highly useful) feature.  Consult the Advanced Features
     section of the _M_H manual for more information.

     By using the `-alias aliasfile' switch, the user can direct
     _s_e_n_d to consult the named files for alias definitions (more
     than one file, each  preceeded by `-alias', can be named).
     See _m_h-_a_l_i_a_s (5) for more information.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Draft-Folder:        To find the default draft-folder
     postproc:            Program to post the message

SEE ALSO
     mh-alias(5), post(8)

DEFAULTS
     `file' defaults to <mh-dir>/draft
     `-nocheck'
     `-alias /usr/misc/mh/lib/MailAliases'

CONTEXT
     None

BUGS
     With the `-check' option, _w_h_o_m makes no guarantees that the
     addresses listed as being ok are really deliverable, rather,
     an address being listed as ok means that at the time that
     _w_h_o_m was run the address was thought to be deliverable by
     the transport service.  For local addresses, this is abso-
     lute; for network addresses, it means that the host is
     known; for uucp addresses, it (often) means that the _U_U_C_P
     network is available for use.



9


Printed 1/10/87                MH                               1



