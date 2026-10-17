|file toolequ.text
|--------------------------------------------------------------------------------
|
|  user interface toolbox equate file
|
|     this file contains global variable and data structure
|  definitions for the macintosh user interface toolbox.
|  it is included with all toolbox source files and
|  possibly with some application programs.
|
|  written by andy hertzfeld    may 5, 1982
|
|  modification history:
|
|     ajh  23-may-82    added window stuff
|     ajh  22-jun-82    added icon stuff
|     ajh  27-jun-82    added menu stuff
|     ajh  31-jul-82    added control stuff| changed menu data structures
|     ajh  10-aug-82    got rid of scrapwindow
|     ajh  29-aug-82    added dragpattern,ishapehook to globals, dragmsg to controls
|     ajh  06-sep-82    added growwindow message
|     ajh  12-sep-82    got rid of alert and dialog window| added oneone, minusone
|     ajh  14-sep-82    changed for new font manager
|     ajh  20-sep-82    added "goaway" boolean to windows| added symbolptr globals
|     blh  24-sep-82    added alert, dialog globals and data structures
|     ajh  05-oct-82    added contrlaction field to control data structure
|     blh   7-oct-82    added indirect globals--took main globals out of low mem
|     ajh  12-oct-82    switched over to new font manager globals| grafport now 64 bytes
|     ajh  16-nov-82    added tasklock for desk manager
|     ajh  21-nov-82    lglobals,gportsize change for rom 1.8
|     blh  17-dec-82    changed dialog, resource constants and globals
|     blh  19-dec-82    changed names for resource upheaval
|     ajh  19-dec-82    added curactive,curdeactive,deskhook globals
|     ajh  24-dec-82    made defprocs defhandles, other data structure changes
|     ajh  28-dec-82    broke off resource type and id defs into separate file
|     ajh  05-jan-83    moved global base to /e80| put dialog stuff at end
|     ajh  24-jan-83    replaced miconlist with mbarenable
|     blh   5-feb-83    removed alertwindow, dialogwindow globals.  changed
|                       dialog data structure defs.
|     ajh   5-feb-83    added microsoft bytes, other changes
|     ajh   6-feb-83    globals for scaling font manager
|     ajh  12-feb-83    got rid of checkflag, made curdragaction 4 bytes long
|     ajh   6-mar-83    added gotstrike field for fontmgr (used byte from defsize)
|     ajh   7-mar-83    added scaledisable
|     ajh   5-apr-83    added ghostwindow field in microsoft globals
|     blh  20-apr-83    changed alert and dialog templates. removed alertkind.
|     sc   20-apr-83    added journal stuff at end
|     sc   09-may-83    added text edit defs, changed dialogs
|     ajh  10-may-83    added closeornhook, fpaddress
|     ajh  25-may-83    added apppacks table
|     ajh  31-may-83    changed size of grafport for new quickdraw, added sysresname
|     ajh  02-jun-83    changed picscrap to thescrap
|     ajh  08-jun-83    added appparmhandle
|     ajh  14-jun-83    added dserrcode
|     sc   23-jun-83    removed systemkind (it's now any negative number)
|     ajh  05-jul-83    made it match the documentation
|     sc   05-aug-83    new te record
|     ajh  05-aug-83    added wgiconmsg to window defproc messages
|     blh  06-aug-83    added reserrproc at end of tool globals.  made lastmap
|                       a temporary filler.
|     sc   15-aug-83    dropped scrapinfo, thescrap
|     ajh  28-aug-83    changed applemark, checkmark to final values
|     ajh  29-aug-83    added "trackctl" message
|     blh   6-sep-83    added resreadonly global, replacing rfiller.
|     sc    9-sep-83    added tewdbreak for european stuff.
|     ajh  12-sep-83    added curdekind
|
|-----------------------------------------------------------------------------------

|
| important constants
|
screenwidth     =       512
screenheight    =       342
screenradius    =       /00100010       |rounded corners for desk area
gportsize       =       108             |a grafport is 108 bytes
|

|------------------------------------------------------------
|
| toolbox global variables (kept in low memory)
|
|   this section contains global variables for the font manager,
| the window manager and the menu manager
|
|------------------------------------------------------------

toolgbase       =          /980  |toolvars     |base address of toolbox globals
lglobals        =          0            |quickdraw globals accessed a5@(0)

|----------------------------
|
| font manager globals
|
|----------------------------
|
romfont0       =         toolgbase      |handle to rom-based system font
apfontid       =         romfont0+4     |family id of standard application font
gotstrike      =         apfontid+2     |boolean indicating if we've got the strike
fmdefaultsize  =         gotstrike+1    |byte holding default size

curfminput     =         fmdefaultsize+1 |quickdraw fminput record
curfmfamily    =         curfminput     |current font family
curfmsize      =         curfmfamily+2  |current font size
curfmface      =         curfmsize+2    |current font face
curfmneedbits  =         curfmface+1    |boolean specifying whether it needs strike
curfmdevice    =         curfmneedbits+1 |current font device
curfmnumer     =         curfmdevice+2  |current numerator of scale factor
curfmdenom     =         curfmnumer+4   |current denominator of scale factor

fmgroutrec     =         curfmdenom+4   |quickdraw fontoutput record
fouterror      =         fmgroutrec     |error code
foutfonthandle =         fouterror+2    |handle to font bits
foutbold       =         foutfonthandle+4 |bolding factor
foutitalic     =         foutbold+1     |italic factor
foutuloffset   =         foutitalic+1   |underline offset
foutulshadow   =         foutuloffset+1 |underline halo
foutulthick    =         foutulshadow+1 |underline thickness
foutshadow     =         foutulthick+1  |shadow factor
foutextra      =         foutshadow+1   |extra horizontal width
foutascent     =         foutextra+1    |height above baseline
foutdescent    =         foutascent+1   |height below baseline
foutwidmax     =         foutdescent+1  |maximum width of character
foutleading    =         foutwidmax+1   |space between lines
foutunused     =         foutleading+1  |unused byte (must have even number)
foutnumer      =         foutunused+1   |point for numerators of scale factor
foutdenom      =         foutnumer+4    |point for denominators of scale factor

fmdotsperinch  =         foutdenom+4     |h,v dotsperinch of current device
fmstyletab     =         fmdotsperinch+4 |style heuristic table supplied by device
toolscratch    =         fmstyletab+24   |8 byte scratch area

|----------------------------
|
| window manager globals
|
|----------------------------
|
windowlist      =       toolscratch+8   |z-ordered list of windows (nearest first)
saveupdate      =       windowlist+4    |boolean enabling update accumulation
paintwhite      =       saveupdate+2    |boolean enabling erasing newly drawn windows
wmgrport        =       paintwhite+2    |grafport used by window manager
deskport        =       wmgrport+4      |wide open port for use by applications
|
oldstructure    =       deskport+4      |handle to saved structure region
oldcontent      =       oldstructure+4  |handle to saved content region
grayrgn         =       oldcontent+4    |handle of rounded gray desk region
savevisrgn      =       grayrgn+4       |handle of temporarily saved vis region
draghook        =       savevisrgn+4    |proc ptr to allow user control during dragging
temprect        =       draghook+4      |rectangle used as scratch by toolbox
oneone          =       temprect+8      |long constant /00010001
minusone        =       oneone+4        |long constant /ffffffff

|-----------------------------------------------
|
| general purpose bitmap for plotting bitmaps
|
|-----------------------------------------------

iconbitmap      =       minusone+8      |bitmap used for plotting things

|----------------------------
|
| menu manager globals
|
|----------------------------

menulist        =       iconbitmap+14   |pointer to current menubar list structure
mbarenable      =       menulist+4      |word for enabling menubar for deskorns
curdekind       =       mbarenable+2    |kind of deactivated window
menuflash       =       curdekind+2     |integer holding flash feedback count
themenu         =       menuflash+2     |integer holding id of hilited menu
savedhandle     =       themenu+2       |handle of saved menubits
mrmachook       =       savedhandle+4   |hook for mr. macintosh
menuhook        =       mrmachook+4     |hook for user control during menuselect

|  control manager globals

dragpattern     =       menuhook+4      |pattern dragthergn uses
deskpattern     =       dragpattern+8   |pattern to paint the desk with
dragflag        =       deskpattern+8   |implicit parameter to dragcontrol, 1 word
curdragaction   =       dragflag+2      |implicit actionproc parameter

|  floating point package globals

fpstate         =       curdragaction+4  |6 bytes of floating point state


|-----------------------------
|
| resource manager globals
|
|-----------------------------

topmaphndl      =       fpstate+6       |handle to topmost map of list
sysmaphndl      =       topmaphndl+4    |handle to system map
sysmap          =       sysmaphndl+4    |reference number of system map
curmap          =       sysmap+2        |reference number of current map
resreadonly     =       curmap+2        |readonly flag
resload         =       resreadonly+2   |auto-load feature
reserr          =       resload+2       |resource error code


|-------------------------------
|
|  misc toolbox globals
|
|-------------------------------

tasklock        =       reserr+2        |flag to prevent re-entering systemtask
fscaledisable   =       tasklock+1      |flag to disable font scaling
curactivate     =       fscaledisable+1 |window to receive activate event
curdeactive     =       curactivate+4   |window to receive deactivate event
deskhook        =       curdeactive+4   |hook for painting the desk

tedotext        =       deskhook+4      |text edit private hook
terecal         =       tedotext+4      | ""

microsoft       =       terecal+4       |12 bytes for our friends in seattle
ghostwindow     =       microsoft+12    |window that frontwindow ignores
closeornhook    =       ghostwindow+4   |hook for closing desk ornaments

|--------------------------------------------------------------
|
|    alert/dialog manager globals
|
|--------------------------------------------------------------

restproc        =       closeornhook+4  |address of restart failsafe procedure
saveproc        =       restproc+4      |address of save failsafe procedure
savesp          =       saveproc+4      |safe sp for restart or save
anumber         =       savesp+4        |which alert is active?
acount          =       anumber+2       |how many times this alert called?
dabeeper        =       acount+2        |beep routine
dastrings       =       dabeeper+4      |string parameters for substutution


|--------------------------------------------------------------
|
|    textedit globals
|
|--------------------------------------------------------------

tescrplength    =       dastrings+16
tescrphandle    =       tescrplength+4

| application package dispatch table -- 8 longwords

apppacks        =       tescrphandle+4  |handle to math code

| misc stuff (last minute additions)

sysresname      =       apppacks+32     |name of system.rsrc
appparmhandle   =       sysresname+20   |handle to hold application parameters
dserrcode       =       appparmhandle+4 |last deep shit alert id
reserrproc      =       dserrcode+2     |resource error proc.
tewdbreak       =       reserrproc+4    |default word break routine
dlgfont         =       tewdbreak+4     |default dialog font
lasttglobal     =       dlgfont+2       |address of last global

|---------------------------------------------------
|
| toolbox data structure definitions
|
|   this section contains data structure definitions
| for the font manager, the window manager
| and the menu manager.
|
|---------------------------------------------------

|---------------------------------------
|
|   font manager data structures
|
|---------------------------------------

sysfont         =       0               |system font is font id 0
applfont        =       1               |application font is font id 1

| font manager input record offsets

fminfamily     =         0              |offset to family
fminsize       =         2              |offset to size
fminface       =         4              |offset to face
fminneedbits   =         5              |offset to needbits boolean
fmindevice     =         6              |offset to device number
fminnumer      =         8              |offset to numerator of scale
fmindenom      =         12             |offset to denominator of scale

|
| kerned strike font header data structure
|
fformat         =       0               |format of font
fminchar        =       2               |lowest character in font
fmaxchar        =       4               |highest character in font
fmaxwd          =       6               |maximum width of characters
fbbox           =       8               |left side of bounding box
fbboy           =       10              |top of bounding box
fbbdx           =       12              |width of bounding box
fbbdy           =       14              |height of bounding box
flength         =       16              |length of font
fascent         =       18              |ascent above baseline
fdescent        =       20              |descent below baseline
fleading        =       22              |space between lines
fraster         =       24              |offset to actual bits?
|
| definitions for font style bits (in standard style byte, bits 0-6)
|
boldbit         =       0
italicbit       =       1
ulinebit        =       2
outlinebit      =       3
shadowbit       =       4
condensebit     =       5
extendbit       =       6

| font manager defined control/status codes

fmgrctl1       =         8              |printer drivers support ctl/status 8

|---------------------------------------
|
|   window manager data structures
|
|---------------------------------------
|
| window kind constants
|
|                                       |system windows are negative include desk ornaments, etc.
dialogkind      =       2               |dialog windows
userkind        =       8               |this and above numbers are for user

|
| window definition procedure messages
|
wdrawmsg        =       0               |draw yourself
whitmsg         =       1               |hit test
wcalcrgnmsg     =       2               |recalculate your regions
winitmsg        =       3               |initialize yourself
wdisposemsg     =       4               |dispose any private data
wgrowmsg        =       5               |drag out grow outline
wgiconmsg       =       6               |draw the grow icon
|
| window hit-test codes
|
wnohit          =       0               |not in window at all
wincontent      =       1               |in content area of window
windrag         =       2               |in drag area of window
wingrow         =       3               |in grow area of window
wingoaway       =       4               |in go away area of window
|
| window data structure definition
|
windowport      =       0               |grafport data structure
windowkind      =       gportsize       |integer -- logical type of window
wvisible        =       windowkind+2    |boolean (1 byte) -- visible flag
whilited        =       wvisible+1      |boolean (1 byte) -- select (hilite) flag
wgoaway         =       whilited+1      |boolean (1 byte) -- has go away button
wspare          =       wgoaway+1       |spare byte reserved for future expansion
structrgn       =       wspare+1        |handle to structure region of window
contrgn         =       structrgn+4     |handle to content region of window
updatergn       =       contrgn+4       |handle to update region of window
windowdef       =       updatergn+4     |handle to window definition procedure
wdatahandle     =       windowdef+4     |handle  to window proc-defined data
wtitlehandle    =       wdatahandle+4   |handle to title string
wtitlewidth     =       wtitlehandle+4  |width in pixels of title string (integer)
wcontrollist    =       wtitlewidth+2   |handle to list of controls of this window
nextwindow      =       wcontrollist+4  |pointer to next window in z-ordered list
windowpic       =       nextwindow+4    |picture handle for updates
wrefcon         =       windowpic+4     |long integer defined by application
|
windowsize      =       wrefcon+4       |size of window data structure

|--------------------------------------------------
|
|  menu manager data structure definitions
|
|--------------------------------------------------

maxmenu         =       96              |a maximum of 16*6 menus in menubar
mlistsize       =       102             |a menu list is 102 bytes long
|
| menu definition procedure messages
|
mdrawmsg        =       0               |draw yourself
mchoosemsg      =       1               |select an item
msizemsg        =       2               |calculate your size
|
|  character definitions for menumarks and other special characters
|
nomark          =       0
applemark       =       17
checkmark       =       18

|
| menulist data structure definition -- one per menubar
|
lastmenu        =       0               |number of menus in this menulist
lastright       =       2               |coordinate of 1st free point in menubar
menuoh          =       0               |menu handle
menuleft        =       4               |coordinate of left edge of menu
|
| menuinfoblk data structure -- one per menu
|
menuid          =       0               |16 bit integer unique for each menubar
menuwidth       =       2               |16 bit integer indicating menu width
menuheigth      =       4               |16 bit integer indicating menu height
menudefhandle   =       6               |handle to menu definition proc
menuenable      =       10              |longword of enable flags, one bit/item
menudata        =       14              |menu item string
|
menublksize     =       14              |size of a menu block is 14 bytes plus
                                        |the datastring
|
| menustring data structure -- one per item
|
itemicon        =       0               |offset to icon byte
itemcmd         =       1               |offset to apple (command key) byte
itemmark        =       2               |offset to checkmark character byte
itemstyle       =       3               |offset to style byte

|------------------------------------------------------------------
|
|  control manager (button and dial) manager data structures
|
|------------------------------------------------------------------

|
| control definition procedure messages
|
drawctlmsg      =       0               |draw message
hitctlmsg       =       1               |hit test message
calcctlmsg      =       2               |calc region message
newctlmsg       =       3               |init message
dispctlmsg      =       4               |dispose any private data message
posctlmsg       =       5               |adjust indicator position message
thumbctlmsg     =       6               |calculate rectangles for thumb dragging
dragctlmsg      =       7               |custom drag message
trackctlmsg     =       8               |track yourself message
|
|  findwindow classification codes
|
indesk          =       0               |not in any window
inmenubar       =       1               |in the menu bar
insyswindow     =       2               |in a system window
incontent       =       3               |in content area of user window
indrag          =       4               |in drag area of user window
ingrow          =       5               |in grow area of user window
ingoaway        =       6               |in go away area of user window
|
inbutton        =      10               |in a push button
incheckbox      =      11               |in a checkbox button
|
inupbutton      =      20               |in up button area of a dial
indownbutton    =      21               |in down button area of a dial
inpageup        =      22               |in page up (gray) area of a dial
inpagedown      =      23               |in page down (gray) area of a dial
|
inthumb         =      129              |in thumb area of a dial
|

nextcontrol     =       0               |handle to next control in the list
contrlowner     =       nextcontrol+4    |windowptr to owning window
contrlrect      =       contrlowner+4   |bounding rectangle of control
contrlvis       =       contrlrect+8    |one byte boolean indicating visible state
contrlhilite    =       contrlvis+1     |one byte field indicating hilite state
contrlvalue     =       contrlhilite+1  |integer holding current value of control
contrlmin       =       contrlvalue+2   |integer holding minimum value of control
contrlmax       =       contrlmin+2     |integer holding maximum value of control
contrldefhandle =       contrlmax+2     |handle to control definition procedure
contrldata      =       contrldefhandle+4 |pointer to data handle for definition proc
contrlaction    =       contrldata+4    |pointer to local actionproc
contrlrfcon     =       contrlaction+4  |longword refcon defined by application
contrltitle     =       contrlrfcon+4   |handle to title string (or string itself)

contrlsize      =       contrltitle     |size of control data structure - title


|------------------------------------------------
|
| alert box manager data structure definitions
|
|------------------------------------------------

|dialog record

dbounds         =       0               |dialog bounds rectangle
dvisible        =       dbounds+8       |visible flag
dwindproc       =       dvisible+2      |window proc id
dgoaway         =       dwindproc+2     |go away flag
drefcon         =       dgoaway+2       |reference constant
ditems          =       drefcon+4       |item list id and handle
dtitle          =       ditems+2        |dialog window title

|alert record

abounds         =       0               |alert box height and width
aitems          =       abounds+8       |item list id
astages         =       aitems+2        |stages word

|window subclass dialogwindow

items           =       windowsize      |item list
tehandle        =       items+4         |handle to textedit object
editfield       =       tehandle+4      |current field being edited
editopen        =       editfield+2     |is editting open?
adefitem        =       editopen+2      |default item for alerts
dwindlen        =       adefitem+2      |dialog window length

|stage definition--packed 2 to a byte, 4 stages in a word

volbits         =       3               |low 2 bits are volume (0..3)
albit           =       4               |alert bit (put up box this time?)
okdismissal     =       8               |bit for ok/cancel default in each stage
                                        |(not necessarily different for each
                                        | stage, but saves a byte)

|------------------------------------------------
|
| dialoglist data structure definitions
|
|------------------------------------------------

dlgmaxindex     =       0               |maximum index (=items-1) stored here

|in each item

itmhndl         =       0               |handle to the item
itmrect         =       itmhndl+4       |bounding rect of item
itmtype         =       itmrect+8       |item type
itmdata         =       itmtype+1       |item string, must be even length


|------------------------------------------------------------------
|
| toolbox error codes (passed through graferror)
|
|------------------------------------------------------------------

fontdecerror    =       -64             |error during font declaration
fontnotdeclared =       -65             |font not declared
fontsuberr      =       -66             |font substitution occured
|

|------------------------------------------------------------------
|
|    equates for journal interface control call
|
|------------------------------------------------------------------

jplayctl        =       16                      | playback call
jrecordctl      =       17                      | record call
jrefnum         =       -2                      | refnum of journal
jctickcount     =       0                       | journal code for tickcount
jcgetmouse      =       1                       | journal code for getmouse
jcbutton        =       2                       | journal code for button
jcgetkeys       =       3                       | journal code for getkeys
jcevent         =       4                       | journal code for getnextevent(avail)

|------------------------------------------------------------------
|
| record for text edit object
|
|------------------------------------------------------------------

tedestrect  =           0                       | 0 destination rectangle (keep together)
teviewrect  =           tedestrect+8            | 8 view rectangle rectangle

teselrect   =           teviewrect+8            | 10 select rectangle
telinehite  =           teselrect+8             | 18 current font lineheight
teascent    =           telinehite+2            | 1a current font ascent
teselpoint  =           teascent+2              | 1c selection point(mouseloc)

teselstart  =           teselpoint+4            | 20 selection start (keep together)
teselend    =           teselstart+2            | 22 selection end

teactive    =           teselend+2              | 24 st if active

tewordbreak =           teactive+2              | 26 word break routine
teclikproc  =           tewordbreak+4           | 2a click loop routine

tecliktime  =           teclikproc+4            | 2e time of last click
teclikloc   =           tecliktime+4            | 32 location of double click

tecartime   =           teclikloc+2             | 33 time for next caret toggle
tecaron     =           tecartime+4             | 38 is caret on?
tecaract    =           tecaron+1               | 39 is caret active?
testyle     =           tecaract+1              | 3a fill style

telength    =           testyle+2               | 3c length of text below {must stay together}
tetexth     =           telength+2              | 40 text handle goes here

terecback   =           tetexth+4               | 42 set true if backgnd recal
terecline   =           terecback+2             | 44 current recal line
telftclick  =           terecline+2             | 46 st if click was to left
telftcaret  =           telftclick+1            | 47 st if caret was to left

tecronly    =           telftcaret+1            | 48 st if cr only for line breaks

tefontstuff =           tecronly+2              | 50 space for font specifier
tegrafport  =           tefontstuff+8           | 52 grafport for editting

tehihook    =           tegrafport+4            | 56 hook for hilite routine
tecarhook   =           tehihook+4              | 5a hook for hilite routine

tenlines    =           tecarhook+4             | 5e number of lines (must keep together)
telines     =           tenlines+2              | 62 line starts(array of words)

terecsize   =           telines+8               | 6a base size of a record



