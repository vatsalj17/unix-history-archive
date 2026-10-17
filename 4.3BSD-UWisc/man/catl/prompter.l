


PROMPTER(1)         UNIX Programmer's Manual          PROMPTER(1)



NAME
     prompter - prompting editor front-end

SYNOPSIS
     prompter [-erase chr] [-kill chr] [-prepend] [-noprepend]
          [-rapid] [-norapid] file [-help]
9     This program is normally not invoked directly by users but
     takes the place of an editor and acts as an editor
     front-end.  It operates on an 822-style message draft skele-
     ton specified by file, normally provided by _c_o_m_p, _d_i_s_t,
     _f_o_r_w, or _r_e_p_l.

     _P_r_o_m_p_t_e_r is an editor which allows rapid composition of mes-
     sages.  It is particularly useful to network and low-speed
     (less than 2400 baud) users of _M_H.  It is an _M_H program in
     that it can have its own profile entry with switches, but it
     is not invoked directly by the user.  The commands _c_o_m_p,
     _d_i_s_t, _f_o_r_w, and _r_e_p_l invoke _p_r_o_m_p_t_e_r as an editor, either
     when invoked with `-editor prompter', or by the profile
     entry "Editor: prompter", or when given the command
     `edit prompter' at "What now?" level.

     For each empty component _p_r_o_m_p_t_e_r finds in the draft, the
     user is prompted for a response; A <RETURN> will cause the
     whole component to be left out.  Otherwise, a `\' preceding
     a <RETURN> will continue the response on the next line,
     allowing for multiline components.  Continuation lines must
     begin with a space or tab.

     Each non-empty component is copied to the draft and
     displayed on the terminal.

     The start of the message body is denoted by a blank line or
     a line of dashes.  If the body is non-empty, the prompt,
     which isn't written to the file, is

         "--------Enter additional text",

     or (if `-prepend' was given)

         "--------Enter initial text".

     Message-body typing is terminated with an end-of-file (usu-
     ally CTRL-D).  At this point control is returned to the cal-
     ling program, where the user is asked "What now?".  See
     _w_h_a_t_n_o_w for the valid options to this query.

     By using the `-prepend' switch, the user can add type-in to
     the beginning of the message body and have the rest of the
     body follow.  This is useful for the _f_o_r_w command.

     By using the `-rapid' switch, if the draft already contains



Printed 1/10/87                MH                               1


9


PROMPTER(1)         UNIX Programmer's Manual          PROMPTER(1)



     text in the message-body, it is not displayed on the user's
     terminal.  This is useful for low-speed terminals.

     The line editing characters for kill and erase may be speci-
     fied by the user via the arguments `-kill chr' and
     `-erase chr', where chr may be a character; or `\nnn', where
     "nnn" is the octal value for the character.

     An interrupt (usually CTRL-C) during component typing will
     abort _p_r_o_m_p_t_e_r and the _M_H command that invoked it.  An
     interrupt during message-body typing is equivalent to
     CTRL-D, for historical reasons.  This means that _p_r_o_m_p_t_e_r
     should finish up and exit.

     The first non-flag argument to _p_r_o_m_p_t_e_r is taken as the name
     of the draft file, and subsequent non-flag arguments are
     ignored.

FILES
     $HOME/.mh_profile                  The user profile
     /tmp/prompter*                     Temporary copy of message

PROFILE COMPONENTS
     prompter-next:       To name the editor to be used on exit from _p_r_o_m_p_t_e_r
     Msg-Protect:         To set mode when creating a new draft

SEE ALSO
     comp(1), dist(1), forw(1), repl(1), whatnow(1)

DEFAULTS
     `-prepend'
     `-norapid'

CONTEXT
     None

BUGS
     _P_r_o_m_p_t_e_r uses _s_t_d_i_o (3), so it will lose if you edit files
     with nulls in them.
















Printed 1/10/87                MH                               2



