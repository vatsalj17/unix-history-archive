.so /usr/lib/tmac/tmac.s
.so /usr/lib/tmac/tmac.cs
.tr ~ 
.\" .RP here gives release paper (no cs)
.\" following arg should be removed so itds can get their jollies
.TI
Using the SONY WORM drive and jukebox
.AH "Andrew Hume" MH 11271 6262 2C-471 research!andrew ""
.SA
.FP palatino
This note describes the software available for handling
Write-Once-Read-Many (WORM) optical disks.
Some of the software is applicable to most devices with a SCSI interface.
.SE
.KW "optical disk" "SCSI" "" ""
.TY IM y
.NU 11271 880913 01 39199-11 ""
.MY "" "" y "" "" "" ""
.PR 0
.RL y
.CO y
Paul Glick
Ted Kowalski
Marty Shannon
M J Sheehan
E J Sitar
.CE
.CV y
.CE
.SC 11
.SH
.FP palatino
Introduction
.de CS
.DS
.nf
.ft CW
.ps 8
.vs 9.5
.ta 8n +8n +8n +8n +8n
.in 10n
..
.de CE
.DE
..
.PP
Optical drives are quite trendy nowadays (mid 1988).
The AT&T Bell Laboratories Computation Centers are doing
cost and performance evaluations of available hardware.
There even plans for an official AT&T file backup product based on WORMs.
For now and the next several years, WORM disks are the dominant
optical disk technology.
.PP
WORM disks range in size from 3.5 to 13 inches in diameter
and have capacities between a few hundred megabytes to a few gigabytes.
They can either be double-sided or single-sided
(but in general drives are single-sided;
double-sided disks must be ejected and then turned over
in order to access the second side).
There is also a tradeoff between capacity and speed of access.
Disks which spin at a constant angular velocity (CAV) are much faster
(a factor of 2\-3) at moving the heads to a given spot than disks
which spin at a constant linear velocity at the reading head (CLV).
On the other hand, the CLV disks typically have a 50% greater capacity
than the CAV disks.
All other characteristics are about the same, including transfer rate.
For example, the disks we use are
.TS
center;
c l.
disk type	WDM-3DL0 (CLV)
disk size	12in
disk capacity	3.2GB (1.6GB/side)
seek time	600ms (1/3 stroke?)
read rate	~200KB/s
write rate	~75KB/s
.TE
I/O rates are measured on a multi-user timesharing system;
dedicated systems can do about 40% better.
.PP
Most WORM disks interface to computers by the SCSI bus.
Our computing environment (VAXen) doesn't support SCSI directly;
we use SCSI controllers that attach to the Unibus.
This is not as bad as it seems because for once, DEC had a good idea and
designed and implemented a device independent protocol
(Mass Storage Control Protocol or MSCP) for its mass storage devices.
The protocol handles arbitrary sized disks and views mass storage
as a virtual array of 512 byte blocks with no errors.
The lucky break for us is that third party vendors sell controllers
that take a SCSI disk on one side and on the other appear as a MSCP disk,
hiding dirty laundry such as bad block replacement and so on.
The end result is that we describe these controllers as regular DEC disks
and everything just works; at least, in theory.
I describe some rude exceptions below.
.PP
The software is divided into two parts.
The main body of code implements a file system within an arbitrary Unix file.
Normally, the file is a raw optical disk but it can be a regular Unix file.
In this latter guise, the software is much like a faster analog of
.I tar (1).
The other bunch of code is specific to SCSI and takes advantage of
SONY functionality beyond the basic disk functions.
.SH
The WORM software
.PP
The WORM file system code is both extensive and uninteresting.
It is adequately described in the manual entry.
The data layout on disk is a little peculiar, at least to the WORM manufacturers.
It is a linked list of segments, every
.CW "worm write"
command generating a new segment.
Each segment has the name, address,
.I stat
information, and data for a list of files.
The only trick here is that the pointer to the next segment
must be preallocated.
The main advantages of the design are robustness, simplicity
and efficiency.
The main disadvantage is that as more of the disk gets used,
it requires more CPU time to build the internal directory
as it is built by processing each segment in turn.
(This is fixed by the
.CW "worm btree"
command which build converts the directory into
.I cbt (1)
form and stores this at the end of the disk.)
.PP
The other disadvantage is that we need to recognize a block
that we have never written (the end of the linked list).
Regrettably, the WORM manufacturers regard this as an error
and so it is important that the MSCP-SCSI controller allows
the user to distinguish between this kind of error
(called a blank check in WORMland)
and a real read error (failing to read data we wrote).
In our environment, which admittedly deals poorly with errors of any kind,
this entails the driver in the operating system returning a different error
code (say,
.CW ENXIO
rather than the normal
.CW EIO ).
Every controller manufacturer signals blank check differently!
Emulex doesn't want you to know; they maintain that the mapping
from SCSI error codes to MSCP error codes, that is, how a blank check
is reported, is proprietary and won't tell you.
U.S. Design does exactly the right thing by returning
the MSCP ``forced data error'' code.
The MSCP standard says this is a special data error
whose meaning is controller-specific.
T.D. Systems returns a ``header compare'' data error,
rationalizing that no SCSI disk would ever return such an error.
(There is also a rumor that this is compatible with Emulex
but I am unable to verify.)
Unfortunately, other MSCP devices might so we are forced to handle
this special case in the MSCP driver.
.SH
The SCSI software
.PP
There are two programs:
.I scsish
and
.I "worm mount" .
The former is a shell with many commands to manipulate SCSI devices
although I have only tried it on SONY disks.
There is more than a little dependence on SONY specifics.
The latter does some jukebox specific stuff conveniently.
.PP
Caveat:
be careful about doing direct SCSI manipulation of a drive
you are accessing via MSCP.
The MSCP controllers have strong beliefs that they are the sole master of the drive
and are sensitive to changes in the state of the drive.
Unexpected changes are often rewarded by a hung controller,
cured only by a system reboot.
.PP
The command grammar for
.I scsish
is available via the
.CW help
command; this listing is generated from the
.I yacc (1)
grammar and is always correct.
Before discussing individual commands, we need to absorb some SCSI basics.
SCSI transactions are between an initiator and a target.
Both have bus ID's, a number between 0 and 7.
Typically, the host controller bus ID is 7 and the jukebox is 2.
The target is considered to have up to 8 logical units, each with a distinct
Logical Unit Number (LUN).
These numbers are typically set via DIP switches on the drives used.
Transactions have a SCSI status; the most common are
.I good
(the transaction succeeded),
.I busy
(the target did not respond)
and
.I check
(the command failed).
In the latter, a
.CW sense
command can be used to find out why it failed.
Normally,
.I scsish
reports errors rather than retrying.
(It will retry a few times on receiving a busy.)
In general, commands only take a LUN (or drive).
The target bus ID is set by the
.CW id
command (it defaults to the jukebox).
The following is a description of
.I some
of the commands.
An effort has been made to make the output somewhat self-explanatory;
if in doubt, consult the jukebox manual.
A basic checkout consists of
.CW sense~0 ,
.CW config ,
.CW status ,
and perhaps the diagnostics (see the
.CW internal
command).
Some commands accept shortened forms (like
.CW rel
for
.CW release ).
.de GG
.IP "\\fB\\$1\\f1" 20n
.if \w"\\fB\\$1\\f1"-20n \{
.br \}
..
.ne 1i
.GG alternate
gives the bad block replacement numbers.
.GG capacity
gives the capacity of the disk; both the number and size of blocks
in decimal and hexadecimal.
.CS
capacity 0
drive 0: capacity 1638000x1024 (18fe70x400)
.CE
.GG config
describes the configuration of your jukebox, controllers,
and drives.
It gives the numbers of drives and controllers, their versions (ROM IDs)
and the type of Unibus controller.
.CS
config
config(2,0): WORM device, '    SONY     WDA-3000-10 2.D', 1 controller, 2 drives
	Unibus-SCSI controller=T.D. Systems Viking
	ROMS: upper controller=x2, IF-129=x24, SY-46=x24, SS-30=x24
.CE
.GG copy
does third party copying between two drives.
(Third party means the initiator just says go;
the data only goes between the drives.)
I have copied an entire side of a disk in this way
at about 75KB/s independent of host speed.)
.GG disk~eject~\f4drive
eject the disk in the given drive.
.GG disk~release~\f4drive~shelf~side
release the disk in
.I drive
to
.I shelf .
The side (specified as
.CW a
or
.CW b )
means whether or not to invert the disk on the way to the shelf
(say
.CW b
to invert).
If the shelf and side are missing, it returns to wherever it came from.
.CS
disk rel 0 34 a
.CE
.GG disk~set~\f4shelf~side~drive
the opposite of
.CW disk~release .
New disks are loaded from shelf
.CW 127~a .
Note that 127 is a virtual shelf; the disks will reside on any shelf
which hasn't been the destination of a
.CW disk~release
command.
In general, the disk won't be moved to a physical drive until it needs to.
Thus, to ensure physical loading a disk, you need to follow a
.CW disk~set
by (say) a
.CW start
command.
Also, to even the load, logical units alternate between physical drives.
This is transparent unless one drive has a problem, in which case
(in a two drive system) the problems will only appear every second time
you use the disk.
.GG id~\f4number
set the target bus ID to
.I number .
The jukebox, and default, is 2.
.GG inquiry~\f4drive
give a simple status for the the given drive or if no drive is given, all drives.
.CS
inq
drive 2,0: disk,write protect,,,ready (0x9)
drive 2,1: disk,writable,,,not ready (0x0)
drive 2,2: no disk,writable,,,not ready (0x40)
drive 2,3: no disk,writable,,,not ready (0x40)
drive 2,4: no disk,writable,,,not ready (0x40)
drive 2,5: no disk,writable,,,not ready (0x40)
drive 2,6: no disk,writable,,,not ready (0x40)
drive 2,7: no disk,writable,,,not ready (0x40)
.CE
.GG internal~\f4n
there are several internal diagnostics; the full list is given by
.CS
internal
internal 0: internal command table
internal 1: error information table
internal 2: arm controller diagnostics
internal 3: scsi control board diagnostics
internal 4: drive controller diagnostics
internal 5: jukebox status
.CE
The arm controller diagnostics take about 4 minutes,
the drive controller diagnostics take about 100 seconds.
The others are quite short.
The drive controller diagnostic needs a LUN.
.CS
internal 3
scsi control board diagnostics:
	ended normally (time: 7s)
internal 4 0
drive 0[upper]: drive controller diagnostics
diagnostic result: no faults (time: 97s)
test 0[drive on/off]: good
test 1[read disk id]: good
test 2[move]: good
test 3[seek]: good
test 4[blank sector search]: good
test 5[written sector search]: good
test 6[search writable area]: good
test 7[write]: diagnostic could not be done
test 8[ECC margin check]: diagnostic could not be done
test 9[read data compare]: diagnostic could not be done
diagnostic count (drive:avail): 0:199 1:0 2:0 3:0 4:0 5:0 6:0 7:0
internal 5
jukebox status: component(fatal err/err/cmds)
upper drive(0/0/38) lower drive(0/0/65) sys control(0/0/115) backup mem(0/4/4454)
.CE
The
.CW internal~4
diagnostic requires a LUN with a disk in it.
.GG media~\f4drive~start~nblocks
do a media reliability check for the given blocks
(it does about 200 blocks/sec).
Prefixing the media command with
.CW ext
gives details for every funny block.
You can also dump the information to a disk file.
.CS
media 1 0 1000
drive 1: media check for 1000 blocks [0-999], lower drive
999 good, 1 unwritten, 
ext media 1 0 1000
drive 1: detailed media check for 1000 blocks [0-999], lower drive
lbn 0: unwritten
999 good, 1 unwritten, 
.CE
.GG read~id~\f4drive
gives the id as used by the
.I worm
commands.
.CS
read id 1
id='backupdb2a'
.CE
.GG reset
resets the controller.
The behavior depends on the controller type.
The T.D. Systems controller resets quickly and reliably.
The U.S. Design controller causes a SCSI bus reset which resets the jukebox.
This takes about 90 seconds.
.GG sense~\f4drive
gives the sense data for the last operation.
A prefix of
.CW ext
gives the extended sense.
.CS
sense 0
sa=0x0 mscp=0x0 status=0x0(good) (ext: no sense)
	 70 0 0 0 dd dd dd dd dd dd dd dd dd dd dd dd
sense 1
sa=0x0 mscp=0x0 status=0x0(good) (ext: illegal request)
	 70 0 5 0 dd dd dd dd dd dd dd dd dd dd dd dd
.CE
.GG sleep~\f4number
sleep for
.I number
seconds.
This gives you time to rush to the hardware!
.GG start~\f4drive
start the given drive.
This is necessary before most commands for a disk.
.GG status
tells you all sorts of stuff, some of it interesting.
.CS
status
drive 0: ready,disk in LUN,power on,disk in drive 0, return shelf 18a (36)
drive 1: not ready,no disk in LUN,power on,disk in shelf 0
drive 2: not ready,no disk in LUN,power on,disk in shelf 0
drive 3: not ready,no disk in LUN,power on,disk in shelf 0
drive 4: not ready,no disk in LUN,power on,disk in shelf 0
drive 5: not ready,no disk in LUN,power on,disk in shelf 0
drive 6: not ready,no disk in LUN,power on,disk in shelf 0
drive 7: not ready,no disk in LUN,power on,disk in shelf 0
0: disk,
5: disk,
6: disk,
8: disk,
18: no disk
21: disk,
I/O shelf: no disk
carrier: disk shelf=8a (16)
upper drive: disk, LUN=0
lower drive: no disk
.CE
.GG stop~\f4drive
stop the drive from spinning.
.GG test~\f4drive
does a Test Unit Ready.
It simply returns a good/check answer.
.SH
Reliability
.PP
My WORM experience covers four pieces of hardware:
WORM drive, WORM controller, jukebox and the UNIBUS-SCSI controller.
.PP
The WORM drives have proved fairly reliable.
Two of the six drives in our installation have required service.
One has an intermittent loading problem where the disk being loaded
is continually inserted and ejected.
The other was a brand new drive in the jukebox.
It gave no overt signs of trouble but media quality diagnostics
done in that drive always gave terrible results.
The drives in the jukebox can be used by themselves;
the only difference is a  DIP switch setting
(which regrettably requires taking off the drive's front panel).
.PP
The WORM controllers had an early problem with infant mortality
(two of four were D.O.A.).
There has been no trouble since then.
.PP
The jukebox really has two parts.
One is the robotics section and the other is a smart controller
which makes the jukebox look like 8 logical drives
independent of how many physical drives there are.
(In fact, SONY measures the time to change disks by mounting two
disks in a one drive jukebox and then copying from one to another.)
The controller has worked flawlessly.
In fact, be warned;
it has battery backup and remembers where disks were and are!
The mechanical parts have been pounded on and so far have not had problems.
All in all, our jukebox experience is limited but positive.
The maintenance contract on the jukebox is relatively inexpensive.
.PP
The UNIBUS-SCSI controller has been the most exasperating piece of the puzzle.
The first attempt was the Emulex UC13 controller.
It never really worked quite right and Emulex
refused to say how a blank check was returned.
The second attempt was the U.S. Design 1158.
After four or five months,
they were able to decide exactly what the switch settings
were supposed to be and the MSCP emulation worked just fine.
The SCSI controller (that is, an 1158 acting as a pure SCSI controller)
seems to work okay (apart from not working in single user mode).
Unfortunately, the combination of the two controllers
(necessary for controlling the jukebox) is still flaky.
The MSCP controller has a tendency to hang and the SCSI controller
generates stray interrupts.
.PP
The third and most successful controller has been the T.D. Systems
Viking UDD controller.
This combines both controllers on one quad high board
(as opposed to two hex high boards for the U.S. Design setup)
for the same price as one U.S. Design board.
Despite some unpleasantness from the manufacturer (or more exactly, its president),
these boards have worked and worked well.
.SH
Ordering information
.PP
The SONY WORMS and my software are deployed at three sites within Area 112.
The distinct configurations are listed below;
the total number of WORM drives is 9.
.TS
center;
c c c
c a l.
Site	Machine	Configuration
_
1121	8550	1 controller/1 drive
1122	11/750	1 controller/1 drive
1122	11/750	jukebox (1 controller/1 drive)
1127	11/750	1 controller/2 drives
1127	11/750	jukebox (1 controller/2 drives)
.TE
.PP
The WORM hardware was purchased from U.S. Design
because they offered the UNIBUS controller.
If you want to buy from someone closer than Maryland, call SONY for a list
of distributers.
All costs are approximately correct as of mid-1988.
.TS
center;
c c c c
l a l.
Part	Part #	Cost	Supplier
_
SONY disk controller	WDA-3000 controller	$10K	U.S. Design
SONY disk drive	WDA-3000 disk drive	$12.5K	U.S. Design
SONY jukebox	WDA-3000-10 Auto Changer	$75K	U.S. Design
1158 controller	B58V-3B	$1895	U.S. Design
T.D. Systems UDD controller	Viking UDD controller	$1800	Trimarchi
.TE
You will need a converter to connect the 50 pin Berg connector on the
controller boards to the 50 pin D connector on the SONY gear.
We use ones that came with the U.S. Design controller.
It is not hard to make your own.
.TS
center;
c s
a l.
Phone numbers
_
(201) 930-6030	SONY (Bart Connors)
(617) 937-9465	T.D. Systems
(800) 356-6638	Trimarchi Inc (distributor for T.D. Systems in NJ)
(301) 577-2880	U.S. Design (Harry Garonzik)
.TE
.SG
.sp 2
Att.
.br
manual pages
.bp
worm(1)
scsi(4)
