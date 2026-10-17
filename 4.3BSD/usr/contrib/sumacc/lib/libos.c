/*	libos.c		1.0	84/05/01	*/

/*
 * OS library.
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
#include "osintf.h"
#include "libmac.h"


/*
 * OS INTERFACE
 */

/* Memory Manager */

void	SetApplBase(b) Ptr b; { _macc1(RN,_setapplb,N,L) }
void	InitApplZone() { _macc1(RN,_initappl,N,N) }
void	MaxApplZone() { _macc1(RN,_maxapplz,N,N) }
void	InitZone(g,m,l,s) ProcPtr g; Ptr l,s; { _macc4(RN,_initzone,N,L,S,L,L) }
THz	GetZone() { _macc1(RI,_getzone,L,N) }
void	SetZone(hz) THz hz; { _macc1(RN,_setzone,N,L) }

THz	ApplicZone() { _macc1(RI,_appliczo,L,N) }
THz	SystemZone() { _macc1(RI,_systemzo,L,N) }

int	CompactMem(n) { _macc1(RI,_compactm,L,L) }
void	PurgeMem(n) { _macc1(RN,_purgemem,N,L) }
int	FreeMem() { _macc1(RI,_freemem,L,N) }
void	ResrvMem(n) { _macc1(RN,_resrvmem,N,L) }
int	MaxMem(g) int *g; { _macc1(RI,_maxmem,L,L) }
Ptr	TopMem() { _macc1(RI,_topmem,L,N) }

void	SetGrowZone(g) ProcPtr g; { _macc1(RN,_setgrowz,N,L) }
void	SetApplLimit(l) Ptr l; { _macc1(RN,_setappll,N,L) }

Ptr	NewPtr(c) { _macc1(RI,_newptr,L,L) }
void	DisposPtr(p) Ptr p; { _macc1(RN,_dispospt,N,L) }
int	GetPtrSize(p) Ptr p; { _macc1(RI,_getptrsi,L,L) }
void	SetPtrSize(p,s) Ptr p; { _macc2(RN,_setptrsi,N,L,L) }
THz	PtrZone(p) Ptr p; { _macc1(RI,_ptrzone,L,L) }

Handle	NewHandle(c) { _macc1(RI,_newhandl,L,L) }
void	DisposHandle(h) Handle h; { _macc1(RN,_disposha,N,L) }
int	GetHandleSize(h) Handle h; { _macc1(RI,_gethandl,L,L) }
void	SetHandleSize(h,s) Handle h; { _macc2(RN,_sethandl,N,L,L) }
THz	HandleZone(h) Handle h; { _macc1(RI,_handlezo,L,L) }
Handle	RecoverHandle(p) Ptr p; { _macc1(RI,_recoverh,L,L) }
void	EmptyHandle(h) Handle h; { _macc1(RN,_emptyhan,N,L) }
void	ReAllocHandle(h,s) Handle h; { _macc2(RN,_realloch,N,L,L) }

void	HLock(h) Handle h; { _macc1(RN,_hlock,N,L) }
void	HUnlock(h) Handle h; { _macc1(RN,_hunlock,N,L) }
void	HPurge(h) Handle h; { _macc1(RN,_hpurge,N,L) }
void	HNoPurge(h) Handle h; { _macc1(RN,_hnopurge,N,L) }
void	MoreMasters() { _macc1(RI,_moremast,N,N) }

void	BlockMove(s,d,c) Ptr s,d; { _macc3(RN,_blockmov,N,L,L,L) }
int	MemError() { _macc1(RI,_memerror,S,N) }

int	GZCritical() { _macc1(RI,_gzcritic,B,N) }
Handle	GZSaveHnd() { _macc1(RI,_gzsavehn,L,N) }


/* Driver, vertical retrace, dispatcher, core utilities */

int	VInstall(vp) VBLTask *vp; { _macc1(RI,_vinstall,S,L) }
int	VRemove(vp) VBLTask *vp; { _macc1(RI,_vremove,S,L) }

int	GetTrapAddress(tn) { _macc1(RI,_gettrapa,L,S) }
void	SetTrapAddress(ta,tn) { _macc2(RN,_settrapa,N,L,S) }

int	WriteParam() { _macc1(RI,_writepar,S,N) }
int	SetDateTime(t) { _macc1(RI,_setdatet,S,L) }
int	ReadDateTime(t) int *t; { _macc1(RI,_readdate,S,L) }
void	GetDateTime(s) int *s; { _macc1(RN,_getdatet,N,L) }
void	SetTime(d) DateTimeRec *d; { _macc1(RN,_settime,N,L) }
void	GetTime(d) DateTimeRec *d; { _macc1(RN,_gettime,N,L) }
void	Date2Secs(d,s) DateTimeRec *d; int *s; { _macc2(RN,_date2sec,N,L,L) }
void	Secs2Date(s,d) DateTimeRec *d; { _macc2(RN,_secs2dat,N,L,L) }
void	Delay(nt,ft) int *ft; { _macc2(RN,_delay,N,L,L) }
int	EqualString(s1,s2,c,d) char *s1,*s2; { _macc4(RI,_equalstr,B,PS,PS,B,B) }
void	UprString(s,d) char *s; { _macc2(RN,_uprstrin,N,VPS,B) }
int	InitUtil() { _macc1(RI,_initutil,S,N) }

void	UnLoadSeg(a) Ptr a; { _mact1(RN,0xa9f1,N,L) }
void	ExitToShell() { _mact1(RN,0xa9f4,N,N) }
void	GetAppParms(n,rn,p) char *n; int *rn; Handle *p; { _mact3(RN,0xa9f5,N,VPS,VS,L) }
void	CountAppFiles(mes,c) { _macc2(RN,_countapp,N,VS,VS) }
void	GetAppFiles(i,f) AppFile *f; { _macc2(RN,_getappfi,N,S,L) }
void	ClrAppFiles(i) { _macc1(RN,_clrappfi,N,S) }

void	InitQueue(h) QHdr *h; { _macc1(RN,_initqueu,N,L) }
void	Enqueue(qe,qh) QElem *qe; QHdr *qh; { _macc2(RN,_enqueue,N,L,L) }
int	Dequeue(qe,qh) QElem *qe; QHdr *qh; { _macc2(RI,_dequeue,S,L,L) }
Handle	GetDCtlEntry(rn) { _macc1(RI,_getdctle,L,S) }


/* Input / Output */

int	PBOpen(pb,a) IOParam *pb; { _macc2(RI,_pbopen,S,L,B) }
int	PBClose(pb,a) IOParam *pb; { _macc2(RI,_pbclose,S,L,B) }
int	PBRead(pb,a) IOParam *pb; { _macc2(RI,_pbread,S,L,B) }
int	PBWrite(pb,a) IOParam *pb; { _macc2(RI,_pbwrite,S,L,B) }
int	PBControl(pb,a) CntrlParam *pb; { _macc2(RI,_pbcontro,S,L,B) }
int	PBStatus(pb,a) CntrlParam *pb; { _macc2(RI,_pbstatus,S,L,B) }
int	PBKillIO(pb,a) CntrlParam *pb; { _macc2(RI,_pbkillio,S,L,B) }

int	PBGetVInfo(pb,a) VolumeParam *pb; { _macc2(RI,_pbgetvin,S,L,B) }
int	PBGetVol(pb,a) VolumeParam *pb; { _macc2(RI,_pbgetvol,S,L,B) }
int	PBSetVol(pb,a) VolumeParam *pb; { _macc2(RI,_pbsetvol,S,L,B) }
int	PBEject(pb,a) VolumeParam *pb; { _macc2(RI,_pbeject,S,L,B) }
int	PBOffLine(pb,a) VolumeParam *pb; { _macc2(RI,_pbofflin,S,L,B) }
int	PBFlushVol(pb,a) VolumeParam *pb; { _macc2(RI,_pbflushv,S,L,B) }
int	PBMountVol(pb) VolumeParam *pb; { _macc1(RI,_pbmountv,S,L) }
int	PBUnMountVol(pb) VolumeParam *pb; { _macc1(RI,_pbopen,S,L) }
int	PBCreate(pb,a) IOParam *pb; { _macc2(RI,_pbcreate,S,L,B) }
int	PBDelete(pb,a) IOParam *pb; { _macc2(RI,_pbdelete,S,L,B) }
int	PBOpenRF(pb,a) IOParam *pb; { _macc2(RI,_pbopenrf,S,L,B) }
int	PBRename(pb,a) IOParam *pb; { _macc2(RI,_pbrename,S,L,B) }
int	PBGetFInfo(pb,a) FileParam *pb; { _macc2(RI,_pbgetfin,S,L,B) }
int	PBSetFInfo(pb,a) FileParam *pb; { _macc2(RI,_pbsetfin,S,L,B) }
int	PBSetFLock(pb,a) IOParam *pb; { _macc2(RI,_pbsetflo,S,L,B) }
int	PBRstFLock(pb,a) IOParam *pb; { _macc2(RI,_pbrstflo,S,L,B) }
int	PBSetFVers(pb,a) IOParam *pb; { _macc2(RI,_pbsetfve,S,L,B) }
int	PBAllocate(pb,a) IOParam *pb; { _macc2(RI,_pballoca,S,L,B) }
int	PBGetEOF(pb,a) IOParam *pb; { _macc2(RI,_pbgeteof,S,L,B) }
int	PBSetEOF(pb,a) IOParam *pb; { _macc2(RI,_pbseteof,S,L,B) }
int	PBGetFPos(pb,a) IOParam *pb; { _macc2(RI,_pbgetfpo,S,L,B) }
int	PBSetFPos(pb,a) IOParam *pb; { _macc2(RI,_pbsetfpo,S,L,B) }
int	PBFlushFile(pb,a) IOParam *pb; { _macc2(RI,_pbflushf,S,L,B) }

void	AddDrive(drn,dn,q) DrvQEl *q; { _macc3(RN,_adddrive,N,S,S,L) }

int	FSOpen(n,vrn,rn) char *n; int *rn; { _macc3(RI,_fsopen,S,PS,S,VS) }
int	FSClose(rn) { _macc1(RI,_fsclose,S,S) }
int	FSRead(rn,c,b) int *c; Ptr b; { _macc3(RI,_fsread,S,S,L,L) }
int	FSWrite(rn,c,b) int *c; Ptr b; { _macc3(RI,_fswrite,S,S,L,L) }
int	Control(rn,o,p) CSParam *p; { _macc3(RI,_control,S,S,S,L) }
int	Status(rn,o,p) CSParam *p; { _macc3(RI,_status,S,S,S,L) }
int	KillIO(rn) { _macc1(RI,_killio,S,S) }

/* volume level calls */

int	GetVInfo(n,vn,vrn,fb) char *vn; int *vrn; int *fb; { _macc4(RI,_getvinfo,S,S,VPS,VS,L) }
int	GetFInfo(fn,vrn,fi) char *fn; FInfo *fi; { _macc3(RI,_getfinfo,S,PS,S,L) } 
int	GetVol(n,vrn) char *n; int *vrn; { _macc2(RI,_getvol,S,VPS,VS) }
int	SetVol(n,vrn) char *n; { _macc2(RI,_setvol,S,PS,S) }
int	UnMountVol(n,vrn) char *n; { _macc2(RI,_unmountv,S,PS,S) }
int	Eject(n,vrn) char *n; { _macc2(RI,_eject,S,PS,S) }
int	FlushVol(n,vrn) char *n; { _macc2(RI,_flushvol,S,PS,S) }

/* file level calls for unopened files */

int	Create(n,vrn,c,t) char *n,*c,*t; { _macc4(RI,_create,S,PS,S,TS,TS) }
int	FSDelete(n,vrn) char *n; { _macc2(RI,_fsdelete,S,PS,S) }
int	OpenRF(n,vrn,rn) char *n; int *rn; { _macc3(RI,_openrf,S,PS,S,VS) }
int	Rename(o,vrn,n) char *o,*n; { _macc3(RI,_rename,S,PS,S,PS) }
int	SetFInfo(fn,vrn,fi) char *fn; FInfo *fi; { _macc3(RI,_setfinfo,S,PS,S,L) } 
int	SetFLock(n,vrn) char *n; { _macc2(RI,_setflock,S,PS,S) }
int	RstFLock(n,vrn) char *n; { _macc2(RI,_rstflock,S,PS,S) }
int	SetFType(n,ov,vrn,nv) char *n; { _macc4(RI,_setftype,S,PS,S,S,S) }

/* file level calls for opened files */

int	Allocate(rn,bc) int *bc; { _macc2(RI,_allocate,S,S,L) }
int	GetEOF(rn,e) int *e; { _macc2(RI,_geteof,S,S,L) }
int	SetEOF(rn,e) { _macc2(RI,_seteof,S,S,L) }
int	GetFPos(rn,p) int *p; { _macc2(RI,_getfpos,S,S,L) }
int	SetFPos(rn,pm,po) { _macc3(RI,_setfpos,S,S,S,L) }
int	FlushFile(rn) { _macc1(RI,_flushfil,S,S) }

/* for Serial Driver */

int	DrvrInstall(s,rn) char *s; { _macc2(RI,_drvrinst,S,PS,S) }
int	DrvrRemove(rn) { _macc1(RI,_drvrremo,S,S) }
int	OpenDriver(n) char *n; { _macc1(RI,_opendriv,S,PS) }
void	CloseDriver(rn) { _macc1(RN,_closedri,N,S) }
int	SerReset(rn,p) { _macc2(RI,_serreset,S,S,S) }
int	SerSetBuf(rn,p,l) Ptr p; { _macc3(RI,_sersetbu,S,S,L,S) }
int	SerHShake(rn,f) SerShk *f; { _macc2(RI,_serhshak,S,S,L) }
int	SerSetBrk(rn) { _macc1(RI,_sersetbr,S,S) }
int	SerClrBrk(rn) { _macc1(RI,_serclrbr,S,S) }
int	SerGetBuf(rn,c) int *c; { _macc2(RI,_sergetbu,S,S,L) }
int	SerStatus(rn,f) SerStaRec *f; { _macc2(RI,_serstatu,S,S,L) }
int	RamSDOpen(p,t,i) char *t; { _macc3(RI,_ramsdope,S,S,TS,S) }
void	RamSDClose(p) { _macc1(RN,_ramsdclo,N,S) }

/* for Disk Driver */

int	DiskEject(dn) { _macc1(RI,_diskejec,S,S) }
int	SetTagBuffer(p) Ptr p; { _macc1(RI,_settagbu,S,L) }
int	DriveStatus(dn,sts) DrvSts *sts; { _macc2(RI,_drivesta,S,S,L) }

/* for Sound Driver */

void	SetSoundVol(l) { _macc1(RN,_setsound,N,S) }
void	GetSoundVol(l) int *l; { _macc1(RN,_getsound,N,VS) }
void	StartSound(t,n,com) Ptr t; ProcPtr com; { _macc3(RN,_startsou,N,L,L,L) }
void	StopSound() { _macc1(RN,_stopsoun,N,N) }
int	SoundDone() { _macc1(RI,_sounddon,B,N) }
void	SysBeep(d) { _mact1(RN,0xa9c8,N,S) }

/* for the System error handler */

void	SysError(c) { _macc1(RN,_syserror,N,S) }

/* for MacsBug */

void	MacsBug() { _mact1(RN,0xa9ff,N,N) }
void	MacsBugPrint(s) char *s; { _mact1(RN,0xabff,N,PS) }
