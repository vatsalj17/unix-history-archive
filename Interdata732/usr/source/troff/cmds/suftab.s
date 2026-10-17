* suftab - suffix table

suftab	equ	*
	dc	sufa+32
	dc	0
	dc	sufc+32
	dc	sufd+32
	dc	sufe+32
	dc	suff+32
	dc	sufg+32
	dc	sufh+32
	dc	sufi+32
	dc	0
	dc	sufk+32
	dc	sufl+32
	dc	sufm+32
	dc	sufn+32
	dc	sufo+32
	dc	sufp+32
	dc	0
	dc	sufr+32
	dc	sufs+32
	dc	suft+32
	dc	0
	dc	0
	dc	0
	dc	0
	dc	sufy+32
	dc	0

sufa	equ	*
 db 2,x'80'+c't'	/-TA
 db 2,x'80'+c's'	/-SA
 db 3,x'80'+c't',c'r'	/-TRA
 db 3,x'80'+c'd',c'r'	/-DRA
 db 3,x'80'+c'b',c'r'	/-BRA
 db 2,x'80'+c'p'	/-PA
 db 2,x'80'+c'n'	/-NA
 db 2,x'80'+c'm'	/-MA
 db 3,x'80'+c'p',c'l'	/-PLA
 db 2,x'80'+c'l'	/-LA
 db 2,x'80'+c'k'	/-KA
 db 3,x'80'+c't',c'h'	/-THA
 db 3,x'80'+c's',c'h'	/-SHA
 db 2,x'80'+c'g'	/-GA
 db 2,x'80'+c'd'	/-DA
 db 2,x'80'+c'c'	/-CA
 db 2,x'80'+c'b'	/-BA
 db 0
sufc	equ	*
 db 4,c'e',c't',x'80'+c'i'	/ET-IC
 db 7,c'a',c'l',x'80'+c'i',c's',x'80'+c't',c'i'	/AL-IS-TIC
 db 4,c's',x'80'+c't',c'i'	/S-TIC
 db 4,c'p',x'80'+c't',c'i'	/P-TIC
 db 5,x'80'+c'l',c'y',c't',x'80'+c'i'	/-LYT-IC
 db 4,c'o',c't',x'80'+c'i'	/OT-IC
 db 5,c'a',c'n',x'80'+c't',c'i'	/AN-TIC
 db 4,c'n',x'80'+c't',c'i'	/N-TIC
 db 4,c'c',x'80'+c't',c'i'	/C-TIC
 db 4,c'a',c't',x'80'+c'i'	/AT-IC
 db 4,c'h',x'80'+c'n',c'i'	/H-NIC
 db 3,c'n',x'80'+c'i'	/N-IC
 db 3,c'm',x'80'+c'i'	/M-IC
 db 4,c'l',x'80'+c'l',c'i'	/L-LIC
 db 4,c'b',x'80'+c'l',c'i'	/B-LIC
 db 4,x'80'+c'c',c'l',c'i'	/-CLIC
 db 3,c'l',x'80'+c'i'	/L-IC
 db 3,c'h',x'80'+c'i'	/H-IC
 db 3,c'f',x'80'+c'i'	/F-IC
 db 3,c'd',x'80'+c'i'	/D-IC
 db 3,x'80'+c'b',c'i'	/-BIC
 db 3,c'a',x'80'+c'i'	/A-IC
 db 3,x'80'+c'm',c'a'	/-MAC
 db 3,c'i',x'80'+c'a'	/I-AC
 db 0
sufd	equ	*
 db 4,x'80'+c'w',c'o',c'r'	/-WORD
 db 4,x'80'+c'l',c'o',c'r'	/-LORD
 db 4,x'80'+c'f',c'o',c'r'	/-FORD
 db 4,x'80'+c'y',c'a',c'r'	/-YARD
 db 4,x'80'+c'w',c'a',c'r'	/-WARD
 db 5,x'80'+c'g',c'u',c'a',c'r'	/-GUARD
 db 4,x'80'+c't',c'a',c'r'	/-TARD
 db 5,x'80'+c'b',c'o',c'a',c'r'	/-BOARD
 db 4,x'80'+c'n',c'a',c'r'	/-NARD
 db 5,x'80'+c'l',c'i',c'a',c'r'	/-LIARD
 db 4,x'80'+c'i',c'a',c'r'	/-IARD
 db 4,x'80'+c'g',c'a',c'r'	/-GARD
 db 4,x'80'+c'b',c'a',c'r'	/-BARD
 db 3,x'80'+c'r',c'o'	/-ROD
 db 4,x'80'+c'w',c'o',c'o'	/-WOOD
 db 4,x'80'+c'h',c'o',c'o'	/-HOOD
 db 4,x'80'+c'm',c'o',c'n'	/-MOND
 db 4,x'80'+c't',c'e',c'n'	/-TEND
 db 5,x'80'+c's',c't',c'a',c'n'	/-STAND
 db 4,x'80'+c'l',c'a',c'n'	/-LAND
 db 4,x'80'+c'h',c'a',c'n'	/-HAND
 db 4,x'80'+c'h',c'o',c'l'	/-HOLD
 db 4,x'80'+c'f',c'o',c'l'	/-FOLD
 db 5,x'80'+c'f',c'i',c'e',c'l'	/-FIELD
 db 3,x'80'+c'v',c'i'	/-VID
 db 3,x'80'+c'c',c'i'	/-CID
 db 4,x'80'+c's',c'a',c'i'	/-SAID
 db 4,x'80'+c'm',c'a',c'i'	/-MAID
 db 4,c't',x'80'+c't',c'e'	/T-TED
 db 3,c't',x'80'+c'e'	/T-ED
 db 4,x'80'+c'd',c'r',c'e'	/-DRED
 db 4,x'80'+c'c',c'r',c'e'	/-CRED
 db 4,x'80'+c'b',c'r',c'e'	/-BRED
 db 5,c'v',x'80'+c'e',c'l',c'e'	/V-ELED
 db x'40'+4,c'a',c'l',x'80'+c'e'	/AL/ED
 db x'60'+3,x'80'+c'e',c'e'	//EED
 db x'20'+5,c'e',c'd',x'80'+c'd',c'e'	/ED-DED
 db 4,c'd',x'80'+c'd',c'e'	/D-DED
 db x'20'+4,c'e',c'd',x'80'+c'e'	/ED-ED
 db 3,c'd',x'80'+c'e'	/D-ED
 db 5,x'80'+c'd',c'u',c'c',c'e'	/-DUCED
 db x'c0'+2,c'e'	/E/D
 db 5,x'80'+c's',c't',c'e',c'a'	/-STEAD
 db 4,x'80'+c'h',c'e',c'a'	/-HEAD
 db 0
sufe	equ	*
 db 5,c'a',c'r',x'80'+c'i',c'z'	/AR-IZE
 db 5,c'a',c'n',x'80'+c'i',c'z'	/AN-IZE
 db 5,c'a',c'l',x'80'+c'i',c'z'	/AL-IZE
 db 6,x'80'+c'a',c'r',c'd',x'80'+c'i',c'z'	/-ARD-IZE
 db 5,x'80'+c's',c'e',c'l',c'v'	/-SELVE
 db 5,x'80'+c'k',c'n',c'i',c'v'	/-KNIVE
 db 5,x'80'+c'l',c'i',c'e',c'v'	/-LIEVE
 db x'40'+3,x'80'+c'q',c'u'	//QUE
 db 7,c'o',c'n',x'80'+c't',c'i',c'n',x'80'+c'u'	/ON-TIN-UE
 db 3,x'80'+c'n',c'u'	/-NUE
 db 3,x'80'+c'd',c'u'	/-DUE
 db x'c0'+2,c'u'	/U/E
 db x'c0'+5,c'q',c'u',c'a',c't'	/ QUAT/E
 db 4,c'u',x'80'+c'a',c't'	/U-ATE
 db 5,x'80'+c's',c't',c'a',c't'	/-STATE
 db 4,x'80'+c't',c'a',c't'	/-TATE
 db 6,x'80'+c't',c'o',c'r',x'80'+c'a',c't'	/-TOR-ATE
 db 5,c'e',c'n',x'80'+c'a',c't'	/EN-ATE
 db 4,x'80'+c'm',c'a',c't'	/-MATE
 db 5,x'80'+c'h',c'o',c'u',c's'	/-HOUSE
 db 5,x'80'+c'c',c'l',c'o',c's'	/-CLOSE
 db 4,c'i',x'80'+c'o',c's'	/I-OSE
 db 4,x'80'+c'w',c'i',c's'	/-WISE
 db 5,c'a',c's',x'80'+c'u',c'r'	/AS-URE
 db x'20'+4,x'80'+c's',c'u',c'r'	/-SURE
 db 6,x'80'+c'f',c'i',c'g',x'80'+c'u',c'r'	/-FIG-URE
 db x'20'+3,x'80'+c't',c'r'	/-TRE
 db 5,x'80'+c's',c't',c'o',c'r'	/-STORE
 db 4,x'80'+c'f',c'o',c'r'	/-FORE
 db 5,x'80'+c'w',c'h',c'e',c'r'	/-WHERE
 db 6,x'80'+c's',c'p',c'h',c'e',c'r'	/-SPHERE
 db 3,x'80'+c'd',c'r'	/-DRE
 db 3,x'80'+c'c',c'r'	/-CRE
 db 3,x'80'+c'b',c'r'	/-BRE
 db 5,x'80'+c's',c'c',c'o',c'p'	/-SCOPE
 db 4,c'y',x'80'+c'o',c'n'	/Y-ONE
 db 5,x'80'+c's',c't',c'o',c'n'	/-STONE
 db 5,x'80'+c'p',c'h',c'o',c'n'	/-PHONE
 db 4,x'80'+c'g',c'o',c'n'	/-GONE
 db 4,c'e',x'80'+c'o',c'n'	/E-ONE
 db x'20'+4,x'80'+c'e',c'n',c'n'	/-ENNE
 db x'20'+5,c'a',x'80'+c'r',c'i',c'n'	/A-RINE
 db 5,x'80'+c'c',c'l',c'i',c'n'	/-CLINE
 db 4,x'80'+c'l',c'i',c'n'	/-LINE
 db 7,x'80'+c'r',c'o',c'u',x'80'+c't',c'i',c'n'	/*-ROU-TINE*/
 db 4,x'80'+c's',c'o',c'm'	/-SOME
 db 4,x'80'+c'c',c'o',c'm'	/-COME
 db 4,x'80'+c't',c'i',c'm'	/-TIME
 db 3,x'80'+c'z',c'l'	/-ZLE
 db 3,x'80'+c't',c'l'	/-TLE
 db 3,x'80'+c's',c'l'	/-SLE
 db 3,x'80'+c'p',c'l'	/-PLE
 db 5,x'80'+c'v',c'i',c'l',c'l'	/-VILLE
 db 4,c'c',c'k',x'80'+c'l'	/CK-LE
 db 3,x'80'+c'k',c'l'	/-KLE
 db 3,x'80'+c'g',c'l'	/-GLE
 db 3,x'80'+c'f',c'l'	/-FLE
 db 3,x'80'+c'd',c'l'	/-DLE
 db 3,x'80'+c'c',c'l'	/-CLE
 db 5,x'80'+c'p',c'a',x'80'+c'b',c'l'	/-PA-BLE
 db 5,c'f',c'a',x'80'+c'b',c'l'	/FA-BLE
 db 5,x'80'+c'c',c'a',x'80'+c'b',c'l'	/-CA-BLE
 db 6,x'80'+c's',c't',c'a',c'b',c'l'	/-STABLE
 db 4,x'80'+c'a',c'b',c'l'	/-ABLE
 db 3,x'80'+c'b',c'l'	/-BLE
 db 4,x'80'+c'd',c'a',c'l'	/-DALE
 db 4,x'80'+c'm',c'a',c'l'	/-MALE
 db 4,x'80'+c's',c'a',c'l'	/-SALE
 db 4,x'80'+c'l',c'i',c'k'	/-LIKE
 db x'e0'+5,c'g',x'80'+c'u',c'a',c'g'	/-G/UAGE
 db 5,x'80'+c'r',c'i',c'a',c'g'	/-RIAGE
 db 5,c'e',c'r',x'80'+c'a',c'g'	/ER-AGE
 db 4,c'm',x'80'+c'a',c'g'	/M-AGE
 db 4,c'k',x'80'+c'a',c'g'	/K-AGE
 db 4,c'd',x'80'+c'a',c'g'	/D-AGE
 db 4,x'80'+c'w',c'i',c'f'	/-WIFE
 db 5,x'80'+c'k',c'n',c'i',c'f'	/-KNYFE
 db 3,x'80'+c's',c'e'	/-SEE
 db 4,x'80'+c'f',c'r',c'e'	/-FREE
 db x'e0'+2,c'e'	/EE
 db 4,x'80'+c'w',c'i',c'd'	/-WIDE
 db 4,x'80'+c't',c'i',c'd'	/-TIDE
 db 4,x'80'+c's',c'i',c'd'	/-SIDE
 db 6,x'80'+c'q',c'u',c'e',c'n',c'c'	/-QUENCE
 db 7,x'80'+c'f',c'l',c'u',x'80'+c'e',c'n',c'c'	/-FLU-ENCE
 db x'20'+6,c'e',c's',x'80'+c'e',c'n',c'c'	/ES-ENCE
 db 6,c'e',c'r',x'80'+c'e',c'n',c'c'	/ER-ENCE
 db 5,c'i',x'80'+c'e',c'n',c'c'	/I-ENCE
 db x'20'+5,x'80'+c's',c'a',c'n',c'c'	/-SANCE
 db 6,c'e',c'r',x'80'+c'a',c'n',c'c'	/ER-ANCE
 db 6,c'a',c'r',x'80'+c'a',c'n',c'c'	/AR-ANCE
 db 5,x'80'+c'n',c'a',c'n',c'c'	/-NANCE
 db 7,x'80'+c'b',c'a',c'l',x'80'+c'a',c'n',c'c'	/-BAL-ANCE
 db 5,c'i',x'80'+c'a',c'n',c'c'	/I-ANCE
 db 7,x'80'+c'j',c'u',c's',x'80'+c't',c'i',c'c'	/-JUS-TICE
 db 5,x'80'+c's',c't',c'i',c'c'	/-STICE
 db 5,x'80'+c'p',c'i',c'e',c'c'	/-PIECE
 db 5,x'80'+c'p',c'l',c'a',c'c'	/-PLACE
 db x'e0'+1	//E
 db 0
suff	equ	*
 db 5,x'80'+c'p',c'r',c'o',c'o'	/-PROOF
 db 4,x'80'+c's',c'e',c'l'	/-SELF
 db 3,x'80'+c'r',c'i'	/-RIF
 db x'20'+4,x'80'+c'l',c'i',c'e'	/-LIEF
 db 0
sufg	equ	*
 db 3,x'80'+c'l',c'o'	/-LOG
 db 4,x'80'+c'l',c'o',c'n'	/-LONG
 db 5,c't',x'80'+c't',c'i',c'n'	/T-TING
 db 6,x'80'+c's',c't',c'r',c'i',c'n'	/ -STRING
 db 5,c'r',x'80'+c'r',c'i',c'n'	/R-RING
 db 5,c'p',x'80'+c'p',c'i',c'n'	/P-PING
 db 5,c'n',x'80'+c'n',c'i',c'n'	/N-NING
 db 5,c'm',x'80'+c'm',c'i',c'n'	/M-MING
 db 5,c'l',x'80'+c'l',c'i',c'n'	/ L-LING
 db 5,x'80'+c'z',c'l',c'i',c'n'	/-ZLING
 db 5,x'80'+c't',c'l',c'i',c'n'	/-TLING
 db x'20'+5,c's',x'80'+c'l',c'i',c'n'	/S-LING
 db 5,c'r',x'80'+c'l',c'i',c'n'	/R-LING
 db 5,x'80'+c'p',c'l',c'i',c'n'	/-PLING
 db 6,c'n',x'80'+c'k',c'l',c'i',c'n'	/N-KLING
 db 5,c'k',x'80'+c'l',c'i',c'n'	/K-LING
 db 5,x'80'+c'g',c'l',c'i',c'n'	/-GLING
 db 5,x'80'+c'f',c'l',c'i',c'n'	/-FLING
 db 5,x'80'+c'd',c'l',c'i',c'n'	/-DLING
 db 5,x'80'+c'c',c'l',c'i',c'n'	/-CLING
 db 5,x'80'+c'b',c'l',c'i',c'n'	/-BLING
 db 6,c'y',x'80'+c't',c'h',c'i',c'n'	/Y-THING
 db 7,c'e',c'e',c't',c'h',x'80'+c'i',c'n'	/EETH-ING
 db 6,c'e',x'80'+c't',c'h',c'i',c'n'	/E-THING
 db 5,c'g',x'80'+c'g',c'i',c'n'	/G-GING
 db 5,c'd',x'80'+c'd',c'i',c'n'	/D-DING
 db 5,c'b',x'80'+c'b',c'i',c'n'	/B-BING
 db 3,x'80'+c'i',c'n'	/-ING
 db 0
sufh	equ	*
 db 5,x'80'+c'm',c'o',c'u',c't'	/-MOUTH
 db 5,x'80'+c'w',c'o',c'r',c't'	/-WORTH
 db 4,x'80'+c'w',c'i',c't'	/-WITH
 db 5,c't',x'80'+c't',c'i',c's'	/T-TISH
 db 5,c'e',x'80'+c't',c'i',c's'	/E-TISH
 db 5,c'p',x'80'+c'p',c'i',c's'	/P-PISH
 db 5,c'r',x'80'+c'n',c'i',c's'	/R-NISH
 db 5,c'n',x'80'+c'n',c'i',c's'	/N-NISH
 db 5,x'80'+c'p',c'l',c'i',c's'	/-PLISH
 db 5,x'80'+c'g',c'u',c'i',c's'	/ -GUISH
 db 5,x'80'+c'g',c'l',c'i',c's'	/ -GLISH
 db 5,c'b',x'80'+c'l',c'i',c's'	/ B-LISH
 db 5,c'g',x'80'+c'g',c'i',c's'	/G-GISH
 db 5,c'd',x'80'+c'd',c'i',c's'	/D-DISH
 db 3,x'80'+c'i',c's'	/-ISH
 db 5,x'80'+c'g',c'r',c'a',c'p'	/-GRAPH
 db 7,x'80'+c'b',c'o',c'r',x'80'+c'o',c'u',c'g'	/-BOR-OUGH
 db 5,x'80'+c'b',c'u',c'r',c'g'	/-BURGH
 db 4,x'80'+c'v',c'i',c'c'	/-VICH
 db 3,x'80'+c'n',c'a'	/-NAH
 db 3,x'80'+c'l',c'a'	/-LAH
 db 4,x'80'+c'm',c'i',x'80'+c'a'	/-MI-AH
 db 0
sufi	equ	*
 db 3,x'80'+c't',c'r'	/-TRI
 db 3,x'80'+c'c',c'h'	/-CHI
 db x'80'+3,c'i',c'f'	/IF-I
 db x'80'+3,c'e',c'd'	/ED-I
 db 5,x'80'+c'a',c's',c'c',c'i'	/-ASCII
 db 0
sufk	equ	*
 db 4,x'80'+c'w',c'o',c'r'	/-WORK
 db 4,x'80'+c'm',c'a',c'r'	/-MARK
 db 4,x'80'+c'b',c'o',c'o'	/-BOOK
 db 4,x'80'+c'w',c'a',c'l'	/-WALK
 db 5,x'80'+c'c',c'r',c'a',c'c'	/-CRACK
 db 4,x'80'+c'b',c'a',c'c'	/-BACK
 db 0
sufl	equ	*
 db 3,x'80'+c'f',c'u'	/-FUL
 db 5,c's',x'80'+c'w',c'e',c'l'	/S-WELL
 db 4,x'80'+c't',c'e',c'l'	/-TELL
 db 5,x'80'+c's',c'h',c'e',c'l'	/-SHELL
 db 5,x'80'+c's',c't',c'a',c'l'	/-STALL
 db 4,x'80'+c's',c't',c'a'	/-STAL
 db 4,x'80'+c'b',c'a',c'l'	/-BALL
 db 3,c'v',x'80'+c'e'	/V-EL
 db 3,c'u',x'80'+c'e'	/U-EL
 db 3,c'k',x'80'+c'e'	/K-EL
 db 4,c't',c'h',x'80'+c'e'	/TH-EL
 db 5,c't',c'c',c'h',x'80'+c'e'	/TCH-EL
 db 3,c'a',x'80'+c'e'	/A-EL
 db x'60'+4,x'80'+c'q',c'u',c'a'	//QUAL
 db x'20'+3,c'u',x'80'+c'a'	/U-AL
 db 3,x'80'+c't',c'a'	/-TAL
 db 4,c'u',c'r',x'80'+c'a'	/UR-AL
 db x'20'+5,c'g',x'80'+c'o',x'80'+c'n',c'a'	/G-O-NAL
 db 4,c'o',c'n',x'80'+c'a'	/ON-AL
 db 3,x'80'+c'n',c'a'	/-NAL
 db 4,x'80'+c't',c'i',c'a'	/-TIAL
 db 4,x'80'+c's',c'i',c'a'	/-SIAL
 db x'20'+5,x'80'+c't',c'r',c'i',x'80'+c'a'	/-TRI-AL
 db 4,c'r',c'i',x'80'+c'a'	/RI-AL
 db 4,x'80'+c'n',c'i',x'80'+c'a'	/-NI-AL
 db 4,x'80'+c'd',c'i',x'80'+c'a'	/-DI-AL
 db 4,x'80'+c'c',c'i',c'a'	/-CIAL
 db 3,x'80'+c'g',c'a'	/-GAL
 db 4,x'80'+c'm',c'e',c'a'	/-MEAL
* db x'20'+4,x'80'+c'r',c'e',x'80'+c'a'	/-RE-AL
 db x'20'+4,x'80'+c'r',c'e',c'a'	/-REAL
 db 6,c'c',x'80'+c't',c'i',x'80'+c'c',c'a'	/C-TI-CAL
 db 5,x'80'+c's',c'i',x'80'+c'c',c'a'	/-SI-CAL
 db 4,x'80'+c'i',x'80'+c'c',c'a'	/-I-CAL
 db 3,x'80'+c'c',c'a'	/-CAL
 db 3,x'80'+c'b',c'a'	/-BAL
 db 6,x'80'+c'n',c'o',x'80'+c'm',c'i',x'80'+c'a'	/-NO-MI-AL
 db 0
sufm	equ	*
 db 3,x'80'+c'n',c'u'	/-NUM
 db 5,c'o',x'80'+c'r',c'i',x'80'+c'u'	/O-RI-UM
 db x'20'+3,c'i',x'80'+c'u'	/I-UM
 db x'20'+3,c'e',x'80'+c'u'	/E-UM
 db 5,c'i',c'v',x'80'+c'i',c's'	/IV-ISM
 db 4,x'80'+c't',c'i',c's'	/-TISM
 db 5,c'i',x'80'+c'm',c'i',c's'	/I-MISM
 db 5,c'a',c'l',x'80'+c'i',c's'	/AL-ISM
 db x'20'+4,c'e',x'80'+c'i',c's'	/E-ISM
 db x'20'+4,c'a',x'80'+c'i',c's'	/A-ISM
 db 4,x'80'+c'r',c'o',c'o'	/-ROOM
 db 3,x'80'+c'd',c'o'	/-DOM
 db 3,x'80'+c'h',c'a'	/-HAM
 db 6,x'80'+c'a',x'80'+c'r',c'i',c't',c'h'	/-A-RITHM
 db 5,x'80'+c'r',c'i',c't',c'h'	/-RITHM
 db 0
sufn	equ	*
 db 4,x'80'+c't',c'o',c'w'	/-TOWN
 db 4,x'80'+c'd',c'o',c'w'	/-DOWN
 db 4,x'80'+c't',c'u',c'r'	/-TURN
 db 5,x'80'+c's',c'p',c'o',c'o'	/-SPOON
 db 4,x'80'+c'n',c'o',c'o'	/-NOON
 db 4,x'80'+c'm',c'o',c'o'	/-MOON
 db 9,c'a',c'l',x'80'+c'i',x'80'+c'z',c'a',x'80'+c't',c'i',c'o'	/AL-I-ZA-TION
 db 7,x'80'+c'i',x'80'+c'z',c'a',x'80'+c't',c'i',c'o'	/-I-ZA-TION
 db 7,c'l',x'80'+c'i',x'80'+c'a',x'80'+c't',c'i',c'o'	/L-I-A-TION
 db 4,x'80'+c't',c'i',c'o'	/-TION
 db x'20'+5,c's',x'80'+c's',c'i',c'o'	/S-SION
 db 4,x'80'+c's',c'i',c'o'	/-SION
 db 4,c'n',x'80'+c'i',c'o'	/N-ION
 db 4,x'80'+c'g',c'i',c'o'	/-GION
 db 4,x'80'+c'c',c'i',c'o'	/-CION
 db 3,x'80'+c'c',c'o'	/-CON
 db 3,x'80'+c't',c'o'	/-TON
 db 3,x'80'+c's',c'o'	/-SON
 db 3,x'80'+c'r',c'i'	/-RIN
 db 3,x'80'+c'p',c'i'	/-PIN
 db 3,x'80'+c'n',c'i'	/-NIN
 db 3,x'80'+c'm',c'i'	/-MIN
 db 3,x'80'+c'l',c'i'	/-LIN
 db 3,x'80'+c'k',c'i'	/-KIN
 db 5,x'80'+c's',c't',c'e',c'i'	/-STEIN
 db 4,x'80'+c't',c'a',c'i'	/-TAIN
 db 5,c'g',c'h',c't',x'80'+c'e'	/GHT-EN
 db 5,x'80'+c'w',c'o',c'm',x'80'+c'e'	/-WOM-EN
 db 3,x'80'+c'm',c'e'	/-MEN
 db 4,c'o',x'80'+c'k',c'e'	/O-KEN
 db 3,c'k',x'80'+c'e'	/K-EN
 db 4,x'80'+c't',c'e',c'e'	/-TEEN
 db 4,x'80'+c's',c'e',c'e'	/-SEEN
 db x'20'+3,x'80'+c's',c'a'	/-SAN
 db 5,x'80'+c'w',c'o',c'm',x'80'+c'a'	/-WOM-AN
 db 3,x'80'+c'm',c'a'	/-MAN
 db 4,x'80'+c't',c'i',c'a'	/-TIAN
 db 4,x'80'+c's',c'i',c'a'	/-SIAN
 db x'20'+4,c'e',x'80'+c'i',c'a'	/E-IAN
 db 4,x'80'+c'c',c'i',c'a'	/-CIAN
 db x'c0'+3,c'i',c'a'	/IA/N
 db 5,x'80'+c'c',c'l',c'e',c'a'	/-CLEAN
 db 4,x'80'+c'm',c'e',c'a'	/-MEAN
 db x'20'+3,c'e',x'80'+c'a'	/E-AN
 db 0
sufo	equ	*
 db 5,x'80'+c'm',c'a',c'c',x'80'+c'r'	/-MAC-RO
 db 0
sufp	equ	*
 db 5,x'80'+c'g',c'r',c'o',c'u'	/-GROUP
 db 2,x'80'+c'u'	/-UP
 db 4,x'80'+c's',c'h',c'i'	/-SHIP
 db 4,x'80'+c'k',c'e',c'e'	/-KEEP
 db 0
sufr	equ	*
 db 4,x'80'+c'z',c'a',c'r'	/-ZARR
 db x'c0'+2,c'r'	/R/R
 db 3,x'80'+c't',c'o'	/-TOR
 db x'20'+3,x'80'+c's',c'o'	/-SOR
 db x'20'+4,x'80'+c'r',c'i',x'80'+c'o'	/-RI-OR
 db 4,c'i',c'z',x'80'+c'e'	/IZ-ER
 db 5,x'80'+c'c',c'o',c'v',x'80'+c'e'	/-COV-ER
* db 4,c'o',x'80'+c'v',c'e'	/O-VER
 db 4,x'80'+c'o',c'v',c'e'	/-OVER
 db 4,x'80'+c'e',c'v',x'80'+c'e'	/-EV-ER
 db 8,x'80'+c'c',c'o',c'm',x'80'+c'p',c'u',c't',x'80'+c'e'	/-COM-PUT-ER
 db x'20'+5,c'u',c's',x'80'+c't',c'e'	/US-TER
 db 5,c'o',c's',c't',x'80'+c'e'	/OST-ER
 db x'20'+5,x'80'+c'a',c'c',x'80'+c't',c'e'	/-AC-TER
 db 6,x'80'+c'w',c'r',c'i',c't',x'80'+c'e'	/-WRIT-ER
 db x'20'+5,c'i',c's',x'80'+c't',c'e'	/IS-TER
 db x'20'+5,c'e',c's',x'80'+c't',c'e'	/ES-TER
 db x'20'+5,c'a',c's',x'80'+c't',c'e'	/AS-TER
 db 4,x'80'+c's',c't',c'e'	/-STER
 db 5,c'a',c'r',x'80'+c't',c'e'	/AR-TER
 db 4,c'r',c't',x'80'+c'e'	/RT-ER
 db x'20'+5,c'm',x'80'+c'e',x'80'+c't',c'e'	/M-E-TER
 db 5,x'80'+c'w',c'a',x'80'+c't',c'e'	/-WA-TER
 db 3,c'r',x'80'+c'e'	/R-ER
 db 4,c'o',c'p',x'80'+c'e'	/OP-ER
 db 5,x'80'+c'p',c'a',x'80'+c'p',c'e'	/-PA-PER
 db 4,c'w',c'n',x'80'+c'e'	/WN-ER
 db x'20'+4,c's',x'80'+c'n',c'e'	/S-NER
 db 4,c'o',c'n',x'80'+c'e'	/ON-ER
 db 4,c'r',c'm',x'80'+c'e'	/RM-ER
 db 3,x'80'+c'm',c'e'	/-MER
 db 4,c'l',c'l',x'80'+c'e'	/LL-ER
 db 5,c'd',x'80'+c'd',c'l',c'e'	/D-DLER
 db 4,x'80'+c'b',c'l',c'e'	/-BLER
 db 3,c'k',x'80'+c'e'	/K-ER
 db 5,c'n',x'80'+c't',c'h',c'e'	/N-THER
 db 6,x'80'+c'f',c'a',x'80'+c't',c'h',c'e'	/-FA-THER
 db 6,c'e',c'i',x'80'+c't',c'h',c'e'	/EI-THER
 db 4,c't',c'h',x'80'+c'e'	/TH-ER
 db 4,c's',c'h',x'80'+c'e'	/SH-ER
 db 4,x'80'+c'p',c'h',c'e'	/-PHER
 db 4,c'c',c'h',x'80'+c'e'	/CH-ER
 db 4,c'd',c'g',x'80'+c'e'	/DG-ER
 db 4,c'r',c'd',x'80'+c'e'	/RD-ER
 db 6,c'o',c'u',c'n',c'd',x'80'+c'e'	/OUND-ER
 db 4,c'l',c'd',x'80'+c'e'	/LD-ER
 db 4,c'i',c'd',x'80'+c'e'	/ID-ER
 db 5,x'80'+c'd',c'u',c'c',x'80'+c'e'	/-DUC-ER
 db 4,c'n',c'c',x'80'+c'e'	/NC-ER
 db x'40'+2,x'80'+c'e'	/ /ER
 db 3,x'80'+c's',c'a'	/-SAR
 db x'20'+6,c'a',c'c',x'80'+c'u',x'80'+c'l',c'a'	/AC-U-LAR
 db x'20'+6,c'e',c'c',x'80'+c'u',x'80'+c'l',c'a'	/EC-U-LAR
 db x'20'+6,c'i',c'c',x'80'+c'u',x'80'+c'l',c'a'	/IC-U-LAR
 db x'20'+6,c'e',c'g',x'80'+c'u',x'80'+c'l',c'a'	/EG-U-LAR
 db 0
sufs	equ	*
 db x'20'+4,c'u',x'80'+c'o',c'u'	/U-OUS
 db 5,x'80'+c't',c'i',c'o',c'u'	/-TIOUS
 db 5,x'80'+c'g',c'i',c'o',c'u'	/-GIOUS
 db 5,x'80'+c'c',c'i',c'o',c'u'	/-CIOUS
 db x'20'+4,c'i',x'80'+c'o',c'u'	/I-OUS
 db 5,x'80'+c'g',c'e',c'o',c'u'	/-GEOUS
 db 5,x'80'+c'c',c'e',c'o',c'u'	/-CEOUS
 db 4,c'e',x'80'+c'o',c'u'	/E-OUS
 db x'60'+2,x'80'+c'u'	//US
 db 4,x'80'+c'n',c'e',c's'	/-NESS
 db 4,x'80'+c'l',c'e',c's'	/-LESS
 db x'60'+2,x'80'+c's'	//SS
 db x'20'+5,c'p',x'80'+c'o',x'80'+c'l',c'i'	/P-O-LIS
 db x'60'+2,x'80'+c'i'	//IS
 db x'40'+3,x'80'+c'x',c'e'	/X/ES
 db x'40'+3,x'80'+c's',c'e'	/S/ES
 db x'40'+4,c's',c'h',x'80'+c'e'	/SH/ES
 db x'40'+4,c'c',c'h',x'80'+c'e'	/CH/ES
 db x'c0'+1	//S
 db 0
suft	equ	*
 db 6,c'i',c'o',c'n',x'80'+c'i',c's'	/ION-IST
 db 5,c'i',c'n',x'80'+c'i',c's'	/IN-IST
 db 5,c'a',c'l',x'80'+c'i',c's'	/AL-IST
 db 6,c'l',x'80'+c'o',x'80'+c'g',c'i',c's'	/L-O-GIST
 db 5,c'h',c't',x'80'+c'e',c's'	/HT-EST
 db 4,c'i',x'80'+c'e',c's'	/I-EST
 db 5,c'g',x'80'+c'g',c'e',c's'	/G-GEST
 db 4,c'g',x'80'+c'e',c's'	/G-EST
 db 5,c'd',x'80'+c'd',c'e',c's'	/D-DEST
 db 4,c'd',x'80'+c'e',c's'	/D-EST
 db 4,x'80'+c'c',c'a',c's'	/-CAST
 db 5,x'80'+c'h',c'e',c'a',c'r'	/-HEART
 db 4,x'80'+c'f',c'o',c'o'	/-FOOT
 db 3,c'i',x'80'+c'o'	/I-OT
 db 5,x'80'+c'f',c'r',c'o',c'n'	/-FRONT
 db 5,x'80'+c'p',c'r',c'i',c'n'	/-PRINT
 db 4,x'80'+c'm',c'e',c'n'	/-MENT
 db 5,x'80'+c'c',c'i',c'e',c'n'	/-CIENT
 db 4,c'i',x'80'+c'a',c'n'	/I-ANT
 db 6,x'80'+c'w',c'r',c'i',c'g',c'h'	/-WRIGHT
 db 6,x'80'+c'b',c'r',c'i',c'g',c'h'	/-BRIGHT
 db 6,x'80'+c'f',c'l',c'i',c'g',c'h'	/-FLIGHT
 db 6,x'80'+c'w',c'e',c'i',c'g',c'h'	/-WEIGHT
 db 5,x'80'+c's',c'h',c'i',c'f'	/-SHIFT
 db 5,x'80'+c'c',c'r',c'a',c'f'	/-CRAFT
 db x'20'+4,c'd',c'g',x'80'+c'e'	/DG-ET
 db 4,x'80'+c'g',c'o',c'a'	/-GOAT
 db 4,x'80'+c'c',c'o',c'a'	/-COAT
 db 4,x'80'+c'b',c'o',c'a'	/-BOAT
 db 4,x'80'+c'w',c'h',c'a'	/-WHAT
 db 4,x'80'+c'c',c'u',c'i'	/-CUIT
 db 0
sufy	equ	*
 db x'20'+4,c'e',c's',x'80'+c't'	/ES-TY
 db x'20'+5,c'q',c'u',c'i',x'80'+c't'	/QUI-TY
 db 4,x'80'+c't',c'i',x'80'+c't'	/-TI-TY
 db x'20'+5,c'o',c's',x'80'+c'i',x'80'+c't'	/OS-I-TY
 db 4,x'80'+c's',c'i',x'80'+c't'	/-SI-TY
 db 5,c'i',c'n',x'80'+c'i',x'80'+c't'	/IN-I-TY
 db 4,c'n',c'i',x'80'+c't'	/NI-TY
 db x'20'+8,c'f',c'a',x'80'+c'b',c'i',c'l',x'80'+c'i',x'80'+c't'	/FA-BIL-I-TY
 db 8,x'80'+c'c',c'a',x'80'+c'b',c'i',c'l',x'80'+c'i',x'80'+c't'	/-CA-BIL-I-TY
 db 8,x'80'+c'p',c'a',x'80'+c'b',c'i',c'l',x'80'+c'i',x'80'+c't'	/-PA-BIL-I-TY
 db 6,x'80'+c'b',c'i',c'l',x'80'+c'i',x'80'+c't'	/-BIL-I-TY
 db 3,c'i',x'80'+c't'	/I-TY
 db 4,x'80'+c'b',c'u',c'r'	/-BUR-Y
 db 4,x'80'+c't',c'o',x'80'+c'r'	/-TO-RY
 db 5,x'80'+c'q',c'u',c'a',c'r'	/-QUAR-Y
 db x'20'+4,c'u',x'80'+c'a',c'r'	/U-ARY
 db 7,x'80'+c'm',c'e',c'n',x'80'+c't',c'a',x'80'+c'r'	/-MEN-TA-RY
 db 6,c'i',c'o',c'n',x'80'+c'a',c'r'	/ION-ARY
 db 4,c'i',x'80'+c'a',c'r'	/I-ARY
 db 4,c'n',x'80'+c'o',x'80'+c'm'	/N-O-MY
 db 3,x'80'+c'p',c'l'	/-PLY
 db 4,c'g',x'80'+c'g',c'l'	/G-GLY
 db 5,x'80'+c'p',c'a',x'80'+c'b',c'l'	/-PA-BLY
 db 5,c'f',c'a',x'80'+c'b',c'l'	/FA-BLY
 db 5,x'80'+c'c',c'a',x'80'+c'b',c'l'	/-CA-BLY
 db 4,x'80'+c'a',c'b',c'l'	/-ABLY
 db 3,x'80'+c'b',c'l'	/-BLY
 db 2,x'80'+c'l'	/-LY
 db 3,x'80'+c's',c'k'	/-SKY
 db x'20'+6,c'g',x'80'+c'r',c'a',x'80'+c'p',c'h'	/G-RA-PHY
 db 4,c'l',x'80'+c'o',x'80'+c'g'	/L-O-GY
 db 2,x'80'+c'f'	/-FY
 db 3,x'80'+c'n',c'e'	/-NEY
 db 3,x'80'+c'l',c'e'	/-LEY
 db 4,c'c',c'k',x'80'+c'e'	/CK-EY
 db 3,x'80'+c'k',c'e'	/-KEY
 db 4,x'80'+c'b',c'o',c'd'	/-BODY
 db 5,x'80'+c's',c't',c'u',c'd'	/-STUDY
 db x'e0'+4,c'e',c'e',c'd'	/EEDY
 db 2,x'80'+c'b'	/-BY
 db 3,x'80'+c'w',c'a'	/-WAY
 db 3,x'80'+c'd',c'a'	/-DAY
 db 0
	align	adc
	end
