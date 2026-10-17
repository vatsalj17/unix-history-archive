


MH-ALIAS(5)         UNIX Programmer's Manual          MH-ALIAS(5)



NAME
     mh-alias - alias file for MH message system

SYNOPSIS
     any _M_H command
9     This describes both _M_H personal alias files and the (pri-
     mary) alias file for mail delivery, the file

          /usr/misc/mh/lib/MailAliases

     It does not describe aliases files used by the message tran-
     sport system.  Each line of the alias file has the format:

          alias : address-group
     or
          alias ; address-group
     or
          < alias-file

     where:

          address-group  :=  address-list
                      |   "<" file
                      |   "=" UNIX-group
                      |   "+" UNIX-group
                      |   "*"

          address-list   :=  address
                      |   address-list, address

     Continuation lines in alias files end with `\' followed by
     the newline character.

     Alias-file and file are UNIX file names.  UNIX-group is a
     group name (or number) from /_e_t_c/_g_r_o_u_p.  An address is a
     "simple" Internet-style address.  Througout this file, case
     is ignored, except for alias-file names.

     If the line starts with a `<', then the file named after the
     `<' is read for more alias definitions.  The reading is done
     recursively, so a `<' may occur in the beginning of an alias
     file with the expected results.

     If the address-group starts with a `<', then the file named
     after the `<' is read and its contents are added to the
     address-list for the alias.

     If the address-group starts with an `=', then the file
     /_e_t_c/_g_r_o_u_p is consulted for the UNIX-group named after the
     `='.  Each login name occurring as a member of the group is
     added to the address-list for the alias.
9


Printed 1/10/87                MH                               1






MH-ALIAS(5)         UNIX Programmer's Manual          MH-ALIAS(5)



     In contrast, if the address-group starts with a `+', then
     the file /_e_t_c/_g_r_o_u_p is consulted to determine the group-id
     of the UNIX-group named after the `+'.  Each login name
     occurring in the /_e_t_c/_p_a_s_s_w_d file whose group-id is indi-
     cated by this group is added to the address-list for the
     alias.

     If the address-group is simply `*', then the file
     /_e_t_c/_p_a_s_s_w_d is consulted and all login names with a userid
     greater than some magic number (usually 200) are added to
     the address-list for the alias.

     In match, a trailing * on an alias will match just about
     anything appropriate.  (See example below.)

     An approximation of the way aliases are resolved at posting
     time is (it's not really done this way):

          1) Build a list of all addresses from the message to be
          delivered, eliminating duplicate addresses.

          2) If this draft originated on the local host, then for
          those addresses in the message that have no host speci-
          fied, perform alias resolution.

          3) For each line in the alias file, compare "alias"
          against all of the existing addresses.  If a match,
          remove the matched "alias" from the address list, and
          add each new address in the address-group to the
          address list if it is not already on the list.  The
          alias itself is not usually output, rather the
          address-group that the alias maps to is output instead.
          If "alias" is terminated with a `;' instead of a `:',
          then both the "alias" and the address are output in the
          correct format.  (This makes replies possible since _M_H
          aliases and personal aliases are unknown to the mail
          transport system.)

     Since the alias file is read line by line, forward refer-
     ences work, but backward references are not recognized,
     thus, there is no recursion.

     Example:
          </usr/misc/mh/lib/BBoardAliases
          sgroup: fred, fear, freida
          fred: frated@UCI
          UNIX-committee: <unix.aliases
          staff: =staff
          wheels: +wheel
          everyone: *
          news.*: news




Printed 1/10/87                MH                               2






MH-ALIAS(5)         UNIX Programmer's Manual          MH-ALIAS(5)



     The first line says that more aliases should immediately be
     read from the file /_u_s_r/_m_i_s_c/_m_h/_l_i_b/_B_B_o_a_r_d_A_l_i_a_s_e_s.  Follow-
     ing this, "fred" is defined as an alias for "frated@UCI",
     and "sgroup" is defined as an alias for the three names
     "frated@UCI", "fear", and "freida".  Next, the definition of
     "UNIX-committee" is given by reading the file _u_n_i_x._a_l_i_a_s_e_s
     in the users _M_H directory, "staff" is defined as all users
     who are listed as members of the group "staff" in the
     /_e_t_c/_g_r_o_u_p file, and "wheels" is defined as all users whose
     group-id in /_e_t_c/_p_a_s_s_w_d is equivalent to the "wheel" group.
     Finally, "everyone" is defined as all users with a user-id
     in /_e_t_c/_p_a_s_s_w_d greater than 200, and all aliases of the form
     "news.<anything>" are defined to be "news".

     The key thing to understand about aliasing in _M_H is that
     aliases in _M_H alias files are expanded into the headers of
     messages posted.  This aliasing occurs first, at posting
     time, without the knowledge of the message transport system.
     In contrast, once the message transport system is given a
     message to deliver to a list of addresses, for each address
     that appears to be local, a system-wide alias file is con-
     sulted.  These aliases are NOT expanded into the headers of
     messages delivered.

HELPFUL HINTS
     To use aliasing in _M_H quickly, do the following:

          First, in your ._m_h__p_r_o_f_i_l_e, choose a name for your pri-
          mary alias file, say "aliases", and add three lines:

               ali: -alias aliases
               send: -alias aliases
               whom: -alias ailases

          Second, create the file "aliases" in your _M_H directory.

          Third, start adding aliases to your "aliases" file as
          appropriate.

FILES
     /usr/misc/mh/lib/MailAliases       Primary alias file

PROFILE COMPONENTS
     None

SEE ALSO
     ali(1), send(1), whom(1), group(5), passwd(5), conflict(8),
     post(8)

DEFAULTS
     None




Printed 1/10/87                MH                               3






MH-ALIAS(5)         UNIX Programmer's Manual          MH-ALIAS(5)



CONTEXT
     None

HISTORY
     In previous releases of _M_H, only a single, system-wide
     mh-alias file was supported.  This led to a number of prob-
     lems, since only mail-system administrators were capable of
     (un)defining aliases.  Hence, the semantics of mh-alias were
     extended to support personal alias files.  Users of _M_H no
     longer need to bother mail-system administrators for keeping
     information in the system-wide alias file, as each _M_H user
     can create/modify/remove aliases at will from any number of
     personal files.

BUGS
     Although the forward-referencing semantics of _m_h-_a_l_i_a_s files
     prevent recursion, the "< alias-file" command may defeat
     this.  Since the number of file descriptors is finite (and
     very limited), such infinite recursion will terminate with a
     meaningless diagnostic when all the fds are used up.



































Printed 1/10/87                MH                               4



