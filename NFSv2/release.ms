.hy 0
.OH '''
.EH '''
.OF '700-1080-1'- % -'Sun Microsystems, Inc.'
.EF 'Sun Microsystems, Inc.'- % -'700-1080-1'
.TL
\s24READ THIS FIRST\s0
.sp 2
\s18Notes for NFS Release 2.0\s0
.sp 2
December 16, 1985
.PP
.SH
\s+8Introduction\s0
.sp 2
.LP
The 2.0 release is the first release of Sun's implementation
of the Network File System (NFS) to non-Sun developers.
This software
must be licensed from Sun.
Call Bill Keating, (415) 960-1300 x7724
for licensing information and questions.
.LP
The NFS technical hotline number is (415) 960-2770.
Questions can
also be sent via electronic mail to \fIsun!nfsnet\fR.
.LP
This release is intended for use by developers porting NFS.
It is
not intended to be a stand-alone software product.
An in-depth understanding
of the
kernel is required to work with this distribution.
.sp 3
.SH
\s+8Release Description\s0
.sp 2
.LP
This release is a subset of Sun version 2.0 source code.
It contains
all files which Sun has added or changed to implement the NFS, relative
to Berkeley version 4.2. for Vaxen\(dd.
.FS \(dd
Vax is a trademark of Digital Equipment Corporation.
.FE
Makefiles for integrating this
release into the standard Berkeley 4.2 distribution are included.
.LP
The NFS distribution source code will always match Sun's production
source code.
The makefiles and associated system building procedures
are somewhat different, however.
They integrate NFS into a standard
Berkely 4.2 Vax system.
.LP
Sun has made many non-NFS improvements and bug fixes to 4.2.
Some
of these changes are an intrinsic part of the files changed for NFS.
Additionally, some important bug fixes that are required for NFS to
work are included.
These are detailed in the Sun Compatability Issues
and 4.2 Bug Fixes sections.
.LP
Sun has made several changes to the layout of \fB/usr/src\fR.
For example,
\fB/usr/src/sys\fR in Berkeley 4.2 is \fB/usr/src/sun/sys\fR in the Sun source.
These changes are accounted for in the makefiles and the Vax Installation
section.
.bp
.SH
\s+64.2 Bug Fixes\s0
.sp 2
.LP
This section documents items included in the NFS distribution
for the purpose of fixing critical 4.2 bugs.
These fixes are neccessary for
the proper execution of NFS.
.NH
\s+4Kernel Networking Fixes\s0
.sp
.LP
The directories \fBsun/sys/{net, netimp, netinet, netpup, vaxif}\fR contain
numerous bug fixes and performance improvements to the kernel networking code
and ethernet drivers.
We have also raised the arbitrary limit put on UDP
packet size  from 2K to 9K.
.NH
\s+4C Compiler Fixes\s0
.sp
.LP
Two sets of fixes have been supplied for the C compiler:
.RS
\fBlib/mip/trees.c\fR fixes:
.RS
Improper unsigned constant folding.
.br
Spurious type mismatch errors in expressions involving pointers to
void functions.
.br
Erroneous handling of casts in compile-time initializations.
.RE
\fBlib/pcc/local2.c\fR fixes:
.RS
Constants folded incorrectly in expressions containing '&'.
.RE
.RE
.NH
\s+4etc/ifconfig.c\s0
.sp
.LP
The \fIifconfig\fR supplied with 4.2 erroneously resets the 'broadcast'
flag of an interface.
Yellow pages requires a broadcast facility.
The \fIifconfig\fR included with this release fixes this problem.
.NH
\s+4bin/mv.c\s0
.sp
.LP
The original 4.2 \fImv\fR is dependent on the order that the kernel
returns error codes when there is more than one error.
The NFS kernel
happens to return error codes in a different order than the original
4.2 kernel.
The \fImv\fR supplied with this release corrects this problem.
.bp
.SH
\s+6Sun Compatibility Issues\s0
.sp 2
.LP
Some of the non-NFS changes that Sun has made to the 4.2 kernel
and libraries cannot be separated from the NFS changes.
This section
lists the changes that have been included in this distribution.
.NH 0
\s+4Non-NFS Kernel Changes\s0
.sp
.LP
The following files have been included for compatability with
non-NFS kernel changes:
.RS
\fBsun/sys/vax/locore.s\fR
.RS
The cmap structure has changed.
.RE
\fBsun/sys/vax/param.h\fR
.br
\fBbin/ps.c\fR
.RS
the u structure has changed.
.RE
\fBsun/sys/vax/reg.h\fR
.RS
A traceback debugging feature has been added.
.RE
\fBsun/sys/vax/trap.c\fR
.br
\fBlib/libc/sys/SYS.h\fR
.RS
System call argument passing
has been enhanced.
This change involves passing the user block directly instead of
indirectly.
This makes the code cleaner, and it also
allows system calls to be called from inside the kernel.
.RE
\fBsun/sys/vax/autoconf.c\fR
.RS
Sun has moved the initialization of mba devices
in \fBsun/sys/sys/init_main.c\fR.
This impacts the \fIswapconf()\fR routine.
.RE
.RE
.NH
\s+4Sethostid() System Call\s0
.sp
.LP
Sun does not use the \fIsethostid()\fR system call since our hostid
is in PROM.
The PROM is accessed by the \fImachineid()\fR routine in
\fBsun/sys/vax/autoconf.c\fR, a no-op routine for Vaxen.
You can put this call back into
the kernel without any problem if you require it.
.NH
\s+4dk_mspw Kernel Entry Point\s0
.LP
.sp
The \fIdk_mspw\fR entry point has been
replaced with the entry point \fIdk_bps\fR.
\fIdk_bps\fR gives disk transfer data in bytes per second.
\fBsun/sys/vaxmba/hp.c\fR
and \fBetc/iostat.c\fR have been modified to use \fIdk_bps\fR.
.NH
\s+4lib/libc/net/ruserpass.c\s0
.sp
.LP
The \fIruserpass()\fR routine has been cleaned up
to remove Berkeley specific code
and renamed to \fI_ruserpass()\fR.
\fI_ruserpass()\fR
is included because it is used by \fBin.rlogind\fR.
Other
users of \fIruserpass()\fR
such as \fBucb/ftp\fR and \fBlibc/net/rexec\fR
should reference
it as \fI_ruserpass()\fR.
.NH
\s+4Quotas\s0
.sp
.LP
NFS release 2.0 does not support quotas.
Be sure to turn off quotas before compiling your kernels.
.NH
\s+4Utility Rearrangement\s0
.LP
.sp
Beware of a rearrangement of the programs in \fB/bin\fR,
\fB/etc\fR and \fB/usr/bin\fR.
Sun has also added a new utility directory \fB/usr/etc\fR.
Sources are
stored in \fBsrc/usr.etc\fR.
.bp
.SH
\s+6RPC/NFS Changes\s0
.sp 2
.LP
This section highlights the items that have been included in
this distribution because of RPC and NFS.
.NH 0
\s+4Kernel Changes\s0
.sp
.LP
The entire kernel core (\fBsun/sys/sys\fR)
has been affected by the
addition of the vnode/vfs layer.
Some other places that the kernel
must now use vnodes are in
\fBsun/sys/vax/{machdep.c,vm_machdef.c}\fR.
The NFS client and server routines are in
\fBsun/sys/nfs\fR.
The 4.2 filesystem routines have been moved out of
\fBsun/sys/sys\fR to \fBsun/sys/ufs.\fR
.NH
\s+4RPC Libraries\s0
.sp
.LP
The RPC library is in \fBlib/libc/rpc\fR.
The kernel accesses these
libraries via the symbolic link \fBsun/sys/rpc\fR.
.NH
\s+4System Call Interface\s0
.sp
.LP
The following files in lib/libc have been modified because of NFS:
.RS
.B gen/
.RS
.B
closedir.c
opendir.c
readdir.c
mntent.c
scandir.c
seekdir.c
telldir.c
.R
.RE
.RE
.LP
The following files in lib/libc have been added because of NFS:
.RS
.B sys/
.RS
.B
async_daemon.c
fstatfs.c
getfh.c
nfsmount.c
nfssvc.c
statfs.c
unmount.c
.R
.RE
.B gen/
.RS
.B mntent.c
.RE
.RE
.LP
Also, the system call interface has been changed.
Please see the documentation
on this in the \fBSun Compatibility Issues\fR section.
.NH
\s+4Utilities Affected by /usr/include Changes\s0
.sp
.LP
All filesystem specific include files such as \fBinode.h\fR
have been moved to \fB/usr/include/ufs\fR.
Some data structures have been
renamed because of the vnode layer.
The following user level utilities
have been slightly modified because of this:
.RS
.B etc/
.RS
.B
clri.c
mkfs.c
tunefs.c
.R
.RE
.B usr.etc/
.RS
.B
analyze.c
dcheck.c
dumpfs/
icheck.c
mkproto.c
ncheck/
quot.c
.R
.RE
.RE
.NH
\s+4Utilities Affected By Time Skew Problems\s0
.sp
.LP
The following utilities needed to be modified in order to handle
the case where a server's clock is ahead of its client's:
.RS
.B bin/
.RS
.B ls.c
.RE
.B usr.bin/
.RS
.B ranlib.c
.RE
.RE
.KS
.NH
\s+4Modified Utilities\s0
.sp
.LP
The following utilities were modified for the given reasons:
.RS
\fBbin/df.c\fR			Remote filesystem display added
.br
\fBetc/mount.c\fR			Mount and unmount remote filesystems
.br
\fBetc/umount.c\fR		Mount and unmount remote filesystems
.br
\fBetc/newfs.c\fR			Calls inode number randomizer (fsirand)
.br
\fBetc/shutdown.c\fR		Sends shutdown messages to remote client
.br
\fBetc/pstat.c\fR			Reports about vnodes instead of inodes
.br
\fBetc/rcp.c\fR			No more setuid root on remote file access
.br
\fBusr.etc/savecore.c\fR	Modified to use VFS
.br
\fBetc/dump\fR			Uses new fstab format
.br
\fBetc/restore/\fR			Uses new fstab format
.br
\fBetc/fsck/\fR			Bumps inode generation numbers
.RE
.KE
.NH
\s+4New Utilities\s0
.sp
.LP
The following utilities were added:
.RS
.B etc/
.RS
.B
showmount.c
fsirand.c
rcp.rwalld.c
.R
.RE
.B usr.etc/
.RS
.B rpcinfo.c
.RE
.RE
.NH
\s+4Emacs\s0
.sp
.LP
Emacs notes the time at which it writes an edit file in order to detect
someone else writing the file.
Unfortunately, it uses the time of day clock for this
instead of getting it from a \fIstat()\fR of the file.
the latest version of Unipress Emacs fixes this problem.
.NH
\s+4NFS/RPC Daemons\s0
.sp
.LP
Sun uses a super-daemon (\fBetc/inetd\fR)
to fire off the various internet
service daemons.
The RPC portmapper (\fBetc/portmap.c\fR)
and the NFS server
and client daemons (\fBetc/nfsd.c\fR, \fBetc/biod.c\fR)
are initiated in this
manner.
The super-daemon uses file \fB/etc/servers\fR
to tell it what to
fire off.
See \fI man 5 servers\fR for the file format.
.bp
.SH
\s+6Yellow Pages/Network Services Changes\s0
.sp 2
.LP
This section highlights the items that have been included in
this distribution because of the yellow pages and other user-level
network services (such as \fIrstat\fR, \fIyppasswd\fR, etc).
.NH 0
\s+4RPC Services Library\s0
.sp
.LP
Libraries for the yellow pages and other network services are
in \fBlib/libc/rpcsvc\fR. Note that the NFS mount service libraries are
here also.
.NH
\s+4System Calls Interface\s0
.sp
.LP
The following files in \fBlib/libc\fR have been modified to use the
yellow pages:
.RS
.B gen/
.RS
.B
getgrent.c
getpwent.c
.R
.RE
.B net/
.RS
.B
gethostent.c
getnetent.c
getprotoent.c
getservent.c
rcmd.c
.R
.RE
.RE
.LP
The following files in \fBlib/libc\fR have been added because of
the yellow pages:
.RS
.B sys/
.RS
.B
getdirentries.c
getdomainname.c
setdomainname.c
.R
.RE
.RE
.NH
\s+4usr.lib/libdbm\s0
.sp
.LP
The yellow pages require multiple databases per processes. In
order to do this we introduced a new dbm call, \fIdbmclose()\fR.
This allows
a process to close a database and then open a new one.
.NH
\s+4Utility Changes\s0
.sp
.LP
The following utilities were modified so that they use
the yellow pages:
.RS
.B bin/
.RS
.B
login.c
passwd.c
.R
.RE
.B ucb/
.RS
.B
chsh.c
netstat.c
rcp.c
.R
.RE
.B usr.bin/
.RS
.B find.c
.RE
.RE
.LP
The following utilities were added to support the yellow pages:
.RS
.B bin/
.RS
.B domainname.c
.RE
.B etc/
.RS
.B rwall.c
.RE
.B usr.bin/
.RS
.B
ypcat.c
yppasswd.c
ypwhich.c
.R
.RE
.RE
.NH
\s+4Yellow Page Daemons And Database Maintenance Programs\s0
.sp
.LP
The yellow page server daemon is in \fBetc/ypserv\fR.
The yellow page
binder is \fBetc/ypbind.c\fR.
Directory \fBetc/yp\fR contains the yellow page
database maintenance programs.
.NH
\s+4Remote Login Daemon\s0
.sp
.LP
The rlogin daemon
\fBetc/in.rlogind\fR has been changed to use the
yellow pages.
\fBin.rlogind\fR is invoked by the internet super-daemon
\fB/etc/inetd\fR.
.NH
\s+4Additional Network Service Daemons\s0
.sp
.LP
The following network services daemons are also invoked by the
internet super-daemon:
.RS
.B usr/etc/
.RS
.B
rpc.mountd
rpc.rstatd
rpc.rwalld
rpc.yppasswdd
.R
.RE
.RE
.bp
.SH
\s+6Additional Noteworthy Items\s0
.sp 2
.NH 0
\s+4usr/src/sun/doc\s0
.sp
.LP
The directory \fBusr/src/sun/doc\fR
contains the relevant parts of the "Networking
On The Sun" manual.
This version has been modified from the original Sun documentation to use
the -ms macro package
and to remove most of the Sun specific information.
Formatting instructions are in the \fBREADME\fR file.
.NH
\s+4Release Notes\s0
.sp
.LP
A machine readable version of these release notes are in the file
\fBrelese.ms\fR.
.NH
\s+4usr/src/tags\s0
.sp
.LP
This is a vi tags file of the NFS distribution kernel and libc.
The files included are listed in \fBfiles.tag\fR.
.NH
\s+4Examples of /etc Files\s0
.sp
.LP
The following files are included as examples:
.RS
.B samples/
.RS
.B
rc.local
services
servers
.R
.RE
.RE
.bp
.SH
\s+6Vax Installation\s0
.sp 2
.LP
This section lists the steps required to install this distribution
on a Vax running 4.2.
.sp
.IP 1.
Create a directory for the NFS source.
.RS
.B
mkdir /usr/nfs
.br
cd /usr/nfs
.br
tar x
.R
.RE
.sp
.IP 2.
Change the 4.2 source directory structure to match NFS.
.RS
.B
mkdir /usr/src/sun
.R
.RE
.sp
.IP 3.
Move the original kernel source from \fB/usr/src/sys\fR
to \fB/usr/src/sun/sys\fR
and make a symbolic link there from \fB/sys\fR.
.RS
.B
mv /usr/src/sys /usr/src/sun
.br
ln -s /usr/src/sun/sys /sys
.R
.RE
.sp
.IP 4.
Move \fBlib/libc/vax/sys\fR to \fBlib/libc/sys\fR.
.RS
.B
mv /usr/src/lib/libc/vax/sys /usr/src/lib/libc/sys
.R
.RE
.sp
.IP 5.
Copy and merge the NFS files into the system source (things will
never be the same after this...)
.RS
.B
cp -r /usr/nfs/usr/src /usr
.R
.RE
.sp
.IP 6.
Install \fB/usr/include\fR.
.RS
.B
cd /usr/src/include
.br
make install
.R
.RE
.sp
.IP 7.
The original 4.2 C compiler has some fatal bugs
(See the \fB4.2 Bugs\fR section).
If you need these fixes recompile the
compiler.
.RS
.B
cd /usr/src/lib/pcc
.br
make
.br
make install
.R
.RE
.sp
.IP 8.
Create the \fBlibc\fR and \fBdbm\fR libraries.
.RS
Edit \fIruserpass()\fR
to \fI_ruserpass()\fR in
\fBlibc/net/rexec.c\fR
(see the \fBSun Compatibility Issues\fR section)
.RE
compile and install the libraries:
.RS
.B
cd /usr/src/lib/libc
.br
make
.br
make install
.br
cd /usr/src/usr.lib/libdbm
.br
make
.br
make install
.R
.RE
.sp
.IP 9.
Create 
a configuration
file called NFSVAX
in the directory
\fB/usr/src/sun/sys/conf\fR
using the file \fBGENERIC.VAX\fR file as a guide.
The lines
that must be included in an NFS configuration file are:
.RS
options         RPC
.br
options         NFS
.br
pseudo-device   nfs
.br
pseudo-device   rpc
.RE
.sp
.IP 10.
Make a new kernel configuration directory.
.RS
.B
cd /usr/src/sun/sys/conf
.br
mkdir ../NFSVAX
.br
config NFSVAX
.br
cd ../NFSVAX
.br
make depend
.R
.RE
.sp
.IP 11.
Edit the makefile and remove all \fB../sun/*.h\fR dependencies.
Some kernel
source files make reference to \fB../sun/*.h\fR include files.
They are 'ifdef'ed out of the source code, but 'make depend' still puts them
into the makefile.
.sp
.IP 12.
Compile the kernel.
.RS
.B make
.RE
.sp
.IP 13.
Change
\fIruserpass()\fR to \fI_ruserpass()\fR
in
\fBucb/ftp/ftp.c\fR
(see the \fBSun Compatibility Issues\fR section) and
then compile:
.RS
.B
foreach i (/usr/src/{usr.bin,usr.etc,ucb,etc,bin})
.RS
.B
cd $i
.br
make
.br
cd ..
.br
.R
.RE
.B end
.RE
.sp
.IP 14.
Modify the following files so they conform to the new formats.
.RS
\fB/etc/fstab\fR.
See \fB/usr/man/man5/fstab.5\fR
for the format.
.br
\fB/etc/rc.local\fR.
See \fB/usr/nfs.src/samples/rc.local\fR
for an example.
.br
\fB/etc/services\fR.
See \fB/usr/nfs.src/samples/services\fR
for an example.
.RE
.sp
.IP 15.
Create the following files.
.RS
\fB/etc/exports\fR.
See \fB/usr/man/man5/exports.5a\fR for a description
of this file.
.br
\fB/etc/servers\fR.
See \fB/usr/nfs.src/samples/servers\fR for an example.
.RE
.sp
.IP 17.
Install everything.
.RS
.B
cp /vmunix /vmunix.orig
.br
cp /usr/src/sun/sys/NFSVAX/vmunix /vmunix
.br
foreach i (/usr/src/{usr.bin,usr.etc,ucb,etc,bin})
.RS
.B
cd $i
.br
make install
.br
cd ..
.R
.RE
.B end
.RE
.sp
.IP 18.
Boot the system!
