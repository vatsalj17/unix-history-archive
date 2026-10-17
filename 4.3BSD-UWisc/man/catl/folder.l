


FOLDER(1)           UNIX Programmer's Manual            FOLDER(1)



NAME
     folder, folders - set/list current folder/message

SYNOPSIS
     folder [+folder] [msg] [-all] [-fast] [-nofast] [-header]
          [-noheader] [-pack] [-nopack] [-recurse] [-norecurse]
          [-total] [-nototal] [-print] [-noprint] [-list]
          [-nolist] [-push] [-pop] [-help]

     folders
9     Since the _M_H environment is the shell, it is easy to lose
     track of the current folder from day to day.

     When _f_o_l_d_e_r is given the `-print' switch (the default), the
     current folder and/or message may be set, or all folders may
     be listed.  When a `+folder' argument is given, this
     corresponds to a "cd" operation in the _C_S_h_e_l_l; when no
     `+folder' argument is given, this corresponds roughly to a
     "pwd" operation in the _C_S_h_e_l_l.

     _F_o_l_d_e_r will list the current folder, the number of messages
     in it, the range of the messages (low-high), and the current
     message within the folder, and will flag extra files if they
     exist.  An example of the output is:

     inbox+            has  16 messages (  3- 22); cur=  5.

     If a `+folder' and/or `msg' are specified, they will become
     the current folder and/or message.  Specifying `-all' will
     produce a line for each folder in the user's MH directory,
     sorted alphabetically.  These folders are preceded by the
     read-only folders, which occur as "atr-cur-" entries in the
     user's _M_H context.  For example,

     Folder               # of messages ( range )  cur msg (other files)
     /fsd/rs/m/tacc    has  35 messages (  1- 35); cur= 23.
     /rnd/phyl/Mail/EP has  82 messages (  1-108); cur= 82.
     ff                has  no messages.
     inbox+            has  16 messages (  3- 22); cur=  5.
     mh                has  76 messages (  1- 76); cur= 70.
     notes             has   2 messages (  1-  2); cur=  1.
     ucom              has 124 messages (  1-124); cur=  6; (others).
               TOTAL=      339 messages in 7 folders

     The "+" after inbox indicates that it is the current folder.
     The "(others)" indicates that the folder `ucom' has files
     which aren't messages.  These files may either be
     sub-folders, or files that don't belong under the MH file
     naming scheme.

     The header is output if either an `-all' or a `-header'
     switch is specified; it is suppressed by `-noheader'.  Also,



Printed 1/10/87                MH                               1


9


FOLDER(1)           UNIX Programmer's Manual            FOLDER(1)



     if _f_o_l_d_e_r is invoked by a name ending with "s" (e.g., _f_o_l_d_-
     _e_r_s ), `-all' is assumed.  A `-total' switch will produce
     only the summary line.

     If a `+folder' and/or `msg' is given along with the `-all'
     switch, _f_o_l_d_e_r will, in addition to setting the current
     folder and/or message, list the top-level folders for the
     current folder (with `-norecurse') or list all folders under
     the current folder recursively (with `-recurse').

     If `-fast' is given, only the folder name (or names in the
     case of `-all') will be listed.  (This is faster because the
     folders need not be read.)

     The `-pack' switch will compress the message names in a
     folder, removing holes in message numbering.

     The `-recurse' switch will list each folder recursively.
     Use of this option effectively defeats the speed enhancement
     of the `-fast' option, since each folder must be searched
     for subfolders.  Nevertheless, the combination of these
     options is useful.

     If the specified (or default) folder doesn't exist, the user
     will be queried if the folder should be created.  (This is
     the easy way to create an empty folder for use later.)

     The `-push' switch directs _f_o_l_d_e_r to push the current folder
     onto the _f_o_l_d_e_r-_s_t_a_c_k, and make the `+folder' argument the
     current folder.  If `+folder' is not given, the current
     folder and the top of the _f_o_l_d_e_r-_s_t_a_c_k are exchanged.  This
     corresponds to the "pushd" operation in the _C_S_h_e_l_l.

     The `-pop' switch directs _f_o_l_d_e_r to discard the top of the
     _f_o_l_d_e_r-_s_t_a_c_k, after setting the current folder to that
     value.  No `+folder' argument is allowed.  This corresponds
     to the "popd" operation in the _C_S_h_e_l_l.  The `-push' switch
     and the `-pop' switch are mutually exclusive: the last
     occurrence of either one overrides any previous occurrence
     of the other.

     The `-list' switch directs _f_o_l_d_e_r to list the contents of
     the _f_o_l_d_e_r-_s_t_a_c_k.  No `+folder' argument is allowed.  After
     a successful `-push' or `-pop', the `-list' action is taken.
     This corresponds to the "dirs" operation in the _C_S_h_e_l_l.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Current-Folder:      To find the default current folder



Printed 1/10/87                MH                               2






FOLDER(1)           UNIX Programmer's Manual            FOLDER(1)



     Folder-Protect:      To set mode when creating a new folder
     Folder-Stack:        To determine the folder stack
     lsproc:              Program to list the contents of a folder

SEE ALSO
     refile(1), mhpath(1)

DEFAULTS
     `+folder' defaults to the current folder
     `msg' defaults to none
     `-nofast'
     `-noheader'
     `-nototal'
     `-nopack'
     `-norecurse'
     `-print' is the default if no `-list', `-push', or `-pop' is specified

CONTEXT
     If `+folder' and/or `msg' are given, they will become the
     current folder and/or message.

HISTORY
     In previous versions of _M_H, the `-fast' switch prevented
     context changes from occurring for the current folder.  This
     is no longer the case: if `+folder' is given, then _f_o_l_d_e_r
     will always change the current folder to that.





























Printed 1/10/87                MH                               3



