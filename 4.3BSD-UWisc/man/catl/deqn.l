


DEQN(1)             UNIX Programmer's Manual              DEQN(1)



NAME
     deqn  -  typeset mathematics

SYNOPSIS
     eqn [ -dxy ] [ -pn ] [ -sn ] [ -fn ] [ -Pdest ] [ file ] ...
     checkeq [ file ] ...

DESCRIPTION
     _D_e_q_n is a _d_i_t_r_o_f_f(_1) preprocessor equivalent to eqn but
     capable of producing input for _d_i_t_r_o_f_f instead of _v_t_r_o_f_f.
     Usage is almost always

          eqn file ... | ditroff

     If no files are specified, _d_e_q_n reads from the standard
     input.  _D_e_q_n prepares output for the typesetter named in the
     -P option (Varian/Versatec default, see _d_i_t_r_o_f_f(1)).

SEE ALSO
     eqn(1), ditroff(1), dtbl(1)



































Printed 1/10/87                                                 1



