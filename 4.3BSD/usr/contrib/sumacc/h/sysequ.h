| file sysequ.text - macintosh system equates file.
|________________________________________________________________________
|
|
| written by:  bud tribble        6-may-81
|
| all system routines include this equate file.
|
|   modification history:
|
|      integrate lisagraf                              --  alo 23-dec-81
|      configured for alpha release (one heap)         --  alo  5-feb-82
|      moved lisagraf variables up to allow more room  --  alo 10-feb-82
|      added system base date & time variables         --  alo 13-feb-82
|      allowed 20 bytes for system parameter ram area  --  alo  1-mar-82
|      added ptr for sysoutfib,sysinfib,sysrefnum(macpaslib)
|                                                      --  alo 1-apr-82
|      added scc hardware addresses| removed 6551 stuff|
|      pulled mbstate and mbticks from kbd driver vars --  lak 10-may-82
|      added scc write reg 5 globals| clock-keyboard
|      synchronization flags . . .                     --  lak 27-may-82
|      got rid of vbl queue element "mode" word        --  lak 04-jun-82
|      changed keyrepcount to keyreptime| added
|      procedural interface variables for key mapping  --  lak 07-jun-82
|      updated for 512-dots                            --  lak 26-aug-82
|      changed via addresses for timing problem        --  lak 17-sep-82
|      changed iwm addresses for better margin         --  lak 23-sep-82
|      updated to 384 current version (0.5d)           --  lak 10-oct-82
|      added fsinitdbg for filler1 (sysinit, debug)    --  lak 19-oct-82
|      added tagdata field for twiggy driver           --  lak 01-nov-82
|      added drvinstalled field for twiggy driver|
|       changed warmcold to twiggyvars                 --  lak 02-nov-82
|      removed drvinstalled field| removed fib pointers,
|       and changed start of system heap| changed loader
|       variable area                                  --  lak 18 nov 82
|      added a compare string jump vector              --  lak 10 dec 82
|      changed driver definitons, etc.                 --  lak 17 dec 82
|      added via equates                               --  lak 03 jan 83
|      added new i/o param blk equates for new fs      --  lak 17 jan 83
|      added storage manager error codes               --  lak 04 feb 83
|      addw disk cal switch error code                  --  lak 05 feb 83
|      added keyboard task vector in /124 (kybdtask)|
|       broke error codes out into syserr.text
|       added loadtrap lomem var                       --  lak 16 feb 83
|      put dispatch table where macsbug globals used
|       to be and moved down toolbox vars.             --  lak 18 feb 83
|      added dsalerttab pointer                        --  ajh 29 mar 83
|      added bootdrive field                           --  ajh 04 apr 83
|      added pollstack, pollproc, dskerr, dskrtnadr    --  lak 11 apr 83
|      added sonyvars,pwmbuf2                          --  lak 17 apr 83
|      changed screen, pwm address for 512k proto      --  ajh 01 may 83
|      added resource def                              --  bjh 09 may 83
|      added pwmvalue                                  --  lak 09 may 83
|      added heap check hooks                          --  mph 10 may 83
|      changed scc addresses (high bits on)            --  lak 25 may 83
|      added dqfsid,
|       stretched keypadmap to 8 bytes                 --  lak 01 jun 83
|      added mask constant, changed heap check hooks   --  mph 02 jun 83
|      added equates for minstack, defltstack for memory
|       manager| moved unitentries equate to startinit|
|       added unitntrycnt lomem var instead.           --  lak 16 jun 83
|      added "noqueue" bit definition                  --  ajh 23 jun 83
|      added "curiotrap" (replacing "filler3" )        --  ajh 26 jun 83
|      added "fortwiggy" conditional assembly switch   --  ajh 27 jun 83
|      cleaned up: changed unused vars to fillerx equates|
|       changed ardcmd,awrcmd,actlcmd,astscmd to match
|       trap numbers . . .                             --  lak 15 jul 83
|      added mminok for memory manager checking.       --  lak 24 jul 83
|      added mmdefflags for memory manager zone init.  --  lak 31 jul 83
|      added dskverify for disk driver verify/read flag
|        in place of dskdump                           --  lak 06 aug 83
|      removed dneedsstorage equate (no longer used)   --  lak 08 aug 83
|      added loader global definitions, no more spbot  --  ajh 11 aug 83
|      added iwm lomem variable . . .                  --  lak 15 aug 83
|      added evtbufcnt, jcrsrtask, moved findername    --  lak 17 aug 83
|      moved jcrsrtask to grafequ.                     --  lak 18 aug 83
|      added scrap vars to /960  .                     --  sc  18 aug 83
|      added stklowpt var for useful measurements.     --  lak 19 aug 83
|      added doubletime and carettime                  --  sc  19 aug 83
|      added spclikcaret to parameter ram              --  sc  19 aug 83
|      folded spkbd stuff in parameter ram             --  sc  19 aug 83
|      corrected rom size counts (rombsize, romwsize)  --  ajh 22 aug 83
|      added keyclick bit in parameter ram             --  ajh 11 sep 83
|      added growzone handle/pointer warnings          --  sc  12 sep 83
|      added resource manager equates                  --  blh 22 sep 83
|      added 16 bytes of printing globals              --  ajh 30 sep 83
|      added desk ornament message equates,changed events  ajh 08 oct 83
|      added dqdrvsize                                 --  lak 02 nov 83
|      added equates for jgnefilter, scrdmpenb, scrdmptype lak 09 nov 83
|      added screenrow                              -- ajh/wda 10 nov 83
|      added equates for dsalertrect, dsdrawproc,
|        ejectnotify, coreeditvars, qdexist,
|        wwexist and jshell                            --  lak 16 nov 83
|      added equate for alarmstate                     --  lak 22 nov 83
|      added equate for initapplzone notify proc       --  lak 06 dec 83
|      added equates for screen vertical, horiz res    --  lak 19 dec 83
|      added needsgoodbye                              --  ajh 03 jan 84
|      added sysversion                                --  ajh 06 jan 84
|      added dswndupdate, sevtenb flags                --  lak 11 jan 84
|      added fontflag for font manager bug fix         --  ajh 12 jan 84
|_______________________________________________________________________

|________________________________________________________________________
|
|                   system low memory layout
|
|        0000-00ff     exception vectors defined by the hardware
|        0100-0340     system communications area
|        0340-03ff     file system globals (172 bytes used)
|        0400-07ff     system core routine/lisagraf/toolbox dispatch table
|        0800-08ff     mouse/cursor low-memory globals
|        0900-097f     more os stuff
|        0980-0aff     toolbox variables
|        0b00-xxxx     start of system heap
|
|    some other important system data structures are allocated on the heap
|
|________________________________________________________________________

| conditional assembly switch to select twiggy or sony

fortwiggy   =       0

|________________________________________________________________________
|
| system constants
|________________________________________________________________________

| device equates

|   note -- all screen and sound addresses are for the 512k mac,
|   which will also work for the 128k machine since the address
|   space wraps

screenlow       =       /0007a700           | top of screen screen address
soundlow        =       /0007fd00           | low sound buffer address
pwmbuffer       =       /0007fd01           | pwm bytes are low bytes
sndbufwlen      =       /0172               | sound/disk buffer word length
ovlyram         =       /00600000           | low ram address when overlay is on
ovlyscreen      =       /0067a700           | top of screen with overlay

romstart        =       /00400000           | starting address of rom code
romwsize        =       /00008000           | 32768 words in present rom
rombsize        =       /00010000           | 65536 bytes in present rom
memlsize        =       /00008000           | memory contains 32k long words
memwsize        =       /00010000           | 64k words
membsize        =       /00020000           | and 128k bytes
linelen         =       /40                 | horizontal screen line has 64 bytes

| via (6522)
| absolute addresses

vbase           =       /efe1fe             | base address
avbufb          =       vbase               | buffer b
avbufa          =       /effffe             | buffer a
avbufm          =       avbufb              | buffer with mouse button bit
avifr           =       /effbfe             | interrupt flag register
avier           =       /effdfe             | interrupt enable register

| offsets

vbufb           =       512*0               | buffer b
vbufah          =       512*1               | buffer a (with handshake) [ dont use! ]
vdirb           =       512*2               | direction b
vdira           =       512*3               | direction a
vt1c            =       512*4               | timer 1 counter (l.o.)
vt1ch           =       512*5               | timer 1 counter (high order)
vt1l            =       512*6               | timer 1 latch (l.o.)
vt1lh           =       512*7               | timer 1 latch (high order)
vt2c            =       512*8               | timer 2 latch (l.o.)
vt2ch           =       512*9               | timer 2 counter (high order)
vsr             =       512*10              | shift register
vacr            =       512*11              | aux. control reg.
vpcr            =       512*12              | periph. control reg.
vifr            =       512*13              | int. flag reg.
vier            =       512*14              | int. enable reg.
vbufa           =       512*15              | buffer a

vbufd           =       vbufa               | disk head select buffer

| buffer a:

vaout           =       /7f                 | vbufa output bits
vainit          =       /7b                 | vbufa initial values med. volume
vsound          =       /07                 | sound volume bits
vsndpg2         =       3                   | select sound page 2 if 0
voverlay        =       4                   | overlay bit (overlay when 1)
vheadsel        =       5
vpage2          =       6                   | select video page 2 if 0
vsccwrreq       =       7                   | scc write/request line

| buffer b:

vbout           =       /87                 | vbufb output bits
vbinit          =       /07                 | vbufb initial values
rtcdata         =       0
rtcclk          =       1
rtcenb          =       2                   | enabled when 0
vsw             =       3                   | mouse switch (0 when down)
vx2             =       4                   | mouse x level
vy2             =       5                   | mouse y level
vh4             =       6                   | horizontal sync
vsndenb         =       7                   | /sound enable (reset when 1)

| note: ca1 = vsync
|       ca2 = 1 sec clock
|       cb1,cb2 are keyboard interface.

|________________________________________________________________________
|
| scc serial chip addresses

sccrbase        =       /9ffff8             | scc base read address
sccwbase        =       /bffff9             | scc base write address

adata           =       6                   | offset for a channel data
actl            =       2                   | offset for a channel control
bdata           =       4                   | offset for b channel data
bctl            =       0                   | offset for b channel control

sccdata         =       4                   | general offset for data from control
sccwrite        =       /200001             | general offset for write from read

rxbf            =       0                   | scc receive buffer full
txbe            =       2                   | scc transmit buffer empty

|________________________________________________________________________
|
| disk address

dbase           =       /dfe1ff             | disk address base
dph0l           =       dbase               | phase 0 low
dph0h           =       /dfe3ff             | phase 0 high
dmtroff         =       /dff1ff             | iwm motor off
dmtron          =       /dff3ff             | iwm motor on
diskq6l         =       /dff9ff             | shift register
diskq6h         =       /dffbff
diskq7l         =       /dffdff
diskq7h         =       /dfffff

ph0l            =       512*0               | disk address offsets from base
ph0h            =       512*1
ph1l            =       512*2
ph1h            =       512*3
ph2l            =       512*4
ph2h            =       512*5
ph3l            =       512*6
ph3h            =       512*7

mtroff          =       512*8
mtron           =       512*9
intdrive        =       512*10              | enable internal drive address
extdrive        =       512*11              | enable external drive address
q6l             =       512*12
q6h             =       512*13
q7l             =       512*14
q7h             =       512*15

|________________________________________________________________________
|
|  interrupt auto-vector and exception definitions
|________________________________________________________________________

line1010        =       /28                 | 1010 emulator trap (system routines)
line1111        =       /2c                 | 1111 emulator trap (reserved)
autoint1        =       /64                 | level 1 auto-vector
autoint2        =       /68                 | level 2 auto-vector
autoint3        =       /6c                 | level 3 auto-vector
autoint4        =       /70                 | level 4 auto-vector
autoint5        =       /74                 | level 5 auto-vector
autoint6        =       /78                 | level 6 auto-vector
autoint7        =       /7c                 | level 7 auto-vector

|________________________________________________________________________
|
| syscom equates (system communication area)
|________________________________________________________________________

syscom          =       /100            | start of system communication area

| system locations

monkeylives     =       /100            | monkey lives if >= 0
scrvres         =       /102            | screen vertical resolution (dots/inch)
scrhres         =       /104            | screen horizontal resolution (dots/inch)
screenrow       =       /106            | rowbytes of mac screen
memtop          =       /108            | ptr to top of memory
bufptr          =       /10c            | ptr to bottom of code buffer
stklowpt        =       /110            | lowest stack as measured in vbl task
heapend         =       /114            | ptr to end of heap
thezone         =       /118            | ptr to current heap zone
utablebase      =       /11c            | ptr to unit i/o table
macjmp          =       /120            | ptr to macsbug jumptable

dskrtnadr       =       /124            | temp for disk driver
twiggyvars      =       /128            | ptr to twiggy driver locals
dskverify       =       /12c            | used by sony driver for read/verify
loadtrap        =       /12d            | set to non-zero to trap before pgm strt
mminok          =       /12e            | non-zero when initial mem mngr cks ok
dskwr11         =       /12f            | try 1-1 disk writes when non-zero
appllimit       =       /130            |

sonyvars        =       appllimit+4     | pointer to 3-1/2 disk driver vars
pwmvalue        =       sonyvars+4      | current pwm value
pollstack       =       pwmvalue+2      | scc poll data start stack location
pollproc        =       pollstack+4     | proc which handles scc poll data
dskerr          =       pollproc+4      | disk routine result code

sysevtmask      =       dskerr+2        | system event mask
sysevtbuf       =       sysevtmask+2    | ptr to system event queue element buffer
eventqueue      =       sysevtbuf+4     | 5 words, event queue header
evtbufcnt       =       eventqueue+10   | max number of events in sysevtbuf - 1

rndseed         =       evtbufcnt+2
sysversion      =       rndseed+4       | version # of ram-based system
sevtenb         =       sysversion+2    | 1 byte: 0 disables sysevent calls from gne
dswndupdate     =       sevtenb+1       | 1 byte: 0 flags gne to paintbehind ds alertrect
fontflag        =       dswndupdate+1   | boolean for font manager bug
filler3         =       fontflag+1      | 1 byte of filler

vblqueue        =       filler3+1       | 5 words, vbl queue header
ticks           =       vblqueue+10     | tick count, time since boot in 1/60 increments
mbticks         =       ticks+4         | tick count when mouse button last changed
mbstate         =       mbticks+4       | current mouse button state
tocks           =       mbstate+1       | lisa sub-tick count

keymap          =       mbstate+2       | 2 longs, a bitmap of the keyboard
keypadmap       =       keymap+8        | 1 long, a bitmap for numeric pad-18bits
keylast         =       keypadmap+8     | lo byte: ascii for last valid keycode
                                        | hi byte: last valid keycode
                                        | these are 0 when no key is down
keytime         =       keylast+2       | long tickcount when keylast was rec'd
keyreptime      =       keytime+4       | long tickcount when key was last repeated
keythresh       =       keyreptime+4    | word containing threshold for repeat
keyrepthresh    =       keythresh+2     | word containing repeat speed

lvl1dt          =       keyrepthresh+2  | interrupt level 1 dispatch table
lvl2dt          =       lvl1dt+32       | interrupt level 2 dispatch table

unitntrycnt     =       lvl2dt+32       | count of entries in unit table
via             =       unitntrycnt+2   | via base address (use low memory addresses)
sccrd           =       via+4           | scc base read address (to save code space)
sccwr           =       sccrd+4         | scc base write address
iwm             =       sccwr+4         | iwm base address

getparam        =       iwm+4           | parameter blk for reading sys parameter area

sysparam        =       getparam+20     | 20 bytes of system parameter area
spvalid         =       sysparam        | byte 1  = validation field (/a7)
spodometer      =       spvalid+1       |    2-4  = odometer
spporta         =       spodometer+3    |    5-6  = scc port a configuration
spportb         =       spporta+2       |    7-8  = scc port b configuration
spalarm         =       spportb+2       |    9-12 = alarm time
spfont          =       spalarm+4       |   13-14 = default font id
spkbd           =       spfont+2        |      15 = kbd repeat thresh in 4/60ths(4)
                                        |           kbd repeat rates in 2/60ths(4)
spprint         =       spkbd+1         |      16 = print stuff
spvolctl        =       spprint+1       |      17 = volume control (low 3 bits)
spclikcaret     =       spvolctl+1      |      18 = double time in 4/60ths(4)
                                        |           caret blink time in 4/60ths(4)
spmisc1         =       spclikcaret+1   |      19 = english/metric (1),
                                        |           extra (2),
                                        |           country code (5)
spmisc2         =       spmisc1+1       |      20 = paranoia level (1), mouse
                                        |           scaling(1), keyclick (1), boot
                                        |           disk (1), menu flash (2), help
                                        |           level (2)
time            =       sysparam+20     | clock time when last read (extrapolated)
bootdrive       =       time+4          | drive number of boot drive

jshell          =       time+6          | used by journaling shell . . .
filler3a        =       jshell+2        | used by standard file

kbdvars         =       filler3a+2      | keyboard manager variables (4 bytes)
jkybdtask       =       kbdvars+4       | keyboard vbl task hook
kbdtype         =       jkybdtask+4     | high byte holds keyboard model number
alarmstate      =       kbdtype+1       | bit7=parity, bit6=beeped, bit0=1=enabled

curiotrap       =       kbdtype+2       | trap the caused current io transaction

diskvars        =       curiotrap+2     | disk driver variables (62 bytes)

sdvolume        =       diskvars+62     | global volume control (1 byte)
sdenable        =       sdvolume+1      | byte to enable 4 voice engine
soundvars       =       sdenable+1      | sound driver variables (really only 20 bytes)

soundptr        =       soundvars       |pointer to 4ve sound definition table
soundbase       =       soundptr+4      |base address of sound bitmap
soundvbl        =       soundbase+4     |vertical retrace control element
sounddce        =       soundvbl+16     |pointer to sound driver dce
soundactive     =       sounddce+4      |boolean specifying if sound is enabled
soundlevel      =       soundactive+1   |byte specifying current level in buffer
curpitch        =       soundlevel+1    |word holding current pitch value

soundlast       =       curpitch+2      |address past last sound variable

screenvars      =       soundvars+48    | screen driver variables

jgnefilter      =       screenvars+8    | getnextevent filter proc

key1trans       =       jgnefilter+4    | pointer to procedure handling keyboard
                                        |  key translation
key2trans       =       key1trans+4     | pointer to procedure handling numeric
                                        |  keypad key translation
syszone         =       key2trans+4     | pointer to system heap zone
applzone        =       syszone+4       | pointer to application heap zone
rombase         =       applzone+4      | start of rom addresses (jump table?)
rambase         =       rombase+4       | start of ram addresses
basicglob       =       rambase+4       | pointer to basic globals
dsalerttab      =       basicglob+4     | pointer to deep shit alerts
extstsdt        =       dsalerttab+4    | scc ext/sts secondary dispatch table
sccasts         =       extstsdt+16     | scc read reg 0 last ext/sts rupt - a
sccbsts         =       sccasts+1       | scc read reg 0 last ext/sts rupt - b
serialvars      =       sccbsts+1       | async driver variables
findername      =       serialvars+16   | 16 byte name of finder
doubletime      =       findername+16   | double click time(set up by boot blocks)
carettime       =       doubletime+4    | caret blink time
scrdmpenb       =       carettime+4     | screen dump enabled when non-zero
scrdmptype      =       scrdmpenb+1     | ff dumps screen, fe dumps front window
tagdata         =       scrdmptype+1    | sector tag info for twiggy drivers
drvqhdr         =       tagdata+14      | queue header of drive numbers in system
pwmbuf2         =       drvqhdr+10      | pointer to pwm buffer 1 (or 2 if sound
                                        |  buffer 2 is being used)
hpchk           =       pwmbuf2+4       | points to heap check ram code
maskbc          =       hpchk+4         | memory manager byte count mask
maskhandle      =       maskbc          | memory manager handle mask
maskptr         =       maskbc          | memory manager pointer mask
minstack        =       maskbc+4        | min stack size (1024) used in initapplzone
defltstack      =       minstack+4      | default size of stack (for growing azone)
mmdefflags      =       defltstack+4    | default zone flags
gzroothnd       =       mmdefflags+2    | root handle for growzone
gzrootptr       =       gzroothnd+4     | root pointer for growzone
gzmovehnd       =       gzrootptr+4     | moving handle for growzone
dsdrawproc      =       gzmovehnd+4     | alternate deepshit box draw proc
ejectnotify     =       dsdrawproc+4    | proc to call and notify upon ejects
iaznotify       =       ejectnotify+4   | proc to call and notify upon world swaps
endofvars       =       iaznotify+4     | end of final defined vars

|________________________________________________________________________
|
| the file system local variables are kept from /340 to /3ff
|________________________________________________________________________

filevars    =       /340                |
dsalertrect =       /3f8                | rectangle for disk-switch alert

|________________________________________________________________________
|
|   system core routine dispatch table resides from /400-/7ff.  the user
|   can intercept any core routine by changing these pointers.
|________________________________________________________________________

dispatchtab     =       /400

|________________________________________________________________________
|
|    the cursor jump table interface and cursor state variables start at /800
|    for their definitions, see the file "grafequ.text". starting at /8fc and
|    working down we have addresses of os routines which are not accessed
|    through traps.
|________________________________________________________________________

wwexist         =       /8f2            | zero when window manager is initialized
qdexist         =       /8f3            | zero when quickdraw is initialized
jfetch          =       /8f4            | fetch a byte routine for i/o drivers
jstash          =       /8f8            | stash a byte routine for i/o drivers
jiodone         =       /8fc            | iodone entry location


loadvars        =       /900            | loader variables

| low-memory loader globals

curaprefnum    =         loadvars            | refnum of current resfile
launchflag     =         curaprefnum+2       | boolean to distinguish launch/chain
currenta5      =         launchflag+2        | current value of a5
curstackbase   =         currenta5+4         | current stack base
loadfiller     =         curstackbase+4      | empty for now...
curapname      =         loadfiller+4        | place to save name of application
saveseghandle  =         curapname+32        | place to save seg 0 handle
curjtoffset    =         saveseghandle+4     | word for current jump table offset
curpageoption  =         curjtoffset+2       | current page 2 configuration word
loaderpblock   =         curpageoption+4     | param block for internal exittoshell
lastlglobal    =         loaderpblock+10     | address past last loader global

printvars      =         lastlglobal         | 16 bytes for owen
lastpglobal    =         printvars+16        | address of last global
coreeditvars   =         lastpglobal         | 12 bytes for core edit

| low memory vars for the scrap manager

scrapvars   =           /960

scrapinfo   =           scrapvars               | scrap info
scraphandle =           scrapinfo+4             | handle to memory scrap
scrapcount  =           scraphandle+4           | validation byte
scrapstate  =           scrapcount+2            | state of scrap defined below
scrapname   =           scrapstate+2            | pointer to scrap name
scraptag    =           scrapname+4             | actual name goes here
scrapend    =           scraptag+16             | end of scrap vars


toolvars        =       /980            | toolbox variables

|________________________________________________________________________
|
| finally, we have the start of the heap.  note that some important system
| tables are kept on the heap.
|
|________________________________________________________________________

heapstart       =       /0b00           | start of the heap

|________________________________________________________________________
|
| system data structures
|
|________________________________________________________________________

lock            =       7           | lock bit in a master pointer
purge           =       6           | bit for purgeable/unpurgeable
resource        =       5           | bit to flag a resource handle

| queue header definition

qheadsize       =       10          | a queue header is 10 bytes long

qflags          =       0           | misc. flags
qhead           =       2           | 32-bit ptr to first element
qtail           =       6           | 32-bit ptr to last element
qinuse          =       7           | queue-in-use flag bit

| queue element type definitions

vtype           =       1           | vbl queue element is type 1
ioqtype         =       2           | i/o queue element is type 2
timertype       =       3           | timer queue element is type 3
evtype          =       4           | event queue element is type 4
fsqtype         =       5           | file system vcb element

| general purpose queue element definition

qlink           =       0           | link to next queue element
qtype           =       4           | queue element type

| drive queue element offsets

dqellnth    =       12              | drive queue element length including hdr
|change to 14 except for sony driver!
dqdrive     =       6               | drive number
dqrefnum    =       8               | driver refnum
dqfsid      =       10              | file system handling this drive
dqdrvsize   =       12              | number of blocks this drive

| unit table definition -- there is one entry in the unit table for each
| logical driver.  the number of entries is hardwired to 16.  each entry consists
| of one 32-bit memory handle| the first entry corresponds to refnum -1, the
| last to refnum -32.  the unit table is made during sysinit time and is
| allocated from permanent system memory| lo-memory location utablebase points
| to the table. unitntrycnt is initialized to unitentries by startinit: all
| other code should use this low mem variable.

| device control entry definition: for each unit table entry, one driver may be
| installed.  if the driver is installed, the corresponding memory handle in the
| unit table will be the handle of a device control entry.

dctlentrysize   =       40          | each entry is 40 bytes long

dctldriver      =       0           | offset of pointer to driver
dctlflags       =       4           | offset of flags word
dctlqueue       =       6           | offset of queue header
dctlqhead       =       8           | offset of queue first-element pointer
dctlqtail       =       12          | offset of queue last-element pointer
dctlposition    =       16          | offset of longword position pointer
dctlstorage     =       20          | handle of driver's private storage
dctlrefnum      =       24          | refnum of this driver
dctlcurticks    =       26          | long counter for timing systemtask calls
dctlwindow      =       30          | pointer to driver's window (if any)
dctldelay       =       34          | word for number of ticks between systask calls
dctlemask       =       36          | word for desk ornament event mask
dctlmenu        =       38          | word for menu associated with driver

| vertical blanking control block queue element

vblink          =       0           | link to next element
vbltype         =       4           | unique id for validity check
vbladdr         =       6           | address of service routine
vblcount        =       10          | count field for timeout
vblphase        =       12          | phase to allow synchronization
|
invbl           =       6           | bit index for "in vbl" flag


| driver format definitions

drvrflags       =        0          | various flags and permissions
drvrdelay       =        2          | word indicating # of ticks between systask calls
drvremask       =        4          | word indicating what events to accept
drvrmenu        =        6          | word indicating driver menu id
drvropen        =        8          | word offset to open routine
drvrprime       =       10          | word offset to prime routine
drvrctl         =       12          | word offset to control routine
drvrstatus      =       14          | word offset to status routine
drvrclose       =       16          | word offset to warmstart reset routine
drvrname        =       18          | length byte and name of driver

| driver flags word bit field definitions

dreadenable     =       0           | driver enabled for read operations
dwritenable     =       1           | driver enabled for writing
dctlenable      =       2           | driver enabled for control operations
dstatenable     =       3           | driver enabled for status operations
dneedgoodbye    =       4           | driver needs a "goodbye kiss"
dneedtime       =       5           | driver needs "main thread" time
dneedlock       =       6           | driver needs to be accessed at interrupt level

| run-time flags (in dctlflags+1)

dopened         =       5           | flag to mark driver 'open'
drambased       =       6           | 1=ram-based driver, 0=rom-based
drvractive      =       7           | flag to mark the driver active

| desk ornament message definitions

accevent        =       64          | feedevent message
accrun          =       65          | systemtask "run" message
acccursor       =       66          | cursor message for topmost window
accmenu         =       67          | menu message
acccut          =       68          | cut message from system edit
acccopy         =       69          | copy message from system edit
accpaste        =       70          | paste message from system edit
accundo         =       71          | undo message from system edit
accclear        =       72          | clear message from system edit


|  general system equates:

ioqelsize   =       50          | length of i/o parameter block

iolink      =       0           | queue link in header
iotype      =       4           | type byte for safety check
iotrap      =       6           | fs: the trap
iocmdaddr   =       8           | fs: address to dispatch to

iocompletion =      12          | long pointer to completion routine
ioresult    =       16          | io result code (=d0 if sync call)

iofilename  =       18          | file name pointer
iovrefnum   =       22          | volume refnum
iodrvnum    =       iovrefnum   | (used for eject and mountvol)
iorefnum    =       24          | reference number for i/o operation

iofiletype  =       26          | specified along with filename (byte)
iopermssn   =       27          | open: permissions (byte)

ionewname   =       28          | rename: new name pointer (4)
ioleof      =       28          | geteof,seteof: logical end-of-file (4)
ioownbuf    =       28          | open: optional ptr to locked 522-byte buffer
ionewtype   =       28          | setfiletype: new type byte (1)

iobuffer    =       32          | data buffer pointer
iobytecount =       36          | requested byte count
ioreqcount  =       36          | (equate used by file system allocate)
ionumdone   =       40          | actual byte count completed
ioactcount  =       40          | (equate used by file system allocate)

ioposmode   =       44          | initial file positioning mode, eol char
ioposoffset =       46          | file position offset (long word)

noqueuebit  =       9           | tells i/o system not to queue the request
asyntrpbit  =       10          | bit in high byte of trap specifying async

| equates specific to  getfileinfo,setfileinfo

iofqelsize  =       80          | file command parameter block byte length
                                | 1st 28 bytes are the same as the general block
iofdirindex =       28          | getfileinfo - directory index
ioflattrib  =       30          | getfileinfo - in-use bit=7, lock bit=0
ioffltype   =       31          |  file type
ioflusrwds  =       32          | get, setfileinfo - user info (16 bytes)
iofflnum    =       48          | getfileinfo - file number

ioflstblk   =       52          | getfileinfo - start file block (0000 if none)
iofllglen   =       54          |  file logical length (eof)
ioflpylen   =       58          |  file physical length in bytes.
ioflrstblk  =       62          |  start file block, resource fork (0000 if none)
ioflrlglen  =       64          |  file logical length (eof), resource fork
ioflrpylen  =       68          |  file physical length, resource fork

ioflcrdat   =       72          |  file creation date & time (32 bits in seconds)
ioflmddat   =       76          |  last modification date & time (32 bits in seconds)

| equates specific to getvolinfo,getvolume,setvolume,mountvol,unmountvol,eject

iovqelsize  =       64          | volume command parameter block byte length
iovdrvnum   =       iovrefnum   | 1st 28 bytes are the same as the general block
iovnptr     =       iofilename
iovolindex  =       28          | getvol: volume index number

iovcrdate   =       30          | getvolinfo: creation date & time
iovlsbkup   =       34          | getvolinfo: last backup date & time
iovatrb     =       38          | getvolinfo: volume attributes
iovnmfls    =       40          | getvolinfo: # files in directory
iovdirst    =       42          | getvolinfo: start block of file dir
iovblln     =       44          | getvolinfo: length of dir in blocks
iovnmalblks =       46          | getvolinfo: num blks (of alloc size) this dev
iovalblksiz =       48          | getvolinfo: alloc blk byte size
iovclpsiz   =       52          | getvolinfo: bytes to try to allocate at a time
ioalblst    =       56          | starting diskette (512-byte) block in block map
iovnxtfnum  =       58          | getvolinfo: next free file number
iovfrblk    =       62          | getvolinfo: word of # free alloc blks for this vol

| i/o command equates for i/o queue elements (match trap numbers)

ardcmd      =       2           | read command
awrcmd      =       3           | write command
actlcmd     =       4           | control command
astscmd     =       5           | status command

| special offsets for the control and status core routines

cscode      =      26           | a word for the control/status code
csparam     =      28           | operation-defined parameters (20-bytes max)

| system event definitions

nullevt         =       0           | event 0 is the null event
mbutdwnevt      =       1           | mouse button down is event 1
mbutupevt       =       2           | mouse button up is event 2
keydwnevt       =       3           | key down is event 3
keyupevt        =       4           | key up is event 4
autokeyevt      =       5           | auto-repeated key is event 5
updatevt        =       6           | update event
diskinsertevt   =       7           | disk-inserted event
activateevt     =       8           | activate/deactive event
abortevt        =       9           | abort?
networkevt      =       10          | network event
reserveevt      =       10          | reserved for system use
iodrvrevt       =       11          | driver-defined event

app1evt         =       12          | application defined events
app2evt         =       13
app3evt         =       14
app4evt         =       15

| event record definition

evtmax          =       30          | maximum number of events in buffer
evtblksize      =       16          | size in bytes of the event record
evtqblksize     =       evtblksize+6| size of event record counting queue info

evtnum          =       0           | word containing id number of event
evtmessage      =       2           | longword containing event-defined msg
evtticks        =       6           | longword holding ticks when event occured
evtmouse        =       10          | longword holding mouse pos when event occured
evtmeta         =       14          | byte containing meta key flags
evtmbut         =       15          | byte indicating state of mouse button

| system device constants

dskrfn      =       /fffb   | disk's reference number

| control call codes

ejectcode   =       7       | control call eject code
killcode    =       1       | killio code

| status call codes

drvstscode  =       8       | status call code for drive status

| resource manager equates

| high bits of the locn long-word--the resource attributes:
| (code optimization used frequently is a tstb a2@(rattr) instead
| of btst #ressysref, a2@(rattr).  this saves a lot of bytes
| in rmgr)

ressysref       =       7               | reference to system/local reference
ressysheap      =       6               | in system/in application heap
respurgeable    =       5               | purgeable/not purgeable
reslocked       =       4               | locked/not locked
resprotected    =       3               | protected/not protected
respreload      =       2               | read in on openresource?
reschanged      =       1               | existing resource changed since last update
resuser         =       0               | user defined resource attribute.

| when reading/writing, clear reschanged bit.

rcbmask         =       /fd             | reschanged, byte mask and.

| mattr map attributes and masks

mapreadonly     =       7               | is this file read-only?
mapcompact      =       6               | is a compact necessary?
mapchanged      =       5               | is it necessary to write map?

mccmask         =       /60             | mapcompact + mapchanged
mchmask         =       /20             | mapchanged
mcomask         =       /40             | mapcompact



