


MH(1)               UNIX Programmer's Manual                MH(1)



NAME
     mh - Message Handler

SYNOPSIS
     any _M_H command
9     _M_H is the name of a powerful message handling system.
     Rather then being a single comprehensive program, _M_H con-
     sists of a collection of fairly simple single-purpose pro-
     grams to send, receive, save, and retrieve messages.  The
     user should refer to the _M_H _U_s_e_r'_s _M_a_n_u_a_l and the pages for
     the _M_H programs in the Unix Programmers Manual.

     Unlike _m_a_i_l, the standard UNIX mail user interface program,
     _M_H is not a closed system which must be explicitly run, then
     exited when you wish to return to the shell.  You may freely
     intersperse _M_H commands with other shell commands, allowing
     you to read and answer your mail while you have (for exam-
     ple) a compilation running, or search for a file or run pro-
     grams as needed to find the answer to someone's question
     before answering their mail.

     The rest of this manual entry is a quick tutorial which will
     teach you the basics of _M_H.  You should read the manual
     entries for the individual programs for complete documenta-
     tion.

     To get started using _M_H, put the directory /usr/misc/mh on
     your $PATH.  This is best done in one of the files: .pro-
     file, .login, or .cshrc in your home directory.  (Check the
     manual entry for the shell you use, in case you don't know
     how to do this.) Run the _i_n_c command.  If you've never used
     _M_H before, it will create the necessary default files and
     directories after asking you if you wish it to do so.

     _i_n_c moves mail from your system maildrop into your _M_H
     `+inbox' folder, breaking it up into separate files and con-
     verting it to _M_H format as it goes.  It prints one line for
     each message it processes, containing the from field, the
     subject field and as much of the first line of the message
     as will fit.  It leaves the first message it processes as
     your current message.  You'll need to run _i_n_c each time you
     wish to incorporate new mail into your _M_H file.

     _s_c_a_n prints a list of the messages in your current folder.

     The commands: _s_h_o_w, _n_e_x_t, and _p_r_e_v are used to read specific
     messages from the current folder.  _s_h_o_w displays the current
     message, or a specific message, which may be specified by
     its number, which you pass as an argument to _s_h_o_w.  _n_e_x_t and
     _p_r_e_v display, respectively, the message numerically after or
     before the current message.  In all cases, the message
     displayed becomes the current message.  If there is no



Printed 1/10/87                MH                               1


9


MH(1)               UNIX Programmer's Manual                MH(1)



     current message, _s_h_o_w may be called with an argument, or
     _n_e_x_t may be used to advance to the first message.

     _r_m_m (remove message) deletes the current message.  It may be
     called with message numbers passed as arguments, to delete
     specific messages.

     _r_e_p_l is used to respond to the current message (by default).
     It places you in the editor with a prototype response form.
     While you're in the editor, you may peruse the item you're
     responding to by reading the file @.  After completing your
     response, type l to review it, or s to send it.

     _c_o_m_p allows you to compose a message by putting you in the
     editor on a prototype message form, and then lets you send
     it.

     All the _M_H commands may be run with the single argument:
     `-help', which causes them to print a list of the arguments
     they may be invoked with.

     Commands which take a message number as an argument (_s_c_a_n,
     _s_h_o_w, _r_e_p_l, ...) also take one of the words: _f_i_r_s_t, _p_r_e_v,
     _c_u_r, _n_e_x_t, or _l_a_s_t to indicate (respectively) the first,
     previous, current, next, or last message in the current
     folder (assuming they are defined).

     Commands which take a range of message numbers (_r_m_m, _s_c_a_n,
     _s_h_o_w, ...) also take any of the abbreviations:

       <_n_u_m_1>-<_n_u_m_2> - Indicates all messages in the range <num1>
          to <num2>, inclusive. The range must be nonempty.

       <_n_u_m>:+_N
       <_n_u_m>:-_N - Up to _N messages beginning with (or ending
          with) message _n_u_m. _N_u_m may be any of the pre-defined
          symbols: _f_i_r_s_t, _p_r_e_v, _c_u_r, _n_e_x_t or _l_a_s_t.

       _f_i_r_s_t:_N
       _p_r_e_v:_N
       _n_e_x_t:_N
       _l_a_s_t:_N - The first, previous, next or last _N messages, if
          they exist.

     There are many other possibilities such as creating multiple
     folders for different topics, and automatically refiling
     messages according to subject, source, destination, or con-
     tent.  These are beyond the scope of this manual entry.

     Following is a list of all the _M_H commands:

     ali (1)        - list mail aliases



Printed 1/10/87                MH                               2






MH(1)               UNIX Programmer's Manual                MH(1)



     anno (1)       - annotate messages
     burst (1)      - explode digests into messages
     comp (1)       - compose a message
     dist (1)       - redistribute a message to additional addresses
     folder (1)     - set/list current folder/message
     folders (1)    - list all folders
     forw (1)       - forward messages
     inc (1)        - incorporate new mail
     mark (1)       - mark messages
     mhl (1)        - produce formatted listings of MH messages
     mhmail (1)     - send or read mail
     mhook (1)      - MH receive-mail hooks
     mhpath (1)     - print full pathnames of MH messages and folders
     msgchk (1)     - check for messages
     msh (1)        - MH shell (and BBoard reader)
     next (1)       - show the next message
     packf (1)      - compress a folder into a single file
     pick (1)       - select messages by content
     prev (1)       - show the previous message
     prompter (1)   - prompting editor front end
     rcvstore (1)   - incorporate new mail asynchronously
     refile (1)     - file messages in other folders
     repl (1)       - reply to a message
     rmf (1)        - remove folder
     rmm (1)        - remove messages
     scan (1)       - produce a one line per message scan listing
     send (1)       - send a message
     show (1)       - show (list) messages
     sortm (1)      - sort messages
     vmh (1)        - visual front-end to MH
     whatnow (1)    - prompting front-end for send
     whom (1)       - report to whom a message would go

     mh-alias (5)   - alias file for MH message system
     mh-format (5)  - format file for MH message system
     mh-mail (5)    - message format for MH message system
     mh-profile (5) - user customization for MH message system

     ap (8)         - parse addresses 822-style
     conflict (8)   - search for alias/password conflicts
     dp (8)         - parse dates 822-style
     install-mh (8) - initialize the MH environment
     post (8)       - deliver a message


FILES
     /usr/misc/mh                       directory containing _M_H commands
     /usr/misc/mh/lib                   _M_H library

SEE ALSO
     _T_h_e _R_a_n_d _M_H Message Handling System: User's Manual,
     _T_h_e _R_a_n_d _M_H Message Handling System: Tutorial,



Printed 1/10/87                MH                               3






MH(1)               UNIX Programmer's Manual                MH(1)



     _T_h_e _R_a_n_d _M_H Message Handling System: The UCI BBoards Facil-
     ity,
     _M_H._5: _H_o_w _t_o _p_r_o_c_e_s_s _2_0_0 _m_e_s_s_a_g_e_s _a _d_a_y _a_n_d _s_t_i_l_l _g_e_t _s_o_m_e
     _r_e_a_l _w_o_r_k _d_o_n_e



















































Printed 1/10/87                MH                               4



