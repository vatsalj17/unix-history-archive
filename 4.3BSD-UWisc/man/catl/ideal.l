


IDEAL(1)            UNIX Programmer's Manual             IDEAL(1)



NAME
     ideal - ditroff preprocessor for drawing pictures

SYNOPSIS
     ideal [ -P_d_e_v ] [ -n ] [ files ]

DESCRIPTION
     _I_d_e_a_l is yet another troff preprocessor for drawing figures
     on a typesetter.  The -P option tells ideal to prepare out-
     put for printer _d_e_v (default is varian/versatec).  The -n
     option produces raw _i_d_e_a_l output, which passes unharmed
     through either _n_r_o_f_f(1) or _d_i_t_r_o_f_f(1).  A line beginning
     `.IS' marks the start of an _i_d_e_a_l program.  An _i_d_e_a_l program
     ends with `.IE' or `.IF'; `.IE' leaves you below the bottom
     of the picture, while `.IF' (flyback) leaves you at the same
     place you were when you said `.IS'.

     The building block for _i_d_e_a_l programs is a ``box''; boxes
     look like C functions, in that they are named, and delimited
     by braces.  They may include the following kinds of state-
     ments, each terminated by a semicolon:

     _v_a_r  declares one or more complex variables local to the
          box.  Variable names are made up of letters and digits,
          and start with a letter; do not use any of the follow-
          ing keywords as variable names: at, bdlist, boundary,
          box, conn, construct, draw, exterior, interior, left,
          opaque, put, right, spline, text, to, using, var

     _e_q_u_a_t_i_o_n
          declares relative positions of significant points of
          the box

     _c_o_n_n asks for a straight-line path through named points

     _p_e_n  asks for a box to be replicated along a line between
          two points

     _l_e_f_t left-justifies text with respect to a point

     _t_e_x_t centers text with respect to a point

     _r_i_g_h_t
          right-justifies text with respect to a point

     _s_p_l_i_n_e
          draws a spline guided by the named points

     _p_u_t  asks for an instance of a box to be drawn

     _o_p_a_q_u_e
          asks for a box to erase lines already in the picture



Printed 1/10/87                                                 1






IDEAL(1)            UNIX Programmer's Manual             IDEAL(1)



          that are covered by its bounding polygon

     _b_o_u_n_d_a_r_y
          specifies the bounding polygon for an opaque box

     _c_o_n_s_t_r_u_c_t
          builds a partial picture on a separate ``sheet of
          paper''

     _d_r_a_w adds the contents of the named constructed box to the
          current picture

     _I_d_e_a_l expects all components of a picture to be specified as
     boxes; instructions to draw the entire picture should
     comprise a box called ``main.'' Boxes are remembered across
     .IS-.IE boundaries; if you won't need a box again, you can
     reclaim the space it requires by including the command
     `...forget boxname' on a line between any .IS-.IE pair after
     the last use of boxname.  Box _m_a_i_n is an exception to this
     rule: it is always forgotten when the .IE is processed.

     During its first pass, _i_d_e_a_l solves all the equations to
     determine the locations of all points it needs to know.
     These equations must be linear equations in complex vari-
     ables, although they may include non-linear operators: _i_d_e_a_l
     plugs in for as many variables, and does as much function
     evaluation, as it can before solving the linear equation.
     It waits until it has absolutely no hope of reducing an
     equation to a linear equation before complaining.  _I_d_e_a_l
     knows about the following functions:

     _A[_z,_w]
           =__z+_A(_w-_z), _A of the way from _z to _w

     _r_e (_z)
          real part of complex number _z

     _i_m (_z)
          imaginary part of complex number _z

     _c_o_n_j (_z)
          complex conjugate of complex number _z

     _a_b_s (_z)
          absolute value (modulus) of complex number _z

     _c_i_s (_x)
          a unit vector in the direction of the real part of its
          argument _x, which is an angle in degrees (radians if
          the line `...radians' appeared more recently in the
          file than the line `...degrees')




Printed 1/10/87                                                 2






IDEAL(1)            UNIX Programmer's Manual             IDEAL(1)



     _E (_x)
          =__c_i_s(360_x) if _x is measured in degrees

     _i_n_t (_x)
          integer part of the real part of its argument _x

     _a_n_g_l_e (_z)
          arctangent of _i_m(_z)/_r_e(_z)

     During the second pass, _i_d_e_a_l draws the picture.

     To draw a circle, include the line `...libfile circle'
     between the .IS and .IE lines, and _p_u_t the box named _c_i_r_c_l_e,
     giving enough information that the circle can be determined;
     for instance, give the center and the radius, or give three
     points through which the circle passes, or give the center
     and a point on the circle.  The circle has center _c_e_n_t_e_r,
     radius _r_a_d_i_u_s, and passes through _z1, _z2, and _z3.

     To draw an arc, include the line `...libfile arc'  between
     the .IS and .IE lines, and _p_u_t the box named _a_r_c, again giv-
     ing enough information to determine the arc; for instance,
     give the center, radius, and starting and ending angles, or
     give three points on the arc--where to start, where to end,
     and somewhere in between.  The arc has center _c_e_n_t_e_r, radius
     _r_a_d_i_u_s, starts at point _s_t_a_r_t at angle _s_t_a_r_t_a_n_g, passes
     through point _m_i_d_w_a_y at angle _m_i_d_a_n_g, and ends at point _e_n_d
     at angle _e_n_d_a_n_g.  If no _m_i_d_w_a_y is specified, the arc is
     drawn counterclockwise from _s_t_a_r_t to _e_n_d.

     The picture will be scaled to a default width of four inches
     and centered in a column of six inches.  The default width
     can be changed by a `...width' command, which includes a
     number in inches.  The default column width can be changed
     by a `...colwid' command.  To defeat _i_d_e_a_l's notion of the
     size of the picture, you can include lines of the form
     `...minx', `...miny', `...maxx', or `...maxy'; these give
     the various coordinates of the bounding box of the picture
     in the coordinate system used by the picture.

     _I_d_e_a_l supports both C-style comments (between /* and */
     brackets, and they nest), and shell-style comments (between
     # and newline).

SEE ALSO
     ditroff(1), grn(1), pic(1)
     _I_D_E_A_L _U_s_e_r'_s _M_a_n_u_a_l , C. J. Van Wyk.

BUGS
     _I_d_e_a_l is relatively unforgiving about syntax errors.  Bound-
     ing box computation is naive for arcs and text strings.




Printed 1/10/87                                                 3



