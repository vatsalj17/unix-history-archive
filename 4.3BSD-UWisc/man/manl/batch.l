.TH BATCH LOCAL uw
. \" @(#)batch.l	3.0 06/21/84
.UC 4
.SH NAME
batch, pbq, rmbq \- execute commands in a batch queue
.SH SYNOPSIS
.B batch
[
options
]
command
.PP
.B pbq
.PP
.B rmbq
[
job numbers
]
[
-u login
]
.SH DESCRIPTION
.I Batch
queues the command to be executed in order to reduce the system load average.
Jobs are executed in first come first serve order.
Batched jobs are given priority over normal background jobs,
so it is to your advantage to use it.  Note that any I/O redirection
in the batch command line will only redirect output from the
.I batch
command.  Commands may have an arbitrary number of arguments.
.sp
.I Batch options
.sp
.TP
.B \-i file
Redirect \fIfile\fR as the standard input to the command
.TP
.B \-o file
Redirect the standard output of the command to \fIfile\fR
.TP
.B \-e file
Redirect the standard error output of the command to \fIfile\fR
.TP
.B \-O file
Redirect both standard output and standard error to \fIfile\fR
.TP
.B \-n nice
Set priority to \fInice\fP instead of the default.
.TP
.B \-t
Truncate output files.
.TP
.B \-a
Append to output files.
.TP
.B \-f
Notify when the command has completed.
.TP
.B \-s
Notify when the command has started.
.TP
.B \-M
Any notifications made are via the
.IR mail (1)
command.
.TP
.B \-W
Any notifications made are via the
.IR write (1)
command.
.sp
The environment variable \fIBATCH\fR can be used to set default
batch options.
.br
.ti +5
Example: setenv BATCH \'\-sfo OUT \-e ERR\'
.br
will notify upon start and finish of any batch jobs, redirect standard output
to the file OUT, and redirect error output to the file ERR.
.TP
.br
.I Pbq
gives the current status of the batch queue.
The first line shows the current number of high priority jobs running,
the maximum number of concurrent high priority jobs, and the number of
low priority jobs running.
Batch will attempt to keep the maximum number of high priority jobs
going simultaneously.
For each job it lists its rank, owner, job number, time, and the command name.
An
.B R
next to the job number indicates that the job is currently running.
An
.B N
indicates that the job's priority has been lowered due to its long running time.
The time field indicates the real time that the job has been running.
.sp
.TP
.br
.I Rmbq
removes the specified jobs from the batch queue.
You must either be the owner of the job,
or have super-user privileges to remove it.
.I Rmbq -u login
removes all jobs owned by
.I login.
.br
.SH SEE ALSO
.IR mail (1),
.IR write (1)
.SH AUTHOR
Allan Bricker
