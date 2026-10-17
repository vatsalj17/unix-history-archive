|file toolmacs.text
|-----------------------------------------------------
|
|  user interface toolbox assembly language interface
|
|    these macros expand into toolbox traps allowing
|  a convenient, symbolic toolbox interface for the
|  assembly lanaguage programmer.
|
|  version for rom 2.0 --     februrary 10, 1983
|  regenerated for rom 4.0 -- june 3, 1983
|  changed alertsound to errorsound
|  added insertresmenu, opendeskacc -- june 13, 1983
|  added uniqueid, closedeskacc,systemedit,lbin2dec,ldec2bin
|        secs2date,date2secs| removed sound traps -- july 2, 1983
|  added tegettext, tesetjust and teinsert - august 4
|  added sysbeep, drawgrow - august 6
|  added handandhand, ptrandhand - august 6
|  removed endsubresource.  changed beginsubresource to homeresfile.
|  added getfinderinfo and setfinderinfo - august 6
|  added draggrayrgn - august 11
|  removed getfinderinfo and setfinderinfo (august 18?)
|  added scrap macros
|  added resource set/getresfileattr
|  added enqueue, dequeue (19-dec-83, lak)
|  added coulddialog, freedialog, sizersrc (12-jan-84, lak)
|
|-----------------------------------------------------


__initfont = /a8fe

__getfname = /a8ff

__getfnum = /a900

__fmswapfo = /a901

__realfont = /a902

__setfontl = /a903

__drawgrow = /a904

__draggrayrgn = /a905

__newstrin = /a906

__setstrin = /a907

__showhide = /a908

__calcvis = /a909

__calcvbehind = /a90a

__clipabov = /a90b

__paintone = /a90c

__paintbeh = /a90d

__saveold = /a90e

__drawnew = /a90f

__getwmgrp = /a910

__checkupd = /a911

__initwind = /a912

__newwindo = /a913

__disposwindow = /a914

__showwind = /a915

__hidewind = /a916

__getwrefc = /a917

__setwrefc = /a918

__getwtitl = /a919

__setwtitl = /a91a

__movewind = /a91b

__hilitewi = /a91c

__sizewind = /a91d

__trackgoa = /a91e

__selectwi = /a91f

__bringtof = /a920

__sendbehi = /a921

__beginupd = /a922

__endupdat = /a923

__frontwin = /a924

__dragwind = /a925

__dragther = /a926

__invalrgn = /a927

__invalrec = /a928

__validrgn = /a929

__validrec = /a92a

__growwind = /a92b

__findwind = /a92c

__closewin = /a92d

__setwindo = /a92e

__getwindo = /a92f

__initmenu = /a930

__newmenu = /a931

__disposmenu = /a932

__appendme = /a933

__clearmen = /a934

__insertme = /a935

__deleteme = /a936

__drawmenu = /a937

__hiliteme = /a938

__enableit = /a939

__disablei = /a93a

__getmenub = /a93b

__setmenub = /a93c

__menusele = /a93d

__menukey = /a93e

__getitmicon = /a93f

__setitmicon = /a940

__getitmstyle = /a941

__setitmstyle = /a942

__getitmmark = /a943

__setitmmark = /a944

__checkite = /a945

__getitem = /a946

__setitem = /a947

__calcmenu = /a948

__getmhand = /a949

__setmflash = /a94a

__ploticon = /a94b

__flashmen = /a94c

__addresme = /a94d

__countmit = /a950

__insertresmenu = /a951

__pinrect = /a94e

__deltapoi = /a94f

__newcontr = /a954

__disposcontrol = /a955

__killcont = /a956

__showcont = /a957

__hidecont = /a958

__movecont = /a959

__getcrefc = /a95a

__setcrefc = /a95b

__sizecont = /a95c

__hiliteco = /a95d

__getctitl = /a95e

__setctitl = /a95f

__getctlva = /a960

__getminctl = /a961

__getmaxctl = /a962

__setctlva = /a963

__setminctl = /a964

__setmaxctl = /a965

__testcont = /a966

__dragcont = /a967

__trackcon = /a968

__drawcont = /a969

__getctlac = /a96a

__setctlac = /a96b

__findcont = /a96c

__dequeue = /a96e

__enqueue = /a96f

__getnexte = /a970

__eventava = /a971

__getmouse = /a972

__stilldow = /a973

__button = /a974

__tickcoun = /a975

__getkeys = /a976

__waitmous = /a977

__coulddialog = /a979

__freedialog = /a97a

__initdial = /a97b

__getnewdi = /a97c

__newdialo = /a97d

__selitext = /a97e

__isdialog = /a97f

__dialogse = /a980

__drawdial = /a981

__closedia = /a982

__disposdi = /a983

__alert = /a985

__stopaler = /a986

__notealer = /a987

__cautiona = /a988

__couldale = /a989

__freealer = /a98a

__paramtex = /a98b

__errorsou = /a98c

__getditem = /a98d

__setditem = /a98e

__setitext = /a98f

__getitext = /a990

__modaldia = /a991

__detachre = /a992

__setrespu = /a993

__curresfi = /a994

__initreso = /a995

__rsrczone = /a996

__openresf = /a997

__useresfi = /a998

__updatere = /a999

__closeres = /a99a

__setreslo = /a99b

__countres = /a99c

__getindre = /a99d

__counttyp = /a99e

__getindty = /a99f

__getresou = /a9a0

__getnamed = /a9a1

__loadreso = /a9a2

__releaser = /a9a3

__homeresf = /a9a4

__sizersrc = /a9a5

__getresat = /a9a6

__setresat = /a9a7

__getresin = /a9a8

__setresin = /a9a9

__changedr = /a9aa

__addresou = /a9ab

__addrefer = /a9ac

__rmvereso = /a9ad

__rmverefe = /a9ae

__reserror = /a9af

__writeres = /a9b0

__createre = /a9b1

__systemev = /a9b2

__systemcl = /a9b3

__systemta = /a9b4

__systemme = /a9b5

__opendeskacc = /a9b6

__closedeskacc = /a9b7

__getpatte = /a9b8

__getcurso = /a9b9

__getstrin = /a9ba

__geticon = /a9bb

__getpictu = /a9bc

__getnewwi = /a9bd

__getnewco = /a9be

__getrmenu = /a9bf

__getnewmb = /a9c0

__uniqueid = /a9c1

__sysedit = /a9c2

__lbin2dec = /a9c4

__ldec2bin = /a9c5

__secs2date = /a9c6

__date2secs = /a9c7

__sysbeep = /a9c8

__syserror = /a9c9

__puticon = /a9ca

__tegettext = /a9cb

__teinit = /a9cc

__tedispos = /a9cd

__textbox = /a9ce

__tesettext = /a9cf

__tecaltex = /a9d0

__tesetsel = /a9d1

__tenew = /a9d2

__teupdate = /a9d3

__teclick = /a9d4

__tecopy = /a9d5

__tecut = /a9d6

__tedelete = /a9d7

__teactiva = /a9d8

__tedeacti = /a9d9

__teidle = /a9da

__tepaste = /a9db

__tekey = /a9dc

__tescroll = /a9dd

__teinsert = /a9de

__tesetjust = /a9df

__munger = /a9e0

__handtoha = /a9e1

__ptrtoxha = /a9e2

__ptrtohan = /a9e3

__handandh = /a9e4

__initpack = /a9e5

__initmath = /a9e6

__pack0 = /a9e7

__pack1 = /a9e8

__pack2 = /a9e9

__pack3 = /a9ea

__fp68k = /a9eb

__pack4 = /a9eb

__pack5 = /a9ec

__pack6 = /a9ed

__pack7 = /a9ee

__ptrandha = /a9ef

__loadseg = /a9f0

__unloadse = /a9f1

__launch = /a9f2

__chain = /a9f3

__exittosh = /a9f4

__getapppa = /a9f5


__getresfil = /a9f6

__setresfil = /a9f7

__infoscra = /a9f9

__unlodesc = /a9fa

__lodescra = /a9fb

__zeroscra = /a9fc

__getscrap = /a9fd

__putscrap = /a9fe


|_______________________________________________________________________
|
| core routine system macros
|
|_______________________________________________________________________


| first we have the i/o core routine macros.  these are also used by
| the file system.

__open = /a000 + 0

__close = /a000 + 1

__read = /a000 + 2

__write = /a000 + 3

__control = /a000 + 4

__status = /a000 + 5

__killio = /a000 + 6


| other file system calls go here

__getvolinfo = /a000 + 7

__create = /a000 + 8

__delete = /a000 + 9

__openrf = /a000 + 10

__rename = /a000 + 11

__getfileinfo = /a000 + 12

__setfileinfo = /a000 + 13

__unmountvol = /a000 + 14

__mountvol = /a000 + 15

__allocate = /a000 + 16

__geteof = /a000 + 17

__seteof = /a000 + 18

__flushvol = /a000 + 19

__getvol = /a000 + 20

__setvol = /a000 + 21

__initqueue = /a000 + 22

__eject = /a000 + 23

__getfpos = /a000 + 24

| here are the storage manager core routine macros

__initzone = /a000 + 25

__getzone = /a000 + 26 + /100

__setzone = /a000 + 27

__freemem = /a000 + 28

__maxmem = /a000 + 29 + /100

__newptr = /a000 + 30 + /100

__disposptr = /a000 + 31

__setptrsize = /a000 + 32

__getptrsize = /a000 + 33

__newhandle = /a000 + 34 + /100

__disposhandle = /a000 + 35

__sethandlesize = /a000 + 36

__gethandlesize = /a000 + 37

__handlezone = /a000 + 38 + /100

__reallochandle = /a000 + 39

__recoverhandle = /a000 + 40 + /100

__hlock = /a000 + 41

__hunlock = /a000 + 42

__emptyhandle = /a000 + 43

__initapplzone = /a000 + 44

__setappllimit = /a000 + 45

__blockmove = /a000 + 46

| here are the event manager macros

__postevent = /a000 + 47

__oseventavail = /a000 + 48

__getosevent = /a000 + 49

__flushevents = /a000 + 50

| here are the utility core routine macros

__vinstall = /a000 + 51

__vremove = /a000 + 52

__offline = /a000 + 53

__moremasters = /a000 + 54

__readparam = /a000 + 55

__writeparam = /a000 + 56

__readdatetime = /a000 + 57

__setdatetime = /a000 + 58

__delay = /a000 + 59

__cmpstring = /a000 + 60

__drvrinstall = /a000 + 61

__drvrremove = /a000 + 62

__initutil = /a000 + 63

__resrvmem = /a000 + 64

__setfillock = /a000 + 65

__rstfillock = /a000 + 66

__setfiltype = /a000 + 67

__setfpos = /a000 + 68

__flushfil = /a000 + 69

__gettrapaddress = /a000 + 70 + /100

__settrapaddress = /a000 + 71

__ptrzone = /a000 + 72 + /100

__hpurge = /a000 + 73

__hnopurge = /a000 + 74

__setgrowzone = /a000 + 75

__compactmem = /a000 + 76

__purgemem = /a000 + 77

__adddrive = /a000 + 78

__rdrvrinstall = /a000 + 79

__uprstring = /a000 + 84

__setappbase = /a000 + 87

_regs_ = /100
_async_ = /400
_sys_ = /400
_immed_ = /200
_clear_ = /200
_marks_ = /200
_case_ = /400
|file quickmacs.text
|------------------------------------------------
|
|  quickdraw assembly language interface
|
|    these macros expand into toolbox traps allowing
|  a convenient, symbolic quickdraw interface for the
|  assembly language programmer.
|
|  version for rom 2.0 --      februrary 10, 1983
|  updated for rom 2.4 --      mar 16, 1983
|  regenerated for rom 4.0 --  june 3, 1983
|
|------------------------------------------------


__initcurs = /a850

__setcurso = /a851

__hidecurs = /a852

__showcurs = /a853

__shieldcu = /a855

__obscurec = /a856

__bitand = /a858

__bitxor = /a859

__bitnot = /a85a

__bitor = /a85b

__bitshift = /a85c

__bittst = /a85d

__bitset = /a85e

__bitclr = /a85f

__random = /a861

__forecolo = /a862

__backcolo = /a863

__colorbit = /a864

__getpixel = /a865

__stuffhex = /a866

__longmul = /a867

__fixmul = /a868

__fixratio = /a869

__hiword = /a86a

__loword = /a86b

__fixround = /a86c

__initport = /a86d

__initgraf = /a86e

__openport = /a86f

__localtog = /a870

__globalto = /a871

__grafdevi = /a872

__setport = /a873

__getport = /a874

__setpbits = /a875

__portsize = /a876

__moveport = /a877

__setorigi = /a878

__setclip = /a879

__getclip = /a87a

__cliprect = /a87b

__backpat = /a87c

__closepor = /a87d

__addpt = /a87e

__subpt = /a87f

__setpt = /a880

__equalpt = /a881

__stdtext = /a882

__drawchar = /a883

__drawstri = /a884

__drawtext = /a885

__textwidt = /a886

__textfont = /a887

__textface = /a888

__textmode = /a889

__textsize = /a88a

__getfonti = /a88b

__stringwi = /a88c

__charwidt = /a88d

__spaceext = /a88e

__stdline = /a890

__lineto = /a891

__line = /a892

__moveto = /a893

__move = /a894

__moov = /a894

__hidepen = /a896

__showpen = /a897

__getpenst = /a898

__setpenst = /a899

__getpen = /a89a

__pensize = /a89b

__penmode = /a89c

__penpat = /a89d

__pennorma = /a89e

__stdrect = /a8a0

__framerec = /a8a1

__paintrec = /a8a2

__eraserec = /a8a3

__inverre = /a8a4

__fillrect = /a8a5

__equalrec = /a8a6

__setrect = /a8a7

__offsetre = /a8a8

__insetrec = /a8a9

__sectrect = /a8aa

__unionrec = /a8ab

__pt2rect = /a8ac

__ptinrect = /a8ad

__emptyrec = /a8ae

__stdrrect = /a8af

__framerou = /a8b0

__paintrou = /a8b1

__eraserou = /a8b2

__inverro = /a8b3

__fillroun = /a8b4

__stdoval = /a8b6

__frameova = /a8b7

__paintova = /a8b8

__eraseova = /a8b9

__invertov = /a8ba

__filloval = /a8bb

__slopefromangle = /a8bc

__stdarc = /a8bd

__framearc = /a8be

__paintarc = /a8bf

__erasearc = /a8c0

__invertar = /a8c1

__fillarc = /a8c2

__pttoangl = /a8c3

__anglefromslope = /a8c4

__stdpoly = /a8c5

__framepol = /a8c6

__paintpol = /a8c7

__erasepol = /a8c8

__invertpo = /a8c9

__fillpoly = /a8ca

__openpoly = /a8cb

__closepgon = /a8cc

__killpoly = /a8cd

__offsetpo = /a8ce

__packbits = /a8cf

__unpackbi = /a8d0

__stdrgn = /a8d1

__framergn = /a8d2

__paintrgn = /a8d3

__erasergn = /a8d4

__inverrgn = /a8d5

__fillrgn = /a8d6

__newrgn = /a8d8

__disposrgn = /a8d9

__openrgn = /a8da

__closergn = /a8db

__copyrgn = /a8dc

__setempty = /a8dd

__setrecrgn = /a8de

__rectrgn = /a8df

__ofsetrgn = /a8e0

__insetrgn = /a8e1

__emptyrgn = /a8e2

__equalrgn = /a8e3

__sectrgn = /a8e4

__unionrgn = /a8e5

__diffrgn = /a8e6

__xorrgn = /a8e7

__ptinrgn = /a8e8

__rectinrg = /a8e9

__stdbits = /a8eb

__copybits = /a8ec

__scrollre = /a8ef

__setstdpr = /a8ea

__stdtxmea = /a8ed

__stdgetpi = /a8ee

__stdputpi = /a8f0

__stdcomme = /a8f1

__piccomme = /a8f2

__openpict = /a8f3

__closepic = /a8f4

__killpict = /a8f5

__drawpict = /a8f6

__scalept = /a8f8

__mappt = /a8f9

__maprect = /a8fa

__maprgn = /a8fb

__mappoly = /a8fc

