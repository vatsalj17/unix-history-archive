


MHMAIL(1)           UNIX Programmer's Manual            MHMAIL(1)



NAME
     mhmail - send or read mail

SYNOPSIS
     mhmail [ addrs ... [-body text] [-cc addrs ...] [-from addr]
          [-subject subject]] [-help]
9     _M_H_m_a_i_l is intended as a replacement for the standard Bell
     mail program (_b_e_l_l_m_a_i_l (1)), compatible with _M_H.  When
     invoked without arguments, it simply invokes _i_n_c (1) to
     incorporate new messages from the user's maildrop.  When one
     or more users is specified, a message is read from the stan-
     dard input and spooled to a temporary file.  _M_H_m_a_i_l then
     invokes _p_o_s_t (8) with the name of the temporary file as its
     argument to deliver the message to the specified user.

     The `-subject subject' switch can be used to specify the
     "Subject:" field of the message.  The `-body text' switch
     can be used to specify the text of the message; if it is
     specified, then the standard input is not read.  Normally,
     addresses appearing as arguments are put in the "To:" field.
     If the `-cc' switch is used, all addresses following it are
     placed in the "cc:" field.

     By using `-from addr', you can specify the "From:" header of
     the draft.  Naturally, _p_o_s_t will fill-in the "Sender:"
     header correctly.

     This program is intended for the use of programs such as
     _a_t (1), which expect to send mail automatically to various
     users.  Normally, real people (as opposed to the "unreal"
     ones) will prefer to use _c_o_m_p (1) and _s_e_n_d (1) to send mes-
     sages.

FILES
     /usr/misc/mh/inc                   Program to incorporate a maildrop into a folder
     /usr/misc/mh/lib/post              Program to deliver a message
     /tmp/mhmail*                       Temporary copy of message

PROFILE COMPONENTS
     None

SEE ALSO
     inc(1), post(8)

DEFAULTS
     None

CONTEXT
     If _i_n_c is invoked, then _i_n_c's context changes occur.


9


Printed 1/10/87                MH                               1



