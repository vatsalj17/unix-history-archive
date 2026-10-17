# include  "util.h"
# include  "d_returns.h"
# include  <pwd.h>
# include  <stdio.h>
# include  "d_proto.h"
# include  "d_structs.h"

/*  4 Sept 81   D. Krafft (Cornell) & D. Crocker
 *                              getuser log calls didn't name routine
 *                              also converted to using getpwuid()
 */


/*
 *     routine which reads a line from the password file and pulls out the
 *     user's name
 */

d_getuser(uid, name, path)
  int  uid;
  char  *name, *path;
    {
    extern struct passwd *getpwuid ();
    register struct passwd *pwdptr;

/*  if we did this once, just used the stuff we saved  */

    if ((pwdptr = getpwuid (uid)) == 0)
      {
#ifdef D_LOG
      d_log("d_getuser", "couldn't find user in password file.\n");
#endif D_LOG
      return(D_FATAL);
      }

    if (name)
      (void) strcpy(name, pwdptr -> pw_name);


    if (path)
    {
	if (strlen(pwdptr -> pw_dir) > 64)
	  {
#ifdef D_LOG
	  d_log("d_getuser", "user's default path too long  '%s'\n",
			pwdptr -> pw_dir);
#endif D_LOG
	  return(D_FATAL);
	  }
	(void) strcpy(path, pwdptr -> pw_dir);
    }

    return(D_OK);
    }

/*
 *     D_CANON
 *
 *     routine which translates a C style string specification into
 *     internal format.  it understands 'escape' characters equivalents
 *     for certain control characters: newline ('\n'), carriage return ('\r'),
 *     tab ('\t'), form feed ('\f'), and delete ('\x').  in addition, any
 *     character can be specified with a backslash followed by up to 3 octal
 *     digits.
 *
 *     in -- pointer to canonical format string
 *
 *     out -- pointer to buffer for internal format equivalent.
 *
 *     the number of characters in the (null-terminated) output string is
 *     returned.  a return value of -1 indicates an error.
 */

d_canon(in, out)
  register char  *in, *out;
    {
    int  value, count, place;
    char  c;

    count = 0;

/*  check for obvious bad format  */

    if (*in++ != '"')
      return(-1);

/*  loop through all the characters.  lots of special cases  */

    while (1)
      {
      c = *in++;

      switch (c)
        {
        default:

            *out++ = c;
            break;

        case  '\0':

            return(-1);

        case  '"':

            *out = '\0';
            return(count);

        case  '\\':

/*  handle the escapes  */

            c = *in++;

            switch (c)
              {
              case  '"':

                  *out++ = '"';
                  break;

              case  '\0':

                  return(-1);

              case  '\\':

                  *out++ = '\\';
                  break;

              case  'r':

                  *out++ = '\15';
                  break;

              case  'n':

                  *out++ = '\12';
                  break;

              case  't':

                  *out++ = '\11';
                  break;

              case  'x':

                  *out++ = '\377';
                  break;

	      case  'b':

		  *out++ = '\10';
		  break;

	      case  '#':

		  *out++ = '\376';
		  break;

/*  allow the specification of characters by up to 3 octal digits after a  */
/*  backslash                                                              */

              default:

                  if (!d_isodigit(c))
                    {
                    *out++ = c;
                    break;
                    }

                  value = c & 07;

                  for (place = 0; place <= 1; place++)
                    {
                    c = *in++;

                    if (d_isodigit(c))
                      value = (value << 3) | (c & 07);
                    else
                      {
                      in--;
                      break;
                      }
                    }

                  *out++ = value;

              }
        }

      count++;
      }
    }

/*
 *     D_ISODIGIT
 *
 *     this routine returns a non-zero value if the argument is an octal
 *     digit, ie. between 0 and 7.
 *
 *     c -- the character to be tested
 */

d_isodigit(c)
  char  c;
    {

    if ((c >= '0') && (c <= '7'))
      return(1);
    else
      return(0);
    }

/*
 *     D_TRYFORK
 *
 *     the purpose of this routine is to fork, retrying 10 times if necessary
 *     -1 is returned if failure after all retries
 */

d_tryfork()
    {
    register int  try, pid;

    for (try = 0; try <= 9; try++)
      {
      pid = fork();

      if (pid != -1)
        return(pid);
      }

    return(-1);
    }


/*
 *     D_MINIMUM
 *
 *     this routine returns the minimum of its two arguments
 *
 *     a, b -- the minimum of these values is returned
 */

d_minimum(a, b)
  register int  a, b;
    {

    if (a < b)
      return(a);
    else
      return(b);
    }
