


MHL(1)              UNIX Programmer's Manual               MHL(1)



NAME
     mhl - produce formatted listings of MH messages

SYNOPSIS
     /usr/misc/mh/lib/mhl [-bell] [-nobell] [-clear] [-noclear]
          [-folder +folder] [-form formfile] [-length lines]
          [-width columns] [-moreproc program] [-nomoreproc]
          [files ...] [-help]
9     _M_h_l is a formatted message listing program.  It can be used
     as a replacement for _m_o_r_e (1) (the default _s_h_o_w_p_r_o_c ).  As
     with _m_o_r_e, each of the messages specified as arguments (or
     the standard input) will be output.  If more than one mes-
     sage file is specified, the user will be prompted prior to
     each one, and a <RETURN> or <EOT> will begin the output,
     with <RETURN> clearing the screen (if appropriate), and
     <EOT> (usually CTRL-D) suppressing the screen clear.  An
     <INTERRUPT> (usually CTRL-C) will abort the current message
     output, prompting for the next message (if there is one),
     and a <QUIT> (usually CTRL-\) will terminate the program
     (without core dump).

     The `-bell' option tells _m_h_l to ring the terminal's bell at
     the end of each page, while the `-clear' option tells _m_h_l to
     clear the scree at the end of each page (or output a
     formfeed after each message).  Both of these switches (and
     their inverse counterparts) take effect only if the profile
     entry _m_o_r_e_p_r_o_c is defined but empty, and _m_h_l is outputting
     to a terminal.  If the _m_o_r_e_p_r_o_c entry is defined and non-
     empty, and _m_h_l is outputting to a terminal, then _m_h_l will
     cause the _m_o_r_e_p_r_o_c to be placed between the terminal and _m_h_l
     and the switches are ignored.  Furthermore, if the `-clear'
     switch is used and _m_h_l'_s output is directed to a terminal,
     then _m_h_l will consult the $TERM and $TERMCAP envariables to
     determine the user's terminal type in order to find out how
     to clear the screen.  If the `-clear' switch is used and
     _m_h_l'_s output is not directed to a terminal (e.g., a pipe or
     a file), then _m_h_l will send a formfeed after each message.

     To override the default _m_o_r_e_p_r_o_c and the profile entry, use
     the `-moreproc program' switch.

     The `-length length' and `-width width' switches set the
     screen length and width, respectively.  These default to the
     values indicated by $TERMCAP, if appropriate, otherwise they
     default to 40 and 80, respectively.

     The default format file used by _m_h_l is called _m_h_l._f_o_r_m_a_t
     (which is first searched for in the user's _M_H directory, and
     then sought in the /_u_s_r/_m_i_s_c/_m_h/_l_i_b directory), this can be
     changed by using the `-form formatfile' switch.

     Finally, the `-folder +folder' switch sets the _M_H folder



Printed 1/10/87                MH                               1


9


MHL(1)              UNIX Programmer's Manual               MHL(1)



     name, which is used for the "messagename:" field described
     below.  The envariable $mhfolder is consulted for the
     default value, which _s_h_o_w, _n_e_x_t, and _p_r_e_v initialize
     appropriately.

     _M_h_l operates in two phases: 1) read and parse the format
     file, and 2) process each message (file).  During phase 1,
     an internal description of the format is produced as a
     structured list.  In phase 2, this list is walked for each
     message, outputting message information under the format
     constraints from the format file.

     The "mhl.format" form file contains information controlling
     screen clearing, screen size, wrap-around control, tran-
     sparent text, component ordering, and component formatting.
     Also, a list of components to ignore may be specified, and a
     couple of "special" components are defined to provide added
     functionality.  Message output will be in the order speci-
     fied by the order in the format file.

     Each line of mhl.format has one of the formats:

          ;comment
          :cleartext
          variable[,variable...]
          component:[variable,...]

     A line beginning with a `;' is a comment, and is ignored.  A
     line beginning with a `:' is clear text, and is output
     exactly as is.  A line containing only a `:' produces a
     blank line in the output.  A line beginning with "com-
     ponent:" defines the format for the specified component, and
     finally, remaining lines define the global environment.

     For example, the line:

          width=80,length=40,clearscreen,overflowtext="***",overflowoffset=5

     defines the screen size to be 80 columns by 40 rows, speci-
     fies that the screen should be cleared prior to each page,
     that the overflow indentation is 5, and that overflow text
     should be flagged with "***".

     Following are all of the current variables and their argu-
     ments.  If they follow a component, they apply only to that
     component, otherwise, their affect is global.  Since the
     whole format is parsed before any output processing, the
     last global switch setting for a variable applies to the
     whole message if that variable is used in a global context
     (i.e., bell, clearscreen, width, length).

          _v_a_r_i_a_b_l_e       _t_y_p_e       _s_e_m_a_n_t_i_c_s



Printed 1/10/87                MH                               2






MHL(1)              UNIX Programmer's Manual               MHL(1)



          width          integer    screen width or component width
          length         integer    screen length or component length
          offset         integer    positions to indent "component: "
          overflowtext   string     text to use at the beginning of an
                                    overflow line
          overflowoffset integer    positions to indent overflow lines
          compwidth      integer    positions to indent component text
                                    after the first line is output
          uppercase      flag       output text of this component in all
                                    upper case
          nouppercase    flag       don't uppercase
          clearscreen    flag/G     clear the screen prior to each page
          noclearscreen  flag/G     don't clearscreen
          bell           flag/G     ring the bell at the end of each page
          nobell         flag/G     don't bell
          component      string/L   name to use instead of "component" for
                                    this component
          nocomponent    flag       don't output "component: " for this
                                    component
          center         flag       center component on line (works for
                                    one-line components only)
          nocenter       flag       don't center
          leftadjust     flag       strip off leading whitespace on each
                                    line of text
          noleftadjust   flag       don't leftadjust
          compress       flag       change newlines in text to spaces
          nocompress     flag       don't compress
          formatfield    string     format string for this component
          addrfield      flag       field contains addresses
          datefield      flag       field contains dates

     To specify the value of integer-valued and string-valued
     variables, follow their name with an equals-sign and the
     value.  Integer-valued variables are given decimal values,
     while string-valued variables are given arbirtray text
     bracketed by double-quotes.  If a value is suffixed by "/G"
     or "/L", then its value is useful in a global-only or
     local-only context (respectively).

     A line of the form:

         ignores=component,...

     specifies a list of components which are never output.

     The component "MessageName" (case-insensitive) will output
     the actual message name (file name) preceded by the folder
     name if one is specified or found in the environment.  The
     format is identical to that produced by the `-header' option
     to _s_h_o_w.

     The component "Extras" will output all of the components of



Printed 1/10/87                MH                               3






MHL(1)              UNIX Programmer's Manual               MHL(1)



     the message which were not matched by explicit components,
     or included in the ignore list.  If this component is not
     specified, an ignore list is not needed since all
     non-specified components will be ignored.

     If "nocomponent" is NOT specified, then the component name
     will be output as it appears in the format file.

     The default format is:

          : -- using template mhl.format --
          overflowtext="***",overflowoffset=5
          leftadjust,compwidth=9
          ignores=msgid,message-id,received
          Date:formatfield="%<(nodate{text})%{text}%|%(pretty{text})%>"
          To:
          cc:
          :
          From:
          Subject:
          :
          extras:nocomponent
          :
          body:nocomponent,overflowtext=,overflowoffset=0,noleftadjust

     The variable "formatfield" specifies a format string (see
     _m_h-_f_o_r_m_a_t(5)).  The variables "addrfield" and "datefield"
     (which are mutually exclusive), control the interpretation
     of the escapes.

     By default, _m_h_l does not apply any formatting string to
     fields containing address or dates (see _m_h-_m_a_i_l (5) for a
     list of these fields).  Note that this results in faster
     operation since _m_h_l must parse both addresses and dates in
     order to apply a format string to them.  If desired, _m_h_l can
     be given a default format string for either address or date
     fields (but not both).  To do this, on a global line specify
     either the variable addrfield or the variable datefield,
     along with the variable formatfield.

FILES
     /usr/misc/mh/lib/mhl.format        The message template
     or <mh-dir>/mhl.format             Rather than the standard template
     $HOME/.mh_profile                  The user profile

PROFILE COMPONENTS
     moreproc:            Program to use as interactive front-end

SEE ALSO
     show(1), ap(8), dp(8)





Printed 1/10/87                MH                               4






MHL(1)              UNIX Programmer's Manual               MHL(1)



DEFAULTS
     `-bell'
     `-noclear'
     `-length 40'
     `-width 80'

CONTEXT
     None

BUGS
     There should be some way to pass `bell' and `clear' informa-
     tion to the front-end.

     On hosts where _M_H was configured with the BERK option,
     address parsing is not enabled.








































Printed 1/10/87                MH                               5



