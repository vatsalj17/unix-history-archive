


ANNO(1)             UNIX Programmer's Manual              ANNO(1)



NAME
     anno - annotate messages

SYNOPSIS
     anno [+folder] [msgs] [-component field] [-inplace]
          [-noinplace] [-text body] [-help]
9     _A_n_n_o annotates the specified messages in the named folder
     using the field and body.  Annotation is optionally per-
     formed by _d_i_s_t, _f_o_r_w, and _r_e_p_l, to keep track of your dis-
     tribution of, forwarding of, and replies to a message.  By
     using _a_n_n_o, you can perform arbitrary annotations of your
     own.  Each message selected will be annotated with the lines

         field: date
         field: body

     The `-inplace' switch causes annotation to be done in place
     in order to preserve links to the annotated message.

     The field specified should be a valid 822-style message
     field name, which means that it should consist of
     alphanumerics (or dashes) only.  The body specified is arbi-
     trary text.

     If a `-component field' is not specified when _a_n_n_o is
     invoked, _a_n_n_o will prompt the user for the name of field for
     the annotation.

FILES
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     Path:                To determine the user's MH directory
     Current-Folder:      To find the default current folder

SEE ALSO
     dist (1), forw (1), repl (1)

DEFAULTS
     `+folder' defaults to the current folder
     `msgs' defaults to cur
     `-noinplace'

CONTEXT
     If a folder is given, it will become the current folder.
     The first message annotated will become the current message.





9


Printed 1/10/87                MH                               1



