


MHOOK(1)            UNIX Programmer's Manual             MHOOK(1)



NAME
     mhook - MH receive-mail hooks

SYNOPSIS
     $HOME/.maildelivery

     /usr/misc/mh/lib/rcvdist [-form formfile]
          [switches for _p_o_s_t_p_r_o_c] address ...  [-help]

     /usr/misc/mh/lib/rcvpack file [-help]

     /usr/misc/mh/lib/rcvtty [command ...] [-help]
9     A receive-mail hook is a program that is run whenever you
     receive a mail message.  You do NOT invoke the hook your-
     self, rather the hook is invoked on your behalf by _S_e_n_d_M_a_i_l,
     when you include the line
              "| /usr/misc/mh/lib/slocal -user $USER"
     in your .forward file in your home directory.

     The ._m_a_i_l_d_e_l_i_v_e_r_y file, which is an ordinary ASCII file,
     controls how local delivery is performed.  This file is read
     by _s_l_o_c_a_l.

     The format of each line in the ._m_a_i_l_d_e_l_i_v_e_r_y file is

          field pattern action result string

     where

       field:
          The name of a field that is to be searched for a pat-
          tern.  This is any field in the headers of the message
          that might be present.  In addition, the following spe-
          cial fields are also defined:
            _s_o_u_r_c_e: the out-of-band sender information
            _a_d_d_r: the address that was used to cause delivery to
            the recipient
            _d_e_f_a_u_l_t: this matches _o_n_l_y if the message hasn't been
            delivered yet
            *: this always matches

      pattern:
         The sequence of characters to match in the specified
         field.  Matching is case-insensitive but not RE-based.

      action:
         The action to take to deliver the message.  This is one
         of

         _f_i_l_e or >:
              Append the message to the file named by string.
              The standard maildrop delivery process is used.  If



Printed 1/10/87                MH                               1


9


MHOOK(1)            UNIX Programmer's Manual             MHOOK(1)



              the message can be appended to the file, then this
              action succeeds.

              When writing to the file, a new field is added:

                   Delivery-Date: date

              which indicates the date and time that message was
              appended to the file.

         _p_i_p_e or |:
              Pipe the message as the standard input to the com-
              mand named by string, using the Bourne shell _s_h (1)
              to interpret the string.  Prior to giving the
              string to the shell, it is expanded with the fol-
              lowing built-in variables:
                $(sender): the return address for the message
                $(address): the address that was used to cause
                delivery to the recipient
                $(size): the size of the message in bytes
                $(reply-to): either the "Reply-To:" or "From:"
                field of the message
                $(info): miscellaneous out-of-band information

             When a process is invoked, its environment is: the
             user/group id:s are set to recipient's id:s; the
             working directory is the recipient's directory; the
             umask is 0077; the process has no /dev/tty; the
             standard input is set to the message; the standard
             output and diagnostic output are set to /dev/null;
             all other file-descriptors are closed; the envari-
             ables $USER, $HOME, $SHELL are set appropriately,
             and no other envariables exist.

             The process is given a certain amount of time to
             execute.  If the process does not exit within this
             limit, the process will be terminated with extreme
             prejudice.  The amount of time is calculated as
             ((size x 60) + 300) seconds, where size is the
             number of bytes in the message.

             The exit status of the process is consulted in
             determining the success of the action.  An exit
             status of zero means that the action succeeded.  Any
             other exit status (or abnormal termination) means
             that the action failed.

             In order to avoid any time limitations, you might
             implement a process that began by _f_o_r_k_i_n_g.  The
             parent would return the appropriate value immedi-
             ately, and the child could continue on, doing what-
             ever it wanted for as long as it wanted.  This



Printed 1/10/87                MH                               2






MHOOK(1)            UNIX Programmer's Manual             MHOOK(1)



             approach is somewhat risky if the parent is going to
             return an exit status of zero.  If the parent is
             going to return a non-zero exit status, then this
             approach can lead to quicker delivery into your
             maildrop.

        _q_p_i_p_e or <_c_a_r_e_t>:
             Similar to _p_i_p_e, but executes the command directly,
             after built-in variable expansion, without assis-
             tance from the shell.

        _d_e_s_t_r_o_y:
             This action always succeeds.

     result:
        Indicates how the action should be performed:

        _A:
             Perform the action.  If the action succeeded, then
             the message is considered delivered.

        _R:
             Perform the action.  Regardless of the outcome of
             the action, the message is not considered delivered.

        ?:
             Perform the action only if the message has not been
             delivered.  If the action succeeded, then the mes-
             sage is considered delivered.

   The file is always read completely, so that several matches
   can be made and several actions can be taken.  The ._m_a_i_l_-
   _d_e_l_i_v_e_r_y file must be owned either by the user or by root, and
   must be writable only by the owner.  If the ._m_a_i_l_d_e_l_i_v_e_r_y file
   can not be found, or does not perform an action which delivers
   the message, then the file /usr/misc/mh/lib/maildelivery is
   read according to the same rules.  This file must be owned by
   the root and must be writable only by the root.  If this file
   can not be found or does not perform an action which delivers
   the message, then standard delivery to the user's maildrop,
   /usr/spool/mail/$USER, is performed.

   Arguments in the ._m_a_i_l_d_e_l_i_v_e_r_y file are separated by
   white-space or comma.  Since double-quotes are honored, these
   characters may be included in a single argument by enclosing
   the entire argument in double-quotes.  A double-quote can be
   included by preceeding it with a backslash.

   To summarize, here's an example:

        #_f_i_e_l_d   _p_a_t_t_e_r_n          _a_c_t_i_o_n  _r_e_s_u_l_t  _s_t_r_i_n_g
        # lines starting with a '#' are ignored, as are blank lines



Printed 1/10/87                MH                               3






MHOOK(1)            UNIX Programmer's Manual             MHOOK(1)



        #
        # file mail with mmdf2 in the "To:" line into file mmdf2.log
        To       mmdf2            file    A       mmdf2.log
        # Messages from mmdf pipe to the program err-message-archive
        From     mmdf             pipe    A       err-message-archive
        # Anything with the "Sender:" address "uk-mmdf-workers"
        # file in mmdf2.log if not filed already
        Sender   uk-mmdf-workers  file    ?       mmdf2.log
        # "To:" unix - put in file unix-news
        To       Unix             >       A       unix-news
        # if the address is jpo=mmdf - pipe into mmdf-redist
        addr     jpo=mmdf         |       A       mmdf-redist
        # if the address is jpo=ack - send an acknowledgement copy back
        addr     jpo=ack          |       R       "resend -r $(reply-to)"
        # anything from steve - destroy!
        From     steve            destroy A       -
        # anything not matched yet - put into mailbox
        default  -                >       ?       mailbox
        # always run rcvalert
        *        -                |       R       rcvalert

   Four programs are currently standardly available, _r_c_v_d_i_s_t
   (redistribute incoming messages to additional recipients),
   _r_c_v_p_a_c_k (save incoming messages in a _p_a_c_k_f'd file), and _r_c_v_t_t_y
   (notify user of incoming messages).  The fourth program,
   _r_c_v_s_t_o_r_e (1) is described separately.  They all reside in the
   /_u_s_r/_m_i_s_c/_m_h/_l_i_b/ directory.

   The _r_c_v_d_i_s_t program will resend a copy of the message to all
   of the addresses listed on its command line.  It uses the for-
   mat string facility described in _m_h-_f_o_r_m_a_t (5).

   The _r_c_v_p_a_c_k program will append a copy of the message to the
   file listed on its command line.  Its use is obsoleted by the
   ._m_a_i_l_d_e_l_i_v_e_r_y.

   The _r_c_v_t_t_y program executes the named file with the message as
   its standard input, and gives the resulting output to the ter-
   minal access daemon for display on your terminal.  If the ter-
   minal access daemon is unavailable on your system, then _r_c_v_t_t_y
   will write the output to your terminal if, and only if, your
   terminal has "world-writable" permission.  If no file is
   specified, or is bogus, etc., then the _r_c_v_t_t_y program will
   give a one-line scan listing to the terminal access daemon.

FILES
     /usr/misc/mh/lib/mtstailor         tailor file
     $HOME/.maildelivery                The file controlling local delivery
     /usr/misc/mh/lib/maildelivery      Rather than the standard file

SEE ALSO
     rcvstore (1)



Printed 1/10/87                MH                               4






MHOOK(1)            UNIX Programmer's Manual             MHOOK(1)



CONTEXT
     None

HISTORY
     For compatibility with older versions of _M_H, if _s_l_o_c_a_l can't
     find the user's ._m_a_i_l_d_e_l_i_v_e_r_y file, it will attempt to exe-
     cute an old-style rcvmail hook in the user's $HOME direc-
     tory.  In particular, it will first attempt to execute

          .mh_receive file maildrop directory user

     failing that it will attempt to execute

          $HOME/bin/rcvmail user file sender

     before giving up and writing to the user's maildrop.

     In addition, whenever a hook or process is invoked,
     file-descriptor three (3) is set to the message in addition
     to the standard input.


BUGS
     Only two return codes are meaningful, others should be.































Printed 1/10/87                MH                               5



