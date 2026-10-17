


ALI(1)              UNIX Programmer's Manual               ALI(1)



NAME
     ali - list mail aliases

SYNOPSIS
     ali [-alias aliasfile] [-list] [-nolist] [-normalize]
          [-nonormalize] [-user] [-nouser] aliases ...  [-help]
9     _A_l_i searches the named mail alias files for each of the
     given _a_l_i_a_s_e_s.  It creates a list of addresses for those
     _a_l_i_a_s_e_s, and writes that list on standard output.  If the
     `-list' option is specified, each address appears on a
     separate line; otherwise, the addresses are separated by
     commas and printed on as few lines as possible.

     The `-user' option directs _a_l_i to perform its processing in
     an inverted fashion: instead of listing the addresses that
     each given alias expands to, _a_l_i will list the aliases that
     expand to each given address.  If the `-normalize' switch is
     given, _a_l_i will try to track down the official hostname of
     the address.

     Each _a_l_i_a_s is processed as described in _m_h-_a_l_i_a_s (5).

FILES
     $HOME/.mh_profile                  The user profile
     /etc/passwd                        List of users
     /etc/group                         List of groups

PROFILE COMPONENTS
     None

SEE ALSO
     mh-alias(5)

DEFAULTS
     `-alias /usr/misc/mh/lib/MailAliases'
     `-nolist'
     `-nonormalize'
     `-nouser'

CONTEXT
     None

BUGS
     The `-user' option with `-nonormalize' is not entirely accu-
     rate, as it does not replace local nicknames for hosts with
     their official site names.





9


Printed 1/10/87                MH                               1



