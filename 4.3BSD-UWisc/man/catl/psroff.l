


PSROFF(1)           UNIX Programmer's Manual            PSROFF(1)



NAME
     psroff - ditroff to a POSTSCRIPT printer

SYNOPSIS
     psroff [ -t ] [ _t_r_o_f_f _o_p_t_i_o_n_s ] [ _s_p_o_o_l _o_p_t_i_o_n_s ] [ _f_i_l_e_s ]

DESCRIPTION
     _p_s_r_o_f_f is a shell script that runs [_d_i]_t_r_o_f_f(1) in an
     environment to produce output on a POSTSCRIPT printer.  It
     uses _p_s_d_i_t to convert _d_i_t_r_o_f_f intermediate output to
     POSTSCRIPT format, and spools this for printing. If no _f_i_l_e_s
     are specified, the standard input is used.

     In addition to the standard _d_i_t_r_o_f_f options, the following
     options are understood by _p_s_r_o_f_f.

     -t   sends the POSTSCRIPT output to the standard output,
          rather than spooling it to a printer.  Note that this
          overrides the meaning of the _d_i_t_r_o_f_f -t option; if you
          want that, run _d_i_t_r_o_f_f directly.

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

     -d_d_e_s_t
          causes the output to be sent to the named destination.

     -n_n  causes _n copies of the output to be produced.  The
          default is one.

     -h   suppress the printing of the job burst page.




Printed 1/10/87            12 Nov 1985                          1






PSROFF(1)           UNIX Programmer's Manual            PSROFF(1)



     -r   don't page-reverse the output.

     -s   suppress messages from _l_p.

     -m   send mail after files have been printed.

     -w   write to user's terminal after files have been printed.

ENVIRONMENT
     PRINTER (4.2bsd)
          the name of a printer (as in the -P option) for _l_p_r to
          use.  If no -P option is specified, _l_p_r will use this
          printer.  If neither -P nor PRINTER is set, _p_s_r_o_f_f will
          spool to a printer named ``PostScript''.

     LPDEST (System V)
          the name of a printer (as in the -d option) for _l_p to
          use.  If no -d option is specified, _l_p will use this
          printer.  If neither -d nor LPDEST is set, _p_s_r_o_f_f will
          spool to a printer class named ``PostScript''.

FILES
     /usr/lib/tmac/tmac.*  standard macro files

     /usr/misc/lib/font/devpsc/*
                           _d_i_t_r_o_f_f description files for
                           POSTSCRIPT virtual device.

SEE ALSO
     [di]troff(1), psdit(1), eqn(1), tbl(1), pic(1), refer(1),
     ideal(1).
     4.2bsd: lpr(1).
     System V: lp(1).
     J. F. Ossanna, _N_r_o_f_f/_T_r_o_f_f _u_s_e_r'_s _m_a_n_u_a_l
     B. W. Kernighan, _A _T_R_O_F_F _T_u_t_o_r_i_a_l and _A _T_y_p_e_s_e_t_t_e_r-
     _i_n_d_e_p_e_n_d_e_n_t _T_R_O_F_F
     _D_o_c_u_m_e_n_t_e_r'_s _W_o_r_k_b_e_n_c_h

AUTHOR
     Adobe Systems Incorporated

BUGS
     The _e_q_n supplied with _d_i_t_r_o_f_f is different than the origi-
     nal.  Use the options -r576 -m2 for best results.  Other
     programs (e.g., _p_i_c) distributed with _d_i_t_r_o_f_f have the dev-
     ice names compiled in (so much for device independence!).
     They should just have a -T option and read the device
     description file for the information they need.  Use -T576
     with the _p_i_c distributed with DWB.

NOTES
     POSTSCRIPT is a trademark of Adobe Systems Incorporated.



Printed 1/10/87            12 Nov 1985                          2






PSROFF(1)           UNIX Programmer's Manual            PSROFF(1)



     Documenter's Workbench is a trademark of AT&T Technologies.






















































Printed 1/10/87            12 Nov 1985                          3



