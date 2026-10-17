.\" @(#)c6.nfs.spec 1.1 85/04/09	Sun Microsystems
.OH 'NFS Protocol Spec''Page \\\\n(PN'
.EH 'Page \\\\n(PN''NFS Protocol Spec'
.OF 'Sun Microsystems''Release 2.0'
.EF 'Sun Microsystems''Release 2.0'
.	\" LX - listing font and index
.de LX
.LW "\\$1" "\\$2" \\$3"
.IX "\\$1"
..
.RP
.rm DY
.TL
.ps 20
Network File System
.sp .5
Protocol Specification
.NH
Introduction
.LP
The Sun Network Filesystem (NFS) protocol provides transparent remote
access to shared filesystems over local area networks.  The NFS
protocol is designed to be machine, operating system, network architecture,
and transport protocol independent.  This independence is achieved through
the use of Remote Procedure Call (RPC) primitives built on top of an
eXternal Data Representation (XDR).
.LP
The supporting mount protocol allows the server to hand out
remote access privileges to a restricted set of clients.
Thus, it allows clients to attach a remote directory tree
at any point on some local filesystem.
.NH 2 
Remote Procedure Call
.LP
Sun's remote procedure call specification, described in the
.I "RPC Programming Guide" ,
provides a clean, procedure-oriented interface to remote services.
Each server supplies a program that is a set of procedures.
The combination of host address, program number, and procedure number
specifies one remote service procedure.
.LP
RPC is a high-level protocol built on top of low-level transport protocols.
It does not depend on services provided by specific protocols,
so it can be used easily with any underlying transport protocol.
Currently the only supported transport protocol is UDP/IP.
.LP
The RPC protocol includes a slot for authentication parameters on every call.
The contents of the authentication parameters are determined by
the ``flavor'' (type) of authentication used by the server and client.
A server may support several different flavors of authentication at once:
.LX AUTH_NONE
passes no authentication information
(this is called null authentication);
.LX AUTH_UNIX
passes the \*(Un uid, gid, and groups with each call.
.LP
Servers have been known to change over time,
and so can the protocol that they use.
So RPC provides a version number with each RPC request.
Thus, one server can service requests for several different versions
of the protocol at the same time.
.NH 2 
External Data Representation
.LP
Sun's external data representation specification, described in the
.I "XDR Protocol Specification" ,
provides a common way of representing a set of data types over a network.
This takes care of problems such as different byte ordering
on different communicating machines.
It also defines the size of each data type
so that machines with different structure alignment algorithms
can share a common format over the network.
.bp
.LP
In this document we use the XDR data definition language
to specify the parameters and results of each RPC service procedure
that a NFS server provides.
The XDR data definition language reads a lot like C,
although a few new constructs have been added.  The notation
.LS
string	name[SIZE];
string	data<DSIZE>;
.LF
defines
.LX name ,
which is a fixed size block of
.LX SIZE
bytes,
and
.LX data ,
which is a variable size block of up to
.LX DSIZE
bytes.  This same notation is used to indicate fixed length arrays,
and arrays with a variable number of elements up to some maximum.
.LP
The discriminated union definition
.LS
union switch (enum status) {
    NFS_OK:
	struct {
		filename	file1;
		filename	file2;
		integer		count;
	}
    NFS_ERROR:
	struct {
		errstat		error;
		integer		errno;
	}
    default:
	struct {}
}
.LF
means the first thing over the network is an enumeration type called
.LX status ;
if its value is
.LW NFS_OK ,
the next thing on the network will be the structure containing
.LX file1 ,
.LX file2 ,
and
.LX count .
If the value of
.LW status
is neither
.LX NFS_OK
nor
.LX NFS_ERROR ,
then there is no more data to look at.
.NH 2 
Stateless Servers
.LP
The NFS protocol is stateless.  
That is, a server does not need
to maintain state about any of its clients in order to function correctly.
Stateless servers have a distinct advantage over stateful servers
in the event of a crash.  
With stateless servers,
a client need only retry a request until the server responds;
it does not even need to know that the server has crashed.
The client of a stateful server, on the other hand,
needs to detect a server crash and rebuild
the server's state when it comes back up.
.LP
This may not sound like an important issue,
but it affects the protocol in some strange ways.
We feel that it is worth a bit of extra complexity in the protocol
to be able to write very simple servers that don't need fancy crash recovery.
.NH 1 
NFS Protocol Definition
.LP
The NFS protocol is designed to be operating system independent,
but let's face it, it was designed in a \*(Un environment.  
As such, it has some features which are very \*(Un-ish.
When in doubt about how something should work,
a quick look at how it is done on \*(Un
will probably put you on the right track.
.LP
The protocol definition is given as a set of procedures
with arguments and results defined using XDR.
A brief description of the function of each procedure should provide
enough information to allow implementation on most machines.
There is a different section provided
for each supported version of the protocol.
Most of the procedures, and their parameters and results,
are self-explanatory.  
A few do not fit into the normal \*(Un mold, however.
.LP
The
.LX LOOKUP
procedure looks up one component of a pathname at a time.
It is not obvious at first why it does not just take the whole pathname,
traipse down the directories, and return a file handle when it is done.
There are two good reasons not to do this.
First, pathnames need separators between the directory components,
and different operating systems use different separators.
We could define a Network Standard Pathname Representation,
but then every pathname would have to be parsed and converted at each end.
Second, if pathnames were passed, the server would have to
keep track of the mounted filesystems for all of its clients,
so that it could break the pathname at the right point
and pass the remainder on to the correct server.
.LP
Another procedure which might seem strange to \*(Un
people is the 
.LX READDIR
procedure.  
What
.LW READDIR
does is provide a network standard format for representing directories.
The same argument as above could have been used to justify a
.LW READDIR
procedure that returns only one directory entry per call.
The problem is efficiency.
Directories can contain many entries,
and a remote call to return each would just be too slow.
.NH 2 
Version 2
.LP
The released version of the NFS protocol is actually the second.
Even in the second version,
there are various obsolete procedures and parameters,
which will probably be removed in later versions.
.NH 3 
Server/Client Relationship
.LP
The NFS protocol is designed to allow servers to be
as simple and general as possible.
Sometimes the simplicity of the server can be a problem,
if the client wants to implement complicated filesystem semantics.
.LP
For example, \*(Un allows removal of open files.  
A process can open a file and,
while it is open, remove it from the directory.
The file can be read and written as long as the process keeps it open,
even though the file has no name in the filesystem.
It is impossible for a stateless server to implement these semantics.
The client can do some tricks like renaming the file on remove,
and only removing it on close.  
We believe that the server provides
enough functionality to implement most filesystem semantics on the client.
.bp
.LP
Every NFS client can also be a server, and remote and local mounted filesystems
can be freely intermixed.  
This leads to some interesting problems when
a client travels down the directory tree of a remote filesystem and reaches
the mount point on the server for another remote filesystem.
Allowing the server to following the second remote mount means it must do
loop detection, server lookup, and user revalidation.
Instead, we decided not to let clients cross a server's mount point.
When a client does a
.LX LOOKUP
on a directory that the server has mounted a filesystem on,
the client sees the underlying directory instead of the mounted directory.
A client can do remote mounts that match the server's mount points
to maintain the server's view.
.NH 3 
Permission Issues
.LP
The NFS protocol, strictly speaking, does not define the permission checking
used by servers.  However, it is expected that a server will do normal
\*(Un permission checking using
.LX AUTH_UNIX
style authentication as the basis of its protection mechanism.
The server gets the client's effective
.I uid ,
effective
.I gid
and groups
on each call, and uses them to check permission.
There are various problems with this method that can
been resolved in interesting ways.
.LP
Using
.I uid
and
.I gid
implies that the client and server share the same
.I uid
list.  
Every server and client pair must have the same mapping from user to
.I uid
and from group to
.I gid .
Since every client can also be a server this tends to imply
that the whole network shares the same
.I uid/gid
space.  
This is acceptable for the short term,
but a more workable network authentication
method will be necessary before long.
.LP
Another problem arises due to the semantics of open.
\*(Un does its permission checking at open time and then 
that the file is open,
and has been checked on later read and write requests.
With stateless servers this breaks down,
because the server has no idea that the file is open
and it must do permission checking on each read and write call.
On a local filesystem, a user can open a file
then change the permissions so that no one is allowed to touch it,
but will still be able to write to the file because it is open.
On a remote filesystem, by contrast, the write would fail.
To get around this problem the server's permission checking algorithm
should allow the owner of a file to access it
no matter what the permissions are set to.
.LP
A similar problem has to do with paging in from a file over the network.
The \*(Un
kernel checks for execute permission before opening a file for demand paging,
then reads blocks from the open file.  
The file may not have read permission
but after it is opened it doesn't matter.  
An NFS server can't tell the
difference between a normal file read and a demand page-in read.  
To make
this work the server allows reading of files if the
.I uid
given in the call has execute or read permission on the file.
.LP
In \*(Un ,
the user ID zero has access to all files no matter what
permission and ownership they have.  
This super-user permission is not
allowed on the server since anyone who can become super-user
on their workstation could gain access to all remote files.
Instead, the server maps
.I uid
0 to \-2 before doing its access checking.
This works as long as the NFS is not used to supply root filesystems,
where super-user access cannot be avoided.  
Eventually
servers will have to allow some kind of limited super-user access.
.bp
.NH 3 
RPC Information
.IP "Authentication"
The NFS service uses
.LX AUTH_UNIX
style authentication except in the
.LX NULL
procedure where
.LX AUTH_NONE
is also allowed.
.IP "Protocols"
NFS currently is supported on UDP/IP only.
.IP "Constants"
These are the RPC constants needed to call the NFS service.
They are given in decimal.
.TS
l l.
PROGRAM	100003
VERSION	2
.TE
.IP "Port Number"
The NFS protocol currently uses the UDP port number 2049.
This is a bug in the protocol and will be changed very shortly.
.NH 3 
Sizes
.LP
These are the sizes, given in decimal bytes, of various XDR structures used
in the protocol.
.IP "MAXDATA 8192"
.IX "MAXDATA" "" "" "" PAGE MAJOR
The maximum number
of bytes of data in a 
.LW READ
or
.LW WRITE
request.
.IP "MAXPATHLEN 1024"
.IX "MAXPATHLEN" "" "" "" PAGE MAJOR
The maximum number of bytes
in a pathname argument.
.IP "MAXNAMLEN 255"
.IX "MAXNAMLEN" "" "" "" PAGE MAJOR
The maximum number of bytes in a file name argument.
.IP "COOKIESIZE 4"
.IX "COOKIESIZE" "" "" "" PAGE MAJOR
The size in bytes of the opaque ``cookie'' passed by
.LX READDIR .
.IP "FHSIZE 32"
.IX "FHSIZE" "" "" "" PAGE MAJOR
The size in bytes of the opaque file handle.
.bp
.NH 3 
Basic Data Types
.LP
The following XDR definitions are basic structures and types
used in other structures later on.
.NH 4 
stat
.IX "stat" "" "" "" PAGE MAJOR
.LS
typedef enum {
	NFS_OK = 0,
	NFSERR_PERM=1,
	NFSERR_NOENT=2,
	NFSERR_IO=5,
	NFSERR_NXIO=6,
	NFSERR_ACCES=13,
	NFSERR_EXIST=17,
	NFSERR_NODEV=19,
	NFSERR_NOTDIR=20,
	NFSERR_ISDIR=21,
	NFSERR_FBIG=27,
	NFSERR_NOSPC=28,
	NFSERR_ROFS=30,
	NFSERR_NAMETOOLONG=63,
	NFSERR_NOTEMPTY=66,
	NFSERR_DQUOT=69,
	NFSERR_STALE=70,
	NFSERR_WFLUSH=99
} stat;
.LF
.LP
The
.LW stat
type is returned with every procedure's results.  A value of
.LX NFS_OK
indicates that the call completed successfully and the results
are valid.  The other values indicate some kind of error occurred
on the server side during the servicing of the procedure.
The error values are derived from \*(Un error numbers.
.IP NFSERR_PERM
.IX NFSERR_PERM "" "" "" PAGE MAJOR
Not owner.
The caller does not have
correct ownership to perform
the requested operation.
.IP NFSERR_NOENT
.IX NFSERR_NOENT "" "" "" PAGE MAJOR
No such file or directory.
The file or directory specified
does not exist.
.IP NFSERR_IO
.IX NFSERR_IO "" "" "" PAGE MAJOR
I/O error.
Some sort of hard error occurred
when the operation was in progress.
This could be a disk error,
for example.
.IP NFSERR_NXIO
.IX NFSERR_NXIO "" "" "" PAGE MAJOR
No such device or address.
.IP NFSERR_ACCES
.IX NFSERR_ACCES "" "" "" PAGE MAJOR
Permission denied.
The caller does not have
the correct permission to perform
the requested operation.
.IP NFSERR_EXIST
.IX NFSERR_EXIST "" "" "" PAGE MAJOR
File exists.
The file specified already exists.
.IP NFSERR_NODEV
.IX NFSERR_NODEV "" "" "" PAGE MAJOR
No such device.
.IP NFSERR_NOTDIR
.IX NFSERR_NOTDIR "" "" "" PAGE MAJOR
Not a directory.
The caller specified a non-directory
in a directory operation.
.IP NFSERR_ISDIR
.IX NFSERR_ISDIR "" "" "" PAGE MAJOR
Is a directory.
The caller specified a directory
in a non-directory operation.
.IP NFSERR_FBIG
.IX NFSERR_FBIG "" "" "" PAGE MAJOR
File too large.
The operation caused a file
to grow beyond the server's limit.
.IP NFSERR_NOSPC
.IX NFSERR_NOSPC "" "" "" PAGE MAJOR
No space left on device.
The operation caused the server's
filesystem to reach its limit.
.IP NFSERR_ROFS
.IX NFSERR_ROFS "" "" "" PAGE MAJOR
Read-only filesystem.
Write attempted on a read-only filesystem.
.IP NFSERR_NAMETOOLONG
.IX NFSERR_NAMETOOLONG "" "" "" PAGE MAJOR
File name too long.
The file name in an operation was too long.
.IP NFSERR_NOTEMPTY
.IX NFSERR_NOTEMPTY "" "" "" PAGE MAJOR
Directory not empty.
Attempted to remove a directory that was not empty.
.IP NFSERR_DQUOT
.IX NFSERR_DQUOT "" "" "" PAGE MAJOR
Disk quota exceeded.
The client's disk quota on the server has been exceeded.
.IP NFSERR_STALE
.IX NFSERR_STALE "" "" "" PAGE MAJOR
The 
.LX fhandle
given in the arguments was invalid.
That is, the file referred to by that file handle no longer exists,
or access to it has been revoked.
.IP NFSERR_WFLUSH
.IX NFSERR_WFLUSH "" "" "" PAGE MAJOR
The server's write cache used in the
.LX WRITECACHE
call got flushed to disk.
.NH 4 
ftype
.IX "ftype" "" "" "" PAGE MAJOR
.LS
typedef enum {
	NFNON = 0,
	NFREG = 1,
	NFDIR = 2,
	NFBLK = 3,
	NFCHR = 4,
	NFLNK = 5
} ftype;
.LF
.LP
The enumeration
.LW ftype
gives the type of a file.
The type
.LX NFNON
indicates a non-file,
.LX NFREG
is a regular file,
.LX NFDIR
is a directory,
.LX NFBLK
is a block-special device,
.LX NFCHR
is a character-special device, and
.LX NFLNK
is a symbolic link.
.NH 4 
fhandle
.IX "fhandle" "" "" "" PAGE MAJOR
.LS
typedef opaque	fhandle[FHSIZE];
.LF
.LP
The
.LW fhandle
is the file handle that the server passes to the client.
All file operations are done using file handles to refer to a
file or directory.  The file handle can contain
whatever information the server needs
to distinguish an individual file.
.NH 4 
timeval
.IX "timeval" "" "" "" PAGE MAJOR
.LS
typedef struct {
	unsigned seconds;
	unsigned useconds;
} timeval;
.LF
.LP
The
.LW timeval
structure is the number of seconds and microseconds
since midnight January 1, 1970 Greenwich Mean Time.
It is used to pass time and date information.
.NH 4 
fattr
.IX "fattr" "" "" "" PAGE MAJOR
.LS
typedef struct {
	ftype	 type;
	unsigned mode;
	unsigned nlink;
	unsigned uid;
	unsigned gid;
	unsigned size;
	unsigned blocksize;
	unsigned rdev;
	unsigned blocks;
	unsigned fsid;
	unsigned fileid;
	timeval	 atime;
	timeval	 mtime;
	timeval	 ctime;
} fattr;
.LF
.LP
The
.LW fattr
structure contains the attributes of a file;
.LX type
is the type of the file;
.LX nlink
is the number of hard links to the file, that is, the number
of different names for the same file;
.LX uid
is the user identification number of the owner of the file;
.LX gid
is the group identification number of the group of the file;
.LX size
is the size in bytes of the file;
.LX blocksize
is the size in bytes of a block of the file;
.LX rdev
is the device number of the file if it is type
.LW NFCHR
or
.LW NFBLK ;
.LX blocks
is the number of blocks that the file takes up on disk;
.LX fsid
is the file system identifier for the filesystem that contains the file;
.LX fileid
is a number that uniquely identifies the file within its filesystem;
.LX atime
is the time when the file was last accessed for either read or write;
.LX mtime
is the time when the file data was last modified (written); and
.LX ctime
is the time when the status of the file was last changed.
Writing to the file also changes
.LW ctime
if the size of the file changes.
.LP
.LX Mode
is the access mode encoded as a set of bits.
The bits are the same as the mode bits returned by the
.I stat (2)
system call in \*(Un .
Notice that the file type is specified both in the mode bits and in the
file type.  
This is really a bug in the protocol and should be fixed in
future versions.
The descriptions given below specify the bit positions using octal numbers.
.IP 0040000 8
This is a directory.  
The
.LW type
field should be
.LX NFDIR .
.IP 0020000 8
This is a character special file.  
The  
.LW type 
field should be 
.LX NFCHR .
.IP 0060000 8
This is a block special file.  
The   
.LW type  
field should be  
.LX NFBLK .
.IP 0100000 8
This is a regular file.  
The    
.LW type   
field should be
.LX NFREG .  
.IP 0120000 8
This is a symbolic link file.  
The     
.LW type    
field should be
.LX NFLNK .
.IP 0140000 8
This is a named socket.  
The      
.LW type     
field should be 
.LX NFNON . 
.IP 0004000 8
Set user id on execution.
.IP 0002000 8
Set group id on execution.
.IP 0001000 8
Save swapped text even after use.
.IP 0000400 8
Read permission for owner.
.IP 0000200 8
Write permission for owner.
.IP 0000100 8
Execute and search permission for owner.
.IP 0000040 8
Read permission for group.
.IP 0000020 8
Write permission for group.
.IP 0000010 8
Execute and search permission for group.
.IP 0000004 8
Read permission for others.
.IP 0000002 8
Write permission for others.
.IP 0000001 8
Execute and search permission for others.
.NH 4 
sattr
.IX "sattr" "" "" "" PAGE MAJOR
.LS
typedef struct {
    unsigned	mode;
    unsigned	uid;
    unsigned	gid;
    unsigned	size;
    timeval	atime;
    timeval	mtime;
} sattr;
.LF
.LP
The
.LW sattr
structure contains the file attributes which can be set from the client.
The fields are the same as for
.LW fattr
above.  A 
.LW size
of zero means the file should be truncated.
A value of \-1 indicates a field that should be ignored.
.NH 4 
filename
.IX "filename" "" "" "" PAGE MAJOR
.LS
typedef string	filename<MAXNAMLEN>;
.LF
.LP
The type
.LW filename
is used for passing file names or pathname components.
.NH 4 
path
.IX "path" "" "" "" PAGE MAJOR
.LS
typedef string	path<MAXPATHLEN>;
.LF
.LP
The type
.LW path
is a pathname.
The server considers it as a string with no internal structure,
but to the client it is the name of a node in a filesystem tree.
.NH 4 
attrstat
.IX "attrstat" "" "" "" PAGE MAJOR
.LS
typedef union switch (stat status) {
    NFS_OK:
	fattr	attributes;
    default:
	struct {}
} attrstat;
.LF
.LP
The
.LW attrstat
structure is a common procedure result.
It contains a
.LX status
and, if the call succeeded,
it also contains the attributes of the file
on which the operation was done.
.NH 4 
diropargs
.IX "diropargs" "" "" "" PAGE MAJOR
.LS
typedef struct {
    fhandle	dir;
    filename	name;
} diropargs;
.LF
.LP
The
.LW diropargs
structure is used in directory operations.  The
.LW fhandle
.LX dir
is the directory in which to find the file
.LX name .
A directory operation is one in which the directory is affected.
.NH 4 
diropres
.IX "diropres" "" "" "" PAGE MAJOR
.LS
typedef union switch (stat status) {
    NFS_OK:
	struct {
		fhandle	file;
		fattr	attributes;
	}
    default:
	struct {}
} diropres;
.LF
.LP
The results of a directory operation are returned in a 
.LW diropres
structure.  If the call succeeded a new file handle
.LX file
and the
.LX attributes
associated with that file are returned along with the
.LX status .
.bp
.NH 3 
Server Procedures
.LP
The following sections define the RPC procedures supplied by a NFS server.
The RPC procedure number and version are given in the header,
along with the name of the prodedure.
The synopsis of prodecures has this format:
.LS
<proc #>. <proc name> ( <arguments> ) returns ( <results> )
	<argument declarations>
	<results declarations>
.LF
In the first line,
.I "proc name"
is the name of the procedure,
.I arguments
is a list of the names of the arguments, and
.I results
is a list of the names of the results.
The second and third lines give the XDR
.I "argument declarations"
and
.I "results declarations" .
Afterwards, there is a description of what the procedure is expected to do,
and how its arguments and results are used.
If there are bugs or problems with the procedure,
they are listed at the end.
.LP
All of the procedures in the NFS protocol are assumed to be synchronous.
When a procedure returns to the client, the client can assume that the
operation has completed and any data associated with the request is
now on stable storage.  
For example, a client WRITE request may cause the server
to update data blocks, filesystem information blocks (such as indirect
blocks in \*(Un ),
and file attribute information (size and modify times).
When the WRITE returns to the client, it can assume that the write is safe,
even in case of a server crash, and it can discard the data written.
This is a very important part of the statelessness of the server.  
If the
server waited to flush data from remote requests the client would
have to save those requests so that it could resend them in case of a
server crash.
.NE
.NH 4 
Do Nothing (Procedure 0, Version 2)
.IX NFSPROC_NULL "" "" "" PAGE MAJOR
.LS
0. NFSPROC_NULL ( ) returns ( )
.LF
.LP
This procedure does no work.
It is made available in all RPC services
to allow server response testing and timing.
.NE
.NH 4 
Get File Attributes (Procedure 1, Version 2)
.IX NFSPROC_GETATTR "" "" "" PAGE MAJOR
.LS
1. NFSPROC_GETATTR (file) returns (reply)	
	fhandle  file;
	attrstat reply;
.LF
.LP
If
.LX reply.status
is
.LW NFS_OK
then
.LX reply.attributes
contains the attributes for the file given by
.LX file .
.LP
Bugs: the
.LX rdev
field in the attributes structure is a \*(Un device specifier.  
It should be removed or generalized.
.NE
.NH 4 
Set File Attributes (Procedure 2, Version 2)
.IX NFSPROC_SETATTR "" "" "" PAGE MAJOR
.LS
2. NFSPROC_SETATTR (file, attributes) returns (reply)
	fhandle	 file;
	sattr	 attributes;
	attrstat reply;
.LF
.LP
The
.LX attributes
argument contains fields which are either \-1 or are
the new value for the attributes of
.LX file .
If
.LW reply.status
is
.LW NFS_OK
then
.LW reply.attributes
has the attributes of the file after the
.LW setattr
operation has completed.
.LP
Bugs: the use of \-1 to indicate an unused field in
.LW attributes
is wrong.
.NE
.NH 4 
Get Filesystem Root (Procedure 3, Version 2)
.IX NFSPROC_ROOT "" "" "" PAGE MAJOR
.LS
3. NFSPROC_ROOT ( ) returns ( )
.LF
Obsolete.
This procedure is no longer used because finding the root file handle
of a filesystem requires moving pathnames between client and server.
To do this right we would have to define
a network standard representation of pathnames.
Instead, the function of looking up the root file handle is done by the
.LX MNTPROC_MNT
procedure (see section entitled
.I "Mount Protocol Definition"
for details).
.NE
.NH 4 
Look Up File Name (Procedure 4, Version 2)
.IX NFSPROC_LOOKUP "" "" "" PAGE MAJOR
.LS
4. NFSPROC_LOOKUP (which) returns (reply)
	diropargs which;
	diropres  reply;
.LF
.LP
If
.LW reply.status
is
.LW NFS_OK
then
.LW reply.file
and
.LW reply.attributes
are the file handle and attributes
for the file
.LX which.name
in the directory given by
.LX which.dir .
.LP
Bugs: there is some question as to what is the correct reply
to a LOOKUP request when
.LW which.name
is a mount point on the server for a remote mounted filesystem.
Currently, we return the
.LW fhandle
of the underlying directory.  This is not completely acceptable,
as the clients see a different view of the filesystem than the server does.
.NE
.NH 4 
Read From Symbolic Link (Procedure 5, Version 2)
.IX NFSPROC_READLINK "" "" "" PAGE MAJOR
.LS
5. NFSPROC_READLINK (file) returns (reply)
	fhandle	file;
	union switch (stat status) {
	    NFS_OK:
		path   data;
	    default:
		struct {}
	} reply;
.LF
.LP
If 
.LX status
has the value NFS_OK then
.LX reply.data
is the data in the symbolic link given by
.LX file .
.NE
.NH 4 
Read From File (Procedure 6, Version 2)
.IX NFSPROC_READ "" "" "" PAGE MAJOR
.LS
6. NFSPROC_READ (file, offset, count, totalcount) returns (reply)
	fhandle	 file;
	unsigned offset;
	unsigned count;
	unsigned totalcount;
	union switch (stat status) {
	    NFS_OK:
		fattr	attributes;
		string	data<MAXDATA>;
	    default:
		struct {}
	} reply;
.LF
.LP
Returns up to
.LX count
bytes of
.LX data
from the file given by
.LX file ,
starting at
.LX offset
bytes from the beginning of the file.
The first byte of the file is at offset zero.
The file attributes after the read takes place are returned in
.LX attributes .
.LP
Bugs: the argument
.LX totalcount
is unused, and should be removed.
.NE
.NH 4 
Write to Cache (Procedure 7, Version 2)
.IX NFSPROC_WRITECACHE "" "" "" PAGE MAJOR
.LS
7. NFSPROC_WRITECACHE ( ) returns ( )
.LF
.LP
Obsolete.
.NE
.NH 4 
Write to File (Procedure 8, Version 2)
.IX NFSPROC_WRITE "" "" "" PAGE MAJOR
.LS
8. NFSPROC_WRITE (file,beginoffset,offset,totalcount,data) returns (reply)
	fhandle	 file;
	unsigned beginoffset;
	unsigned offset;
	unsigned totalcount;
	string	 data<MAXDATA>;
	attrstat reply;
.LF
.LP
Writes
.LX data
beginning
.LX offset
bytes from the beginning of 
.LX file .
The first byte of the file is at offset zero.
If
.LX reply.status
is
.LW NFS_OK
then
.LX reply.attributes
contains the attributes of the file after the write has completed.
The write operation is atomic.  Data from this
.LW WRITE
will not be mixed with data from another client's
.LW WRITE .
.LP
Bugs: the arguments
.LX beginoffset
and
.LX totalcount
are ignored and should be removed.
.NE
.NH 4 
Create File (Procedure 9, Version 2)
.IX NFSPROC_CREATE "" "" "" PAGE MAJOR
.LS
9. NFSPROC_CREATE (where, attributes) returns (dir)
	diropargs where;
	sattr	  attributes;
	diropres  dir;
.LF
.LP
The file
.LX where.name
is created in the directory given by
.LX where.dir .
The initial attributes of the new file are given by
.LX attributes .
A
.LX reply.status
of
.LW NFS_OK
indicates that the file was created and
.LX reply.file
and 
.LX reply.attributes
are its file handle and attributes.
Any other
.LX reply.status
means that the operation failed and no file was created.
.LP
Bugs: this routine should pass an exclusive create flag meaning,
create the file only if it is not already there.
.NE
.NH 4 
Remove File (Procedure 10, Version 2)
.IX NFSPROC_REMOVE "" "" "" PAGE MAJOR
.LS
10. NFSPROC_REMOTE (which) returns (status)
	diropargs which;
	stat	  status;
.LF
.LP
The file
.LX which.name
is removed from the directory given by
.LX which.dir .
A
.LX status
of
.LW NFS_OK
means the directory entry was removed.
.NE
.NH 4 
Rename File (Procedure 11, Version 2)
.IX NFSPROC_RENAME "" "" "" PAGE MAJOR
.LS
11. NFSPROC_RENAME (from, to) returns (status)
	diropargs from;
	diropargs to;
	stat	  status;
.LF
.LP
The existing file
.LX from.name
in the directory given by
.LX from.dir
is renamed to
.LX to.name
in the directory given by
.LX to.dir .
If
.LX status
is
.LW NFS_OK
the file was renamed.  
The
.LW RENAME
operation is atomic on the server;
it cannot be interrupted in the middle.
.NE
.NH 4 
Create Link to File (Procedure 12, Version 2)
.IX NFSPROC_LINK "" "" "" PAGE MAJOR
.LS
12. NFSPROC_LINK (from, to) returns (status)
	fhandle   from;
	diropargs to;
	stat	  status;
.LF
.LP
Creates the file
.LX to.name
in the directory given by
.LX to.dir ,
which is a hard link to the existing file given by
.LX from .
If the return value of
.LX status
is
.LW NFS_OK
a link was created.  Any other return value
indicates an error and the link is not created.
.LP
A hard link should have the property that changes to either of
the linked files are reflected in both files.  When a hard link
is made to a file, the attributes for the file should have a value for
.LX nlink
which is one greater than the value before the link.
.NE
.NH 4 
Create Symbolic Link (Procedure 13, Version 2)
.IX NFSPROC_SYMLINK "" "" "" PAGE MAJOR
.LS
13. NFSPROC_SYMLINK (from, to, attributes) returns (status)
	diropargs from;
	path	  to;
	sattr	  attributes;
	stat	  status;
.LF 
.LP
Creates the file
.LX from.name
with ftype
.LX NFLNK
in the directory given by
.LX from.dir .
The new file contains the pathname
.LX to
and has initial attributes given by
.LX attributes .
If the return value of
.LX status
is
.LW NFS_OK
a link was created.  
Any other return value
indicates an error and the link is not created.
.LP
A symbolic link is a pointer to another file.
The name given in
.LW to
is not interpreted by the server,
just stored in the newly created file.  
A
.LW READLINK
operation returns the data to the client for interpretation.
.LP
Bugs: on \*(Un servers the attributes are never used,
since symbolic links always have mode 0777.
.NE
.NH 4 
Create Directory (Procedure 14, Version 2)
.IX NFSPROC_MKDIR "" "" "" PAGE MAJOR
.LS
14. NFSPROC_MKDIR (where, attributes) returns (reply)
	diropargs where;
	sattr	  attributes;
	diropres  reply;
.LF
.LP
The new directory
.LX where.name
is created in the directory given by
.LX where.dir .
The initial attributes of the new directory are given by
.LX attributes .
A
.LX reply.status
of
.LW NFS_OK
indicates that the new directory was created and
.LX reply.file
and 
.LX reply.attributes
are its file handle and attributes.
Any other
.LX reply.status
means that the operation failed and no directory was created.
.NE
.NH 4 
Remove Directory (Procedure 15, Version 2)
.IX NFSPROC_RMDIR "" "" "" PAGE MAJOR
.LS
15. NFSPROC_RMDIR (which) returns (status)
	diropargs	which;
	stat		status;
.LF
.LP
The existing, empty directory
.LX which.name
in the directory given by
.LX which.dir
is removed.  If
.LX status
is
.LW NFS_OK
the directory was removed.
.bp
.NH 4 
Read From Directory (Procedure 16, Version 2)
.IX NFSPROC_READDIR "" "" "" PAGE MAJOR
.LS
16. NFSPROC_READDIR (dir, cookie, count) returns (entries)
	fhandle	 dir;
	opaque	 cookie[COOKIESIZE];
	unsigned count;
	union switch (stat status) {
	    NFS_OK:
		typedef union switch (boolean valid) {
		    TRUE:
			struct {
			    unsigned	fileid;
			    filename	name;
			    opaque	cookie[COOKIESIZE];
			    entry	nextentry;
			}
		    FALSE:
			struct {}
		} entry;
		boolean	eof;
	    default:
	} entries;
.LF
.LP
Returns a variable number of directory entries,
with a total size of up to
.LX count
bytes, from the directory given by
.LX dir .
Each
.LX entry
contains a
.LX fileid
which is a unique number to identify the file within a filesystem, the
.LX name
of the file, and a
.LX cookie
which is an opaque pointer to the next entry in the directory.
The cookie is used in the next READDIR call to get more entries
starting at a given point in the directory.
The special cookie zero (all bits zero) can be used
to get the entries starting at the beginning of the directory.
The
.LX fileid
field should be the same number as the
.LW fileid
in the the attributes of the file (see 
the section entitled
.I fattr
under
.I "Basic Data Types" ).
The
.LX eof
flag has a value of
.LW TRUE
if there are no more entries in the directory;
.LX valid
is used to mark the end of the entries.
If the returned value of
.LX status
is
.LW NFS_OK
then it is followed by a variable number of
.LX entries .
.NE
.NH 4 
Get Filesystem Attributes (Procedure 17, Version 2)
.IX NFSPROC_STATFS "" "" "" PAGE MAJOR
.LS
17. NFSPROC_STATFS (file) returns (reply)
	fhandle	 file;
	union switch (stat status) {
	    NFS_OK:
		struct {
		    unsigned	tsize;
		    unsigned	bsize;
		    unsigned	blocks;
		    unsigned	bfree;
		    unsigned	bavail;
		} fsattr;
	    default:
		struct {}
	} reply;
.LF
.bp
.LP
If
.LX reply.status
is
.LW NFS_OK
then
.LX reply.fsattr
gives the attributes for the filesystem that contains
.LX file .
The attribute fields contain the following values:
.IP tsize 8
The optimum transfer size of the server in bytes.
This is the number of bytes the server would like to have
in the data part of
.I READ
and
.LW WRITE
requests.
.IP bsize 8
The block size in bytes of the filesystem.
.IP blocks 8
The total number of
.LW bsize
blocks on the filesystem.
.IP bfree 8
The number of free
.LW bsize
blocks on the filesystem.
.IP bavail 8
The number of
.LW bsize
blocks available to non-privileged users.
.LP
Bugs: this call does not work well
if a filesystem has variable size blocks.
.bp
.NH
Mount Protocol Definition
.LP
The mount protocol is separate from, but related to, the NFS protocol.
It provides all of the operating system specific services
to get the NFS off the ground \(em looking up path names,
validating user identity, and checking access permissions.
Clients use the mount protocol to get the first file handle,
which allows them entry into a remote filesystem.
.LP
The mount protocol is kept separate from the NFS protocol to make it
easy to plug in new access checking and validation methods without
changing the NFS server protocol.
.LP
Notice that the protocol definition implies stateful servers
because the server maintains a list of client's mount requests.
The mount list information is not critical for the correct
functioning of either the client or the server.
It is intented for advisory use only, for example,
to warn possible clients when a server is going down.
.NH 2 
Version 1
.LP
Version one of the mount protocol communicates with
the version two of the NFS protocol.
The only connecting point is the
.LX fhandle
structure, which is the same for both protocols.
.NH 3 
RPC Information
.IP "Authentication"
The mount service uses
.LX AUTH_UNIX
style authentication only.
.IP "Protocols"
The mount service is currently supported on UDP/IP only.
.IP "Constants"
These are the RPC constants needed to call the MOUNT service.
They are given in decimal.
.TS
l l.
PROGRAM	100005
VERSION	1
.TE
.IP "Port Number"
Consult the server's portmapper, described in the
.I "RPC Protocol Specification" ,
to find which port number the mount service is registered on.
.NH 3 
Sizes
.LP
These are the sizes given in decimal bytes of various XDR structures used
in the protocol.
.IP "MNTPATHLEN 1024"
.IX "MNTPATHLEN"
The maximum number of bytes in a pathname argument.
.IP "MNTNAMLEN 255"
.IX "MNTNAMLEN"
The maximum number of bytes in a name argument.
.IP "FHSIZE 32"
.IX "FHSIZE"
The size in bytes of the opaque file handle.
.bp
.NH 3 
Basic Data Types
.NH 4 
fhandle
.LS
typedef opaque fhandle[FHSIZE];
.LF
.LP
The
.LX fhandle
is the file handle that the server passes to the client.
All file operations are done using file handles to refer to a
file or directory.  The file handle can contain whatever information
the server needs to distinguish an individual file.
.LP
This is the same as the
.LW fhandle
XDR definition in version 2 of the NFS protocol;
see the section on
.LW fhandle
under
.I "Basic Data Types" .
.NH 4 
fhstatus
.IX "fhstatus" "" "" "" PAGE MAJOR
.LS
typedef union switch (unsigned status) {
    0:
	fhandle	directory;
    default:
	struct {}
}
.LF
.LP
If a
.LX status
of zero is returned, the call completed successfully,
and a file handle for the
.LX directory
follows.  
A non-zero status indicates some sort of error.
In this case the status is a \*(Un error number.
.NH 4 
dirpath
.IX "dirpath" "" "" "" PAGE MAJOR
.LS
typedef string dirpath<MNTPATHLEN>;
.LF
.LP
The type
.LX dirpath
is a normal \*(Un pathname of a directory.
.NH 4 
name
.IX "name" "" "" "" PAGE MAJOR
.LS
typedef string name<MNTNAMLEN>;
.LF
.LP
The type
.LX name
is an arbitrary string used for various names.
.bp
.NH 3 
Server Procedures
.LP
The following sections define the RPC procedures supplied by a mount server.
The RPC procedure number and version are given in the header,
along with the name of the procedure.
The synopsis of procedures has this format:
.LS
<proc #>. <proc name> ( <arguments> ) returns ( <results> )
	<argument declarations>
	<results declarations>
.LF
In the first line,
.I "proc name"
is the name of the procedure,
.I arguments
is a list of the names of the arguments, and
.I results
is a list of the names of the results.
The second and third lines give the XDR
.I "argument declarations"
and
.I "results declarations" .
Afterwards, there is a description of what the procedure is expected to do,
and how its arguments and results are used.
If there are bugs or problems with the procedure,
they are listed at the end.
.NE
.NH 4 
Do Nothing (Procedure 0, Version 1)
.IX MNTPROC_NULL "" "" "" PAGE MAJOR
.LS
0. MNTPROC_NULL ( ) returns ( )
.LF
.LP
This procedure does no work.
It is made available in all RPC services
to allow server response testing and timing.
.NE
.NH 4 
Add Mount Entry (Procedure 1, Version 1)
.IX MNTPROC_MNT "" "" "" PAGE MAJOR
.LS
1. MNTPROC_MNT (directory) returns (reply)
	dirpath  dirname;
	fhstatus reply;
.LF
.LP
If
.LX reply.status
is 0,
.LX reply.directory
contains the file handle for the directory
.LX dirname .
This file handle may be used in the NFS protocol.
This procedure also adds a new entry to the mount list
for this client mounting
.LX dirname .
.NE
.NH 4 
Return Mount Entries (Procedure 2, Version 1)
.IX MNTPROC_DUMP "" "" "" PAGE MAJOR
.LS
2. MNTPROC_DUMP ( ) returns (mountlist)
	union switch (boolean more_entries) {
	    TRUE:
		struct {
			name	  hostname;
			dirpath	  directory;
			mountlist nextentry;
		}
	    FALSE:
		struct {}
	} mountlist;
.LF
.LP
Returns the list of remote mounted filesystems.  The
.LX mountlist
contains one entry for each
.LX hostname
and
.LX directory
pair.
.NE
.NH 4 
Remove Mount Entry (Procedure 3, Version 1)
.IX MNTPROC_UMNT "" "" "" PAGE MAJOR
.LS
3. MNTPROC_UMNT (directory) returns ( )
	dirpath	directory;
.LF
.LP
Removes the mount list entry for
.LX directory .
.NE
.NH 4 
Remove All Mount Entries (Procedure 4, Version 1)
.IX MNTPROC_UMNTALL "" "" "" PAGE MAJOR
.LS
4. MNTPROC_UMNTALL ( ) returns ( )
.LF
.LP
Removes all of the mount list entries for this client.
.NE
.NH 4 
Return Export List (Procedure 5, Version 1)
.IX MNTPROC_EXPORT "" "" "" PAGE MAJOR
.LS
5. MNTPROC_EXPORT ( ) returns (exportlist)
	union switch (boolean more_entries) {
	    TRUE:
		struct {
		    dirpath	filesys;
		    typedef union switch (boolean more_groups) {
			TRUE:
			    struct {
				name	grname;
				groups	nextgroup;
			    }
			FALSE:
			    struct {}
		    } groups;
		    mountlist	nextentry;
		}
	    FALSE:
		struct {}
	} exportlist;
.LF
.LP
Returns in
.LX exportlist
a variable number of export list entries.
Each entry contains a filesystem name
and a list of groups that are allowed to import it.
The filesystem name is in
.LX exportlist.filesys ,
and the group name is in
.LX exportlist.groups.grname .
.LP
Bugs: the exportlist should contain more information
about the status of the filesystem, such as a read-only flag.
