


BATCH(LOCAL)        UNIX Programmer's Manual         BATCH(LOCAL)



NAME
     batch, pbq, rmbq - execute commands in a batch queue

SYNOPSIS
     batch [ options ] command

     pbq

     rmbq [ job numbers ] [ -u login ]

DESCRIPTION
     _B_a_t_c_h queues the command to be executed in order to reduce
     the system load average.  Jobs are executed in first come
     first serve order.  Batched jobs are given priority over
     normal background jobs, so it is to your advantage to use
     it.  Note that any I/O redirection in the batch command line
     will only redirect output from the _b_a_t_c_h command.  Commands
     may have an arbitrary number of arguments.

     _B_a_t_c_h _o_p_t_i_o_n_s


     -i file
          Redirect _f_i_l_e as the standard input to the command

     -o file
          Redirect the standard output of the command to _f_i_l_e

     -e file
          Redirect the standard error output of the command to
          _f_i_l_e

     -O file
          Redirect both standard output and standard error to
          _f_i_l_e

     -n nice
          Set priority to _n_i_c_e instead of the default.

     -t   Truncate output files.

     -a   Append to output files.

     -f   Notify when the command has completed.

     -s   Notify when the command has started.

     -M   Any notifications made are via the _m_a_i_l(1) command.

     -W   Any notifications made are via the _w_r_i_t_e(1) command.

          The environment variable _B_A_T_C_H can be used to set



Printed 1/10/87                uw                               1






BATCH(LOCAL)        UNIX Programmer's Manual         BATCH(LOCAL)



          default batch options.
               Example: setenv BATCH '-sfo OUT -e ERR'
          will notify upon start and finish of any batch jobs,
          redirect standard output to the file OUT, and redirect
          error output to the file ERR.

     _P_b_q  gives the current status of the batch queue.  The first
          line shows the current number of high priority jobs
          running, the maximum number of concurrent high priority
          jobs, and the number of low priority jobs running.
          Batch will attempt to keep the maximum number of high
          priority jobs going simultaneously.  For each job it
          lists its rank, owner, job number, time, and the com-
          mand name.  An R next to the job number indicates that
          the job is currently running.  An N indicates that the
          job's priority has been lowered due to its long running
          time.  The time field indicates the real time that the
          job has been running.


     _R_m_b_q removes the specified jobs from the batch queue.  You
          must either be the owner of the job, or have super-user
          privileges to remove it.  _R_m_b_q -_u _l_o_g_i_n removes all
          jobs owned by _l_o_g_i_n.

SEE ALSO
     _m_a_i_l(1), _w_r_i_t_e(1)

AUTHOR
     Allan Bricker

























Printed 1/10/87                uw                               2



