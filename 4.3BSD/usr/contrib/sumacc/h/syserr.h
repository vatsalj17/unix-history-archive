| file syserr.text - macintosh system error equates file.
|__________________________________________________________________
|
|
| all system routines reporting errors include this equate file.
|
| modification history:
| 16 feb 83  lak  broke out from sysequ.text
| 22 feb 83  lak  added equates from sysutil and events.
| 10 may 83  mph  added new memory manager error codes.
| 07 jun 83  lak  adjusted error codes for file system
| 15 jun 83  ajh  added deep shit error definitions
| 23 jun 83  lak  added initiwmerr for sony driver.
| 18 jul 83  lak  added more memory manager deep shit errors.
| 11 aug 83  lak  added more disk driver error codes.
| 17 aug 83  lak  added file system deep shit error code.
| 18 aug 83  sc   added scrap manager error codes.
| 19 aug 83  lak  added stackinheap deep shit code.
| 21 aug 83  lak  added spdadjerr, seekerr, and sectnferr for disk driver.
|                 removed badnyberr| moved clock/pram error codes up by 4.
| 23 aug 83  lak  added notopenerr for drivers.
| 27 aug 83  ajh  added memfull deep shit alert
| 06 sep 83  ajh  added dsbadlaunch
| 22 sep 83  blh  added resource mgr errors
| 10 oct 83  lak  added noerr equate
| 13 oct 83  lak  added noerr, dsreinsert equates, dsnotthe1
|__________________________________________________________________

| general system errors (vbl mgr, queueing, etc.)

noerr       =        0      | success is absence of errors
qerr        =       -1      | queue element not found during deletion
vtyperr     =       -2      | invalid queue element
corerr      =       -3      | core routine number out of range
unimperr    =       -4      | unimplemented core routine

| i/o system errors

controlerr  =       -17
statuserr   =       -18
readerr     =       -19
writerr     =       -20
baduniterr  =       -21
unitemptyerr =      -22
openerr     =       -23
closerr     =       -24
dremoverr   =       -25     | tried to remove an open driver
dinsterr    =       -26     | drvrinstall couldn't find driver in resources
aborterr    =       -27     | io call aborted by killio
notopenerr  =       -28     | couldn't rd/wr/ctl/sts cause driver not opened

|  file system error codes:

dirfulerr   =       -33     | directory full
dskfulerr   =       -34     | disk full
nsverr      =       -35     | no such volume
ioerr       =       -36     | i/o error (bummers)
bdnamerr    =       -37     | there may be no bad names in the final system!
fnopnerr    =       -38     | file not open
eoferr      =       -39     | end of file
poserr      =       -40     | tried to position to before start of file (r/w)
mfulerr     =       -41     | memory full(open) or file won't fit (load)
tmfoerr     =       -42     | too many files open
fnferr      =       -43     | file not found

wprerr      =       -44     | diskette is write protected
flckderr    =       -45     | file is locked
vlckderr    =       -46     | volume is locked
fbsyerr     =       -47     | file is busy (delete)
dupfnerr    =       -48     | duplicate filename (rename)
opwrerr     =       -49     | file already open with with write permission
paramerr    =       -50     | error in user parameter list
rfnumerr    =       -51     | refnum error
gfperr      =       -52     | get file position error
volofflinerr =      -53     | volume not on line error (was ejected)
permerr     =       -54     | permissions error (on file open)
volonlinerr =       -55     | drive volume already on-line at mountvol
nsdrverr    =       -56     | no such drive (tried to mount a bad drive num)
nomacdskerr =       -57     | not a mac diskette (sig bytes are wrong)
extfserr    =       -58     | volume in question belongs to an external fs
fsdserr     =       -59     | file system deep s--t error:
                            |  during rename the old entry was deleted but could
                            |   not be restored . . .
badmdberr   =       -60     | bad master directory block
wrpermerr   =       -61     | write permissions error

| disk, serial ports, clock specific errors

nodriveerr  =       -64     | drive not installed
offlinerr   =       -65     | r/w requested for an off-line drive

nonyberr    =       -66     | couldn't find 5 nybbles in 200 tries
noadrmkerr  =       -67     | couldn't find valid addr mark
datavererr  =       -68     | read verify compare failed
badcksmerr  =       -69     | addr mark checksum didn't check
badbtslperr =       -70     | bad addr mark bit slip nibbles
nodtamkerr  =       -71     | couldn't find a data mark header
baddcksum   =       -72     | bad data mark checksum
baddbtslp   =       -73     | bad data mark bit slip nibbles
wrunderrun  =       -74     | write underrun occurred

cantsteperr =       -75     | step handshake failed
tk0baderr   =       -76     | track 0 detect doesn't change
initiwmerr  =       -77     | unable to initialize iwm
twosideerr  =       -78     | tried to read 2nd side on a 1-sided drive
spdadjerr   =       -79     | unable to correctly adjust disk speed
seekerr     =       -80     | track number wrong on address mark
sectnferr   =       -81     | sector number never found on a track

clkrderr    =       -85     | unable to read same clock value twice
clkwrerr    =       -86     | time written did not verify
prwrerr     =       -87     | parameter ram written didn't read-verify
priniterr   =       -88     | initutil found the parameter ram uninitialized

rcvrerr     =       -89     | scc receiver error (framing, parity, or)
breakrecd   =       -90     | break received (scc)

| storage allocator error codes

memfullerr  =       -108    | not enough room in heap zone
nilhandleerr =      -109    | handle was nil in handlezone or other|
memwzerr    =       -111    | whichzone failed (applied to free block)|
mempurerr   =       -112    | trying to purge a locked or non-purgable block|

memadrerr   =       -110    | address was odd, or out of range|
memazerr    =       -113    | address in zone check failed|
mempcerr    =       -114    | pointer check failed|
membcerr    =       -115    | block check failed|
memscerr    =       -116    | size check failed|

| resource manager error codes (other than i/o errors)

resnotfound     =       -192            | resource not found
resfnotfound    =       -193            | resource file not found
addresfailed    =       -194            | addresource failed
addreffailed    =       -195            | addreference failed
rmvresfailed    =       -196            | rmveresource failed
rmvreffailed    =       -197            | rmvereference failed

| scrap manager error codes

noscraperr  =       -100    | no scrap exists error
notypeerr   =       -102    | no object of that type in scrap


| application error codes
|
|  errors -1024 to -4095 are reserved for use by the current application


|  deep shit alert id definitions

dssyserr     =      32767   | general system error
dsbuserror   =      1       | bus error
dsaddresserr =      2       | address error
dsillinsterr =      3       | illegal instruction error
dszerodiverr =      4       | zero divide error
dschkerr     =      5       | check trap error
dsovflowerr  =      6       | overflow trap error
dspriverr    =      7       | privelege violation error
dstraceerr   =      8       | trace mode error
dslineaerr   =      9       | line 1010 trap error
dslineferr   =     10       | line 1111 trap error
dsmiscerr    =     11       | miscellaneous hardware exception error
dscoreerr    =     12       | unimplemented core routine error
dsirqerr     =     13       | uninstalled interrupt error

dsiocoreerr  =     14       | io core error
dsloaderr    =     15       | segment loader error
dsfperr      =     16       | floating point error

dsnopackerr  =     17       | package 0 not present
dsnopk1      =     18       | package 1 not present
dsnopk2      =     19       | package 2 not present
dsnopk3      =     20       | package 3 not present
dsnopk4      =     21       | package 4 not present
dsnopk5      =     22       | package 5 not present
dsnopk6      =     23       | package 6 not present
dsnopk7      =     24       | package 7 not present

dsmemfullerr =     25       | out of memory!
dsbadlaunch  =     26       | can't launch file

dsstknheap   =     28       | stack has moved into application heap
dsfserr      =     27       | file system map has been trashed
dsreinsert   =     30       | request user to reinsert off-line volume
dsnotthe1    =     31       | not the disk i wanted

| storage allocator trouble codes (deep shit ids)

memtrbbase  =       32              | memory manager trouble code base.
mtsetlog    =       memtrbbase      | set logical size error.
mtadjfre    =       memtrbbase+1    | adjust free error.
mtadjcnt    =       memtrbbase+2    | adjust counters error.
mtmkebkf    =       memtrbbase+3    | make block free error.
mtsetsiz    =       memtrbbase+4    | set size error.
mtinitmem   =       memtrbbase+5    | initialize memory manager error.
mtbcerr     =       memtrbbase+6    |
mtczerr     =       memtrbbase+7    |
mtcz1err    =       memtrbbase+8    |
mtcz2err    =       memtrbbase+9    |
mtcz3err    =       memtrbbase+10   |
mteqcerr    =       memtrbbase+11   |
mtevcerr    =       memtrbbase+12   |
mthcerr     =       memtrbbase+13   |
mtpcerr     =       memtrbbase+14   |
mtscerr     =       memtrbbase+15   |
mtrc1err    =       memtrbbase+16   |
mtrc2err    =       memtrbbase+17   |
mtsaberr    =       memtrbbase+18   |
mtacerr     =       memtrbbase+19   |
mtizcerr    =       memtrbbase+20   |
mtprcerr    =       memtrbbase+21   |

| some miscellaneous result codes

evtnotenb   =         1     | event not enabled at postevent
noevtavail  =        -1     | no event available (getosevent,oseventavail)

