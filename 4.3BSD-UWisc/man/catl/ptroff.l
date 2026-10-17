


PTROFF(1)           UNIX Programmer's Manual            PTROFF(1)



NAME
     ptroff - troff to a POSTSCRIPT printer

SYNOPSIS
     ptroff [ -t ] [ -F_f_a_m_i_l_y ] [ _t_r_o_f_f _o_p_t_i_o_n_s ] [ _s_p_o_o_l _o_p_t_i_o_n_s
     ] [ _f_i_l_e_s ]

DESCRIPTION
     _p_t_r_o_f_f is a shell script that runs [_o]_t_r_o_f_f(1) in an
     environment to produce output on a POSTSCRIPT printer.  It
     uses _p_s_c_a_t to convert _t_r_o_f_f C/A/T output to a POSTSCRIPT
     print file, and spools this for printing.  If no _f_i_l_e_s are
     specified, the standard input is used.

     In addition to the standard [_o]_t_r_o_f_f options, the following
     options are understood by _p_t_r_o_f_f.

     -t   sends the POSTSCRIPT output to the standard output,
          rather than spooling it to a printer.  Note that this
          overrides the meaning of the _t_r_o_f_f -t option; if you
          want the C/A/T file, run [_o]_t_r_o_f_f directly.

     -F _f_o_n_t_f_a_m_i_l_y
          names a font family to load into [_o]_t_r_o_f_f and _p_s_c_a_t.
          _F_o_n_t_f_a_m_i_l_y is used to name both a *._h_e_a_d file contain-
          ing _t_r_o_f_f ``fp'' commands, and a *._c_t file containing a
          character mapping for _p_s_c_a_t.  Fontfamilies ``Times''
          and ``Helvetica'' are probably defined at your site,
          others may be available.

     In 4.2bsd UNIX systems, the following spooler options are
     passed on to _l_p_r.

     -P_p_r_i_n_t_e_r
          causes the output to be sent to the named printer.

     -#_n  causes _n copies of the output to be produced.  The
          default is one.

     -h   suppress the printing of the job burst page.

     -C _c_l_a_s_s
          set the job classification for use on the burst page.

     -J _n_a_m_e
          set the job name for use on the burst page.  Otherwise,
          the name of the first input file will be used.

     -m   send mail after files have been printed.

     In System V UNIX systems, the following spooler options are
     passed on to _l_p.



Printed 1/10/87            12 Nov 1985                          1






PTROFF(1)           UNIX Programmer's Manual            PTROFF(1)



     -d_d_e_s_t
          causes the output to be sent to the named destination.

     -n_n  causes _n copies of the output to be produced.  The
          default is one.

     -h   suppress the printing of the job burst page.

     -r   don't page-reverse the output.

     -s   suppress messages from _l_p.

     -m   send mail after files have been printed.

     -w   write to user's terminal after files have been printed.

ENVIRONMENT
     PRINTER (4.2bsd)
          the name of a printer (as in the -P option) for _l_p_r to
          use.  If no -P option is specified, _l_p_r will use this
          printer.  If neither -P nor PRINTER is set, _p_t_r_o_f_f will
          spool to a printer named ``PostScript''.

     LPDEST (System V)
          the name of a printer (as in the -d option) for _l_p to
          use.  If no -d option is specified, _l_p will use this
          printer.  If neither -d nor LPDEST is set, _p_t_r_o_f_f will
          spool to a printer class named ``PostScript''.

FILES
     /usr/misc/lib/ps/troff.font/*.ct
                           character correspondence tables for
                           _p_s_c_a_t.

     /usr/misc/lib/ps/troff.font/*.head
                           _t_r_o_f_f headers containing ``.fp'' com-
                           mands.

     /usr/misc/lib/ps/troff.font/ftXX
                           font width files for _t_r_o_f_f.

     /usr/misc/lib/ps/pscat.pro
                           default POSTSCRIPT prologue for _p_s_c_a_t.

SEE ALSO
     pscat(1), pscatmap(8).
     4.2bsd: troff(1).
     System V: otroff(1).
     J. F. Ossanna, _N_r_o_f_f/_T_r_o_f_f _u_s_e_r'_s _m_a_n_u_a_l
     _D_o_c_u_m_e_n_t_e_r'_s _W_o_r_k_b_e_n_c_h





Printed 1/10/87            12 Nov 1985                          2






PTROFF(1)           UNIX Programmer's Manual            PTROFF(1)



AUTHOR
     Adobe Systems Incorporated

BUGS
     Because of the character mapping tables used, explicit use
     of the ``.fp'' troff directive will usually produce the
     wrong result.  Users must build and reference correct tables
     with _c_a_t_m_a_p(8) for non-standard ``.fp'' combinations to
     work.

     More flexibility is allowed by using _t_r_o_f_f, _p_s_c_a_t, and the
     spooler separately.

NOTES
     POSTSCRIPT is a trademark of Adobe Systems Incorporated.
     Times and Helvetica are registered trademark of Allied Cor-
     poration.
     Documenter's Workbench is a trademark of AT&T Technologies.





































Printed 1/10/87            12 Nov 1985                          3



