


PSREV(1)            UNIX Programmer's Manual             PSREV(1)



NAME
     psrev - select or reverse page order of a POSTSCRIPT file

SYNOPSIS
     psrev [ -R ] [ -s_p_a_g_e_s_p_e_c ] [ _f_i_l_e ]

DESCRIPTION
     _p_s_r_e_v reads in a POSTSCRIPT _f_i_l_e (or the standard input if
     no file is named), and if the input conforms to the
     POSTSCRIPT file structuring conventions, reverses and
     selects the specified pages for printing and writes the
     resulting file  on the standard output.

     The possible options are:

     -s _p_a_g_e_s_p_e_c
          specifies a range (or several ranges) of pages to be
          printed.  _P_a_g_e_s_p_e_c is a string, not containing spaces,
          of the form ``pagerange[,pagespec]''.  _P_a_g_e_r_a_n_g_e may be
          a single page number or a range of the form ``N-M''
          which means print pages N through M.  -N means print
          from the beginning of the document to page N. M- means
          print from page M to the end of the document.

     -R   does not reverse the page order, (but subset the pages
          if specified).

FILES
     /usr/tmpX/RV*         temporary file if input is a pipe.

SEE ALSO
     enscript(1), postscript(7).

DIAGNOSTICS
     Should be self explanatory.

AUTHOR
     Adobe Systems Incorporated

NOTES
     POSTSCRIPT is a trademark of Adobe Systems Incorporated.














Printed 1/10/87            12 Nov 1985                          1



