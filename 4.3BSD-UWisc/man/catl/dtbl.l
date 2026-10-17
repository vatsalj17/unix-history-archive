


DTBL(1)             UNIX Programmer's Manual              DTBL(1)



NAME
     dtbl - format tables for

SYNOPSIS
     dtbl [ files ] ...

DESCRIPTION
     _D_t_b_l is a preprocessor for formatting tables for _d_i_t_r_o_f_f(1).
     The input files are copied to the standard output, except
     for lines between .TS and .TE command lines, which are
     assumed to describe tables and reformatted.  Details are
     given in the reference manual.

     As an example, letting \t represent a tab (which should be
     typed as a genuine tab) the input

          .TS
          c s s
          c c s
          c c c
          l n n.
          _
          Household Population
          Town\tHouseholds
          \tNumber\tSize
          _
          Bedminster\t789\t3.26
          Bernards Twp.\t3087\t3.74
          Bernardsville\t2018\t3.30
          Bound Brook\t3425\t3.04
          Branchburg\t1644\t3.49
          Bridgewater\t7897\t3.81
          Far Hills\t240\t3.19
          _
          .TE

     yields

          c s s c c s c c c l n n.  _ Household Population
          Town Households      Number    Size _ Bedmin-
          ster     789  3.26 Bernards Twp.  3087 3.74 Bernards-
          ville  2018 3.30 Bound Brook    3425 3.04 Bran-
          chburg     1644 3.49 Bridgewater    7897 3.81 Far
          Hills 240  3.19 _

     If no arguments are given, _d_t_b_l reads the standard input, so
     it may be used as a filter.  When it is used with _d_e_q_n the
     _d_t_b_l command should be first, to minimize the volume of data
     passed through pipes.

SEE ALSO
     ditroff(1), deqn(1)



Printed 1/10/87                                                 1






DTBL(1)             UNIX Programmer's Manual              DTBL(1)



     M. E. Lesk, _T_B_L.






















































Printed 1/10/87                                                 2



