/*
 * File: rmemmon.c
 *  Contents: gd, memmon
 */

/*
 * gd.c - graphics driver for AED 1024.
 *  Rewrite this file to use a different output device.
 *
 *  Except for gdwidth(), which is used only for layout, these functions
 *  provide a ONE-DIMENSIONAL interface to the AED.
 */

#include "../h/rt.h"
#include "gc.h"

#ifdef MemMon

#include <sgtty.h>
#include <varargs.h>

#define Height 768			/* screen height */
#define Width 1024			/* screen width */
#define PromptArea 50			/* pixels to reserve for prompting */
#define PromptSize 40			/* size of prompt block proper */
#define MaxColors 256			/* maximum number of distinct colors */
#define MaxScale 16			/* maximum scaling factor */
#define EndRun() if(runaddr)putc(runaddr=0,ofile);  /* end a run of pixels */

static FILE *ifile = 0;			/* AED input file */
static FILE *ofile = 0;			/* AED output file */
static int yscale = 1;			/* vertical scaling factor */
static int npixels = Height * Width - PromptArea;  /* screen size (scaled) */
static runaddr = 0;			/* curr addr if pixel run in progress */
static int bgcolor;			/* current background color */

/*
 * gdinit(filename) - initialize for graphics output.
 */

gdinit(fname)
char *fname;
   {
   FILE *newf;
   int ofd;
   struct sgttyb ttyb;
   static int ldisc = NTTYDISC;
   static int lbits = LLITOUT;

   if ((newf = fopen(fname,"w")) == NULL)
      error("can't open MemMon file");
   if (isatty(ofd = fileno(newf)))  {
      if (ioctl(ofd, TIOCSETD, &ldisc))
         error("can't select new tty driver for MemMon file");
      if (ioctl(ofd, TIOCGETP, &ttyb))
         error("can't get sgtty block for MemMon file");
      if (ioctl(ofd, TIOCLBIS, &lbits))
         error("can't set LLITOUT on MemMon file");
      ttyb.sg_flags &= ~(RAW+ECHO);
      if (ioctl(ofd, TIOCSETP, &ttyb))
         error("can't set tty attributes for MemMon file");
      if (ifile = fopen(fname,"r")) {
#ifndef VMS
         setbuf(ifile,NULL);
#endif VMS
	 }
      else {
         fprintf(stderr, "can't read MemMon file;  will not pause\n");
	 fflush(stderr);
	 }
      }
   ofile = newf;
   fputs("\033SEN18D88",ofile);	/* set encoding mode to binary */
   aedout("gii",0,767);		/* set normal window boundaries */
}

/*
 * gdmap(array,n) - set color map.  Each entry of array (of size n) has three
 *  bits each of red, green, blue values in least signifigant bits.  The
 *  constants in the table below were determined empirically.
 */

gdmap(a,n)
int a[], n;
   {
   unsigned char buf[3*MaxColors+4];
   unsigned char *p;
   static unsigned char rmap[] = { 0, 20, 45, 70, 100, 140, 190, 255};
   static unsigned char gmap[] = { 0, 40, 60, 80, 110, 150, 195, 255};
   static unsigned char bmap[] = { 0, 40, 60, 80, 110, 150, 195, 255};

   EndRun();
   p = buf;
   *p++ = 'K';
   *p++ = 0;
   *p++ = n;
   while (n--)  {
      *p++ = rmap [ *a >> 6 ];
      *p++ = gmap [ (*a >> 3) & 7 ];
      *p++ = bmap [ *a++ & 7 ];
      }
   fwrite(buf,1,p-buf,ofile);
   }

/*
 * gdflood(c) - fill screen with color c.
 */

gdflood(c)
int c;
   {
   EndRun();
   aedout("[b",c);			/* set background color */
   aedout("~");				/* erase screen (kills scaling) */
   aedout("Ebb",1,yscale);		/* reset scaling */
   bgcolor = c;				/* save background color */
   }

/*
 * gdscale(n) - set vertical scaling factor.
 */

gdscale(n)
int n;
   {
   EndRun();
   if (n > MaxScale)
      n = MaxScale;
   yscale = n;
   npixels = (Height / n) * Width - PromptArea;
   aedout("Ebb",1,n);		/* set zoom to handle y scaling (only!) */
   }

/*
 * gdsize(n) - return size of display with scaling factor n;
 */

gdsize(n)
int n;
   {
   if (n > MaxScale)
      n = MaxScale;
   return (Height / n) * Width - PromptArea;
   }

/*
 * gdwidth() - return width of display with current scaling.
 */

gdwidth()
   {
   return Width;
   }

/*
 * gdpaint(start,n,color,b) - paint n pixels in given color.
 *  If b >= 0, the last pixel is to be that color instead (for a border)
 */

gdpaint(s,n,c,b)
int s, n, c, b;
   {
   if (s < 0 || s >= npixels || n <= 0)
      return;
   if (s + n > npixels)
      n = npixels - s;
   if (runaddr && runaddr != s)
      putc(runaddr=0,ofile);
   if (!runaddr)  {
      aedout("Qx", s % Width, Height - 1 - s / Width);
      aedout("s");
      }
   runaddr = s + n;
   if (b >= 0)
      n--;
   while (n > 254)  {
      putc(254,ofile);
      putc(c,ofile);
      n -= 254;
      }
   if (n > 0)  {
      putc(n,ofile);
      putc(c,ofile);
      }
   if (b >= 0)  {
      putc(1,ofile);
      putc(b,ofile);
      }
   }

/*
 * gdflush() - flush output.
 */

gdflush()
   {
   fflush(ofile);
   }

/*
 * gdpause(color) - pause for acknowledgement, showing color to hint at reason.
 * Return last character before '\n', 0 if none, or EOF if not interactive.
 */

gdpause(color)
int color;
   {
   int c1, c2, pline;

   if (!ifile)
      return EOF;
   EndRun();
   fputs("\r\007\033",ofile);		/* ring bell */
   pline = Height - Height / yscale;
   aedout("Qx",Width-PromptSize,pline);
   aedout("sbbb",PromptSize,color,0);
   fflush(ofile);
   c1 = 0;
   while ((c2 = getc(ifile)) != '\n')
      if (c2 == EOF)  {
         ifile = 0;
         c1 = EOF;
         break;
         }
      else
         c1 = c2;
   aedout("Qx",Width-PromptSize,pline);
   aedout("sbbb",PromptSize,bgcolor,0);
   return c1;
   }

/*
 * gdterm() - terminate graphics
 */

gdterm()
   {
   if (!ofile)
      return;			/* if no file, just return */
   EndRun();
   aedout("Gs","3DNNN");	/* reset encoding */
   aedout("\r");		/* exit graphics mode */
   }

/*
 * aedout(s,args) - output command to the AED.
 *  s is a string specifying the command format a la printf.  The first
 *  character (or two chars if first is '+') are the AED command.  Additional
 *  characters specify formats for outputting additional arguments (see below).
 */

/*VARARGS1*/
static aedout (s, va_alist)
char *s;
va_dcl
   {
   va_list ap;
   char c;
   unsigned int n, x, y;

   va_start(ap);
   if (putc(*s++,ofile) == '+')
      putc(*s++,ofile);
   while (c = *s++)
      switch (c)  {		/* Output formats for add'l args:  */
         case 'b':			/* b - single byte unaltered */
         case 'c':			/* c - single char unaltered */
            n = va_arg(ap,int);
            putc(n, ofile);
            break;
         case 'i':			/* i - 16-bit integer as two bytes */
            n = va_arg(ap,int);
            putc(n>>8, ofile);
            putc(n, ofile);
            break;
         case 's':			/* s - string terminated by '\0' */
            fputs(va_arg(ap,int),ofile);
            break;
         case 'x':			/* x - two args give x and y coords */
            x = va_arg(ap,int);
            y = va_arg(ap,int);
            putc(((x>>4)&0xF0) | (y>>8), ofile);
            putc(x, ofile);
            putc(y, ofile);
            break;
         default:			/* unrecognized - just echoed */
            putc(c, ofile);
         }
   va_end(ap);
   }


/*
 * memmon.c - memory monitoring code excluding the device driver.
 *  When MemMon is undefined, gc.h defines these entrypoints
 *  as null macros.
 */

/*
 * Color list, indexed by block type or defined symbol.
 *  Every entry has 3 bits each (left to right) for red, green, blue.
 *  White is used for blocks not expected in the block region.
 */

static int clist[] = {
		/*  --- these entries change color in marking phase ---  */
    0777,	/*  0. T_Null     white    null value  */
    0777,	/*  1. T_Integer  white    integer  */
    0777,	/*  2. T_Longint  white    long integer  */
    0007,	/*  3. T_Real     blue     real number  */
    0540,	/*  4. T_Cset     brown    cset  */
    0405,	/*  5. T_File     purple   file block  */
    0777,	/*  6. T_Proc     white    procedure block  */
    0060,	/*  7. T_List     green    list header block  */
    0770,	/*  8. T_Table    yellow   table header block  */
    0077,	/*  9. T_Record   cyan     record block  */
    0773,	/* 10. T_Telem    lt yel   table element block  */
    0272,	/* 11. T_Lelem    lt grn   list element block  */
    0756,	/* 12. T_Tvsubs   pink     substring trapped variable  */
    0777,	/* 13. T_Tvkywd   white    keyword trapped variable  */
    0751,	/* 14. T_Tvtbl    orange   table elem trapped variable  */
    0600,	/* 15. T_Set      red      set header block  */
    0700,	/* 16. T_Selem    brt red  set element block  */
    0004,	/* 17. T_Refresh  navy     refresh block  */
    0777,	/* 18. T_Coexpr   white    co-expression block */
    0777,	/* 19.            white    unused */
    0777,	/* 20.            white    unused */
    0777,	/* 21.            white    unused */
    0777,	/* 22. UNcolr     white    unknown type  */
    0776,	/* 23. STcolr     ivory    unbroken string  */
    0344,	/* 24. STBcolr    blue     string separator  */
		/*  --- these entries remain constant ---  */
    0000,	/* 25. BGcolr     black    background  */
    0222,	/* 26. AVcolr     med gry  available space  */
    0000,	/* 27. BKBcolr    black    block border  */
    0700,	/* 28. GCcolr     red      begin garbage collection */
    0770,	/* 29. GARcolr    yellow   screen contents are garbage */
    0007,	/* 30. VALcolr    blue     screen contents are valid data */
    0070,	/* 31. GOcolr     green    ready to go after garb coll */
		/*  --- these entries are used for color changes ---  */
    0222,	/* 32. MKcolr     med gry  marked block  */
    0000,	/* 33. UMcolr     black    unmarked block  */
    0222,	/* 34. UMBcolr    med gry  unmarked block border  */
    };

#define NColors 33	/* number of colors copied directly to color map */
#define TColors 25	/* number of those which are transmutable */

#define UNcolr 22
#define STcolr 23
#define STBcolr 24
#define BGcolr 25
#define AVcolr 26
#define BKBcolr 27
#define GCcolr 28
#define GARcolr 29
#define VALcolr 30
#define GOcolr 31
#define MKcolr 32
#define UMcolr 33
#define UMBcolr 34

/*
 * Color map buffer, initialized in MMInit and thereafter unaltered.
 *  The color map downloaded to the display is always NColors+Bcolors
 *  long and normally starts at cbuff+NColors.  After marking and before
 *  compaction, marked blocks are shown in color by downloading a map
 *  beginning at the front of cbuff.
 */

int cbuff[3*NColors];

/*
 * Miscellaney.
 */

static char *mmdev;		/* name of monitoring device, if used */
static int currcolor = UNcolr;	/* current block output color */
static int sstart;		/* origin of string space display */
static int scale = 16;		/* initial scaling;  may decrease  */

#define NSkip 4			/* empty lines between blocks and strings */
#define Gran 4			/* granularity of display (in bytes) */
#define PaintBlk(addr,size,color,b)  \
    gdpaint(((addr)-blkbase)/Gran,(size)/Gran,color,b)
#define PaintStr(addr,size,color,b)  \
    gdpaint(sstart+((addr)-strbase)/Gran,(size)/Gran,color,b)

/*
 * MMInit() - initialization.
 */

MMInit()
   {
   int i;
   extern char *getenv();

   if (!(mmdev = getenv("MEMMON")))
      return;
   for (i=0; i<TColors; i++)  {
      cbuff[i] = clist[UMcolr];
      cbuff[i+NColors] = clist[i];
      cbuff[i+2*NColors] = clist[MKcolr];
      }
   for (i=TColors; i<NColors; i++)
      cbuff[i] = cbuff[i+NColors] = clist[i];
   cbuff[BKBcolr] = clist[UMBcolr];
   gdinit(mmdev);
   }

/*
 * MMType(t) - set type for subsequent block allocation.
 */

MMType(t)
int t;
   {
   currcolor = t;
   }

/*
 * MMAlc(n) - show allocation of n block bytes at blkfree.
 */

MMAlc(n)
int n;
   {
   if (!mmdev)
      return;
   if (!sstart)
      refresh();
   PaintBlk(blkfree,n,currcolor,BKBcolr);
   gdflush();
   currcolor = UNcolr;
   }

/*
 * MMStr(n) - show allocation of n string bytes at strfree.
 *  Each pixel on the display represents multiple bytes.  A pixel in which any
 *  string ends will be STBcolr;  pixels corresponding to nonterminal characters
 *  of one string will be STcolr.  This method makes long strings individually
 *  distinguishable from each other and from runs of short strings, and allows
 *  continuous output with no backtracking as the string space is allocated.
 */

MMStr(slen)
int slen;
   {
   int s, e;

   if (!mmdev)
      return;
   if (!sstart)
      refresh();
   s = (strfree-strbase+Gran-1) / Gran;	/* start (first pixel wholly owned) */
   e = (strfree-strbase+slen-1) / Gran;	/* end pixel */
   if (e < s)				/* if no new pixels, return */
      return;
   gdpaint(sstart+s,e-s+1,STcolr,STBcolr);
   gdflush();
   }

/*
 * MMBGC() - begin garbage collection.
 */

MMBGC()
   {
   if (!mmdev)
      return;
   gdpause(GCcolr);
   }

/*
 * MMMark(block,type) - mark indicated block during garbage collection.
 */

MMMark(block,type)
char *block;
int type;
   {
   if (mmdev)
      PaintBlk(block,BlkSize(block),NColors+type,BKBcolr);
   }

/*
 * MMSMark - Mark String.
 */

MMSMark(saddr,slen)
char *saddr;
int slen;
   {
   int s, e;

   if (!mmdev)
      return;
   s = (saddr-strbase+Gran-1) / Gran;	/* start (first pixel wholly owned) */
   e = (saddr-strbase+slen-1) / Gran;	/* end pixel */
   if (e >= s)				/* if anything to paint... */
      gdpaint(sstart+s,e-s+1,NColors+STcolr,NColors+STBcolr);
   }

/*
 * MMEGC() - end garbage collection.
 */

MMEGC()
   {
   if (!mmdev)
      return;
   if (gdpause(GARcolr) != EOF)
       gdmap(cbuff,NColors+TColors);
   while (gdpause(VALcolr) == '-')  {
       gdmap(cbuff+NColors,NColors+TColors);
       gdpause(GARcolr);
       gdmap(cbuff,NColors+TColors);
       }
   refresh();
   gdpause(GOcolr);
   }

/*
 * MMTerm() - terminate memory monitoring.
 */

MMTerm()
   {
   if (mmdev)
      gdterm();
   }

/*
 * refresh() - redraw screen, initially or after garbage collection.
 */

static refresh()
   {
   char *p;
   int n;

   if (layout())
      gdflood(BGcolr);
   else
      PaintBlk(blkbase,blkfree-blkbase,AVcolr,-1);
   PaintStr(strfree,strend-strfree,AVcolr,-1);
   PaintBlk(blkfree,maxblk-blkfree,AVcolr,-1);
   gdmap(cbuff+NColors,NColors+TColors);
   PaintStr(strbase,strfree-strbase,STcolr,BKBcolr);
   for (p = blkbase; p < blkfree; p += n)
      PaintBlk(p,n=BlkSize(p),BlkType(p),BKBcolr);
   }

/*
 * layout - determine screen layout.  Return NZ iff different from previous.
 */

static layout()
{
   int hpixels, spixels, avail, mid, width, ideal, diff;
   static int oldscale, oldss;

   hpixels = (maxblk - blkbase) / Gran + 1;
   spixels = (strend - strbase) / Gran + 1;
   while (scale > 1 && (hpixels + spixels > gdsize(scale)))
      --scale;
   gdscale(scale);
   avail = gdsize(scale); 
   width = gdwidth();
   mid = avail / 2;
   mid -= mid % width;
   ideal = ((hpixels + width - 1) / width + NSkip) * width;   
   if (ideal <= mid && spixels <= avail - mid)
      sstart = mid;
   else if (ideal + spixels <= avail)
      sstart = ideal;
   else if (hpixels + spixels <= avail)
      sstart = avail - spixels;
   else
      sstart = hpixels + 1;
   diff = (scale != oldscale) || (sstart != oldss); 
   oldscale = scale;
   oldss = sstart;
   return diff;
   }

#else MemMon
static int x;		/* prevent null module when MemMon not defined */
#endif MemMon
