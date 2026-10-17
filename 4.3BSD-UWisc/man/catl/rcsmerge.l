


RCSMERGE(1)         UNIX Programmer's Manual          RCSMERGE(1)



NAME
     rcsmerge - merge RCS revisions

SYNOPSIS
     rcsmerge -r_r_e_v_1 [ -r_r_e_v_2 ] [ -p ] file

DESCRIPTION
     _R_c_s_m_e_r_g_e incorporates the changes between _r_e_v_1 and _r_e_v_2 of
     an RCS file into the corresponding working file. If -p is
     given, the result is printed on the std. output, otherwise
     the result overwrites the working file.

     A file name ending in ',v' is an RCS file name, otherwise a
     working file name. _M_e_r_g_e derives the working file name from
     the RCS file name and vice versa, as explained in _c_o (1). A
     pair consisting of both an RCS and a working file name may
     also be specified.

     _R_e_v_1 may not be omitted. If _r_e_v_2 is omitted, the latest
     revision on the trunk is assumed.  Both _r_e_v_1 and _r_e_v_2 may be
     given numerically or symbolically.

     _R_c_s_m_e_r_g_e prints a warning if there are overlaps, and delim-
     its the overlapping regions as explained in _c_o -_j.  The com-
     mand is useful for incorporating changes into a checked-out
     revision.

EXAMPLES
     Suppose you have released revision 2.8 of f.c. Assume furth-
     ermore that you just completed revision 3.4, when you
     receive updates to release 2.8 from someone else.  To com-
     bine the updates to 2.8 and your changes between 2.8 and
     3.4, put the updates to 2.8 into file f.c and execute

             rcsmerge  -p  -r2.8  -r3.4  f.c  >f.merged.c

     Then examine f.merged.c.  Alternatively, if you want to save
     the updates to 2.8 in the RCS file, check them in as revi-
     sion 2.8.1.1 and execute _c_o -_j:

             ci  -r2.8.1.1  f.c
             co  -r3.4  -j2.8:2.8.1.1  f.c

     As another example, the following command undoes the changes
     between revision 2.4 and 2.8 in your currently checked out
     revision in f.c.

             rcsmerge  -r2.8  -r2.4  f.c

     Note the order of the arguments, and that f.c will be
     overwritten.




Printed 1/10/87              6/29/83                            1






RCSMERGE(1)         UNIX Programmer's Manual          RCSMERGE(1)



IDENTIFICATION
     Author: Walter F. Tichy, Purdue University, West Lafayette,
     IN, 47907.
     Revision Number: 3.0 ; Release Date: 83/01/15 .
     Copyright 8c9 1982 by Walter F. Tichy.

SEE ALSO
     ci (1), co (1), merge (1), ident (1), rcs (1), rcsdiff (1),
     rlog (1), rcsfile (5).
     Walter F. Tichy, "Design, Implementation, and Evaluation of
     a Revision Control System," in _P_r_o_c_e_e_d_i_n_g_s _o_f _t_h_e _6_t_h _I_n_t_e_r_-
     _n_a_t_i_o_n_a_l _C_o_n_f_e_r_e_n_c_e _o_n _S_o_f_t_w_a_r_e _E_n_g_i_n_e_e_r_i_n_g, IEEE, Tokyo,
     Sept. 1982.

BUGS
     _R_c_s_m_e_r_g_e does not work for files that contain lines with a
     single `.'.






































Printed 1/10/87              6/29/83                            2



