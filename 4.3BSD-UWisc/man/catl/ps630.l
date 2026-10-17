


PS630(1)            UNIX Programmer's Manual             PS630(1)



NAME
     ps630 - convert Diablo 630 print files to POSTSCRIPT format

SYNOPSIS
     ps630 [ -f_b_o_d_y_f_o_n_t ] [ -F_b_o_l_d_f_o_n_t ] [ -s_p_i_t_c_h ] [ -p_f_i_l_e ] [
     _f_i_l_e_s ]

DESCRIPTION
     _p_s_6_3_0 reads in _f_i_l_e_s (Diablo 630 format print files) and
     converts them to POSTSCRIPT format for printing on a
     POSTSCRIPT printer.  If no _f_i_l_e_s are specified, the standard
     input is used.  By default, the POSTSCRIPT program is sent
     to the standard output.  _p_s_6_3_0 can convert _n_r_o_f_f files gen-
     erated with the -Txerox switch.  Typewheel emulation infor-
     mation may be specified as options.  Font specifications
     (for bold and regular) are POSTSCRIPT font names (e.g.,
     Times-Roman, Times-Bold, Courier-Bold, Courier-BoldOblique).
     10, 12, or 15 characters per inch may be selected.

     The possible options are:

     -f _b_o_d_y_f_o_n_t
          sets the font to be used for normal printing.  The
          default is Courier.

     -F _b_o_l_d_f_o_n_t
          sets the font to be used for boldface.  The default is
          Courier-Bold.

     -s _p_i_t_c_h
          selects type size for printing (both the regular and
          bold fonts are scaled to this size).  _P_i_t_c_h is in char-
          acters per inch and must be one of 10, 12, or 15.  The
          default is 12.

     -p _f_i_l_e
          causes the POSTSCRIPT file to be written to the named
          file rather than the standard output.

SEE ALSO
     enscript(1), nroff(1).
     4.2bsd: lpr(1), pr(1).
     System V: lp(1).
     Diablo 630 documentation

DIAGNOSTICS
     Should be self explanatory.

AUTHOR
     Adobe Systems Incorporated





Printed 1/10/87            12 Nov 1985                          1






PS630(1)            UNIX Programmer's Manual             PS630(1)



BUGS
     Some applications produce ``bold'' by double striking a
     character.  This will not appear as bold when translated
     into POSTSCRIPT.  Only the bold produced by issuing the
     proper Diablo command sequence (escape-O) will result in
     bold characters.

     Times-Roman and Helvetica are narrow fonts that may look
     squeezed if no adjustment of page width is made by the
     application.

     The translator uses exact positioning on the paper.  Output
     from an application that has attempted to compensate for
     slippage on vertical movement may appear slightly uneven.

     The following Diablo 630 commands are not supported: print
     suppression, HY-Plot, extended character set, downloading
     print wheel information or program mode, page lengths other
     than 11 inches, paper feeder control, hammer energy control,
     remote diagnostic, backward printing control (note, however,
     that ``reverse printing'' is supported).

NOTES
     POSTSCRIPT is a trademark of Adobe Systems Incorporated.
     Times and Helvetica are registered trademarks of Allied Cor-
     poration.
     Diablo Systems, Inc. is a Xerox Company.




























Printed 1/10/87            12 Nov 1985                          2



