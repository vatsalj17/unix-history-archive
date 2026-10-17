


MARK(1)             UNIX Programmer's Manual              MARK(1)



NAME
     mark - mark messages

SYNOPSIS
     mark [+folder] [msgs] [-sequence name ...] [-add] [-delete]
          [-list] [-public] [-nopublic] [-zero] [-nozero] [-help]
9     The _m_a_r_k command manipulates message sequences by adding or
     deleting message numbers from folder-specific message
     sequences, or by listing those sequences and messages.  A
     message sequence is a keyword, just like one of the
     "reserved" message names, such as "first" or "next".  Unlike
     the "reserved" message names, which have a fixed semantics
     on a per-folder basis, the semantics of a message sequence
     may be defined, modified, and removed by the user.  Message
     sequences are folder-specific, e.g., the sequence name
     "seen" in the context of folder "+inbox" need not have any
     relation whatsoever to the sequence of the same name in a
     folder of a different name.

     Three action switches direct the operation of _m_a_r_k.  These
     switches are mutually exclusive: the last occurrence of any
     of them overrides any previous occurrence of the other two.

     The `-add' switch tells _m_a_r_k to add messages to sequences or
     to create a new sequence.  For each sequence named via the
     `-sequence name' argument (which must occur at least once)
     the messages named via `msgs' (which defaults to "cur" if no
     `msgs' are given), are added to the sequence.  The messages
     to be added need not be absent from the sequence.  If the
     `-zero' switch is specified, the sequence will be emptied
     prior to adding the messages.  Hence, `-add -zero' means
     that each sequence should be initialized to the indicated
     messages, while `-add -nozero' means that each sequence
     should be appended to by the indicated messages.

     The `-delete' switch tells _m_a_r_k to delete messages from
     sequences, and is the dual of `-add'.  For each of the named
     sequences, the named messages are removed from the sequence.
     These messages need not be already present in the sequence.
     If the `-zero' switch is specified, then all messages in the
     folder are appended to the sequence prior to removing the
     messages.  Hence, `-delete -zero' means that each sequence
     should contain all messages except those indicated, while
     `-delete -nozero' means that only the indicated messages
     should be removed from each sequence.  As expected, the com-
     mand `mark -sequence seen -delete all' deletes the sequence
     "seen" from the current folder.

     When creating (or modifying) a sequence, the `-public'
     switch indicates that the sequence should be made readable
     for other _M_H users.  In contrast, the `-nopublic' switch
     indicates that the sequence should be private to the user's



Printed 1/10/87                MH                               1


9


MARK(1)             UNIX Programmer's Manual              MARK(1)



     _M_H environment.

     The `-list' switch tells _m_a_r_k to list both the sequences
     defined for the folder and the messages associated with
     those sequences.  _M_a_r_k will list each sequence named via
     `-sequence name' (or all of them if `-sequence' isn't used),
     and the messages associated with that sequence.  The `-zero'
     switch does not affect the operation of `-list'.

     The current restrictions on sequences are:

       The name used to denote a message sequence must consist
       solely of alphabetic characters, and can not be one of the
       "reserved" message names (e.g., "first", "cur", and so
       forth).

       Only a certain number of sequences may be defined for a
       given folder.  This number is usually limited to 10.

       The name used to denote a message sequence can not occur
       as part of a message range, e.g., constructs like
       "seen:20" or "seen-10" are forbidden.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Current-Folder:      To find the default current folder

SEE ALSO
     pick (1)

DEFAULTS
     `+folder' defaults to the current folder
     `-add' if `msgs' is specified, `-list' otherwise
     `msgs' defaults to cur (or all if `-list' is specified)
     `-nopublic' if the folder is read-only, `-public' otherwise
     `-nozero'

CONTEXT
     If a folder is given, it will become the current folder.













Printed 1/10/87                MH                               2



