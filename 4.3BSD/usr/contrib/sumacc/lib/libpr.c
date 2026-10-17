/*	libpr.c		1.0	84/06/28	*/

/*
 * Print library.
 */

/*
 * history
 * 06/28/84	Schuster	Created.
 */

#include "quickdraw.h"
#include "toolintf.h"
#undef v
#undef h
#include "macprint.h"
#include "libmac.h"

/*
 * PRINT INTERFACE
 */

void 	PrOpen() { _macc1(RN,_propen,N,N) }
void 	PrClose() { _macc1(RN,_prclose,N,N) }
void 	PrintDefault(h) THPrint h; { _macc1(RN,_printdefault,N,L) }
int	PrValidate(h) THPrint h; { _macc1(RI,_prvalidate,B,L) }
int	PrStlDialog(h) THPrint h; { _macc1(RI,_prstldialog,B,L) }
int	PrJobDialog(h) THPrint h; { _macc1(RI,_prjobdialog,B,L) }
TPPrDlg PrStlInit(h) THPrint h; { _macc1(RI,_prstlinit,L,L) }
TPPrDlg PrJobInit(h) THPrint h; { _macc1(RI,_prjobinit,L,L) }
int	PrDlgMain(h,p) THPrint h; ProcPtr p; { _macc2(RI,_prdlgmain,B,L,L) }
void	PrJobMerge(s,d) THPrint s,d; { _macc2(RN,_prjobmerge,N,L,L) }
TPPrPort PrOpenDoc(h,p,b) THPrint h; TPPrPort p; Ptr b; { _macc3(RI,_propendoc,L,L,L,L) }
void	PrCloseDoc(p) TPPrPort p; { _macc1(RN,_prclosedoc,N,L) }
void	PrOpenPage(p,r) TPPrPort p; Rect *r; { _macc2(RN,_propenpage,N,L,L) }
void	PrClosePage(p) TPPrPort p; { _macc1(RN,_prclosepage,N,L) }
void	PrPicFile(h,p,b,d,s) THPrint h; TPPrPort p; Ptr b; Ptr d; TPPrStatus s; { _macc5(RN,_prpicfile,N,L,L,L,L,L) }
void	PrCfgDialog() { _macc1(RN,_prcfgdialog,N,N) }
void	PrHack(p1,p2,p3) { _macc3(RN,_prhack,N,L,L,L) }

void	PrDrvrOpen() { _macc1(RN,_prdrvropen,N,N) }
void	PrDrvrClose() { _macc1(RN,_prdrvrclose,N,N) }
void	PrCtlCall(w,p1,p2,p3) { _macc4(RN,_prctlcall,N,S,L,L,L) }
int	PrError() { _macc1(RI,_prerror,S,N) }
void	PrSetError(w) { _macc1(RN,_prseterror,N,S) }
