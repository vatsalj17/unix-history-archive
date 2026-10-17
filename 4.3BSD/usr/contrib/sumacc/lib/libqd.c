/*	libqd.c		1.0	84/05/01	*/

/*
 * Quickdraw library.
 *
 * Copyright (C) 1984, Stanford Univ. SUMEX project.
 * May be used but not sold without permission.
 */

/*
 * history
 * 05/01/84	Croft	Created.
 */

/* LINTLIBRARY */

#include "quickdraw.h"
#undef v
#undef h
#include "libmac.h"

/*
 * QUICKDRAW
 */

/* GrafPort Routines */

void	InitGraf(g) GrafPtr *g; { _mact1(RN,0xa86e,N,L) }
void	OpenPort(p) GrafPtr p; { _mact1(RN,0xa86f,N,L) }
void	InitPort(p) GrafPtr p; { _mact1(RN,0xa86d,N,L) }
void	ClosePort(p) GrafPtr p; { _mact1(RN,0xa87d,N,L) }
void	SetPort(p) GrafPtr p; { _mact1(RN,0xa873,N,L) }
void	GetPort(p) GrafPtr *p; { _mact1(RN,0xa874,N,L) }
void	GrafDevice(device) { _mact1(RN,0xa872,N,S) }
void	SetPortBits(bm) BitMap *bm; { _mact1(RN,0xa875,N,L) }
void	PortSize(w,h) { _mact2(RN,0xa876,N,S,S) }
void	MovePortTo(leftg,topg) { _mact2(RN,0xa877,N,S,S) }
void	SetOrigin(h,v) { _mact2(RN,0xa878,N,S,S) }
void	SetClip(rgn) RgnHandle rgn; { _mact1(RN,0xa879,N,L) }
void	GetClip(rgn) RgnHandle rgn; { _mact1(RN,0xa87a,N,L) }
void	ClipRect(r) Rect *r; { _mact1(RN,0xa87b,N,L) }
void	BackPat(pat) Pattern *pat; { _mact1(RN,0xa87c,N,L) }

/* Cursor Routines */

void	InitCursor() { _mact1(RN,0xa850,N,N) }
void	SetCursor(c) Cursor *c; { _mact1(RN,0xa851,N,L) }
void	HideCursor() { _mact1(RN,0xa852,N,N) }
void	ShowCursor() { _mact1(RN,0xa853,N,N) }
void	ObscureCursor() { _mact1(RN,0xa856,N,N) }

/* Line Routines */

void	HidePen() { _mact1(RN,0xa896,N,N) }
void	ShowPen() { _mact1(RN,0xa897,N,N) }
void	GetPen(pt) Point *pt; { _mact1(RN,0xa89a,N,L) }
void	GetPenState(ps) PenState *ps; { _mact1(RN,0xa898,N,L) }
void	SetPenState(ps) PenState *ps; { _mact1(RN,0xa899,N,L) }
void	PenSize(w,h) { _mact2(RN,0xa89b,N,S,S) }
void	PenMode(mode) { _mact1(RN,0xa89c,N,S) }
void	PenPat(pat) Pattern *pat; { _mact1(RN,0xa89d,N,L) }
void	PenNormal() { _mact1(RN,0xa89e,N,N) }
void	MoveTo(h,v) { _mact2(RN,0xa893,N,S,S) }
void	Move(dh,dv) { _mact2(RN,0xa894,N,S,S) }
void	LineTo(h,v) { _mact2(RN,0xa891,N,S,S) }
void	Line(dh,dv) { _mact2(RN,0xa892,N,S,S) }

/* Text Routines */

void	TextFont(font) { _mact1(RN,0xa887,N,S) }
void	TextFace(face) { _mact1(RN,0xa888,N,S) }
void	TextMode(mode) { _mact1(RN,0xa889,N,S) }
void	TextSize(size) { _mact1(RN,0xa88a,N,S) }
void	SpaceExtra(extra) { _mact1(RN,0xa88e,N,L) }
void	DrawChar(c) { _mact1(RN,0xa883,N,S) }
void	DrawString(s) char *s; { _mact1(RN,0xa884,N,PS) }
void	DrawText(cp,f,c) char *cp; { _mact3(RN,0xa885,N,L,S,S) }
int	CharWidth(c) { _mact1(RI,0xa88d,S,S) }
int	StringWidth(s) char *s; { _mact1(RI,0xa88c,S,PS) }
int	TextWidth(cp,f,c) char *cp; { _mact3(RI,0xa886,S,L,S,S) }
void	GetFontInfo(info) FontInfo *info; { _mact1(RN,0xa88b,N,L) }

/* Point Calculations */

void	AddPt(src,dst) Point *src,*dst; { _mact2(RN,0xa87e,N,SS,L) }
void	SubPt(src,dst) Point *src,*dst; { _mact2(RN,0xa87f,N,SS,L) }
void	SetPt(pt,h,v) Point *pt; { _mact3(RN,0xa880,N,L,S,S) }
int	EqualPt(p1,p2) Point *p1,*p2; { _mact2(RI,0xa881,B,SS,SS) }
void	ScalePt(pt,from,to) Point *pt; Rect *from,*to; { _mact3(RN,0xa8f8,N,L,L,L) }
void	MapPt(pt,from,to) Point *pt; Rect *from,*to; { _mact3(RN,0xa8f9,N,L,L,L) }
void	LocalToGlobal(pt) Point *pt; { _mact1(RN,0xa870,N,L) }
void	GlobalToLocal(pt) Point *pt; { _mact1(RN,0xa871,N,L) }

/* Rectangle Calculations */

void	SetRect(rt,l,t,r,b) Rect *rt; { _mact5(RN,0xa8a7,N,L,S,S,S,S) } /* a macro would be better */
int	EqualRect(r1,r2) Rect *r1,*r2; { _mact2(RI,0xa8a6,B,L,L) }
int	EmptyRect(r) Rect *r; { _mact1(RI,0xa8ae,B,L) }
void	OffsetRect(r,dh,dv) Rect *r; { _mact3(RN,0xa8a8,N,L,S,S) }
void	MapRect(r,from,to) Rect *r,*from,*to; { _mact3(RN,0xa8fa,N,L,L,L) }
void	InsetRect(r,dh,dv) Rect *r; { _mact3(RN,0xa8a9,N,L,S,S) }
int	SectRect(sr1,sr2,dr) Rect *sr1,*sr2,*dr; { _mact3(RI,0xa8aa,B,L,L,L) }
void	UnionRect(sr1,sr2,dr) Rect *sr1,*sr2,*dr; { _mact3(RN,0xa8ab,N,L,L,L) }
int	PtInRect(pt,r) Point *pt; Rect *r; { _mact2(RI,0xa8ad,B,SS,L) }
void	Pt2Rect(p1,p2,r) Point *p1,*p2; Rect *r; { _mact3(RN,0xa8ac,N,SS,SS,L) }

/* Graphical Operations on Rectangles */

void	FrameRect(r) Rect *r; { _mact1(RN,0xa8a1,N,L) }
void	PaintRect(r) Rect *r; { _mact1(RN,0xa8a2,N,L) }
void	EraseRect(r) Rect *r; { _mact1(RN,0xa8a3,N,L) }
void	InvertRect(r) Rect *r; { _mact1(RN,0xa8a4,N,L) }
void	FillRect(r,pat) Rect *r; Pattern *pat; { _mact2(RN,0xa8a5,N,L,L) }

/* RoundRect Routines */

void	FrameRoundRect(r,w,h) Rect *r; { _mact3(RN,0xa8b0,N,L,S,S) }
void	PaintRoundRect(r,w,h) Rect *r; { _mact3(RN,0xa8b1,N,L,S,S) }
void	EraseRoundRect(r,w,h) Rect *r; { _mact3(RN,0xa8b2,N,L,S,S) }
void	InvertRoundRect(r,w,h) Rect *r; { _mact3(RN,0xa8b3,N,L,S,S) }
void	FillRoundRect(r,w,h,p) Rect *r; Pattern *p; { _mact4(RN,0xa8b4,N,L,S,S,L) }

/* Oval Routines */

void	FrameOval(r) Rect *r; { _mact1(RN,0xa8b7,N,L) }
void	PaintOval(r) Rect *r; { _mact1(RN,0xa8b8,N,L) }
void	EraseOval(r) Rect *r; { _mact1(RN,0xa8b9,N,L) }
void	InvertOval(r) Rect *r; { _mact1(RN,0xa8ba,N,L) }
void	FillOval(r,pat) Rect *r; Pattern *pat; { _mact2(RN,0xa8bb,N,L,L) }

/* Arc Routines */

void	FrameArc(r,start,arc) Rect *r; { _mact3(RN,0xa8be,N,L,S,S) }
void	PaintArc(r,start,arc) Rect *r; { _mact3(RN,0xa8bf,N,L,S,S) }
void	EraseArc(r,start,arc) Rect *r; { _mact3(RN,0xa8c0,N,L,S,S) }
void	InvertArc(r,start,arc) Rect *r; { _mact3(RN,0xa8c1,N,L,S,S) }
void	FillArc(r,start,arc,p) Rect *r; Pattern *p; { _mact4(RN,0xa8c2,N,L,S,S,L) }
void	PtToAngle(r,p,a) Rect *r; Point *p; int *a; { _mact3(RN,0xa8c3,N,L,SS,VS) }

/* Polygon Routines */

PolyHandle OpenPoly() { _mact1(R((PolyHandle)0),0xa8cb,L,N) }
void	ClosePoly() { _mact1(RN,0xa8cc,N,N) }
void	KillPoly(p) PolyHandle p; { _mact1(RN,0xa8cd,N,L) }
void	OffsetPoly(p,dh,dv) PolyHandle p; { _mact3(RN,0xa8ce,N,L,S,S) }
void	MapPoly(p,f,t) PolyHandle p; Rect *f,*t; { _mact3(RN,0xa8fc,N,L,L,L) }
void	FramePoly(p) PolyHandle p; { _mact1(RN,0xa8c6,N,L) }
void	PaintPoly(p) PolyHandle p; { _mact1(RN,0xa8c7,N,L) }
void	ErasePoly(p) PolyHandle p; { _mact1(RN,0xa8c8,N,L) }
void	InvertPoly(p) PolyHandle p; { _mact1(RN,0xa8c9,N,L) }
void	FillPoly(p,pat) PolyHandle p; Pattern *pat; { _mact2(RN,0xa8ca,N,L,L) }

/* Region Calculations */

RgnHandle NewRgn() { _mact1(R((RgnHandle)0),0xa8d8,L,N) }
void	DisposeRgn(r) RgnHandle r; { _mact1(RN,0xa8d9,N,L) }
void	CopyRgn(s,d) RgnHandle s,d; { _mact2(RN,0xa8dc,N,L,L) }
void	SetEmptyRgn(r) RgnHandle r; { _mact1(RN,0xa8dd,N,L) }
void	SetRectRgn(rg,l,t,r,b) RgnHandle rg; { _mact5(RN,0xa8de,N,L,S,S,S,S) }
void	RectRgn(rg,r) RgnHandle rg; Rect *r; { _mact2(RN,0xa8df,N,L,L) }
void	OpenRgn() { _mact1(RN,0xa8da,N,N) }
void	CloseRgn(r) RgnHandle r; { _mact1(RN,0xa8db,N,L) }
void	OffsetRgn(r,dh,dv) RgnHandle r; { _mact3(RN,0xa8e0,N,L,S,S) }
void	MapRgn(r,f,t) RgnHandle r; Rect *f,*t; { _mact3(RN,0xa8fb,N,L,L,L) }
void	InsetRgn(r,dh,dv) RgnHandle r; { _mact3(RN,0xa8e1,N,L,S,S) }
void	SectRgn(sr1,sr2,dr) RgnHandle sr1,sr2,dr; { _mact3(RN,0xa8e4,N,L,L,L) }
void	UnionRgn(sr1,sr2,dr) RgnHandle sr1,sr2,dr; { _mact3(RN,0xa8e5,N,L,L,L) }
void	DiffRgn(sr1,sr2,dr) RgnHandle sr1,sr2,dr; { _mact3(RN,0xa8e6,N,L,L,L) }
void	XorRgn(sr1,sr2,dr) RgnHandle sr1,sr2,dr; { _mact3(RN,0xa8e7,N,L,L,L) }
int	EqualRgn(r1,r2) RgnHandle r1,r2; { _mact2(RI,0xa8e3,B,L,L) }
int	EmptyRgn(r) RgnHandle r; { _mact1(RI,0xa8e2,B,L) }
int	PtInRgn(p,r) Point *p; RgnHandle r; { _mact2(RI,0xa8e8,B,SS,L) }
int	RectInRgn(r,rg) Rect *r; RgnHandle rg; { _mact2(RI,0xa8e9,B,L,L) }

/* Graphical Operations on Regions */

void	FrameRgn(r) RgnHandle r; { _mact1(RN,0xa8d2,N,L) }
void	PaintRgn(r) RgnHandle r; { _mact1(RN,0xa8d3,N,L) }
void	EraseRgn(r) RgnHandle r; { _mact1(RN,0xa8d4,N,L) }
void	InvertRgn(r) RgnHandle r; { _mact1(RN,0xa8d5,N,L) }
void	FillRgn(r,pat) RgnHandle r; Pattern *pat; { _mact2(RN,0xa8d6,N,L,L) }

/* Graphical Operations on BitMaps */

void	ScrollRect(r,dh,dv,rg) Rect *r; RgnHandle rg; { _mact4(RN,0xa8ef,N,L,S,S,L) }
void	CopyBits(sb,db,sr,dr,m,rg) BitMap *sb,*db; Rect *sr,*dr; RgnHandle rg; { _mact6(RN,0xa8ec,N,L,L,L,L,S,L) }

/* Picture Routines */

PicHandle OpenPicture(r) Rect *r; { _mact1(R((PicHandle)0),0xa8f3,L,L) }
void	ClosePicture() { _mact1(RN,0xa8f4,N,N) }
void	DrawPicture(p,r) PicHandle p; Rect *r; { _mact2(RN,0xa8f6,N,L,L) }
void	PicComment(kind,size,cp) char *cp; { _mact3(RN,0xa8f2,N,S,S,L) }
void	KillPicture(p) PicHandle p; { _mact1(RN,0xa8f5,N,L) }

/*  The Bottleneck Interface:   */

void	SetStdProcs(qp) QDProcs *qp; { _mact1(RN,0xa8ea,N,L) }
void	StdText(cnt,cp,num,dem) char *cp; Point *num,*dem; { _mact4(RN,0xa882,N,S,L,SS,SS) }
void	StdLine(p) Point *p; { _mact1(RN,0xa890,N,SS) }
void	StdRect(v,r) Rect *r; { _mact2(RN,0xa8a0,N,S,L) }
void	StdRRect(v,r,w,h) Rect *r; { _mact4(RN,0xa8af,N,S,L,S,S) }
void	StdOval(v,r) Rect *r; { _mact2(RN,0xa8b6,N,S,L) }
void	StdArc(v,r,s,a) Rect *r; { _mact4(RN,0xa8bd,N,S,L,S,S) }
void	StdPoly(v,p) PolyHandle p; { _mact2(RN,0xa8c5,N,S,L) }
void	StdRgn(v,r) RgnHandle r; { _mact2(RN,0xa8d1,N,S,L) }
void	StdBits(sb,sr,dr,m,rg) BitMap *sb; Rect *sr,*dr; RgnHandle rg; { _mact5(RN,0xa8eb,N,L,L,L,S,L) }
void	StdComment(kind,size,cp) char *cp; { _mact3(RN,0xa8f1,N,S,S,L) }
int	StdTxMeas(cnt,cp,num,dem,info) char *cp; Point *num,*dem; FontInfo *info; { _mact5(RN,0xa8ed,S,S,L,L,L,L) }
void	StdGetPic(cp,cnt) char *cp; { _mact2(RN,0xa8ee,N,L,S) }
void	StdPutPic(cp,cnt) char *cp; { _mact2(RN,0xa8f0,N,L,S) }

/* Misc Utility Routines */

int	GetPixel(h,v) { _mact2(RI,0xa865,B,S,S) }
int	Random() { _mact1(RI,0xa861,S,N) }
void	StuffHex(cp,s) { _mact2(RN,0xa866,N,L,PS) }
void	ForeColor(color) { _mact1(RN,0xa862,N,L) }
void	BackColor(color) { _mact1(RN,0xa863,N,L) }
void	ColorBit(bit) { _mact1(RN,0xa864,N,S) }

