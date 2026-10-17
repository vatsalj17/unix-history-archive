Revision 1.1  86/12/26  17:59:16  root Initial revision





























































Printed 1/10/87                                                 1






NAG(1)              UNIX Programmer's Manual               NAG(1)



NAME
     nag - annoying reminder service daemon.

SYNOPSIS
     nag
     okok

DESCRIPTION
     _N_a_g issues reminders from your ~/.nag file as specified.
     You start _n_a_g in your ._p_r_o_f_i_l_e or ._l_o_g_i_n, where it lies
     around much like _l_e_a_v_e(_1) until you logout.  It reads the
     .nag file, and executes reminder commands at intervals
     before a time.  Reminders are silenced with _o_k_o_k. When you
     log off, _n_a_g exits just before it would have executed a com-
     mand on your behalf.

     _N_a_g ignores interrupts, quits and terminates.  To get rid of
     it, you can log off or use ``kill -9.''

     The .nag file has the format:

          [ status ] day time interval command

     _s_t_a_t_u_s is either
     (1) '#', marking a comment line
     (2) ':', turning off execution of commands for the event.

     _d_a_y is one of
     (1) A date such as mm/dd/yy, or reasonable alternatives.
     (2) `*' meaning every day.
     (3) A day of the week.  Day names may be recognizably abbre-
     viated.

     _t_i_m_e is a standard time specification, like ``12:05'' or
     ``3AM''

     _i_n_t_e_r_v_a_l is a colon separated list of signed integer offsets
     from the event _t_i_m_e the _c_o_m_m_a_n_d will be executed.  Negative
     values are before the event, 0 at the event, and positive
     values later.

     _c_o_m_m_a_n_d is a string up to a newline that will be executed by
     _s_y_s_t_e_m(_3) on your behalf at the specified _i_n_t_e_r_v_a_l until you
     shut it up with _o_k_o_k.

     Some shell variables are set for use by the command:
     pretime is set to the -interval.
     posttime is set to the interval.
     now is set to an hh:mm string of the present time, in 24
     hour format.
     then is set to an hh:mm string of the event time.




Printed 1/10/87   Thu Sep  4 11:48:01 PDT 1986                  1






NAG(1)              UNIX Programmer's Manual               NAG(1)



     _o_k_o_k shuts up the the nagging.  It opens the terminal and
     prompts with the command(s) nearing their times, like rm -i.
     Answering `y' silences further executions of the command.
     It does this by editing the .nag file, which is noticed by
     any running _n_a_g processes the next time they wake up.

     When a silenced event is past it's last possible occurrance,
     _n_a_g re-enables it by writing the .nag file.  This makes `*'
     reminders work the next day.


EXAMPLES
     A .nag file might have the following entries:

      8/8/88 2PM -60:-30:-15:0 writebig "$pretime to Client call
     at $then"
      Mon 3PM -60:-30:-15:0 hey "Status report due in $pretime
     minutes"
      Wed 3:30PM -30:-15:-5:0 echo "Committee at $then in $pre-
     time minutes"
      * 13:00 -45:-30:0 writebig "Lunch in $pretime minutes"
      * 7PM -5:-4:-3:-2:-1:0 echo "GO HOME!!! It's $now"
      * 10:30 -10:-5:0 echo "Donut truck in $pretime minutes"

FILES
     ~/.nag              file describing things to do.

ENVIRONMENT
     NAGFILE             a file to use as the

SEE ALSO
     at(1), calendar(1), hey(1), leave(1), sleep(1), cron(8)

DIAGNOSTICS
     Complains about various syntax errors and times out of
     range, and problems accessing .nag file

BUGS
     Probably won't work with `stty nostop' set.

     Intervals will be approximate because of sleep granularity
     and scheduling contention.

     There is no locking of the .nag file.  This could be trouble
     if a _n_a_g and an _o_k_o_k decide to write at the same time.

AUTHOR
     David Brower
     {sun, cbosgd, amdahl, mtxinu}!rtech!gonzo!daveb






Printed 1/10/87   Thu Sep  4 11:48:01 PDT 1986                  2



