/*	libos.c		1.0	84/05/30	*/

/*
 * Package library.
 *
 * Copyright (C) 1984, Stanford Univ. SUMEX project.
 * May be used but not sold without permission.
 */

/*
 * history
 * 05/30/84	Croft	Created.
 */

/* LINTLIBRARY */

#include "quickdraw.h"
#undef v
#undef h
#include "osintf.h"
#include "packintf.h"
#include "libmac.h"


/*Standard File*/

void	SFPutFile(p,pr,n,dlg,rep) Point *p; char *pr,*n; ProcPtr dlg; SFReply *rep; { _macc5(RN,_sfputfil,N,SS,PS,PS,L,L) }

void	SFPPutFile(p,pr,n,dlg,rep,dlgid,fil) Point *p; char *pr,*n; ProcPtr dlg,fil; SFReply *rep; { _macc7(RN,_sfpputfi,N,SS,PS,PS,L,L,S,L) }

void	SFGetFile(p,pr,fil,nt,t,dlg,rep) Point *p; char *pr,*t; ProcPtr fil,dlg; SFReply *rep; { _macc7(RN,_sfgetfil,N,SS,PS,L,S,L,L,L) }

void	SFPGetFile(p,pr,ff,nt,t,dlg,rep,dlgid,fp) Point *p; char *pr,*t; ProcPtr ff,dlg,fp; SFReply *rep; { _macc9(RN,_sfpgetfi,N,SS,PS,L,S,L,L,L,S,L) }

/*interface to Disk Initialization Package*/

void	DILoad() { _macc1(RN,_diload,N,N) }
void	DIUnLoad() { _macc1(RN,_diunload,N,N) }
int	DIBadMount(p,mes) Point *p; { _macc2(RI,_dibadmou,S,SS,L) }
int	DIFormat(drv) { _macc1(RI,_diformat,S,S) }
int	DIVerify(drv) { _macc1(RI,_diverify,S,S) }
int	DIZero(drv,vol) char *vol; { _macc2(RI,_dizero,S,S,PS) }

/*interface to International Utilities*/

Handle	IUGetIntl(id) { _macc1(RI,_iugetint,L,S) }
void	IUSetIntl(rn,id,ip) Handle ip; { _macc3(RN,_iusetint,N,S,S,L) }
void	IUDateString(dt,lf,res) char *res; { _macc3(RN,_iudatest,N,L,S,VPS) }
void	IUDatePString(dt,lf,res,ip) char *res; Handle ip; { _macc4(RN,_iudateps,N,L,S,VPS,L) }
void	IUTimeString(dt,s,res) char *res; { _macc3(RN,_iutimest,N,L,B,VPS) }
void	IUTimePString(dt,s,res,ip) char *res; Handle ip; { _macc4(RN,_iutimeps,N,L,B,VPS,L) }

int	IUMetric() { _macc1(RI,_iumetric,B,N) }
int	IUCompString(a,b) char *a,*b; { _macc2(RI,_iucompst,S,PS,PS) }
int	IUEqualString(a,b) char *a,*b; { _macc2(RI,_iuequals,S,PS,PS) }
int	IUMagString(a,b,al,bl) Ptr a,b; { _macc4(RI,_iumagstr,S,L,L,S,S) }
int	IUMagIDString(a,b,al,bl) Ptr a,b; { _macc4(RI,_iumagids,S,L,L,S,S) }

/*interface to Binary-Decimal Conversion*/

void	StringToNum(s,n) char *s; int *n; { _macc2(RN,_stringto,N,PS,L) }
void	NumToString(n,s) char *s; { _macc2(RN,_numtostr,N,L,VPS) }
