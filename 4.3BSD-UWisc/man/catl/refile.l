


REFILE(1)           UNIX Programmer's Manual            REFILE(1)



NAME
     refile - file message in other folders

SYNOPSIS
     refile [msgs] [-draft] [-link] [-nolink] [-preserve]
          [-nopreserve] [-src +folder] [-file file] +folder ...
          [-help]
9     _R_e_f_i_l_e moves (_m_v (1)) or links (_l_n (1)) messages from a
     source folder into one or more destination folders.  If you
     think of a message as a sheet of paper, this operation is
     not unlike filing the sheet of paper (or copies) in file
     cabinet folders.  When a message is filed, it is linked into
     the destination folder(s) if possible, and is copied other-
     wise.  As long as the destination folders are all on the
     same file system, multiple filing causes little storage
     overhead.  This facility provides a good way to cross-file
     or multiply-index messages.  For example, if a message is
     received from Jones about the ARPA Map Project, the command

          refile cur +jones +Map

     would allow the message to be found in either of the two
     folders `jones' or `Map'.

     The option `-file file' directs _r_e_f_i_l_e to use the specified
     file as the source message to be filed, rather than a mes-
     sage from a folder.  Note that the file should be a validly
     formatted message, just like any other _M_H message.  It
     should NOT be in mail drop format (to convert a file in mail
     drop format to a folder of _M_H messages, see _i_n_c (1)).

     If a destination folder doesn't exist, _r_e_f_i_l_e will ask if
     you want to create it.  A negative response will abort the
     file operation.

     The option `-link' preserves the source folder copy of the
     message (i.e., it does a _l_n(1) rather than a _m_v(1)),
     whereas, `-nolink' deletes the filed messages from the
     source folder.  Normally, when a message is filed, it is
     assigned the next highest number available in each of the
     destination folders.  Use of the `-preserve' switch will
     override this message renaming, but name conflicts may
     occur, so use this switch cautiously.

     If `-link' is not specified (or `-nolink' is specified), the
     filed messages will be removed (_u_n_l_i_n_k (2)) from the source
     folder, similar to the way _m_v (1) works.

     If the user has a profile component such as

          rmmproc:  /bin/rm
9


Printed 1/10/87                MH                               1






REFILE(1)           UNIX Programmer's Manual            REFILE(1)



     then instead of simply renaming the message file, _r_e_f_i_l_e
     will call the named program to delete the file.

     The `-draft' switch tells _r_e_f_i_l_e to file the <mh-dir>/draft.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Current-Folder:      To find the default current folder
     Folder-Protect:      To set mode when creating a new folder
     rmmproc:             Program to delete the message

SEE ALSO
     folder(1)

DEFAULTS
     `-src +folder' defaults to the current folder
     `msgs' defaults to cur
     `-nolink'
     `-nopreserve'

CONTEXT
     If `-src +folder' is given, it will become the current
     folder.  If neither `-link' nor `all' is specified, the
     current message in the source folder will be set to the last
     message specified; otherwise, the current message won't be
     changed.

     If the Previous-Sequence profile entry is set, in addition
     to defining the named sequences from the source folder,
     _r_e_f_i_l_e will also define those sequences for the destination
     folders.  See _m_h-_p_r_o_f_i_l_e (1) for information concerning the
     previous sequence.




















Printed 1/10/87                MH                               2



