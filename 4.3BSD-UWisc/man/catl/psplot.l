


PSPLOT(1)           UNIX Programmer's Manual            PSPLOT(1)



NAME
     psplot - convert plot(5) files to POSTSCRIPT format

SYNOPSIS
     psplot [ -g_p_r_o_l_o_g_u_e ] [ _f_i_l_e_s ]

DESCRIPTION
     _p_s_p_l_o_t reads _f_i_l_e_s in _p_l_o_t(5) format and converts them to
     POSTSCRIPT format on the standard output.  If no _f_i_l_e_s are
     specified, the standard input is used.  The conversion is
     almost one-for-one, with one POSTSCRIPT function call for
     each _p_l_o_t primitive.  The behavior of the generated file can
     be modified by changing the definitions of the POSTSCRIPT
     functions in the prologue.  If no -g option is specified,
     the default prologue is used.

ENVIRONMENT
     PSLIBDIR path name of a directory to use instead of
     /usr/misc/lib/ps for psplot prologue file.

FILES
     /usr/misc/lib/ps/psplot.pro
                           default prologue file.

SEE ALSO
     ps4014(1).
     4.2bsd: graph(1), plot(1), plot(3), plot(5), lpr(1).
     System V: lp(1).

AUTHOR
     Adobe Systems Incorporated

BUGS
     _p_l_o_t format has been removed from some newer UNIX systems
     (e.g., Systems V), but _p_s_p_l_o_t lives on.

NOTES
     POSTSCRIPT is a trademark of Adobe Systems Incorporated.

















Printed 1/10/87            12 Nov 1985                          1



