# define SEOS 1
# define SCOMMENT 2
# define SLABEL 3
# define SUNKNOWN 4
# define SHOLLERITH 5
# define SICON 6
# define SRCON 7
# define SDCON 8
# define SBITCON 9
# define SOCTCON 10
# define SHEXCON 11
# define STRUE 12
# define SFALSE 13
# define SNAME 14
# define SNAMEEQ 15
# define SFIELD 16
# define SSCALE 17
# define SINCLUDE 18
# define SLET 19
# define SASSIGN 20
# define SAUTOMATIC 21
# define SBACKSPACE 22
# define SBLOCK 23
# define SCALL 24
# define SCHARACTER 25
# define SCLOSE 26
# define SCOMMON 27
# define SCOMPLEX 28
# define SCONTINUE 29
# define SDATA 30
# define SDCOMPLEX 31
# define SDIMENSION 32
# define SDO 33
# define SDOUBLE 34
# define SELSE 35
# define SELSEIF 36
# define SEND 37
# define SENDFILE 38
# define SENDIF 39
# define SENTRY 40
# define SEQUIV 41
# define SEXTERNAL 42
# define SFORMAT 43
# define SFUNCTION 44
# define SGOTO 45
# define SASGOTO 46
# define SCOMPGOTO 47
# define SARITHIF 48
# define SLOGIF 49
# define SIMPLICIT 50
# define SINQUIRE 51
# define SINTEGER 52
# define SINTRINSIC 53
# define SLOGICAL 54
# define SOPEN 55
# define SPARAM 56
# define SPAUSE 57
# define SPRINT 58
# define SPROGRAM 59
# define SPUNCH 60
# define SREAD 61
# define SREAL 62
# define SRETURN 63
# define SREWIND 64
# define SSAVE 65
# define SSTATIC 66
# define SSTOP 67
# define SSUBROUTINE 68
# define STHEN 69
# define STO 70
# define SUNDEFINED 71
# define SWRITE 72
# define SLPAR 73
# define SRPAR 74
# define SEQUALS 75
# define SCOLON 76
# define SCOMMA 77
# define SCURRENCY 78
# define SPLUS 79
# define SMINUS 80
# define SSTAR 81
# define SSLASH 82
# define SPOWER 83
# define SCONCAT 84
# define SAND 85
# define SOR 86
# define SNEQV 87
# define SEQV 88
# define SNOT 89
# define SEQ 90
# define SLT 91
# define SGT 92
# define SLE 93
# define SGE 94
# define SNE 95
# define SDECODE 96
# define SENCODE 97

# line 99 "gram.in"
#include "defs"
#include "string_defs"

#ifdef	C_OVERLAY
#define	yyerror(x)	{fprintf(diagfile, "%s\n", x); done(3); exit(3);}
#else
#define	yyerror(x)	error(x, 0, 0, FATAL)
#endif

static int nstars;
static int ndim;
static int vartype;
static ftnint varleng;
static struct { ptr lb, ub; } dims[8];
static struct labelblock *labarray[MAXLABLIST];
static int lastwasbranch = NO;
static int thiswasbranch = NO;
extern ftnint yystno;
extern flag intonly;

ftnint convci();
double convcd();
struct addrblock *nextdata(), *mkbitcon();
struct constblock *mklogcon(), *mkaddcon(), *mkrealcon();
struct constblock *mkstrcon(), *mkcxcon();
struct listblock *mklist();
struct listblock *mklist();
struct impldoblock *mkiodo();
struct extsym *comblock();

#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#ifndef YYSTYPE
#define YYSTYPE int
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
-1, 20,
	1, 31,
	-2, 205,
-1, 24,
	1, 35,
	-2, 205,
-1, 148,
	1, 221,
	-2, 170,
-1, 166,
	77, 242,
	-2, 170,
-1, 219,
	76, 156,
	-2, 123,
-1, 241,
	73, 205,
	-2, 202,
-1, 268,
	1, 264,
	-2, 127,
-1, 272,
	1, 273,
	77, 273,
	-2, 129,
-1, 317,
	76, 157,
	-2, 125,
-1, 348,
	1, 244,
	14, 244,
	73, 244,
	77, 244,
	-2, 171,
-1, 409,
	90, 0,
	91, 0,
	92, 0,
	93, 0,
	94, 0,
	95, 0,
	-2, 137,
-1, 428,
	1, 267,
	77, 267,
	-2, 127,
-1, 430,
	1, 269,
	77, 269,
	-2, 127,
-1, 432,
	1, 271,
	77, 271,
	-2, 127,
-1, 480,
	77, 267,
	-2, 127,
	};
# define YYNPROD 278
# define YYLAST 1345
short yyact[]={

 233, 230, 447, 449, 448, 235, 389, 441, 395, 199,
 390, 290, 225, 347, 248, 440, 346, 187, 270, 276,
 105,   5, 307, 114, 459,  17, 261, 126, 240, 295,
 191, 198, 181, 267, 202, 195, 197, 116,  95, 326,
 180, 119, 208, 255, 259, 110, 324, 325, 326, 305,
 271, 102, 446,  96,  97,  98, 463, 445,  53,  43,
 487,  86, 269,  58, 101,  91, 101, 127,  44, 253,
 254, 255, 118, 153, 117, 153, 516,  88, 128, 129,
 130, 131, 101, 133,  54,  55,  56,  47, 300, 101,
  89, 514, 496, 299,  90, 498,  59,  85, 419, 111,
  81, 461,  49,  87, 462, 436,  60, 435, 122, 485,
 511,  84, 160, 161, 324, 325, 326, 332, 331, 330,
 329, 328, 434, 333, 335, 334, 337, 336, 338, 147,
 183, 184, 118, 292, 117,  82,  83, 182, 185, 361,
 426, 263, 186, 190, 416, 260, 401, 237, 438, 288,
 153, 439, 420, 252, 153, 419, 292, 239, 392, 211,
 382, 393, 387, 383, 153, 220, 212, 215, 217, 216,
 210, 368, 272, 272, 272, 367, 156, 157, 158, 159,
 153, 366, 352, 154, 155, 101, 251, 264, 205, 172,
 226, 236, 236, 112, 277, 278, 279, 280, 281, 109,
 108, 284, 285, 162, 164, 274, 275, 171, 107, 376,
 106, 294, 377, 265, 287, 104, 349, 293, 286, 350,
 266, 321, 309, 168, 322, 315, 219, 323, 490, 304,
 303, 399, 490, 301, 394, 218, 518, 323, 504, 503,
 502, 339, 497, 313, 247, 252, 500, 494, 153, 489,
 160, 161, 249, 153, 153, 153, 153, 153, 252, 252,
 310, 311, 345, 469, 246, 400, 471, 323, 101, 314,
 317, 418, 323, 320,   4, 272, 296, 188, 302, 101,
 340, 242, 238, 252, 342, 343, 222, 219, 196, 344,
 209, 207, 371, 163, 169, 135, 139, 319, 468, 228,
 170, 353, 372, 373, 374, 362, 391, 101, 363, 364,
 386, 323, 323, 100, 103, 323, 206, 375, 323, 221,
 397, 323, 115, 378, 340, 379, 479, 263, 160, 161,
 253, 254, 255, 256, 101, 404, 370, 148, 201, 166,
  29, 323,  93, 323, 323, 323, 160, 161, 324, 325,
 326, 153, 252, 153, 323,   6, 252, 252, 252, 252,
 252, 250, 245, 340,  99, 421, 153, 424, 272, 272,
 272,  80, 121, 226, 405, 406, 407, 408, 409, 410,
 411, 412, 413, 414,  79,  78, 450, 451, 425, 437,
 232, 103, 103, 103, 103,  77,  76, 386,  75, 165,
 189,  74, 192, 193, 194, 470, 323, 323, 323, 323,
 323, 323, 323, 323, 323, 323, 464, 473,  73,  72,
 466, 152,  57, 252, 192, 213, 214,  50, 252, 429,
 431, 433, 442, 476, 224, 475, 272, 272, 272, 231,
 241, 477, 243, 323,  48, 465,  46, 450, 451, 486,
 236, 478, 472, 425, 488, 484,  45,  42, 509, 481,
 482, 483,  31, 151, 327, 151, 323, 318, 316, 491,
 268, 268, 268, 323, 492, 204, 388, 495, 493, 203,
 153, 103, 134, 381, 291, 380, 452, 385, 450, 451,
 442, 508, 507, 505, 384, 132, 297, 429, 431, 433,
 323, 115, 252, 308,  52,  35, 306, 323, 192, 113,
 512, 323, 150,  25, 150,  24,  23, 515, 323,  22,
  21,  20, 236,  19, 156, 157, 158, 159, 499, 341,
 519, 154, 155, 101, 289,  92, 506,   9,   8,   7,
 151, 510,   3,   2, 151, 160, 161, 253, 254, 255,
 200, 513,   1,   0, 151,   0, 160, 161, 253, 254,
 255, 256, 236,   0,   0, 517,   0,   0,   0,   0,
 151, 244, 200, 369, 423, 257,   0,   0,   0, 160,
 161, 253, 254, 255, 256, 258,   0,   0,   0, 150,
 192,   0, 247, 150,   0,   0,   0,   0, 160, 161,
 283, 282,   0, 150, 262,   0,   0,   0, 262,   0,
   0,   0, 341,   0,   0, 402, 360,   0,   0, 150,
   0, 160, 161, 253, 254, 255, 256, 160, 161, 253,
 254, 255, 256,   0,   0,   0, 298,   0, 151,   0,
   0, 200,   0, 151, 151, 151, 151, 151, 160, 161,
 324, 325, 326, 332, 331, 330, 329, 328,   0, 333,
 335, 334, 337, 336, 338,   0, 428, 430, 432, 351,
   0,   0,   0, 291, 355, 356, 357, 358, 359,   0,
   0, 453,   0, 460,   0,   0,   0, 150,   0, 308,
   0, 467, 150, 150, 150, 150, 150,   0,   0,   0,
 262, 348,   0, 262, 262,   0, 160, 161, 253, 254,
 255, 256, 156, 157, 158, 159, 474,   0,   0, 154,
 155, 101, 460, 160, 161, 324, 325, 326, 332,   0,
 460, 460, 460,   0, 480, 430, 432,   0,   0,   0,
 200, 151, 453, 151,   0,   0, 453, 160, 161, 324,
 325, 326, 332, 331, 330, 341, 151,   0, 333, 335,
 334, 337, 336, 338,   0,   0, 160, 161, 324, 325,
 326, 332, 422,   0, 422,   0, 454, 333, 335, 334,
 337, 336, 338, 453,   0,   0,   0, 427,   0,   0,
 150,   0, 150,   0,   0,   0,   0,   0,   0,   0,
   0, 262,   0,   0, 365, 150, 160, 161, 324, 325,
 326, 332, 331, 330, 329, 328,   0, 333, 335, 334,
 337, 336, 338, 444,   0,   0,   0, 200,   0,   0,
   0,   0,   0,   0,   0,   0,   0, 454, 417,   0,
   0, 454,   0, 160, 161, 324, 325, 326, 332, 331,
 330, 329, 328,   0, 333, 335, 334, 337, 336, 338,
   0,   0,   0,   0,   0,   0, 262,   0,   0,   0,
 151,   0,   0,   0, 415,   0,   0,   0, 454, 160,
 161, 324, 325, 326, 332, 331, 330, 329, 328,   0,
 333, 335, 334, 337, 336, 338,   0,   0,   0,   0,
   0, 501,   0,   0,   0,   0,  12,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0, 150,
  10,  53,  43,  70,  86,  14,  58,  67,  91,  36,
  63,  44,  40,  65,  69,  30,  64,  33,  32,  11,
  88,  34,  18,  39,  37,  27,  16,  54,  55,  56,
  47,  51,  41,  89,  61,  38,  66,  90,  28,  59,
  85,  13,   0,  81,  62,  49,  87,  26,  71,  60,
  15, 403,   0,  68,  84,   0, 160, 161, 324, 325,
 326, 332, 331, 330, 329, 328,   0, 333, 335, 334,
 337, 336, 338,   0,   0, 396,   0,   0,  82,  83,
 160, 161, 324, 325, 326, 332, 331, 330, 329, 328,
  94, 333, 335, 334, 337, 336, 338, 160, 161, 324,
 325, 326, 332, 331,   0,   0,   0,   0, 333, 335,
 334, 337, 336, 338,   0,   0,   0,   0,   0,   0,
   0, 120,   0, 123, 124, 125,   0,   0,   0,   0,
   0,   0,   0,   0, 136, 137,   0,   0, 138,   0,
 140, 141, 142,   0,   0, 143, 144, 145,   0, 146,
 156, 157, 158, 159,   0,   0,   0, 154, 155, 101,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
 173, 174, 175, 176, 177, 178, 179, 156, 157, 158,
 159,   0,   0,   0, 154, 155, 101,   0, 156, 157,
 158, 159,   0,   0,   0, 154, 155, 101,   0, 156,
 157, 158, 159,   0,   0,   0, 154, 155, 101,   0,
   0,   0,   0, 156, 157, 158, 159,   0, 229, 223,
 154, 155, 101,   0, 160, 161, 227,   0, 156, 157,
 158, 159,   0,   0, 234, 154, 155, 101,   0, 156,
 157, 158, 159,   0,   0, 229, 154, 155, 101,   0,
   0, 160, 161, 443,   0,   0, 229,   0,   0,   0,
   0, 234, 160, 161, 227,   0,   0, 229,   0,   0,
   0,   0, 234, 160, 161, 398,   0, 156, 157, 158,
 159, 229,   0, 234, 154, 155, 101, 160, 161, 354,
   0, 156, 157, 158, 159,   0, 229, 234, 154, 155,
 101,   0, 160, 161,   0,   0,   0, 273, 156, 157,
 158, 159, 234, 160, 161, 154, 155, 101,   0,   0,
   0,   0,   0, 234, 156, 157, 158, 159, 458, 457,
 456, 154, 155, 101, 156, 157, 158, 159,   0,   0,
   0, 154, 155, 101,   0, 312,   0,   0,   0,   0,
   0, 160, 161,   0,   0,   0,   0,   0,   0, 149,
   0, 234,   0,   0,   0, 160, 161, 167,   0,   0,
   0,   0,   0,   0,  70,   0, 247,   0,  67,   0,
   0,  63, 160, 161,  65,  69,   0,  64,   0,   0,
   0,   0, 455,   0,   0,   0,   0,   0, 160, 161,
   0,   0, 149,   0,   0,  61,   0,  66, 160, 161,
   0,   0,   0,   0,   0,  62,   0,   0,   0,  71,
   0,   0,   0,   0,  68 };
short yypact[]={

-1000,  18, 354, 902,-1000,-1000,-1000,-1000,-1000,-1000,
 337,-1000,-1000,-1000,-1000,-1000,-1000, 320, 293, 138,
 133, 131, 123, 122,  22, 116,  50,-1000,-1000,-1000,
-1000,  39,-1000,-1000,-1000, -14,-1000,-1000,-1000,-1000,
-1000,-1000, 293,-1000,-1000,-1000,-1000,-1000, 223,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,1249, 220,1206, 221, 221, 220, 112,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000, 293, 293, 293, 293,-1000,
-1000,-1000, 204,-1000, 293, -10, 293, 293, 293, 215,
 265,-1000,-1000, 111,-1000,-1000,-1000,-1000, 302, 218,
-1000,-1000,-1000, 217,-1000,-1000,-1000,-1000,  50, 293,
 293, 215, 265,-1000, 160, 214, 313,-1000, 213,1065,
1143,1143, 209, 313, 293, 208, 293,-1000,-1000, 171,
-1000,-1000, 548,1223,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000, 171,  68, 110,-1000,-1000, 254,1143,
1154,1154,1154,-1000,-1000,-1000,-1000,-1000,-1000, 519,
-1000,-1000,-1000, 204, 204, 293, -14,-1000,  75, -14,
  22,-1000, 203,-1000,-1000,-1000, 293,-1000,  11,-1000,
-1000, 265,-1000, 205,1273,  50, -33, 293, 313,1143,
1192,-1000, 293,-1000,-1000,-1000,-1000,-1000,1143,1143,
 227,-1000,1143,-1000, 147,-1000, 569, 313,-1000,1143,
-1000,-1000,-1000,1143,1143,-1000, 569,-1000,1143,-1000,
-1000,  22, 313,-1000, 627, 142,-1000,1223, 105,-1000,
1128,-1000,1223,1223,1223,1223,1223, -40, 542,  62,
 254,-1000,-1000, 254, 254,  62, 727,-1000, 104,  98,
  94, 569,-1000,1154,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000, 548,-1000,-1000,-1000, 204, 203,-1000, 135,
-1000,-1000,-1000, 203, 293,-1000,-1000,  86,-1000,-1000,
 265,  85, 292,-1000,-1000,-1000,  84,-1000, 159,-1000,
 921, 569,1114,-1000, 569, 155, 191, 569,  69, 293,
 897,-1000,1103,1143,1143,1143,1143,1143,1143,1143,
1143,1143,1143,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
 800,  67, -44, 687, 764, 198,  78,-1000,-1000,-1000,
 171, 500, 171, 569,-1000, -12, -40, -40, -40, 466,
-1000, 254,  62,  63,  62,1223,1154,1154,1154,  45,
  30,  28,-1000,-1000,-1000,-1000,-1000,  52,-1000,-1000,
  74,1092,-1000, 293, -25,1239,-1000, 265,  27,-1000,
 -24,-1000,-1000, 293,1143, 293, 229,-1000, 189,1143,
 193,1143,-1000, 313,-1000, -35, -44, -44, -44, 644,
 668, 668, 938, 687, 267,-1000,1143,-1000, 313, 313,
  22,-1000, 548,-1000,-1000,-1000, 254, 249,-1000,-1000,
-1000,-1000,-1000,-1000,1154,1154,1154,-1000,-1000,1092,
-1000,-1000,  33,-1000,-1000,-1000,1239,-1000,-1000, -21,
 707,-1000,-1000,-1000,-1000,1143,-1000,-1000,-1000, 175,
 153,-1000, 292, 292,-1000, 569,-1000, 157,-1000,-1000,
 173,1143, 569,  15, 168,  21,-1000,1143, 172,1223,
 168, 166, 165, 164,-1000,1092,-1000,1239,-1000,-1000,
1143,-1000,-1000,-1000,-1000,  34, 313,-1000,-1000, 569,
-1000, 477,-1000,-1000,-1000,-1000, 569,-1000,-1000,  14,
 569,1143,  -1,-1000,1143, 162, 313, 569,-1000,-1000 };
short yypgo[]={

   0, 552, 543, 542, 539, 538, 537, 535,1010,  38,
  40,  32,  17,  25, 295, 534,  11, 523, 521, 520,
 519, 516, 515, 513, 509,  41, 506,  27,  29, 505,
 504,  42,  50,   8,  30,  37,  20,  35, 496, 439,
 495,  36,  31, 494, 487,   2,   4,   3,   0,   1,
 486,  23,  22,   9,  24, 485, 483,  15,   7,  16,
  13,  28,  34, 479, 476, 475,   6,  10, 468, 467,
 299, 390, 464,   5, 421, 264, 340, 462, 458, 457,
 456, 446, 444, 434, 427,  12, 422, 419, 129, 418,
 401,  44, 399, 398, 223, 396,  33, 395, 385, 384,
  19, 371, 362,  14, 361,  26,  62,  18 };
short yyr1[]={

   0,   1,   1,   2,   2,   2,   2,   2,   2,   2,
   3,   4,   4,   4,   4,   4,   4,   9,  11,  14,
  10,  10,  12,  12,  12,  15,  15,  16,  16,   7,
   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
   5,  17,  17,  13,  29,  30,  30,  30,  30,  30,
  30,  30,  30,  30,  30,  30,  27,  27,  27,  18,
  18,  18,  18,  35,  35,  19,  19,  20,  20,  21,
  21,  37,  38,  38,  22,  22,  40,  41,  44,  43,
  43,  45,  45,  46,  46,  46,  46,  24,  24,  51,
  51,  26,  26,  52,  34,  53,  53,  42,  42,  28,
  28,  56,  55,  55,  57,  57,  58,  58,  59,  59,
  60,  61,  23,  23,  62,  65,  63,  64,  64,  66,
  66,  67,  25,  68,  68,  69,  69,  32,  32,  32,
  70,  70,  70,  70,  70,  70,  70,  70,  70,  70,
  70,  70,  70,  70,  48,  48,  72,  72,  72,  72,
  72,  72,  39,  39,  39,  39,  73,  73,  47,  47,
  71,  71,  71,  71,  71,  71,  49,  50,  50,  50,
  74,  74,  75,  75,  75,  75,  75,  75,  75,  75,
   6,   6,   6,   6,   6,   6,   6,  77,  54,  76,
  76,  76,  76,  76,  76,  76,  76,  76,  76,  76,
  79,  80,  80,  80,  80,  36,  36,  82,  83,  83,
  85,  85,  84,  84,  78,  78,   8,  31,  33,  81,
  86,  86,  86,  86,  86,  86,  86,  86,  86,  86,
  86,  86,  86,  86,  87,  99,  99,  99,  89, 101,
 101, 101,  92,  92,  88,  88, 102, 102, 103, 103,
 103, 103, 104,  94,  90,  97,  93,  95,  98,  98,
  91,  91, 105, 105,  96,  96,  96, 107, 107, 107,
 107, 107, 107, 106, 106, 106, 106, 100 };
short yyr2[]={

   0,   0,   3,   2,   2,   2,   3,   3,   2,   1,
   1,   3,   3,   4,   4,   5,   3,   0,   1,   1,
   0,   1,   0,   2,   3,   1,   3,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   2,   1,
   5,   5,   5,   2,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   0,   4,   6,   3,
   4,   5,   3,   1,   3,   3,   3,   3,   3,   3,
   3,   3,   1,   3,   3,   3,   0,   4,   0,   2,
   3,   1,   3,   1,   2,   1,   1,   1,   3,   1,
   1,   1,   3,   3,   2,   1,   5,   1,   3,   0,
   3,   0,   2,   3,   1,   3,   1,   1,   1,   3,
   1,   1,   3,   3,   4,   0,   2,   1,   3,   1,
   3,   1,   0,   0,   1,   1,   3,   1,   3,   1,
   1,   1,   3,   3,   3,   3,   2,   3,   3,   3,
   3,   3,   2,   3,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   6,   4,   9,   0,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   5,   1,   1,   1,
   1,   3,   1,   1,   3,   3,   3,   3,   2,   3,
   1,   6,   2,   2,   6,   2,   2,   5,   3,   4,
   5,   2,   1,   1,  10,   1,   3,   4,   3,   3,
   1,   3,   3,   7,   7,   0,   1,   3,   1,   3,
   1,   2,   1,   1,   1,   3,   0,   0,   0,   1,
   2,   2,   2,   2,   3,   4,   4,   3,   2,   3,
   2,   3,   1,   3,   3,   1,   1,   1,   3,   1,
   1,   1,   1,   1,   3,   3,   3,   3,   1,   1,
   2,   2,   1,   7,   3,   3,   3,   3,   4,   4,
   1,   3,   1,   5,   1,   1,   1,   3,   3,   3,
   3,   3,   3,   1,   5,   5,   5,   0 };
short yychk[]={

-1000,  -1,  -2,  -3, 256,   3,   1,  -4,  -5,  -6,
  18,  37,   4,  59,  23,  68,  44, -13,  40, -17,
 -18, -19, -20, -21, -22, -23,  65,  43,  56, -76,
  33, -77,  36,  35,  39, -29,  27,  42,  53,  41,
  30,  50, -79,  20,  29, -80, -81,  48, -82,  63,
 -84,  49, -30,  19,  45,  46,  47, -86,  24,  57,
  67,  52,  62,  28,  34,  31,  54,  25,  71,  32,
  21,  66, -87, -89, -90, -93, -95, -97, -98, -99,
-101,  61,  96,  97,  72,  58,  22,  64,  38,  51,
  55,  26,  -7,   5,  -8,  -9,  -9,  -9,  -9,  44,
 -14,  14, -11, -14,  77, -36,  77,  77,  77,  77,
 -36,  77,  77, -24, -51, -14, -35,  84,  82, -25,
  -8, -76,  69,  -8,  -8,  -8, -27,  81, -25, -25,
 -25, -25, -40, -25, -39, -14,  -8,  -8,  -8,  73,
  -8,  -8,  -8,  -8,  -8,  -8,  -8, -88, -75,  73,
 -39, -71, -74, -48,  12,  13,   5,   6,   7,   8,
  79,  80, -88,  73, -88, -92, -75,  81, -94,  73,
 -94, -88,  77,  -8,  -8,  -8,  -8,  -8,  -8,  -8,
 -10, -11, -10, -11, -11,  -9, -25, -12,  73, -14,
 -35, -34, -14, -14, -14, -37,  73, -41, -42, -53,
 -39,  73, -62, -63, -65,  77,  14,  73, -31,  73,
 -31, -34, -35, -14, -14, -37, -41, -62,  75,  73,
 -61,   6,  73,  74, -83, -85, -32,  81, -70,  73,
 -49, -39, -71, -48,  89, -73, -32, -73,  73, -60,
 -61, -14,  73, -14, -74,-102, -75,  73,-103,  81,
-104,  15, -48,  81,  82,  83,  84, -74, -74, -91,
  77,-105, -39,  73,  77, -91, -32, -96, -70,-106,
-107, -32, -49,  73, -96, -96,-100,-100,-100,-100,
-100,-100, -74,  81, -12, -12, -11, -27,  74, -15,
 -16, -14,  81, -27, -36, -28,  73, -38, -39,  82,
  77, -42,  73, -13, -51,  82, -26, -52, -14, -60,
 -32, -32,  73, -34, -32, -73, -68, -32, -69,  70,
 -32,  74,  77, -48,  81,  82,  83, -72,  88,  87,
  86,  85,  84,  90,  92,  91,  94,  93,  95, -60,
 -32, -70, -32, -32, -32, -36, -59, -60,  74,  74,
  77, -74,  77, -32,  81, -74, -74, -74, -74, -74,
  74,  77, -91, -91, -91,  77,  77,  77,  77, -70,
-106,-107,-100,-100, -12, -28,  74,  77, -28, -34,
 -55, -56,  74,  77, -43, -44, -53,  77, -64, -66,
 -67,  14,  74,  77,  75, -33,  74, -33,  81,  76,
  74,  77, -14,  74, -85, -32, -32, -32, -32, -32,
 -32, -32, -32, -32, -32,  74,  77,  74,  73,  77,
  74,-103, -74,  74,-103,-105,  77, -74, -70,-106,
 -70,-106, -70,-106,  77,  77,  77, -16,  74,  77,
 -57, -58, -32,  81, -39,  82,  77, -45, -46, -47,
 -48, -49, -50, -14, -71,  73,  11,  10,   9, -54,
 -14,  74,  77,  80, -52, -32, -54, -14,  69,  74,
 -73,  73, -32, -60, -70, -59, -60, -36, -54,  77,
 -70, -54, -54, -54, -57,  76, -45,  81, -47,  74,
  75, -66, -67, -33,  74, -73,  77,  74,  74, -32,
  74, -74,  74,  74,  74, -58, -32, -46, -47, -78,
 -32,  76, -60,  74,  77, -73,  77, -32,  74, -60 };
short yydef[]={

   1,  -2,   0,   0,   9,  10,   2,   3,   4,   5,
   0, 216,   8,  17,  17,  17,  17,   0,   0,  30,
  -2,  32,  33,  34,  -2,  36,  37,  39, 122, 180,
 216,   0, 216, 216, 216,  56, 122, 122, 122, 122,
  76, 122,   0, 216, 216, 192, 193, 216, 195, 216,
 216, 216,  44, 200, 216, 216, 216, 219, 216, 212,
 213,  45,  46,  47,  48,  49,  50,  51,  52,  53,
  54,  55,   0,   0,   0,   0,   0,   0, 232, 216,
 216, 216, 216, 216, 216, 216, 235, 236, 237, 239,
 240, 241,   6,  29,   7,  20,  20,   0,   0,  17,
 122,  19,  22,  18,   0,   0, 206,   0,   0,   0,
   0, 206, 115,  38,  87,  89,  90,  63,   0,   0,
 217, 182, 183,   0, 185, 186,  43, 217,   0,   0,
   0,   0,   0, 115,   0, 152,   0, 191,   0,   0,
 156, 156,   0,   0,   0,   0,   0, 220,  -2,   0,
 172, 173,   0,   0, 160, 161, 162, 163, 164, 165,
 144, 145, 222,   0, 223,   0,  -2, 243,   0,   0,
 228, 230,   0, 277, 277, 277, 277, 277, 277,   0,
  11,  21,  12,  22,  22,   0,  56,  16,   0,  56,
 205,  62,  99,  66,  68,  70,   0,  75,   0,  97,
  95,   0, 113,   0,   0,   0,   0,   0,   0,   0,
   0,  59,   0,  65,  67,  69,  74, 112,   0,  -2,
   0, 111,   0, 196,   0, 208, 210,   0, 127,   0,
 129, 130, 131,   0,   0, 198, 157, 199,   0, 201,
 110,  -2,   0, 207, 248,   0, 170,   0,   0, 249,
   0, 252,   0,   0,   0,   0,   0, 178, 248, 224,
   0, 260, 262,   0,   0, 227,   0, 229,  -2, 265,
 266,   0,  -2,   0, 231, 233, 234, 238, 254, 256,
 257, 255, 277, 277,  13,  14,  22,  99,  23,   0,
  25,  27,  28,  99,   0,  94, 101,   0,  72,  78,
   0,   0,   0, 116,  88,  64,   0,  91,   0, 218,
   0, 218,   0,  60, 189,   0,   0,  -2, 124,   0,
   0, 197,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0, 146, 147, 148, 149, 150, 151, 211,
   0, 127, 136, 142,   0,   0,   0, 108,  -2, 245,
   0,   0,   0, 250, 251, 174, 175, 176, 177, 179,
 244,   0, 226,   0, 225,   0,   0,   0,   0, 127,
   0,   0, 258, 259,  15,  41,  24,   0,  42,  61,
   0,   0,  71,   0,   0,   0,  98,   0,   0, 117,
 119, 121,  40,   0,   0,   0,   0,  57,   0, 156,
 154,   0, 190,   0, 209, 132, 133, 134, 135,  -2,
 138, 139, 140, 141, 143, 128,   0, 187,   0,   0,
 205, 247, 248, 171, 246, 261,   0,   0,  -2, 268,
  -2, 270,  -2, 272,   0,   0,   0,  26, 100,   0,
 102, 104, 107, 106,  73,  77,   0,  79,  81,  83,
   0,  85,  86, 158, 159,   0, 167, 168, 169,   0,
 152, 114,   0,   0,  92,  93, 181,   0, 184, 218,
   0, 156, 126,   0, 127,   0, 109,   0,   0,   0,
  -2,   0,   0,   0, 103,   0,  80,   0,  84,  96,
   0, 118, 120,  58, 153,   0,   0, 166, 203, 204,
 263,   0, 274, 275, 276, 105, 107,  82,  83, 188,
 214, 156,   0, 253,   0,   0,   0, 215, 155, 194 };
#
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*	parser for yacc output	*/

int yydebug = 0; /* 1 for debugging */
YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

yyparse() {

	short yys[YYMAXDEPTH];
	short yyj, yym;
	register YYSTYPE *yypvt;
	register short yystate, *yyps, yyn;
	register YYSTYPE *yypv;
	register short *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

 yystack:    /* put a state and value onto the stack */

	if( yydebug  ) printf( "state %d, char 0%o\n", yystate, yychar );
		if( ++yyps> &yys[YYMAXDEPTH] ) { yyerror( "yacc stack overflow" ); return(1); }
		*yyps = yystate;
		++yypv;
		*yypv = yyval;

 yynewstate:

	yyn = yypact[yystate];

	if( yyn<= YYFLAG ) goto yydefault; /* simple state */

	if( yychar<0 ) if( (yychar=yylex())<0 ) yychar=0;
	if( (yyn += yychar)<0 || yyn >= YYLAST ) goto yydefault;

	if( yychk[ yyn=yyact[ yyn ] ] == yychar ){ /* valid shift */
		yychar = -1;
		yyval = yylval;
		yystate = yyn;
		if( yyerrflag > 0 ) --yyerrflag;
		goto yystack;
		}

 yydefault:
	/* default state action */

	if( (yyn=yydef[yystate]) == -2 ) {
		if( yychar<0 ) if( (yychar=yylex())<0 ) yychar = 0;
		/* look through exception table */

		for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ; /* VOID */

		while( *(yyxi+=2) >= 0 ){
			if( *yyxi == yychar ) break;
			}
		if( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
		}

	if( yyn == 0 ){ /* error */
		/* error ... attempt to resume parsing */

		switch( yyerrflag ){

		case 0:   /* brand new error */

			yyerror( "syntax error" );
		yyerrlab:
			++yynerrs;

		case 1:
		case 2: /* incompletely recovered error ... try again */

			yyerrflag = 3;

			/* find a state where "error" is a legal shift action */

			while ( yyps >= yys ) {
			   yyn = yypact[*yyps] + YYERRCODE;
			   if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE ){
			      yystate = yyact[yyn];  /* simulate a shift of "error" */
			      goto yystack;
			      }
			   yyn = yypact[*yyps];

			   /* the current yyps has no shift onn "error", pop stack */

			   if( yydebug ) printf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
			   --yyps;
			   --yypv;
			   }

			/* there is no state on the stack with an error shift ... abort */

	yyabort:
			return(1);


		case 3:  /* no shift yet; clobber input char */

			if( yydebug ) printf( "error recovery discards char %d\n", yychar );

			if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
			yychar = -1;
			goto yynewstate;   /* try again in the same state */

			}

		}

	/* reduction by production yyn */

		if( yydebug ) printf("reduce %d\n",yyn);
		yyps -= yyr2[yyn];
		yypvt = yypv;
		yypv -= yyr2[yyn];
		yyval = yypv[1];
		yym=yyn;
			/* consult goto table to find next state */
		yyn = yyr1[yyn];
		yyj = yypgo[yyn] + *yyps + 1;
		if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
		switch(yym){
			
case 3:
# line 153 "gram.in"
{ lastwasbranch = NO; } break;
case 5:
# line 156 "gram.in"
{ if(yypvt[-1] && (yypvt[-1]->labelno==dorange))
			enddo(yypvt[-1]->labelno);
		  if(lastwasbranch && thislabel==NULL)
			error("statement cannot be reached",0,0,WARN1);
		  lastwasbranch = thiswasbranch;
		  thiswasbranch = NO;
		} break;
case 6:
# line 164 "gram.in"
{ doinclude( yypvt[-0] ); } break;
case 7:
# line 166 "gram.in"
{ lastwasbranch = NO;  endproc(); } break;
case 8:
# line 168 "gram.in"
{ error("unclassifiable statement",0,0,EXECERR);  flline(); } break;
case 9:
# line 170 "gram.in"
{ flline();  needkwd = NO;  inioctl = NO; 
		  yyerrok; yyclearin; } break;
case 10:
# line 175 "gram.in"
{
		if(yystno != 0)
			{
			yyval = thislabel =  mklabel(yystno);
			if( ! headerdone )
				puthead(NULL, procclass);
			if(thislabel->labdefined)
				error("label %s already defined",
					convic(thislabel->stateno),0,EXECERR);
			else	{
				if(thislabel->blklevel!=0 && thislabel->blklevel<blklevel
				    && thislabel->labtype!=LABFORMAT)
					error("there is a branch to label %s from outside block",
					      convic( (ftnint) (thislabel->stateno) ),0,WARN1);
				thislabel->blklevel = blklevel;
				thislabel->labdefined = YES;
				if(thislabel->labtype != LABFORMAT)
					putlabel(thislabel->labelno);
				}
			}
		else    yyval = thislabel = NULL;
		} break;
case 11:
# line 200 "gram.in"
{ startproc(yypvt[-0], CLMAIN); } break;
case 12:
# line 202 "gram.in"
{ startproc(yypvt[-0], CLBLOCK); } break;
case 13:
# line 204 "gram.in"
{ entrypt(CLPROC, TYSUBR, (ftnint) 0,  yypvt[-1], yypvt[-0]); } break;
case 14:
# line 206 "gram.in"
{ entrypt(CLPROC, TYUNKNOWN, (ftnint) 0, yypvt[-1], yypvt[-0]); } break;
case 15:
# line 208 "gram.in"
{ entrypt(CLPROC, yypvt[-4], varleng, yypvt[-1], yypvt[-0]); } break;
case 16:
# line 210 "gram.in"
{ if(parstate==OUTSIDE || procclass==CLMAIN
			|| procclass==CLBLOCK)
				error("misplaced entry statement", 0,0,EXECERR);
		  entrypt(CLENTRY, 0, (ftnint) 0, yypvt[-1], yypvt[-0]);
		} break;
case 17:
# line 218 "gram.in"
{ newproc(); } break;
case 18:
# line 222 "gram.in"
{ yyval = newentry(yypvt[-0]); } break;
case 19:
# line 226 "gram.in"
{ yyval = mkname(toklen, token); } break;
case 20:
# line 229 "gram.in"
{ yyval = NULL; } break;
case 22:
# line 234 "gram.in"
{ yyval = 0; } break;
case 23:
# line 236 "gram.in"
{ yyval = 0; } break;
case 24:
# line 238 "gram.in"
{yyval = yypvt[-1]; } break;
case 25:
# line 242 "gram.in"
{ yyval = (yypvt[-0] ? mkchain(yypvt[-0],0) : 0 ); } break;
case 26:
# line 244 "gram.in"
{ if(yypvt[-0]) yypvt[-2] = yyval = hookup(yypvt[-2], mkchain(yypvt[-0],0)); } break;
case 27:
# line 248 "gram.in"
{ yypvt[-0]->vstg = STGARG; } break;
case 28:
# line 250 "gram.in"
{ yyval = 0;  substars = YES; } break;
case 29:
# line 256 "gram.in"
{
		char *s;
		s = copyn(toklen+1, token);
		s[toklen] = '\0';
		yyval = s;
		} break;
case 37:
# line 271 "gram.in"
{ saveall = YES; } break;
case 39:
# line 274 "gram.in"
{ fmtstmt(thislabel); setfmt(thislabel); } break;
case 41:
# line 279 "gram.in"
{ settype(yypvt[-3], yypvt[-4], yypvt[-1]);
		  if(ndim>0) setbound(yypvt[-3],ndim,dims);
		} break;
case 42:
# line 283 "gram.in"
{ settype(yypvt[-2], yypvt[-4], yypvt[-1]);
		  if(ndim>0) setbound(yypvt[-2],ndim,dims);
		} break;
case 43:
# line 289 "gram.in"
{ varleng = yypvt[-0]; } break;
case 44:
# line 293 "gram.in"
{ varleng = (yypvt[-0]<0 || yypvt[-0]==TYLONG ? 0 : typesize[yypvt[-0]]); } break;
case 45:
# line 296 "gram.in"
{ yyval = TYLONG; } break;
case 46:
# line 297 "gram.in"
{ yyval = TYREAL; } break;
case 47:
# line 298 "gram.in"
{ yyval = TYCOMPLEX; } break;
case 48:
# line 299 "gram.in"
{ yyval = TYDREAL; } break;
case 49:
# line 300 "gram.in"
{ yyval = TYDCOMPLEX; } break;
case 50:
# line 301 "gram.in"
{ yyval = TYLOGICAL; } break;
case 51:
# line 302 "gram.in"
{ yyval = TYCHAR; } break;
case 52:
# line 303 "gram.in"
{ yyval = TYUNKNOWN; } break;
case 53:
# line 304 "gram.in"
{ yyval = TYUNKNOWN; } break;
case 54:
# line 305 "gram.in"
{ yyval = - STGAUTO; } break;
case 55:
# line 306 "gram.in"
{ yyval = - STGBSS; } break;
case 56:
# line 310 "gram.in"
{ yyval = varleng; } break;
case 57:
# line 312 "gram.in"
{
		  if( ! ISICON(yypvt[-1]) )
			{
			yyval = 0;
			error("length must be an integer constant", 0, 0, DCLERR);
			}
		  else yyval = yypvt[-1]->const.ci;
		} break;
case 58:
# line 321 "gram.in"
{ yyval = 0; } break;
case 59:
# line 325 "gram.in"
{ incomm( yyval = comblock(0, 0) , yypvt[-0] ); } break;
case 60:
# line 327 "gram.in"
{ yyval = yypvt[-1];  incomm(yypvt[-1], yypvt[-0]); } break;
case 61:
# line 329 "gram.in"
{ yyval = yypvt[-2];  incomm(yypvt[-2], yypvt[-0]); } break;
case 62:
# line 331 "gram.in"
{ incomm(yypvt[-2], yypvt[-0]); } break;
case 63:
# line 335 "gram.in"
{ yyval = comblock(0, 0); } break;
case 64:
# line 337 "gram.in"
{ yyval = comblock(toklen, token); } break;
case 65:
# line 341 "gram.in"
{ setext(yypvt[-0]); } break;
case 66:
# line 343 "gram.in"
{ setext(yypvt[-0]); } break;
case 67:
# line 347 "gram.in"
{ setintr(yypvt[-0]); } break;
case 68:
# line 349 "gram.in"
{ setintr(yypvt[-0]); } break;
case 71:
# line 357 "gram.in"
{
		struct equivblock *p;
		if(nequiv >= MAXEQUIV)
			error("too many equivalences",0,0,FATAL);
		p  =  & eqvclass[nequiv++];
		p->eqvinit = 0;
		p->eqvbottom = 0;
		p->eqvtop = 0;
		p->equivs = yypvt[-1];
		} break;
case 72:
# line 370 "gram.in"
{ yyval = ALLOC(eqvchain); yyval->eqvitem = yypvt[-0]; } break;
case 73:
# line 372 "gram.in"
{ yyval = ALLOC(eqvchain); yyval->eqvitem = yypvt[-0]; yyval->nextp = yypvt[-2]; } break;
case 76:
# line 380 "gram.in"
{ if(parstate == OUTSIDE)
			{
			newproc();
			startproc(0, CLMAIN);
			}
		  if(parstate < INDATA)
			{
			enddcl();
			parstate = INDATA;
			}
		} break;
case 77:
# line 394 "gram.in"
{ ftnint junk;
		  if(nextdata(&junk,&junk) != NULL)
			{
			error("too few initializers",0,0,ERR);
			curdtp = NULL;
			}
		  frdata(yypvt[-3]);
		  frrpl();
		} break;
case 78:
# line 405 "gram.in"
{ toomanyinit = NO; } break;
case 81:
# line 410 "gram.in"
{ dataval(NULL, yypvt[-0]); } break;
case 82:
# line 412 "gram.in"
{ dataval(yypvt[-2], yypvt[-0]); } break;
case 84:
# line 417 "gram.in"
{ if( yypvt[-1]==OPMINUS && ISCONST(yypvt[-0]) )
			consnegop(yypvt[-0]);
		  yyval = yypvt[-0];
		} break;
case 89:
# line 430 "gram.in"
{ int k;
		  yypvt[-0]->vsave = 1;
		  k = yypvt[-0]->vstg;
		if( ! ONEOF(k, M(STGUNKNOWN)|M(STGBSS)|M(STGINIT)) )
			error("can only save static variables", yypvt[-0], 0, DCLERR);
		} break;
case 90:
# line 437 "gram.in"
{ yypvt[-0]->extsave = 1; } break;
case 93:
# line 445 "gram.in"
{ if(yypvt[-2]->vclass == CLUNKNOWN)
			{ yypvt[-2]->vclass = CLPARAM;
			  yypvt[-2]->paramval = yypvt[-0];
			}
		  else error("cannot make %s parameter", yypvt[-2], 0, DCLERR);
		} break;
case 94:
# line 454 "gram.in"
{ if(ndim>0) setbounds(yypvt[-1], ndim, dims); } break;
case 95:
# line 458 "gram.in"
{ ptr np;
		  vardcl(np = yypvt[-0]->namep);
		  if(np->vstg == STGBSS)
			np->vstg = STGINIT;
		  else if(np->vstg == STGCOMMON)
			extsymtab[np->vardesc.varno].extinit = YES;
		  else if(np->vstg==STGEQUIV)
			eqvclass[np->vardesc.varno].eqvinit = YES;
		  else if(np->vstg != STGINIT)
			error("inconsistent storage classes", np, 0, DCLERR);
		  yyval = mkchain(yypvt[-0], 0);
		} break;
case 96:
# line 471 "gram.in"
{ chainp p; struct impldoblock *q;
		q = ALLOC(impldoblock);
		q->tag = TIMPLDO;
		q->varnp = yypvt[-1]->datap;
		p = yypvt[-1]->nextp;
		if(p)  { q->implb = p->datap; p = p->nextp; }
		if(p)  { q->impub = p->datap; p = p->nextp; }
		if(p)  { q->impstep = p->datap; p = p->nextp; }
		frchain( & (yypvt[-1]) );
		yyval = mkchain(q, 0);
		q->datalist = hookup(yypvt[-3], yyval);
		} break;
case 97:
# line 486 "gram.in"
{ curdtp = yypvt[-0]; curdtelt = 0; } break;
case 98:
# line 488 "gram.in"
{ yyval = hookup(yypvt[-2], yypvt[-0]); } break;
case 99:
# line 492 "gram.in"
{ ndim = 0; } break;
case 101:
# line 496 "gram.in"
{ ndim = 0; } break;
case 104:
# line 501 "gram.in"
{ dims[ndim].lb = 0;
		  dims[ndim].ub = yypvt[-0];
		  ++ndim;
		} break;
case 105:
# line 506 "gram.in"
{ dims[ndim].lb = yypvt[-2];
		  dims[ndim].ub = yypvt[-0];
		  ++ndim;
		} break;
case 106:
# line 513 "gram.in"
{ yyval = 0; } break;
case 108:
# line 518 "gram.in"
{ nstars = 1; labarray[0] = yypvt[-0]; } break;
case 109:
# line 520 "gram.in"
{ if(nstars < MAXLABLIST)  labarray[nstars++] = yypvt[-0]; } break;
case 110:
# line 524 "gram.in"
{ if(yypvt[-0]->labinacc)
			error("illegal branch to inner block, statement %s",
				convic( (ftnint) (yypvt[-0]->stateno) ),0,WARN1);
		  else if(yypvt[-0]->labdefined == NO)
			yypvt[-0]->blklevel = blklevel;
		  yypvt[-0]->labused = YES;
		} break;
case 111:
# line 534 "gram.in"
{ yyval = mklabel( convci(toklen, token) ); } break;
case 115:
# line 544 "gram.in"
{ needkwd = 1; } break;
case 116:
# line 545 "gram.in"
{ vartype = yypvt[-0]; } break;
case 119:
# line 553 "gram.in"
{ setimpl(vartype, varleng, yypvt[-0], yypvt[-0]); } break;
case 120:
# line 555 "gram.in"
{ setimpl(vartype, varleng, yypvt[-2], yypvt[-0]); } break;
case 121:
# line 559 "gram.in"
{ if(toklen!=1 || token[0]<'a' || token[0]>'z')
			{
			error("implicit item must be single letter", 0, 0, DCLERR);
			yyval = 0;
			}
		  else yyval = token[0];
		} break;
case 122:
# line 569 "gram.in"
{ switch(parstate)	
			{
			case OUTSIDE:	newproc();
					startproc(0, CLMAIN);
			case INSIDE:	parstate = INDCL;
			case INDCL:	break;

			default:
				error("declaration among executables", 0, 0, DCLERR);
			}
		} break;
case 123:
# line 582 "gram.in"
{ yyval = 0; } break;
case 125:
# line 587 "gram.in"
{ yyval = mkchain(yypvt[-0], 0); } break;
case 126:
# line 589 "gram.in"
{ yyval = hookup(yypvt[-2], mkchain(yypvt[-0],0) ); } break;
case 128:
# line 594 "gram.in"
{ yyval = yypvt[-1]; } break;
case 132:
# line 601 "gram.in"
{ yyval = mkexpr(yypvt[-1], yypvt[-2], yypvt[-0]); } break;
case 133:
# line 603 "gram.in"
{ yyval = mkexpr(OPSTAR, yypvt[-2], yypvt[-0]); } break;
case 134:
# line 605 "gram.in"
{ yyval = mkexpr(OPSLASH, yypvt[-2], yypvt[-0]); } break;
case 135:
# line 607 "gram.in"
{ yyval = mkexpr(OPPOWER, yypvt[-2], yypvt[-0]); } break;
case 136:
# line 609 "gram.in"
{ if(yypvt[-1] == OPMINUS)
			yyval = mkexpr(OPNEG, yypvt[-0], 0);
		  else 	yyval = yypvt[-0];
		} break;
case 137:
# line 614 "gram.in"
{ yyval = mkexpr(yypvt[-1], yypvt[-2], yypvt[-0]); } break;
case 138:
# line 616 "gram.in"
{ yyval = mkexpr(OPEQV, yypvt[-2],yypvt[-0]); } break;
case 139:
# line 618 "gram.in"
{ yyval = mkexpr(OPNEQV, yypvt[-2], yypvt[-0]); } break;
case 140:
# line 620 "gram.in"
{ yyval = mkexpr(OPOR, yypvt[-2], yypvt[-0]); } break;
case 141:
# line 622 "gram.in"
{ yyval = mkexpr(OPAND, yypvt[-2], yypvt[-0]); } break;
case 142:
# line 624 "gram.in"
{ yyval = mkexpr(OPNOT, yypvt[-0], 0); } break;
case 143:
# line 626 "gram.in"
{ yyval = mkexpr(OPCONCAT, yypvt[-2], yypvt[-0]); } break;
case 144:
# line 629 "gram.in"
{ yyval = OPPLUS; } break;
case 145:
# line 630 "gram.in"
{ yyval = OPMINUS; } break;
case 146:
# line 633 "gram.in"
{ yyval = OPEQ; } break;
case 147:
# line 634 "gram.in"
{ yyval = OPGT; } break;
case 148:
# line 635 "gram.in"
{ yyval = OPLT; } break;
case 149:
# line 636 "gram.in"
{ yyval = OPGE; } break;
case 150:
# line 637 "gram.in"
{ yyval = OPLE; } break;
case 151:
# line 638 "gram.in"
{ yyval = OPNE; } break;
case 152:
# line 642 "gram.in"
{ yyval = mkprim(yypvt[-0], 0, 0, 0); } break;
case 153:
# line 644 "gram.in"
{ yyval = mkprim(yypvt[-5], 0, yypvt[-3], yypvt[-1]); } break;
case 154:
# line 646 "gram.in"
{ yyval = mkprim(yypvt[-3], mklist(yypvt[-1]), 0, 0); } break;
case 155:
# line 648 "gram.in"
{ yyval = mkprim(yypvt[-8], mklist(yypvt[-6]), yypvt[-3], yypvt[-1]); } break;
case 156:
# line 652 "gram.in"
{ yyval = 0; } break;
case 158:
# line 657 "gram.in"
{ if(yypvt[-0]->vclass == CLPARAM)
			yyval = cpexpr(yypvt[-0]->paramval);
		} break;
case 160:
# line 663 "gram.in"
{ yyval = mklogcon(1); } break;
case 161:
# line 664 "gram.in"
{ yyval = mklogcon(0); } break;
case 162:
# line 665 "gram.in"
{ yyval = mkstrcon(toklen, token); } break;
case 163:
# line 666 "gram.in"
 { yyval = mkintcon( convci(toklen, token) ); } break;
case 164:
# line 667 "gram.in"
 { yyval = mkrealcon(TYREAL, convcd(toklen, token)); } break;
case 165:
# line 668 "gram.in"
 { yyval = mkrealcon(TYDREAL, convcd(toklen, token)); } break;
case 166:
# line 672 "gram.in"
{ yyval = mkcxcon(yypvt[-3],yypvt[-1]); } break;
case 167:
# line 676 "gram.in"
{ yyval = mkbitcon(4, toklen, token); } break;
case 168:
# line 678 "gram.in"
{ yyval = mkbitcon(3, toklen, token); } break;
case 169:
# line 680 "gram.in"
{ yyval = mkbitcon(1, toklen, token); } break;
case 171:
# line 685 "gram.in"
{ yyval = yypvt[-1]; } break;
case 174:
# line 691 "gram.in"
{ yyval = mkexpr(yypvt[-1], yypvt[-2], yypvt[-0]); } break;
case 175:
# line 693 "gram.in"
{ yyval = mkexpr(OPSTAR, yypvt[-2], yypvt[-0]); } break;
case 176:
# line 695 "gram.in"
{ yyval = mkexpr(OPSLASH, yypvt[-2], yypvt[-0]); } break;
case 177:
# line 697 "gram.in"
{ yyval = mkexpr(OPPOWER, yypvt[-2], yypvt[-0]); } break;
case 178:
# line 699 "gram.in"
{ if(yypvt[-1] == OPMINUS)
			yyval = mkexpr(OPNEG, yypvt[-0], 0);
		  else	yyval = yypvt[-0];
		} break;
case 179:
# line 704 "gram.in"
{ yyval = mkexpr(OPCONCAT, yypvt[-2], yypvt[-0]); } break;
case 181:
# line 708 "gram.in"
{
		if(yypvt[-2]->labdefined)
			error("no backward DO loops",0,0,EXECERR);
		yypvt[-2]->blklevel = blklevel+1;
		exdo(yypvt[-2]->labelno, yypvt[-0]);
		} break;
case 182:
# line 715 "gram.in"
{ exendif();  thiswasbranch = NO; } break;
case 184:
# line 718 "gram.in"
{ exelif(yypvt[-2]); } break;
case 185:
# line 720 "gram.in"
{ exelse(); } break;
case 186:
# line 722 "gram.in"
{ exendif(); } break;
case 187:
# line 726 "gram.in"
{ exif(yypvt[-1]); } break;
case 188:
# line 730 "gram.in"
{ yyval = mkchain(yypvt[-2], yypvt[-0]); } break;
case 189:
# line 734 "gram.in"
{ exequals(yypvt[-2], yypvt[-0]); } break;
case 190:
# line 736 "gram.in"
{ exassign(yypvt[-0], yypvt[-2]); } break;
case 193:
# line 740 "gram.in"
{ inioctl = NO; } break;
case 194:
# line 742 "gram.in"
{ exarif(yypvt[-6], yypvt[-4], yypvt[-2], yypvt[-0]);  thiswasbranch = YES; } break;
case 195:
# line 744 "gram.in"
{ excall(yypvt[-0], 0, 0, labarray); } break;
case 196:
# line 746 "gram.in"
{ excall(yypvt[-2], 0, 0, labarray); } break;
case 197:
# line 748 "gram.in"
{ if(nstars < MAXLABLIST)
			excall(yypvt[-3], mklist(yypvt[-1]), nstars, labarray);
		  else
			error("too many alternate returns",0,0,ERR);
		} break;
case 198:
# line 754 "gram.in"
{ exreturn(yypvt[-0]);  thiswasbranch = YES; } break;
case 199:
# line 756 "gram.in"
{ exstop(yypvt[-2], yypvt[-0]);  thiswasbranch = yypvt[-2]; } break;
case 200:
# line 760 "gram.in"
{ if(parstate == OUTSIDE)
			{
			newproc();
			startproc(0, CLMAIN);
			}
		} break;
case 201:
# line 769 "gram.in"
{ exgoto(yypvt[-0]);  thiswasbranch = YES; } break;
case 202:
# line 771 "gram.in"
{ exasgoto(yypvt[-0]);  thiswasbranch = YES; } break;
case 203:
# line 773 "gram.in"
{ exasgoto(yypvt[-4]);  thiswasbranch = YES; } break;
case 204:
# line 775 "gram.in"
{ if(nstars < MAXLABLIST)
			putcmgo(fixtype(yypvt[-0]), nstars, labarray);
		  else
			error("computed GOTO list too long",0,0,ERR);
		} break;
case 207:
# line 787 "gram.in"
{ nstars = 0; yyval = yypvt[-0]; } break;
case 208:
# line 791 "gram.in"
{ yyval = (yypvt[-0] ? mkchain(yypvt[-0],0) : 0); } break;
case 209:
# line 793 "gram.in"
{ if(yypvt[-0])
			if(yypvt[-2]) yyval = hookup(yypvt[-2], mkchain(yypvt[-0],0));
			else yyval = mkchain(yypvt[-0],0);
		} break;
case 211:
# line 801 "gram.in"
{ if(nstars<MAXLABLIST) labarray[nstars++] = yypvt[-0]; yyval = 0; } break;
case 212:
# line 805 "gram.in"
{ yyval = 0; } break;
case 213:
# line 807 "gram.in"
{ yyval = 1; } break;
case 214:
# line 811 "gram.in"
{ yyval = mkchain(yypvt[-0], 0); } break;
case 215:
# line 813 "gram.in"
{ yyval = hookup(yypvt[-2], mkchain(yypvt[-0],0) ); } break;
case 216:
# line 817 "gram.in"
{ if(parstate == OUTSIDE)
			{
			newproc();
			startproc(0, CLMAIN);
			}
		  if(parstate < INDATA) enddcl();
		} break;
case 217:
# line 827 "gram.in"
{ intonly = YES; } break;
case 218:
# line 831 "gram.in"
{ intonly = NO; } break;
case 219:
# line 836 "gram.in"
{ endio(); } break;
case 221:
# line 841 "gram.in"
{ ioclause(IOSUNIT, yypvt[-0]); endioctl(); } break;
case 223:
# line 844 "gram.in"
{ doio(NULL); } break;
case 224:
# line 846 "gram.in"
{ doio(yypvt[-0]); } break;
case 225:
# line 848 "gram.in"
{ doio(yypvt[-0]); } break;
case 226:
# line 850 "gram.in"
{ doio(yypvt[-0]); } break;
case 227:
# line 852 "gram.in"
{ doio(yypvt[-0]); } break;
case 228:
# line 854 "gram.in"
{ doio(NULL); } break;
case 229:
# line 856 "gram.in"
{ doio(yypvt[-0]); } break;
case 230:
# line 858 "gram.in"
{ doio(NULL); } break;
case 231:
# line 860 "gram.in"
{ doio(yypvt[-0]); } break;
case 232:
# line 862 "gram.in"
{ doio(NULL); } break;
case 233:
# line 864 "gram.in"
{ doio(yypvt[-0]); } break;
case 235:
# line 871 "gram.in"
{ iostmt = IOBACKSPACE; } break;
case 236:
# line 873 "gram.in"
{ iostmt = IOREWIND; } break;
case 237:
# line 875 "gram.in"
{ iostmt = IOENDFILE; } break;
case 239:
# line 882 "gram.in"
{ iostmt = IOINQUIRE; } break;
case 240:
# line 884 "gram.in"
{ iostmt = IOOPEN; } break;
case 241:
# line 886 "gram.in"
{ iostmt = IOCLOSE; } break;
case 242:
# line 890 "gram.in"
{
		ioclause(IOSUNIT, NULL);
		ioclause(IOSFMT, yypvt[-0]);
		endioctl();
		} break;
case 243:
# line 896 "gram.in"
{
		ioclause(IOSUNIT, NULL);
		ioclause(IOSFMT, NULL);
		endioctl();
		} break;
case 244:
# line 904 "gram.in"
{ ioclause(IOSUNIT, yypvt[-1]); endioctl(); } break;
case 245:
# line 906 "gram.in"
{ endioctl(); } break;
case 248:
# line 914 "gram.in"
{ ioclause(IOSPOSITIONAL, yypvt[-0]); } break;
case 249:
# line 916 "gram.in"
{ ioclause(IOSPOSITIONAL, NULL); } break;
case 250:
# line 918 "gram.in"
{ ioclause(yypvt[-1], yypvt[-0]); } break;
case 251:
# line 920 "gram.in"
{ ioclause(yypvt[-1], NULL); } break;
case 252:
# line 924 "gram.in"
{ yyval = iocname(); } break;
case 253:
# line 928 "gram.in"
{					/* E-D */
		iosetecdc( yypvt[-5] );			/* E-D */
		ioclause( IOSUNIT, yypvt[-1] );		/* E-D */
		ioclause(  IOSFMT, yypvt[-3] );		/* E-D */
		endioctl();				/* E-D */
		} break;
case 254:
# line 937 "gram.in"
{ iostmt = IOREAD; } break;
case 255:
# line 941 "gram.in"
{ iostmt = IOWRITE; } break;
case 256:
# line 945 "gram.in"
{ iostmt = IOREAD; } break;
case 257:
# line 949 "gram.in"
{ iostmt = IOWRITE; } break;
case 258:
# line 953 "gram.in"
{
		iostmt = IOWRITE;
		ioclause(IOSUNIT, NULL);
		ioclause(IOSFMT, yypvt[-1]);
		endioctl();
		} break;
case 259:
# line 960 "gram.in"
{
		iostmt = IOWRITE;
		ioclause(IOSUNIT, NULL);
		ioclause(IOSFMT, NULL);
		endioctl();
		} break;
case 260:
# line 969 "gram.in"
{ yyval = mkchain(yypvt[-0],0); } break;
case 261:
# line 971 "gram.in"
{ yyval = hookup(yypvt[-2], mkchain(yypvt[-0],0)); } break;
case 263:
# line 976 "gram.in"
{ yyval = mkiodo(yypvt[-1],yypvt[-3]); } break;
case 264:
# line 980 "gram.in"
{ yyval = mkchain(yypvt[-0], 0); } break;
case 265:
# line 982 "gram.in"
{ yyval = mkchain(yypvt[-0], 0); } break;
case 267:
# line 987 "gram.in"
{ yyval = mkchain(yypvt[-2], mkchain(yypvt[-0], 0) ); } break;
case 268:
# line 989 "gram.in"
{ yyval = mkchain(yypvt[-2], mkchain(yypvt[-0], 0) ); } break;
case 269:
# line 991 "gram.in"
{ yyval = mkchain(yypvt[-2], mkchain(yypvt[-0], 0) ); } break;
case 270:
# line 993 "gram.in"
{ yyval = mkchain(yypvt[-2], mkchain(yypvt[-0], 0) ); } break;
case 271:
# line 995 "gram.in"
{ yyval = hookup(yypvt[-2], mkchain(yypvt[-0], 0) ); } break;
case 272:
# line 997 "gram.in"
{ yyval = hookup(yypvt[-2], mkchain(yypvt[-0], 0) ); } break;
case 274:
# line 1002 "gram.in"
{ yyval = mkiodo(yypvt[-1], mkchain(yypvt[-3], 0) ); } break;
case 275:
# line 1004 "gram.in"
{ yyval = mkiodo(yypvt[-1], mkchain(yypvt[-3], 0) ); } break;
case 276:
# line 1006 "gram.in"
{ yyval = mkiodo(yypvt[-1], yypvt[-3]); } break;
case 277:
# line 1010 "gram.in"
{ startioctl(); } break;
		}
		goto yystack;  /* stack new state and value */

	}
