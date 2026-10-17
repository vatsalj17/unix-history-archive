


3270(1)             UNIX Programmer's Manual              3270(1)



NAME
     3270 - IBM 3277/3278 terminal emulator
     mset - set terminal keyboard mapping

SYNOPSIS
     3270 host
     mset [ name ]

DESCRIPTION
     _3_2_7_0 makes an ordinary ASCII terminal appear to be an IBM
     3278 terminal and allows full-screen interaction with any
     VM/CMS system over a TCP/IP connection using the Telnet pro-
     tocol as implemented by the WISCNET software for VM/CMS.
     The _3_2_7_0 program uses _c_u_r_s_e_s(3) to redraw the the screen
     according to the 3270 protocol, and allows escape sequences
     to represent special function keys present on 3270 terminals
     but not on most ASCII terminals.  The program may be viewed
     as a special-purpose version of _t_e_l_n_e_t(1) which understands
     3270 terminals, and it is similar to _t_n_3_2_7_0(1) from Berke-
     ley, but it was written from scratch and does not share any
     code with either of these programs.

ENVIRONMENT
     The specifcation of escape screen-drawing sequences is taken
     from the _T_E_R_M and _T_E_R_M_C_A_P variables as explained in
     _t_e_r_m_c_a_p(3X).  Similarly, the specification of keyboard
     escape sequences is taken from the _T_E_R_M and _M_A_P_3_2_7_0 environ-
     ment variables, allowing the user to customize input escape
     seqences according to personal taste or to accomodate spe-
     cial keys on a terminal that transmit escape sequences.  In
     more detail, the algorithm is as follows: If _M_A_P_3_2_7_0 is
     defined and the value does not start with `/', the value is
     taken to be a definition, as defined below under PROFILE.
     Otherwise, a database of descriptions is searched for an
     item matching the value of the _T_E_R_M environment variable.
     The database is the file named by the _M_A_P_3_2_7_0 variable, if
     it is defined and begins with `/'; otherwise is a default
     database in an installation-dependent location, normally
     /_u_s_r/_l_o_c_a_l/_3_2_7_0_p_r_o_f_i_l_e.  If the named terminal is not found
     in the database, the database is searched again for the spe-
     cial name `unknown'.  If that name is not found, a built-in
     terminal definition is used (see below).

     The program _m_s_e_t can be used to search for a definition (as
     described above) and put it into the environment variable
     _M_A_P_3_2_7_0 thus speeding startup of _3_2_7_0.  The following com-
     mand does the trick for _c_s_h(1) (sorry, no support for
     _s_h(1)).

          set noglob; eval `mset`





Printed 1/10/87           22 March 1985                         1






3270(1)             UNIX Programmer's Manual              3270(1)



     _3_2_7_0 determines the number of lines on the display from the
     `li' termcap capability (or from the terminal driver on 4.3
     BSD systems) and attempts to emulate the biggest model of
     3270 terminal possible (24, 32, or 43 lines).

PROFILE
     The format of `profile' file is the same as `map' files used
     by the Berkeley tn3270 program:

     Everything inside a `string' (see below) is left `as is'
     (but newlines inside strings are not allowed).  Otherwise,
     whitespace is ignored and everything from a `#' up to end-
     of-line is ignored.  After this space-and-comment stripping,
     the format of a file is

         (name ("|" name)* definition)+

     where `definition' is
         "{" (action "=" string ( "|" string )* ";")* "}"

     where `name' is a terminal name (sequence of alphanumerics),
     `action' is one of the action names described below, and
     `string' is similar in format to a C string:

         "'" ( "^" char
             | "\" ("E" | "n" | "r" | "b" | "t" | "f" | "v" | "'")
             | "\" octdigit [ octdigit [octdigit]]
             | otherchar
             )* "'"

     The meanings of the escapes are
         "^" char = (char=='?' ? '\177' : (c & 037))
             (NB: even if 'char' is "'")
         "\E" = escape (^[, \033)
         "\n" = newline (^J, \012)
         "\t" = horizontal tab (^I, \011)
         "\r" = return (^M, \015)
         "\b" = backspace (^H, \010)
         "\f" = formfeed (^L, \014)
         "\v" = vertical tab (^K, \013)
         (the last three are not recognized by tn3270)
         "\" num = character with value 'num', where num is a one-, two-, or
             three-digit octal number (this is also an extension over tn3270)
         In all other cases
         "\" char = char
             (including the cases char="\" and char="'")

     It is illegal for any string to be prefix (or equal to)
     another in any one definition.

ACTIONS
     The following actions names are recognized.  Note that not



Printed 1/10/87           22 March 1985                         2






3270(1)             UNIX Programmer's Manual              3270(1)



     all actions are currently implemented.  An escape sequence
     that corresponds to an unimplemented action causes the ter-
     minal bell to ring.

     The following correspond to keys on the 3270 keyboard:
          enter,
          pfk1, pfk2, ..., pfk24  (`PF' keys)
          clear
          pa1, pa2, pa3
          nl (`newline' or `elbow' key)
          left, down, up, right (arrow keys)
          insrt (toggles insert mode)
          delete (delete character)
          eeof (erase to end-of-field)
          tab
          btab (back-tab)

     The following are currently all the same: They cause the
     screen to be redrawn

          reset, master_reset, reshow

     The following are currently unimplemented

          home, einp, dp, fm, cursel, flinp, escape,
          settab, clrtab, setmrg, sethom,
          coltab, colbak, indent, undent

     A sample entry (and the one used if no definition can be
     found in the database) is as follows:
     unknown {
          enter='^m'; clear='^a'; nl='^p'; tab='^i'; btab='^u';
          left='^h'; right='^l'; up='^k'; down='^j';
          delete='^x'; eeof='^f'; insrt='^w';
          pfk1='\E1'; pfk2='\E2'; pfk3='\E3'; pfk4='\E4';
          pfk5='\E5'; pfk6='\E6'; pfk7='\E7'; pfk8='\E8';
          pfk9='\E9'; pfk10='\E0'; pfk11='\E-'; pfk12='\E=';
          pfk13='\Eq'; pfk14='\Ew'; pfk15='\Ee'; pfk16='\Er';
          pfk17='\Et'; pfk18='\Ey'; pfk19='\Eu'; pfk20='\Ei';
          pfk21='\Eo'; pfk22='\Ep'; pfk23='\E['; pfk24='\E]';
          pa1='\Ea'; pa2='\Es'; pa3='\Ed'; reshow='^v';
     }

FILES
     /etc/termcap
     /usr/local/3270profile

AUTHOR
     Marvin Solomon

SEE ALSO
     telnet(1), termcap(3x), termcap(5), tn3270(5), curses(3x)



Printed 1/10/87           22 March 1985                         3






3270(1)             UNIX Programmer's Manual              3270(1)



BUGS
     Not all 3270 functions are supported, nor all tn3270
     features, nor all telnet commands.

     _3_2_7_0 cannot be used as a replacement for _t_e_l_n_e_t.

     The program runs in _c_b_r_e_a_k mode, so some control charcters
     (such as stop, start, and interrupt) are caught by the ter-
     minal driver, and cannot be used in escape sequences.














































Printed 1/10/87           22 March 1985                         4



