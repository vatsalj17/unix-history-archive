


SORTM(1)            UNIX Programmer's Manual             SORTM(1)



NAME
     sortm - sort messages

SYNOPSIS
     sortm [+folder] [msgs] [-datefield field] [-verbose]
          [-noverbose] [-help]
9     _S_o_r_t_m sorts the specified messages in the named folder
     according to the chronological order of the "Date:" field of
     each message.  Messages which are in the folder, but not
     specified by `msgs', are moved to the end of the folder.  If
     a message does not exist (the folder has a gap), _s_o_r_t_m may
     fill the gap if convenient.

     The `-verbose' switch directs _s_o_r_t_m to tell the user the
     general actions that it is taking to place the folder in
     sorted order.

     The `-datefield field' switch tells _s_o_r_t_m the name of the
     field to use when making the date comparison.  If the user
     has a special field in each message, such as "BB-Posted:" or
     "Delivery-Date:", then the `-datefield' switch can be used
     to direct _s_o_r_t_m which field to examine.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Current-Folder:      To find the default current folder

SEE ALSO
     folder (1)

DEFAULTS
     `+folder' defaults to the current folder
     `msgs' defaults to all
     `-datefield date'
     `-noverbose'

CONTEXT
     If a folder is given, it will become the current folder.  If
     the current message is moved, _s_o_r_t_m will preserve its status
     as current.

HISTORY
     Timezones used to be ignored when comparing dates: they
     aren't any more.

BUGS
     If _s_o_r_t_m encounters a message without a date-field, or if
     the message has a date-field that _s_o_r_t_m cannot parse, then
     _s_o_r_t_m attempts to keep the message in the same relative



Printed 1/10/87                MH                               1


9


SORTM(1)            UNIX Programmer's Manual             SORTM(1)



     position.  This does not always work.  For instance, if the
     first message encountered lacks a date which can be parsed,
     then it will usually be placed at the end of the messages
     being sorted.

     When _s_o_r_t_m complains about a message which it can't tem-
     porarlly order, it complains about the message number _p_r_i_o_r
     to sorting.  It should indicate what the message number will
     be _a_f_t_e_r sorting.














































Printed 1/10/87                MH                               2



