


DITROFF(1)          UNIX Programmer's Manual           DITROFF(1)



NAME
     troff - text formatting and typesetting

SYNOPSIS
     ditroff [ option ] ...  [ file ] ...

DESCRIPTION
     _D_i_t_r_o_f_f formats text in the named _f_i_l_e_s for printing on one
     of a few printers.  Different output devices are selected
     through the -P option.  Output will go to those devices
     unless one of the -t or -a options is specified.   _T_r_o_f_f's
     capabilities are described in the _N_r_o_f_f/_T_r_o_f_f _u_s_e_r'_s _m_a_n_u_a_l.

     If no _f_i_l_e argument is present, the standard input is read.
     An argument consisting of a single minus (-) is taken to be
     a file name corresponding to the standard input.  The
     options are:

     -o_l_i_s_t Print only pages whose page numbers appear in the
            comma-separated _l_i_s_t of numbers and ranges.  A range
            _N-_M means pages _N through _M; an initial -_N means from
            the beginning to page _N; and a final _N- means from _N
            to the end.

     -n_N    Number first generated page _N.

     -m_n_a_m_e Prepend the macro file /usr/lib/ditmac/tmac._n_a_m_e to
            the input _f_i_l_e_s.

     -P_d_e_s_t Prepare output for typesetter _d_e_s_t.  -Pva (or
            -Pvarian ) = Benson-Varian Raster plotter (default),
            -Pvp (or -Pversatec ) = Versatec Electrostatic
            Plotter, -Pip (or -Pimagen ) = Canon/Imagen Laser
            Printer, -Pter = Line Printer (or terminals if -t is
            specified).

     -I     Prepare the ditroff output for the imagen printer.

     -W     Prepare the ditroff output for the versatec printer.

     -t     Send the ditroff output to standard output.

     -u_N    Set the emboldening factor to _N.

     -r_a_N   Set register _a (one character name) to _N.

     -i     Read standard input after the input files are
            exhausted.

     -q     Invoke the simultaneous input-output mode of the rd
            request.




Printed 1/10/87                                                 1






DITROFF(1)          UNIX Programmer's Manual           DITROFF(1)



     -a     Send a printable ASCII approximation of the results
            to the standard output.

     -F_d_i_r  Use font directory _d_i_r instead of /usr/lib/font.

FILES
     /usr/lib/ditmac/tmac.*  standard macro files
     /usr/lib/font/dev*      font width tables

BUGS
     The -F option is not very useful since the same directory
     must be available on the machine that drives the printer.

SEE ALSO
     trchange(l) for a list of differences, in _d_i_t_r_o_f_f, to TROFF
     commands
     J. F. Ossanna, _N_r_o_f_f/_T_r_o_f_f _u_s_e_r'_s _m_a_n_u_a_l
     B. W. Kernighan, _A _T_R_O_F_F _T_u_t_o_r_i_a_l
     eqn(1), grn(1), ideal(1), pic(1), refer(1),
     tbl(1), dterm(1), lpr(1), font(1)



































Printed 1/10/87                                                 2



