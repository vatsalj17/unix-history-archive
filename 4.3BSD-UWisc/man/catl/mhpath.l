


MHPATH(1)           UNIX Programmer's Manual            MHPATH(1)



NAME
     mhpath - print full pathnames of MH messages and folders

SYNOPSIS
     mhpath [+folder] [msgs] [-help]
9     _M_h_p_a_t_h expands and sorts the message list `msgs' and writes
     the full pathnames of the messages to the standard output
     separated by newlines.  If no `msgs' are specified, _m_h_p_a_t_h
     outputs the folder pathname instead.

     Contrasted with other MH commands, a message argument to
     _m_h_p_a_t_h may often be intended for _w_r_i_t_i_n_g.  Because of this:
     1) the name "new" has been added to _m_h_p_a_t_h's list of
     reserved message names (the others are "first", "last",
     "prev", "next", "cur", and "all").  The new message is
     equivalent to the message after the last message in a folder
     (and equivalent to 1 in a folder without messages).  The
     "new" message may not be used as part of a message range.
     2) Within a message list, the following designations may
     refer to messages that do not exist: a single numeric mes-
     sage name, the single message name "cur", and (obviously)
     the single message name "new".  All other message designa-
     tions must refer to at least one existing message.  3) An
     empty folder is not in itself an error.

     Message numbers greater than the highest existing message in
     a folder as part of a range designation are replaced with
     the next free message number.

     Examples: The current folder foo contains messages 3 5 6.
     Cur is 4.

          % mhpath
          /r/phyl/Mail/foo

          % mhpath all
          /r/phyl/Mail/foo/3
          /r/phyl/Mail/foo/5
          /r/phyl/Mail/foo/6

          % mhpath 2001
          /r/phyl/Mail/foo/7

          % mhpath 1-2001
          /r/phyl/Mail/foo/3
          /r/phyl/Mail/foo/5
          /r/phyl/Mail/foo/6

          % mhpath new
          /r/phyl/Mail/foo/7

          % mhpath last new



Printed 1/10/87                MH                               1


9


MHPATH(1)           UNIX Programmer's Manual            MHPATH(1)



          /r/phyl/Mail/foo/6
          /r/phyl/Mail/foo/7

          % mhpath last-new
          bad message list "last-new".

          % mhpath cur
          /r/phyl/Mail/foo/4

          % mhpath 1-2
          no messages in range "1-2".

          % mhpath first:2
          /r/phyl/Mail/foo/3
          /r/phyl/Mail/foo/5

          % mhpath 1 2
          /r/phyl/Mail/foo/1
          /r/phyl/Mail/foo/2

     _M_H_p_a_t_h is also useful in back-quoted operations:

          % cd `mhpath +inbox`

          % echo `mhpath +`
          /r/phyl/Mail

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Current-Folder:      To find the default current folder

SEE ALSO
     folder(1)

DEFAULTS
     `+folder' defaults to the current folder
     `msgs' defaults to none

CONTEXT
     None

BUGS
     Like all MH commands, _m_h_p_a_t_h expands and sorts [msgs].  So
     don't expect

          mv `mhpath 501 500`

     to move 501 to 500.  Quite the reverse.  But




Printed 1/10/87                MH                               2






MHPATH(1)           UNIX Programmer's Manual            MHPATH(1)



          mv `mhpath 501` `mhpath 500`

     will do the trick.

     Out of range message 0 is treated far more severely than
     large out of range message numbers.

















































Printed 1/10/87                MH                               3



