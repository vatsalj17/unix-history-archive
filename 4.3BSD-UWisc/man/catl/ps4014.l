


PS4014(1)           UNIX Programmer's Manual            PS4014(1)



NAME
     ps4014 - convert a Tektronix 4014 files to POSTSCRIPT format

SYNOPSIS
     ps4014 [ -RCNm ] [ -p_o_u_t_f_i_l_e ] [ -l_l_e_f_t,_b_o_t_t_o_m ] [
     -s_w_i_d_t_h,_h_e_i_g_h_t ] [ -S_w_i_d_t_h ] [ _f_i_l_e ]

DESCRIPTION
     _p_s_4_0_1_4 reads in a Tektronix 4014-format file and converts it
     to POSTSCRIPT format for printing on a POSTSCRIPT printer.
     If no _f_i_l_e is specified, the standard input is used.  The
     resulting POSTSCRIPT file may be directed to the standard
     output or to a named file.

     The possible options are:

     -p _o_u_t_f_i_l_e
          causes the POSTSCRIPT file to be written to the named
          file rather than the standard output.

     The following parameters affect the size, placement and
     orientation of the image on the printed page.  By default,
     the 4014 image is scaled to occupy nearly the whole page in
     a landscape orientation.

     -l _l_e_f_t,_b_o_t_t_o_m
          specifies the location on the printed page of the bot-
          tom, left corner of the converted raster image.  The
          values _l_e_f_t and _b_o_t_t_o_m are the distances (in inches)
          from the bottom, left corner of the printed page to the
          bottom, left corner of the image.

     -s _w_i_d_t_h,_h_e_i_g_h_t
          Specifies the size of the converted raster image on the
          printed page.  _W_i_d_t_h and _h_e_i_g_h_t are the dimensions (in
          inches) of the resulting image on the printed page.

     -S _w_i_d_t_h
          allows you to scale the image without distoring its
          shape. _W_i_d_t_h specifies the width, in inches, of the
          resulting image on the printed page.  The height of the
          image is computed to maintain the same ratio of height
          to width on the output image as on the input raster-
          format file.

     -R   rotates the image 90 degrees on the page for portrait
          orientation.  The default is landscape orientation.

     The following parameters specify values for 4014 hardware
     options that affect the interpretation of 4014 commands.

     -C   causes an carriage return to move the pen position to



Printed 1/10/87            12 Oct 1985                          1






PS4014(1)           UNIX Programmer's Manual            PS4014(1)



          the left margin but not down to the next line.  By
          default, a carriage return command moves the pen down
          to the next line and over to the left margin.

     -N   causes line feed to move the pen position down to the
          next line but not left to the left margin. By default,
          a line feed command moves the pen down to the next line
          and over to the left margin.

     -m   enables the "Margin 2" mode for the 4014.

ENVIRONMENT
     PSLIBDIR
          path name of a directory to use instead of
          /usr/misc/lib/ps for ps4014 prologue.

FILES
     /usr/misc/lib/ps/ps4014.pro
                           default prologue file.

SEE ALSO
     psplot(1).
     4.2bsd: lpr(1), plot(1), psplot(1).
     System V: lp(1).
     Tektronix 4014 documentation.

DIAGNOSTICS
     Should be self explanatory.

AUTHOR
     Adobe Systems Incorporated

NOTES
     POSTSCRIPT is a trademark of Adobe Systems Incorporated.
     Tektronix is a registered trademark of Tektronix, Inc.




















Printed 1/10/87            12 Oct 1985                          2



