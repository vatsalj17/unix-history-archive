


MH-MAIL(5)          UNIX Programmer's Manual           MH-MAIL(5)



NAME
     mh-mail - message format for MH message system

SYNOPSIS
     any _M_H command
9     _M_H processes messages in a particular format.  It should be
     noted that although neither Bell nor Berkeley mailers pro-
     duce message files in the format that _M_H prefers, _M_H can
     read message files in that antiquated format.

     Each user possesses a mail drop box which initially receives
     all messages processed by _p_o_s_t (8).  _I_n_c (1) will read from
     that drop box and incorporate the new messages found there
     into the user's own mail folders (typically `+inbox').  The
     mail drop box consists of one or more messages.

     Messages are expected to consist of lines of text.  Graphics
     and binary data are not handled.  No data compression is
     accepted.  All text is clear ASCII 7-bit data.

     The general "memo" framework of RFC-822 is used.  A message
     consists of a block of information in a rigid format, fol-
     lowed by general text with no specified format.  The rigidly
     formatted first part of a message is called the header, and
     the free-format portion is called the body.  The header must
     always exist, but the body is optional.  These parts are
     separated by an empty line, i.e., two consecutive newline
     characters.  Within _M_H, the header and body may be separated
     by a line consisting of dashes:

          To:
          cc:
          Subject:
          --------

     The header is composed of one or more header items.  Each
     header item can be viewed as a single logical line of ASCII
     characters.  If the text of a header item extends across
     several real lines, the continuation lines are indicated by
     leading spaces or tabs.

     Each header item is called a component and is composed of a
     keyword or name, along with associated text.  The keyword
     begins at the left margin, may NOT contain spaces or tabs,
     may not exceed 63 characters (as specified by RFC-822), and
     is terminated by a colon (`:').  Certain components (as
     identified by their keywords) must follow rigidly defined
     formats in their text portions.

     The text for most formatted components (e.g., "Date:" and
     "Message-Id:") is produced automatically.  The only ones
     entered by the user are address fields such as "To:", "cc:",



Printed 1/10/87                MH                               1


9


MH-MAIL(5)          UNIX Programmer's Manual           MH-MAIL(5)



     etc.  Internet addresses are assigned mailbox names and host
     computer specifications.  The rough format is
     "local@domain", such as "MH@UCI", or "MH@UCI-ICSA.ARPA".
     Multiple addresses are separated by commas.  A missing
     host/domain is assumed to be the local host/domain.

     As mentioned above, a blank line (or a line of dashes) sig-
     nals that all following text up to the end of the file is
     the body.  No formatting is expected or enforced within the
     body.

     Following is a list of header components that are considered
     meaningful to various MH programs.
     Date:
          Added by _p_o_s_t (8), contains date and time of the
          message's entry into the transport system.

     From:
          Added by _p_o_s_t (8), contains the address of the author
          or authors (may be more than one if a "Sender:" field
          is present).  Replies are typically directed to
          addresses in the "Reply-To:" or "From:" field (the
          former has precedence if present).

     Sender:
          Added by _p_o_s_t (8) in the event that the message already
          has a "From:" line.  This line contains the address of
          the actual sender.  Replies are never sent to addresses
          in the "Sender:" field.

     To:
          Contains addresses of primary recipients.

     cc:
          Contains addresses of secondary recipients.

     Bcc:
          Still more recipients.  However, the "Bcc:" line is not
          copied onto the message as delivered, so these reci-
          pients are not listed.  _M_H uses an encapsulation method
          for blind copies, see _s_e_n_d (1).

     Fcc:
          Causes _p_o_s_t (8) to copy the message into the specified
          folder for the sender, if the message was successfully
          given to the transport system.

     Message-ID:
          A unique message identifier added by _p_o_s_t (8) if the
          `-msgid' flag is set.

     Subject:



Printed 1/10/87                MH                               2






MH-MAIL(5)          UNIX Programmer's Manual           MH-MAIL(5)



          Sender's commentary.  It is displayed by _s_c_a_n (1).

     In-Reply-To:
          A commentary line added by _r_e_p_l (1) when replying to a
          message.

     Resent-Date:
          Added when redistributing a message by _p_o_s_t (8).

     Resent-From:
          Added when redistributing a message by _p_o_s_t (8).

     Resent-To:
          New recipients for a message resent by _d_i_s_t (1).

     Resent-cc:
          Still more recipients.  See "cc:" and "Resent-To:".

     Resent-Bcc:
          Even more recipients.  See "Bcc:" and "Resent-To:".

     Resent-Fcc:
          Copy resent message into a folder.  See "Fcc:" and
          "Resent-To:".

     Resent-Message-Id:
          A unique identifier glued on by _p_o_s_t (8) if the
          `-msgid' flag is set.  See "Message-Id:" and
          "Resent-To:".

     Resent:
          Annotation for _d_i_s_t (1) under the `-annotate' option.

     Forwarded:
          Annotation for _f_o_r_w (1) under the `-annotate' option.

     Replied:
          Annotation for _r_e_p_l (1) under the `-annotate' option.


FILES
     /usr/spool/mail/$USER              Location of mail drop

PROFILE COMPONENTS
     None

SEE ALSO
     _S_t_a_n_d_a_r_d _f_o_r _t_h_e _F_o_r_m_a_t _o_f _A_R_P_A _I_n_t_e_r_n_e_t _T_e_x_t _M_e_s_s_a_g_e_s (aka
     RFC-822)

DEFAULTS
     None



Printed 1/10/87                MH                               3






MH-MAIL(5)          UNIX Programmer's Manual           MH-MAIL(5)



CONTEXT
     None





















































Printed 1/10/87                MH                               4



