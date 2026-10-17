


ENSCRIPT(1)         UNIX Programmer's Manual          ENSCRIPT(1)



NAME
     enscript - convert text files to POSTSCRIPT format for
     printing

SYNOPSIS
     enscript [ -12BGghKklmoqRr ] [ -L_l_i_n_e_s ] [ -f_f_o_n_t ] [
     -F_h_f_o_n_t ] [ -b_h_e_a_d_e_r ] [ -p_o_u_t ] [ _s_p_o_o_l_o_p_t_s ] [ _f_i_l_e_s ]

DESCRIPTION
     _e_n_s_c_r_i_p_t reads in plain text files, converts them to
     POSTSCRIPT format, and spools them for printing on a
     POSTSCRIPT printer.  Fonts, headings, limited formatting
     options and spooling options may be specified.

     For example:
          enscript -Paleph boring.txt
     prints a copy of the file called boring.txt on the printer
     called "aleph".
          enscript -2r boring.c
     prints a two-up landscape listing of the file called
     boring.c on the default printer (see below).

     Font specifications have two parts: A font name as known to
     POSTSCRIPT (e.g., Times-Roman, Times-BoldItalic Helvetica,
     Courier), and a point size (1 point=1/72 inch - 8 point is a
     good small font).  So Courier-Bold8 is 8 point Courier Bold,
     Helvetica12 is 12 point Helvetica.

     The environment variable ENSCRIPT may be used to specify
     defaults.  The value of ENSCRIPT is parsed as a string of
     arguments _b_e_f_o_r_e the arguments that appear on the command
     line.  For example "ENSCRIPT='-fTimes-Roman8'" sets your
     default body font to 8 point Times Roman.

     The possible options are:

     -2   set in two columns.

     -1   set in one column (the default).

     -r   rotate the output 90 degrees (landscape mode).  This is
          good for output that requires a wide page or for pro-
          gram listings when used in conjunction with -2.
          "enscript -2r files" is a nice way to get program list-
          ings.

     -R   don't rotate, also known as portrait mode (the
          default).

     -G   print in gaudy mode: causes page headings, dates, page
          numbers to be printed in a flashy style, at some slight
          performance expense.



Printed 1/10/87            12 Nov 1985                          1






ENSCRIPT(1)         UNIX Programmer's Manual          ENSCRIPT(1)



     -l   simulate a line printer: make pages 66 lines long and
          omit headers.

     -B   omit page headings.

     -b_h_e_a_d_e_r
          sets the string to be used for page headings to _h_e_a_d_e_r.
          The default header is constructed from the file name,
          its last modification date, and a page number.

     -L_l_i_n_e_s
          set the maximum number of lines to output on a page.
          _E_n_s_c_r_i_p_t usually computes how many to put on a page
          based on point size, and may put fewer per page than
          requested by _l_i_n_e_s.

     -f_f_o_n_t
          sets the font to be used for the body of each page.
          Defaults to Courier10, unless two column rotated mode
          is used, in which case it defaults to Courier7.

     -F_h_f_o_n_t
          sets the font to be used for page headings.  Defaults
          to Courier-Bold10.

     -p_o_u_t
          causes the POSTSCRIPT file to be written to the named
          file rather than being spooled for printing.  As a spe-
          cial case, -p - will send the POSTSCRIPT to the stan-
          dard output.

     -g   causes the fact that a file is garbage to be ignored.
          Normally, any file with more than a small number of
          non-printing characters is suspected of being garbage,
          and not printed - this option means "print it anyway."

     -o   If _e_n_s_c_r_i_p_t cannot find characters in a font, the miss-
          ing characters are listed.

     -q   causes _e_n_s_c_r_i_p_t to be quiet about what it is doing.
          _E_n_s_c_r_i_p_t won't report about pages, destination, omitted
          characters, etc. Fatal errors are still reported to the
          standard error output.

     -k   enables page prefeed (if the printer supports it).
          This allows simple documents (e.g., program listings in
          one font) to print somewhat faster by keeping the
          printer running between pages.

     -K   disable page prefeed (the default).

     -h   suppress printing of job burst page.



Printed 1/10/87            12 Nov 1985                          2






ENSCRIPT(1)         UNIX Programmer's Manual          ENSCRIPT(1)



     In 4.2bsd systems, the following spooler options are also
     recognized:

     -P_p_r_i_n_t_e_r
          causes the output to be sent to the named printer.

     -#_n  causes _n copies of the output to be produced.  The
          default is one.

     -m   causes the printer daemon to send mail upon job comple-
          tion.

     -J_n_a_m_e
          sets the job name for use on the burst page.  Other-
          wise, the name of the first input file will be used.

     -C_c_l_a_s_s
          set the job classification for use on the burst page.

     In System V, the following spooler options are also recog-
     nized:

     -t_t_i_t_l_e
          sets job title for use on the burst page.

     -d_d_e_s_t
          causes the output to be sent to the named printer or
          printer class.

     -n_n  causes _n copies of the output to be produced. The
          default is one.

     -w or -m
          causes the printer daemon to write or send mail upon
          job completion.

ENVIRONMENT
     ENSCRIPT
          string of options to be used by _e_n_s_c_r_i_p_t.

     PSLIBDIR
          path name of a directory to use instead of
          /usr/misc/lib/ps for enscript prologue and font metric
          files.

     PSTEMPDIR
          path name of temporary directory to use instead of
          XPSTEMDIRX of spooled temporary files.

     PRINTER (4.2bsd)
          the name of a printer (as in the -P option) for _l_p_r to
          use.  If no -P option is specified, _l_p_r will use this



Printed 1/10/87            12 Nov 1985                          3






ENSCRIPT(1)         UNIX Programmer's Manual          ENSCRIPT(1)



          printer.  If neither -P nor PRINTER is set, _e_n_s_c_r_i_p_t
          will spool to a printer named ``PostScript''.

     LPDEST (System V)
          the name of a printer (as in the -d option) for _l_p to
          use.  If no -d option is specified, _l_p will use this
          printer.  If neither -d nor LPDEST is set, _e_n_s_c_r_i_p_t
          will spool to a printer class named ``PostScript''.

FILES
     /usr/misc/lib/ps/*.afm
                           font metrics files.

     /usr/misc/lib/ps/enscript.pro
                           prologue for enscript files.

     lpr(1 bsd), lp(1 V)   Printer spooler

SEE ALSO
     pr(1), ps630(1), getopt(3).
     4.2bsd: lpr(1), lpq(1), lprm(1).
     System V: lp(1), cancel(1), lpstat(1).

DIAGNOSTICS
     Should be self explanatory.

AUTHOR
     Adobe Systems Incorporated
     Gaudy mode by Guy Riddle of AT&T Bell Laboratories

FEATURES
     Options and the ENSCRIPT environment string are parsed in
     _g_e_t_o_p_t(3) fashion.

BUGS
     Long lines are truncated.  Line truncation may be off by a
     little bit as printer margins vary.  There should be a
     "wrap" option and multiple (truncated or wrapped) columns.

     _E_n_s_c_r_i_p_t generates temporary files which are spooled for
     printing.  The temporary file names are used by the spooling
     software (e.g., _l_p_q), rather than the symbolic job name
     passed to _l_p_r.

     There are too many options.

NOTES
     POSTSCRIPT is a trademark of Adobe Systems Incorporated.
     Times and Helvetica are registered trademarks of Allied Cor-
     poration.





Printed 1/10/87            12 Nov 1985                          4



