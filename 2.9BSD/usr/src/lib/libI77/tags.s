f_back	backspace.c	?^f_back(a) alist *a;$?
f_clos	close.c	?^f_clos(a) cllist *a;$?
f_exit	close.c	?^f_exit()$?
flush_	close.c	?^flush_(u) ftnint *u;$?
ASSERT	dballoc.c	?^#define ASSERT(p)$?
clearbusy	dballoc.c	?^#define clearbusy(p) (struct store *)((int)(p)&~BU?
setbusy	dballoc.c	?^#define setbusy(p) (struct store *)((int)(p)+BUSY)$?
testbusy	dballoc.c	?^#define testbusy(p) ((int)(p)&BUSY)$?
botch	dballoc.c	?^botch(s) char *s;$?
calloc	dballoc.c	?^char *calloc(nbytes,count)$?
free	dballoc.c	?^free(p)$?
malloc	dballoc.c	?^malloc(nbytes)$?
realloc	dballoc.c	?^realloc(p, nbytes)$?
c_dfe	dfe.c	?^c_dfe(a,flag) cilist *a;$?
e_rdfe	dfe.c	?^e_rdfe()$?
e_wdfe	dfe.c	?^e_wdfe()$?
s_rdfe	dfe.c	?^s_rdfe(a) cilist *a;$?
s_wdfe	dfe.c	?^s_wdfe(a) cilist *a;$?
y_getc	dfe.c	?^y_getc()$?
y_putc	dfe.c	?^y_putc(c)$?
y_rend	dfe.c	?^y_rend()$?
y_rnew	dfe.c	?^y_rnew()$?
y_tab	dfe.c	?^y_tab()$?
y_wend	dfe.c	?^y_wend()$?
y_wnew	dfe.c	?^y_wnew()$?
DO	dofio.c	?^#define DO(x)	if(n=x) err(n>0?
do_fio	dofio.c	?^do_fio(number,ptr,len) ftnint *number; ftnlen len;?
en_fio	dofio.c	?^en_fio()$?
fmt_bg	dofio.c	?^fmt_bg()$?
type_f	dofio.c	?^type_f(n)$?
c_le	dolio.c	?^c_le(a,flag) cilist *a;$?
do_lio	dolio.c	?^do_lio(type,number,ptr,len) ftnint *number,*type; ?
do_ud	douio.c	?^do_ud(number,ptr,len) ftnint *number; ftnlen len; ?
do_uio	douio.c	?^do_uio(number,ptr,len) ftnint *number; ftnlen len;?
do_us	douio.c	?^do_us(number,ptr,len) ftnint *number; ftnlen len; ?
c_due	due.c	?^c_due(a,flag) cilist *a;$?
e_rdue	due.c	?^e_rdue()$?
e_wdue	due.c	?^e_wdue()$?
s_rdue	due.c	?^s_rdue(a) cilist *a;$?
s_wdue	due.c	?^s_wdue(a) cilist *a;$?
f_end	endfile.c	?^f_end(a) alist *a;$?
t_runc	endfile.c	?^t_runc(b,flag) unit *b; ioflag flag;$?
f_init	err.c	?^f_init()$?
fatal	err.c	?^fatal(n,s) char *s;$?
prnt_ext	err.c	?^prnt_ext()$?
prnt_fmt	err.c	?^prnt_fmt(n) int n;$?
prnt_int	err.c	?^prnt_int()$?
isdigit	fmt.c	?^#define isdigit(x)	(x>='0' && x<='9')$?
skip	fmt.c	?^#define skip(s)		while(*s==' ') s++$?
ap_end	fmt.c	?^char *ap_end(s) char *s;$?
f_list	fmt.c	?^char *f_list(s) char *s;$?
f_s	fmt.c	?^char *f_s(s,curloc) char *s;$?
gt_num	fmt.c	?^char *gt_num(s,n) char *s; int *n;$?
i_tem	fmt.c	?^char *i_tem(s) char *s;$?
e_d	fmt.c	?^e_d(s,p) char *s,**p;$?
ne_d	fmt.c	?^ne_d(s,p) char *s,**p;$?
op_gen	fmt.c	?^op_gen(a,b,c,d,s) char *s;$?
pars_f	fmt.c	?^pars_f(s) char *s;$?
digit	fmtlib.c	?^#define digit(c)	( (c > 9) ?
icvt	fmtlib.c	?^char *icvt(value,ndigit,sign) long value; int *ndi?
c_di	iio.c	?^c_di(a) icilist *a;$?
c_fi	iio.c	?^c_fi(a) icilist *a;$?
c_li	iio.c	?^c_li(a) icilist *a;$?
c_si	iio.c	?^c_si(a) icilist *a;$?
e_rdfi	iio.c	?^e_rdfi()$?
e_rsfi	iio.c	?^e_rsfi()$?
e_rsli	iio.c	?^e_rsli()$?
e_wdfi	iio.c	?^e_wdfi()$?
e_wsfi	iio.c	?^e_wsfi()$?
e_wsli	iio.c	?^e_wsli()$?
iiopos_	iio.c	?^iiopos_()$?
iiorec_	iio.c	?^iiorec_()$?
s_rdfi	iio.c	?^s_rdfi(a) icilist *a;$?
s_rsfi	iio.c	?^s_rsfi(a) icilist *a;$?
s_rsli	iio.c	?^s_rsli(a) icilist *a;$?
s_wdfi	iio.c	?^s_wdfi(a) icilist *a;$?
s_wsfi	iio.c	?^s_wsfi(a) icilist *a;$?
s_wsli	iio.c	?^s_wsli(a) icilist *a;$?
z_getc	iio.c	?^z_getc()$?
z_putc	iio.c	?^z_putc(c) char c;$?
z_rnew	iio.c	?^z_rnew()$?
z_tab	iio.c	?^z_tab()$?
z_ungetc	iio.c	?^z_ungetc(ch,cf) char ch;$?
z_wnew	iio.c	?^z_wnew()$?
f_inqu	inquire.c	?^f_inqu(a) inlist *a;$?
V	io.c	?^#define V(z)	ioc[z].iocval$?
dofclose	io.c	?^LOCAL dofclose()$?
dofinquire	io.c	?^LOCAL dofinquire()$?
dofmove	io.c	?^LOCAL dofmove(subname)$?
dofopen	io.c	?^LOCAL dofopen()$?
doiolist	io.c	?^LOCAL doiolist(p0)$?
ioset	io.c	?^LOCAL ioset(type, offset, p)$?
iosetc	io.c	?^LOCAL iosetc(offset, p)$?
iosetip	io.c	?^LOCAL iosetip(i, offset)$?
iosetlc	io.c	?^LOCAL iosetlc(i, offp, offl)$?
putio	io.c	?^LOCAL putio(nelt, addr)$?
putiocall	io.c	?^LOCAL putiocall(q)$?
doio	io.c	?^doio(list)$?
endio	io.c	?^endio()$?
endioctl	io.c	?^endioctl()$?
fmtstmt	io.c	?^fmtstmt(lp)$?
ioclause	io.c	?^ioclause(n, p)$?
iocname	io.c	?^iocname()$?
setfmt	io.c	?^setfmt(lp)$?
startioctl	io.c	?^startioctl()$?
startrw	io.c	?^startrw()$?
GETC	lread.c	?^#define GETC(x) (x=(*getn)())$?
isapos	lread.c	?^#define isapos(x)	(ltab[x+1]&AP)$?
isblnk	lread.c	?^#define isblnk(x)	(ltab[x+1]&B)$?
isdigit	lread.c	?^#define isdigit(x)	(ltab[x+1]&D)$?
isexp	lread.c	?^#define isexp(x)	(ltab[x+1]&EX)$?
issep	lread.c	?^#define issep(x)	(ltab[x+1]&SP)$?
e_rsle	lread.c	?^e_rsle()$?
get_repet	lread.c	?^get_repet()$?
l_C	lread.c	?^l_C()$?
l_CHAR	lread.c	?^l_CHAR()$?
l_L	lread.c	?^l_L()$?
l_R	lread.c	?^l_R(flg) int flg;$?
l_read	lread.c	?^l_read(number,ptr,len,type) ftnint *number,type; f?
lr_comm	lread.c	?^lr_comm()$?
rd_int	lread.c	?^rd_int(x) double *x;$?
s_rsle	lread.c	?^s_rsle(a) cilist *a;	/* start read sequential list?
t_getc	lread.c	?^t_getc()$?
t_sep	lread.c	?^t_sep()$?
chk_len	lwrite.c	?^chk_len(w)$?
e_wsle	lwrite.c	?^e_wsle()$?
l_write	lwrite.c	?^l_write(number,ptr,len,type) ftnint *number,type; ?
lwrt_0	lwrite.c	?^lwrt_0()$?
lwrt_A	lwrite.c	?^lwrt_A(p,len) char *p; ftnlen len;$?
lwrt_C	lwrite.c	?^lwrt_C(a,b) float a,b;$?
lwrt_D	lwrite.c	?^lwrt_D(dn) double dn;$?
lwrt_DC	lwrite.c	?^lwrt_DC(a,b) double a,b;$?
lwrt_F	lwrite.c	?^lwrt_F(fn) float fn;$?
lwrt_I	lwrite.c	?^lwrt_I(in) ftnint in;$?
lwrt_L	lwrite.c	?^lwrt_L(ln) ftnint ln;$?
s_wsle	lwrite.c	?^s_wsle(a) cilist *a;$?
t_putc	lwrite.c	?^t_putc(c) char c;$?
f_open	open.c	?^f_open(a) olist *a;$?
fk_open	open.c	?^fk_open(rd,seq,fmt,n) ftnint n;$?
isdev	open.c	?^isdev(s) char *s;$?
isalpha	rdfmt.c	?^#define isalpha(c)	(c>='a' && c<='z')$?
isdigit	rdfmt.c	?^#define isdigit(c)	(c>='0' && c<='9')$?
rd_AW	rdfmt.c	?^rd_AW(p,w,len) char *p; ftnlen len;$?
rd_F	rdfmt.c	?^rd_F(p,w,d,len) ftnlen len; ufloat *p;$?
rd_I	rdfmt.c	?^rd_I(n,w,len) ftnlen len; uint *n;$?
rd_L	rdfmt.c	?^rd_L(n,w) ftnint *n;$?
rd_ed	rdfmt.c	?^rd_ed(p,ptr,len) char *ptr; struct syl *p; ftnlen ?
rd_mvcur	rdfmt.c	?^rd_mvcur()$?
rd_ned	rdfmt.c	?^rd_ned(p,ptr) char *ptr; struct syl *p;$?
f_rew	rewind.c	?^f_rew(a) alist *a;$?
c_sfe	sfe.c	?^c_sfe(a,flag) cilist *a; /* check */$?
e_rsfe	sfe.c	?^e_rsfe()$?
e_wsfe	sfe.c	?^e_wsfe()$?
pr_put	sfe.c	?^pr_put(c)$?
s_rsfe	sfe.c	?^s_rsfe(a) cilist *a; /* start */$?
s_wsfe	sfe.c	?^s_wsfe(a) cilist *a;	/*start*/$?
x_getc	sfe.c	?^x_getc()$?
x_putc	sfe.c	?^x_putc(c)$?
x_rnew	sfe.c	?^x_rnew()			/* find next record */$?
x_tab	sfe.c	?^x_tab()$?
x_wend	sfe.c	?^x_wend(last) char last;$?
x_wnew	sfe.c	?^x_wnew()$?
c_sue	sue.c	?^c_sue(a,flag) cilist *a;$?
e_rsue	sue.c	?^e_rsue()$?
e_wsue	sue.c	?^e_wsue()$?
s_rsue	sue.c	?^s_rsue(a) cilist *a;$?
s_wsue	sue.c	?^s_wsue(a) cilist *a;$?
b_char	util.c	?^b_char(from, to, tolen) char *from, *to; ftnlen to?
canseek	util.c	?^canseek(f) FILE *f; /*SYSDEP*/$?
finode	util.c	?^finode(f) FILE *f;$?
g_char	util.c	?^g_char(a,alen,b) char *a,*b; ftnlen alen;$?
ini_std	util.c	?^ini_std(u,F,w) FILE *F;$?
inode	util.c	?^inode(a) char *a;$?
last_char	util.c	?^last_char(f) FILE *f;$?
nowreading	util.c	?^nowreading(x) unit *x;$?
nowwriting	util.c	?^nowwriting(x) unit *x;$?
abs	wrtfmt.c	?^#define abs(x) (x<0?
w_ed	wrtfmt.c	?^w_ed(p,ptr,len) char *ptr; struct syl *p; ftnlen l?
w_ned	wrtfmt.c	?^w_ned(p,ptr) char *ptr; struct syl *p;$?
wr_mvcur	wrtfmt.c	?^wr_mvcur()$?
wrt_AP	wrtfmt.c	?^wrt_AP(p)$?
wrt_AW	wrtfmt.c	?^wrt_AW(p,w,len) char * p; ftnlen len;$?
wrt_E	wrtfmt.c	?^wrt_E(p,w,d,e,len) ufloat *p; ftnlen len;$?
wrt_F	wrtfmt.c	?^wrt_F(p,w,d,len) ufloat *p; ftnlen len;$?
wrt_G	wrtfmt.c	?^wrt_G(p,w,d,e,len) ufloat *p; ftnlen len;$?
wrt_H	wrtfmt.c	?^wrt_H(a,b)$?
wrt_IM	wrtfmt.c	?^wrt_IM(ui,w,m,len) uint *ui; ftnlen len;$?
wrt_L	wrtfmt.c	?^wrt_L(l,len) ftnint *l;$?
