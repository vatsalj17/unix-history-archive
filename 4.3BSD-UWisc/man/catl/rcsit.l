


RCSIT(1l)           UNIX Programmer's Manual            RCSIT(1l)



NAME
     rcsit - A program to prepare files for RCS

SYNOPSIS
     rcsit [ -chfsmMxp ] [ -qad ] [ -I_f_l_a_g_s ] [ -R_f_l_a_g_s ] [
     -H_d_i_r_e_c_t_o_r_y ] [ -F_h_e_a_d_e_r-_f_i_l_e ] file [ file1 file2 ... ]

DESCRIPTION
     _R_c_s_i_t is a semi-intelligent program to prepare files for
     _r_c_s(_1). This involves putting the correct type of headings
     at the top of the file so that _r_c_s(_1) will update a header
     and log changes in the file.

     By default, _r_c_s_i_t will use default headers ``hard-wired''
     into itself for each different file type that it ``knows''
     about.  (See below for list of known file types).

     If the -H_d_i_r_e_c_t_o_r_y option is specified, then _r_c_s_i_t will use
     ``.header.suffix'' files (where ``suffix'' is a suffix that
     _r_c_s_i_t ``knows'' about) found in _d_i_r_e_c_t_o_r_y. If a directory
     name is not specified on the command line, then the environ-
     ment variable TEMPLATES is used.  If TEMPLATES is not set,
     then the environment varialble, HOME is tried.

     The following template files are recognized:


           Template Name             File Type
           -------------------------------------------
           .header.c                 Standard C
           .header.h                 C Include
           .header.f                 Fortran
           .header.sh                Shell Script
           .header.make              Makefile
           .header.man               Manual
           .header.p                 Pascal
           .header.tex               TeX
9
     _R_c_s_i_t is ``semi-intelligent'' in that it can guess the type
     of headers to put in the file by the type of file (C pro-
     gram, C include, makefile, shell script, or manual).  It
     determines the file type by looking at the name of the file.
     If the name of the file is ``Makefile'' or ``makefile'',
     then the file type is taken to be for _m_a_k_e(_1). The suffix
     after the last ``.'' in the file name is then used.  The
     following table shows the suffixes that _r_c_s_i_t knows about:


           Suffix         File Type
           ----------------------------------------
           c              C Program
           h              C Include



Printed 1/10/87                                                 1


9


RCSIT(1l)           UNIX Programmer's Manual            RCSIT(1l)



           f              Fortran
           mk             _M_a_k_e(_1) file
           sh             Shell Script
           csh            Shell Script
           [1-9]          Manual (digits 1 - 9)
           p              Pascal
           tex            TeX
9
     If the environment variable ``RCSDIR'' is present, then
     _r_c_s_i_t will attempt to make a symbolic link from it to
     ``RCS'' in the current directory if the -_I option is used
     and there is no directory called ``RCS'' already, in the
     current directory. If the -_I option is used and ``RCSDIR''
     is not specified in the environment, then a normal directory
     called ``RCS'' will be created.  This feature can be over-
     rided with the -_d option.

OPTIONS
     c         Force file type to be ``Standard C''.

     h         Force file type to be ``C Include''.

     f         Force file type to be ``Fortran''.

     M         Force file type to be ``Manual''.  Note: If you
               also specify the ``I_f_l_a_g_s'' option, _r_c_s_i_t will run
               _r_c_s(_1) to tell rcs what kind of comment string to
               use for the manual file.

     s         Force file type to be ``Shell Script''.

     m         Force file type to be ``Makefile''.  Note that
               this does the same thing as the -s option does.
               It just prints a different message.

     p         Force file type to be ``Pascal''.

     x         Force file type to be ``TeX''.

     F_h_e_a_d_e_r-_f_i_l_e
               Use _h_e_a_d_e_r-_f_i_l_e as the header file.  (i.e. This
               file is prepended to each of the files to be pro-
               cessed.)

     t         Do not use any Template files for the headers.

     q         Be quiet.  Don't print out what is going on.  Only
               error messages are printed.

     d         Do not attempt to make the directory ``RCS''.  See
               above for more info.
9


Printed 1/10/87                                                 2






RCSIT(1l)           UNIX Programmer's Manual            RCSIT(1l)



     a         Turn off auto guessing of file type.

     I_f_l_a_g_s    Check In file.  Run RCS's _c_i(_1) on the file with
               ``flags'' being passed onto _c_i(_1) as the command
               line arguments.

     R_f_l_a_g_s    Run _r_c_s(_1) with ``flags'' as the command line
               arguments.

FILES
     /tmp/rcsit*    - temporary buffer
     ~/.header.*   - template files to use as the header.

AUTHOR
     Michael Cooper (mcooper@usc-oberon.ARPA)

SEE ALSO
     ci(1), make(1), rcs(1)

DIAGNOSTICS
     Complains about the usual stuff.  (i.e. - the specified file
     doesn't exist, or it can't read it...just things like that.)

BUGS
     Acts oddly in the presence of beautiful women.






























Printed 1/10/87                                                 3



