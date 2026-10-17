


VMH(1)              UNIX Programmer's Manual               VMH(1)



NAME
     vmh - visual front-end to MH

SYNOPSIS
     vmh [-prompt string] [-vmhproc program] [-novmhproc]
          [switches for _v_m_h_p_r_o_c] [-help]
9     _v_m_h is a program which implements the server side of the _M_H
     window management protocol and uses _c_u_r_s_e_s (3) routines to
     maintain a split-screen interface to any program which
     implements the client side of the protocol.  This latter
     program, called the _v_m_h_p_r_o_c, is specified using the
     `-vmhproc program' switch.

     The upshot of all this is that one can run _m_s_h on a display
     terminal and get a nice visual interface.  To do this, for
     example, just add the line

          mshproc: vmh

     to your .mh_profile.  (This takes advantage of the fact that
     _m_s_h is the default _v_m_h_p_r_o_c for _v_m_h.)

     In order to facilitate things, if the `-novmhproc' switch is
     given, and _v_m_h can't run on the user's terminal, the _v_m_h_p_r_o_c
     is run directly without the window management protocol.

     After initializing the protocol, _v_m_h prompts the user for a
     command to be given to the client.  Usually, this results in
     output being sent to one or more windows.  If a output to a
     window would cause it to scroll, _v_m_h prompts the user for
     instructions, roughly permitting the capabilities of _l_e_s_s or
     _m_o_r_e (e.g., the ability to scroll backwards and forwards):

          SPACE      advance to the next windowful
          RETURN  *  advance to the next line
          y       *  retreat to the previous line
          d       *  advance to the next ten lines
          u       *  retreat to the previous ten lines
          g       *  go to an arbitrary line
                     (preceed g with the line number)
          G       *  go to the end of the window
                     (if a line number is given, this acts like `g')
          CTRL-L     refresh the entire screen
          h          print a help message
          q          abort the window

     (A `*' indicates that a numeric prefix is meaningful for
     this command.)

     Note that if a command resulted in more than one window's
     worth of information being displayed, and you allow the com-
     mand which is generating information for the window to



Printed 1/10/87                MH                               1


9


VMH(1)              UNIX Programmer's Manual               VMH(1)



     gracefully finish (i.e., you don't use the `q' command to
     abort information being sent to the window), then _v_m_h will
     give you one last change to peruse the window.  This is use-
     ful for scrolling back and forth.  Just type `q' when you're
     done.

     To abnormally terminate _v_m_h (without core dump), use <QUIT>
     (usually CTRL-\).  For instance, this does the "right" thing
     with _b_b_c and _m_s_h.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To determine the user's MH directory

SEE ALSO
     msh(1)

DEFAULTS
     `-prompt (vmh) '
     `-vmhproc msh'

CONTEXT
     None

BUGS
     The argument to the `-prompt' switch must be interpreted as
     a single token by the shell that invokes _v_m_h.  Therefore,
     one must usually place the argument to this switch inside
     double-quotes.

     At present, there is no way to pass signals (e.g., inter-
     rupt, quit) to the client.  However, generating QUIT when
     _v_m_h is reading a command from the terminal is sufficient to
     tell the client to go away quickly.

     Acts strangely (loses peer or botches window management pro-
     tocol with peer) on random occasions.
















Printed 1/10/87                MH                               2



