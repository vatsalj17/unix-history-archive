


GRN(1)              UNIX Programmer's Manual               GRN(1)



NAME
     grn - ditroff preprocessor for gremlin files

SYNOPSIS
     grn [ options ] ... [ _f_i_l_e ] ...

DESCRIPTION
     _G_r_n is a preprocessor for including _g_r_e_m_l_i_n pictures in
     _d_i_t_r_o_f_f input.  _G_r_n writes to standard output, processing
     only input lines between two that start with `.GS' and
     `.GE'.  Those lines must contain _g_r_n commands (see below).
     These commands request a gremlin file, and the picture in
     that file is converted and placed in the ditroff input
     stream.  The .GS may be followed by a C, L, or R to center,
     left or right justify the whole gremlin picture (default
     justification is center).  If no _f_i_l_e is mentioned, the
     standard input is read.  At the end of the picture, the
     position on the page is the bottom of the gremlin picture.
     If the grn entry is ended with `.GF' instead of `.GE', the
     position is left at the top of the picture.  The following
     command-line options are understood:

     -P_d_e_v
          Prepare output for printer _d_e_v.  The default is varian
          (or versatec since the two are equivalent).  See ditr-
          off(1) for acceptable dev's.

     -L_d_i_r
          Set the library to directory _d_i_r.  If a gremlin file
          cannot be found in the current directory, _d_i_r is
          prepended to the filename for a second try.  The
          default library directory is /usr/local/gremlib.

GRN COMMANDS
     Each input line between the `.GS' and `.GE' may have one grn
     command.  Commands consist of one or two strings separated
     by white space, the first string being the command and the
     second its operand.  Commands may be upper or lower case and
     abbreviated down to one character.

     Commands that affect a picture's environment (those listed
     before default, below) are only in effect for the current
     picture: the environment is reinitialized to the defaults at
     the start of the next picture.  The commands are as follows:

     1 _N  (2, 3, or 4)  Set gremlin's text size number 1 (2, 3,
          or 4) to _N points.  The default is 12 (16, 24, and 36).

     roman _f_f
          (italics, bold, or special)  Set the roman (italics,
          bold, or special) font to ditroff's font _f_f (either a
          name or number).  The default is R (I, B, and S).



Printed 1/10/87               local                             1






GRN(1)              UNIX Programmer's Manual               GRN(1)



     l _N  Set the stipple font to ditroff's stipple font _f_f (name
          or number).  The command stipple can be substituted,
          but only abbreviated down as far as `st' to avoid con-
          fusion with special.  There is NO default for stipples
          (unless one is set by the default command), and it is
          illegal to include a gremlin picture with polygons
          without specifying a stipple font.

     x _N  Magnify the picture (in addition to any default magnif-
          ication) by _N, a floating point number > 0.  The com-
          mand scale may be substituted, abbreviated down to
          `sc'.

     narrow _N
          (medium, or thick)  Set the thickness of gremlin's nar-
          row (medium or thick) lines to _N.  The default is 1 (3
          and 5) pixels.

     pointscale <_o_f_f/_o_n>
          Scale text to match the picture.  Gremlin text is usu-
          ally printed in the point size specified in 1, 2, 3 or
          4 regardless of any scaling factors in the picture.
          Setting pointscale will cause the point sizes to scale
          with the picture (within ditroff's limitations, of
          course).  An operand of anything but _o_f_f will turn text
          scaling on.

     default
          Reset the picture environment defaults to the settings
          in the current picture.  This is meant to be used as a
          global parameter setting mechanism at the beginning of
          the ditroff input file, but can be used at any time to
          reset the default settings.

     width _N
          Forces the picture to be _N inches wide.  This overrides
          any scaling factors present in the same picture.  width
          _0 is ignored.

     height _N
          Forces picture to be _N inches high, overriding other
          scaling factors.  If both ``width'' and ``height'' are
          specified the tighter constraint will determine the
          scale of the picture.  Height and width commands are
          not saved with a default command.  They will, however,
          affect point size scaling if that option is set.

     file _n_a_m_e
          Get picture from gremlin file _n_a_m_e from the current
          directory (or in the library directory see the -L
          option above).  If two file commands are given, the
          second one overrides the first.  If _n_a_m_e doesn't exist,



Printed 1/10/87               local                             2






GRN(1)              UNIX Programmer's Manual               GRN(1)



          an error message is reported and processing continues
          from the .GE line.

NOTES ABOUT DITROFF
     Since _g_r_n is a preprocessor, it doesn't know about current
     indents, point sizes, margins, number registers, etc.  Con-
     sequently, no troff input can be placed between the ``.GS''
     and ``.GE''.  However, gremlin text is now processed by
     troff, so anything legal in a single line of troff input is
     legal in a line of gremlin text (barring ``.'' directives at
     the beginning of a line).

     When using grn along with other preprocessors, it is best to
     run tbl before grn, pic, and/or ideal to avoid overworking
     tbl.  Eqn should always be run last.

     A picture is considered an entity, but that doesn't stop
     troff from trying to break it up if it falls off the end of
     a page.  Placing the picture between ``keeps'' in either the
     -ms or -me macros will ensure proper placement.

     The .GS and .GE lines are sent through to ditroff and used
     in the -ms and -me macros for adjusting the picture.  There
     are -mg macros available for grn, pic and ideal if you don't
     use -ms or -me.  Grn uses number registers g1 through g9 and
     sets register g1 and g2 to the width and height of the grem-
     lin figure (in device units) before entering the .GS (this
     is for those who want to rewrite these macros).

FILES
     /usr/lib/font/dev_x_x_x/DESC.out device _x_x_x description file

SEE ALSO
     gremlin(l), ditroff(1), pic(1), ideal(1)

AUTHOR
     David Slattengren


















Printed 1/10/87               local                             3



