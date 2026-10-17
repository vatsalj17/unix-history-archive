


INSTALL-MH(8)       UNIX Programmer's Manual        INSTALL-MH(8)



NAME
     install-mh - initialize the MH environment

SYNOPSIS
     /usr/misc/mh/lib/install-mh [-auto] [-compat]
9     When a user runs any _M_H program for the first time, the pro-
     gram will invoke _i_n_s_t_a_l_l-_m_h (with the `-auto' switch) to
     query the user for the initial _M_H environment.  The user
     does NOT invoke this program directly.  The user is asked
     for the name of the directory that will be designated as the
     user's _M_H directory.  If this directory does not exist, the
     user is asked if it should be created.  Normally, this
     directory should be under the user's home directory, and has
     the default name of Mail/.  After _i_n_s_t_a_l_l-_m_h has written the
     initial .mh_profile for the user, control returns to the
     original _M_H program.

     As with all _M_H commands, _i_n_s_t_a_l_l-_m_h first consults the $HOME
     envariable to determine the user's home directory.  If $HOME
     is not set, then the /_e_t_c/_p_a_s_s_w_d file is consulted.

     When converting from _m_h._3 to _m_h._4, _i_n_s_t_a_l_l-_m_h is automati-
     cally invoked with the `-compat' switch.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To set the user's MH directory

CONTEXT
     With `-auto', the current folder is changed to "inbox".



















9


Printed 1/10/87                MH                               1



