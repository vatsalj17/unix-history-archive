# define  D_CONTIN	   2	 /*  continue with process  */
# define  D_OK             1     /*  everything is fine  */
# define  D_NO             0     /*  something is wrong  */
# define  D_FATAL         -1     /*  fatal error.  this is returned to  */
				 /*  the calling program.               */
# define  D_NONFATAL      -2     /*  non-fatal error.  this is only used  */
				 /*  internally and will not be returned  */
				 /*  to the calling program.              */
# define  D_INTRPT        -3     /*  interrupt received during system call  */
# define  D_EOF		  -4     /*  unexpected EOF on script file  */
# define  D_UNKNOWN	  -5     /*  unknown command word  */
# define  D_QUOTE         -6     /*  unmatched quotes  */
# define  D_NFIELDS       -7     /*  wrong number of fields for cmd  */
# define  D_RETRY	  -8     /*  retry a failed packet transmission  */




/*  error codes.  these values are loaded into 'd_errno' to indicate the  */
/*  reason for one of the above error returns                             */

# define  D_BUSY           1     /*  the number dialed is busy  */
# define  D_ABAN           2     /*  the call was abandoned or no answer  */
# define  D_BADDIG         3     /*  bad digit passed to dialer (internal)  */
# define  D_NOPWR          4     /*  the dialer has no power  */
# define  D_SYSERR         5     /*  undistinguished system error  */
# define  D_LOCKERR        6     /*  error opening or creating lock file  */
# define  D_FORKERR        7     /*  couldn't fork after several tries  */
# define  D_PORTOPEN       8     /*  error trying to open port  */
# define  D_NOPORT         9     /*  no port or line available  */
# define  D_TSOPEN        10     /*  error opening transcript file  */
# define  D_TSWRITE       11     /*  error writing on transcript file  */
# define  D_PORTRD        12     /*  error on port read  */
# define  D_PORTWRT       13     /*  error on port write  */
# define  D_PORTEOF       14     /*  eof on port  */
# define  D_RCVQUIT       15     /*  a QUIT packet has been received  */
# define  D_PRTSGTTY      16     /*  error doing stty or gtty on port  */
# define  D_SCRERR        17     /*  error in script file  */
# define  D_ACCERR        18     /*  error in access file  */
# define  D_NORESP        19     /*  no response to transmitted packet  */
# define  D_PATHERR       20     /*  error in path packet  */
# define  D_CALLLOG       21     /*  error opening call log file  */
# define  D_PACKERR       22     /*  error in packet format  */
# define  D_INTERR        23     /*  internal package error  */
# define  D_NONUMBS       24     /*  no numbers given to dialer  */
# define  D_NOESC         25     /*  no esacpe character could be found  */
# define  D_BADSPD        26     /*  bad speed designation  */
# define  D_LOGERR        27     /*  log error  */
# define  D_TIMEOUT       28     /*  alarm during port read or write call  */
# define  D_INITERR       29     /*  trouble initializing  */
