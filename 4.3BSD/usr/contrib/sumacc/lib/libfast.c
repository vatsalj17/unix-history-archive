/*	libfast.c	1.0	05/25/84	*/

/*
 * Lint library for "fast" routines (those directly callable
 * from C).
 *
 * Copyright (C) 1984, Stanford Univ. SUMEX project.
 * May be used but not sold without permission.
 */

/*
 * history
 * 05/25/84	Croft	Created C version.
 */

/* LINTLIBRARY */

#include "libmac.h"


/*
 * String conversion utilities (strconv.s).
 */

void	c2pstr(cp) char *cp; { }
void	p2cstr(cp) char *cp; { }
char	*cofpstr(cp) char *cp; { return((char *)0); }
char	*isapstr(cp) char *cp; { return((char *)0); }


/*
 * SANE and ELEMS library.
 */

#include "sane.h"

/* Two address, extended-based arithmetic operations. */

void	AddS(x,y) Single *x; Extended *y; { }
void	AddD(x,y) Double *x; Extended *y; { }
void	AddC(x,y) Comp *x; Extended *y; { }
void	AddX(x,y) Extended *x; Extended *y; { }
            /* y := y + x */

void	SubS(x,y) Single *x; Extended *y; { }
void	SubD(x,y) Double *x; Extended *y; { }
void	SubC(x,y) Comp *x; Extended *y; { }
void	SubX(x,y) Extended *x; Extended *y; { }
            /* y := y - x */

void	MulS(x,y) Single *x; Extended *y; { }
void	MulD(x,y) Double *x; Extended *y; { }
void	MulC(x,y) Comp *x; Extended *y; { }
void	MulX(x,y) Extended *x; Extended *y; { }
            /* y := y * x */

void	DivS(x,y) Single *x; Extended *y; { }
void	DivD(x,y) Double *x; Extended *y; { }
void	DivC(x,y) Comp *x; Extended *y; { }
void	DivX(x,y) Extended *x; Extended *y; { }
            /* y := y / x */

int	CmpX(x,relop,y) Extended *x,*y; { RI }
            /* returns boolean; x r y */

int	RelX(x,y) Extended *x,*y; { RI }
            /* returns RelX;  x RelX y, where RelX in [GT, LT, EQ, UNORD] */

/*
 * Conversions between Extended and the other numeric types,
 * including the types integer and Longint.
 */

void	I2X(x,y) short *x; Extended *y; { }
void	L2X(x,y) int *x; Extended *y; { }
void	S2X(x,y) Single *x; Extended *y; { }
void	D2X(x,y) Double *x; Extended *y; { }
void	C2X(x,y) Comp *x; Extended *y; { }
void	X2X(x,y) Extended *x; Extended *y; { }
            /* y := x (arithmetic assignment) */

void	X2I(x,y) Extended *x; short *y; { }
void	X2L(x,y) Extended *x; int *y; { }
void	X2S(x,y) Extended *x; Single *y; { }
void	X2D(x,y) Extended *x; Double *y; { }
void	X2C(x,y) Extended *x; Comp *y; { }
            /* y := x (arithmetic assignment) */

/*
 * Conversions between the numeric types and the intermediate
 * decimal type.
 */

void	S2Dec(f,x,y) DecForm *f; Single *x; Decimal *y; { }
void	D2Dec(f,x,y) DecForm *f; Double *x; Decimal *y; { }
void	C2Dec(f,x,y) DecForm *f; Comp *x; Decimal *y; { }
void	X2Dec(f,x,y) DecForm *f; Extended *x; Decimal *y; { }
            /* y := x (according to the format f) */

void	Dec2S(x,y) Decimal *x; Single *y; { }
void	Dec2D(x,y) Decimal *x; Double *y; { }
void	Dec2C(x,y) Decimal *x; Comp *y; { }
void	Dec2X(x,y) Decimal *x; Extended *y; { }
            /* y := x */

/*
 * Conversions between the numeric types and strings.
 * (These conversions have a built-in scanner/parser to convert
 * between the intermediate decimal type and a string.)
 */

void	S2Str(f,x,y) DecForm *f; Single *x; char *y; { }
void	D2Str(f,x,y) DecForm *f; Double *x; char *y; { }
void	C2Str(f,x,y) DecForm *f; Comp *x; char *y; { }
void	X2Str(f,x,y) DecForm *f; Extended *x; char *y; { }
            /* y := x (according to the format f) */

void	Str2S(x,y) char *x; Single *y; { }
void	Str2D(x,y) char *x; Double *y; { }
void	Str2C(x,y) char *x; Comp *y; { }
void	Str2X(x,y) char *x; Extended *y; { }
            /* y := x */

/*
 * Numerical 'library' procedures and functions.
 */

void	RemX(x,y,quo) Extended *x,*y; int *quo; { }
            /* new y := remainder of ((old y) / x), such that
                       |new y| <= |x| / 2;
              quo := low order seven bits of integer quotient y / x,
                     so that -127 <= quo <= 127.                     */
void	SqrtX(x) Extended *x; { }
            /* x := sqrt (x) */
void	RintX(x) Extended *x; { }
            /* x := rounded value of x */
void	NegX(x) Extended *x; { }
            /* x := -x */
void	AbsX(x) Extended *x; { }
            /* x := |x| */
void	CpySgnX(x,y) Extended *x,*y; { }
            /* x := x with the sign of y */

void	NextS(x,y) Single *x,*y; { }
void	NextD(x,y) Double *x,*y; { }
void	NextX(x,y) Extended *x,*y; { }
            /* x := next representable value from x toward y */

int	ClassS(x,sgn) Single *x; int *sgn; { RI }
int	ClassD(x,sgn) Double *x; int *sgn; { RI }
int	ClassC(x,sgn) Comp *x; int *sgn; { RI }
int	ClassX(x,sgn) Extended *x; int *sgn; { RI }
            /* returns NumClass; sgn := sign of x (0 for pos, 1 for neg) */

void	ScalbX(n,y) short *n; Extended *y; { }
            /* y := y * 2^n */
void	LogbX(x) Extended *x; { }
            /* returns unbiased exponent of x */

/*
 * Manipulations of the static numeric state.
 */

void	SetRnd(rounddir) { }
void	SetEnv(e) Environ *e; { }
void	ProcExit(e) Environ *e; { }

int	GetRnd() { RI }	/* returns rounddir */
void	GetEnv(e) Environ *e; { }
void	ProcEntry(e) Environ *e; { }

int	TestXcp(exception) { RI }
void	SetXcp(exception,onoff) { }
int	TestHlt(exception) { RI }
void	SetHlt(exception,onoff) { }


/*
 * "Elems" package
 */

void	Log2X(x) Extended *x; { }
            /* x := log2 (x) */
void	LnX(x) Extended *x; { }
            /* x := ln (x) */
void	Ln1X(x) Extended *x; { }
            /* x := ln (1 + x) */
void	Exp2X(x) Extended *x; { }
            /* x := 2^x */
void	ExpX(x) Extended *x; { }
            /* x := e^x */
void	Exp1X(x) Extended *x; { }
            /* x := e^x - 1 */
void	XpwrI(i,x) short *i; Extended *x; { }
            /* x := x^i */
void	XpwrY(y,x) Extended *y,*x; { }
            /* x := x^y */
void	Compound(r,n,x) Extended *r,*n,*x; { }
            /* x := (1 + r)^n */
void	Annuity(r,n,x) Extended *r,*n,*x; { }
            /* x := (1 - (1 + r)^-n) / r */
void	SinX(x) Extended *x; { }
            /* x := sin(x) */
void	CosX(x) Extended *x; { }
            /* x := cos(x) */
void	TanX(x) Extended *x; { }
            /* x := tan(x) */
void	AtanX(x) Extended *x; { }
            /* x := atan(x) */
void	NextRandom(x) Extended *x; { }
            /* x := next random (x) */
