


PSCAT(1)            UNIX Programmer's Manual             PSCAT(1)



NAME
     pscat - convert C/A/T files to POSTSCRIPT format

SYNOPSIS
     pscat [ -F_f_o_n_t_t_a_b_l_e ] [ -i_p_r_o_l_o_g_u_e_f_i_l_e ] [ -l_p_a_p_e_r_l_e_n_g_t_h ] [
     -x_x_o_f_f_s_e_t ] [ -y_y_o_f_f_s_e_t ] [ _f_i_l_e ]

DESCRIPTION
     _p_s_c_a_t converts C/A/T format _f_i_l_e (i.e., as produced by the
     original [_o]_t_r_o_f_f(1)), to POSTSCRIPT format on the standard
     output.  If no _f_i_l_e argument is given, the standard input is
     used.  The average user will probably be happy using
     _p_t_r_o_f_f(1), but explicit use of _p_s_c_a_t gives more control.
     Page offset distances and the page length are specified by
     giving a fixed-point real number optionally followed by a
     units measure.  For example, 1in means one inch, 15mm means
     fifteen millimeters.  If no units are indicated, points (72
     to the inch) are used.  The options are:

     -F _f_o_n_t_t_a_b_l_e
          specifies a font correspondence table for use by the
          converter.  (The default is usually _T_i_m_e_s._c_t where the
          mounted fonts are: R=Times-Roman, I=Times-Italic,
          B=Times-Bold, and S=Symbol.  Note that _t_r_o_f_f should be
          invoked with an appropriate -F switch and explicit
          ``.fp'' commands.)

     -i _p_r_o_l_o_g_u_e_f_i_l_e
          specifies a file to be incorporated as the POSTSCRIPT
          prologue, otherwise the default prologue will be used
          (see below).

     -l _p_a_g_e_l_e_n_g_t_h
          specifies the page length to use when splitting the
          C/A/T file into POSTSCRIPT pages.  The default is 11in.

     -x _x_o_f_f_s_e_t
          moves the output _x_o_f_f_s_e_t to the right on the paper.
          The default is 0.

     -y _y_o_f_f_s_e_t
          moves the output _y_o_f_f_s_e_t downward on the paper.  The
          default is 0.  Different versions of _t_r_o_f_f seem to have
          different notions of where the top edge of the paper
          really is, so careful setting of the -y switch may be
          necessary.

ENVIRONMENT
     PSLIBDIR
          path name of a directory to use instead of
          /usr/misc/lib/ps for pscat prologue and font metric
          files.



Printed 1/10/87            12 Oct 1985                          1






PSCAT(1)            UNIX Programmer's Manual             PSCAT(1)



FILES
     /usr/misc/lib/ps/troff.font/*.ct
                           character correspondence tables (built
                           by _p_s_c_a_t_m_a_p(8)) mapping C/A/T codes to
                           POSTSCRIPT fonts and characters or
                           other actions.

     /usr/misc/lib/ps/pscat.pro
                           default POSTSCRIPT prologue.

SEE ALSO
     ptroff(1), pscatmap(8).
     4.2bsd: troff(1).
     System V: otroff(1), _D_o_c_u_m_e_n_t_e_r'_s _W_o_r_k_b_e_n_c_h.

AUTHOR
     Adobe Systems Incorporated

BUGS
     Because of the character mapping tables used, arbitrary use
     of the ``.fp'' troff directive in will usually produce the
     wrong results.  The _f_a_m_i_l_y._h_e_a_d files which _p_t_r_o_f_f feeds to
     _t_r_o_f_f contain ``.fp'' commands appropriate for a given font
     family.  Users must build and reference correct tables with
     _p_s_c_a_t_m_a_p(8) for non-standard ``.fp'' combinations to work.

     Distance computations are not checked for overflow.

     The mapping from the default troff fonts to Adobe's Times-
     Roman will not look good.  POSTSCRIPT font width tables (and
     ``.fp'' commands) should be used explicitly in troff.

     _p_s_c_a_t determines where page breaks occur by assuming that
     each page is exactly the specified length.  There is no
     indication of page boundaries in the C/A/T file, so improper
     use of the -l option will lead to undesirable results.  Some
     _t_r_o_f_f macro packages generate cut-marks at logical edges of
     the page which may or may not get imaged on a POSTSCRIPT
     device.  Careful use of the -x and -y options may help.

NOTES
     POSTSCRIPT is a trademark of Adobe Systems Incorporated.
     Times is a registered trademark of Allied Corporation.
     Documenter's Workbench is a trademark of AT&T Technologies.











Printed 1/10/87            12 Oct 1985                          2



