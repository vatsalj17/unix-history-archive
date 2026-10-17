#include <u.h>
#include <gnot.h>
#include <event.h>

Bitmap display;
Font *defont;
int w,h;

main(argc,argv)
char *argv[];
{
	Bitmap *b,*m;
	int fd;
	ginit();
	einit();
	if (argc != 4) {
		fprint(2,"usage: matte w h file\n");
		exit(0);
	}
	w = atoi(argv[1]);
	h = atoi(argv[2]);
	if ((fd = open(argv[3],0)) < 0) {
		fprint(2,"can't open %s\n",argv[3]);
		exit(0);
	}
	b = balloc(Rect(0,0,w,h),3);
	m = balloc(Rect(0,0,w,h),1);
	bitblt(m,m->rect.min,m,m->rect,F);
	myread(fd,b->base,w*h);
	close(fd);
	dispfs(b,m);
	edit(b,m,argv[3]);
}

dispfs(b,m)
Bitmap *b,*m;
{
	Bitmap *r;
	r = balloc(b->rect,b->ldepth);
	bitblt(r,r->rect.min,b,b->rect,S);
	bitblt(r,r->rect.min,m,m->rect,S&D);
	fs(r);
	bitblt(&display,display.rect.min,r,r->rect,S);
	bfree(r);
}

char *fsizes[] = {"4", "7", "12", "25", "80", "150", 0};
Menu sizes = {fsizes};

enum {REMOVE, RESTORE, WRITE, EXIT};
char *fmenu[] = {"remove", "restore", "write", "exit", 0};
Menu commands = {fmenu};

#define DR(n)	raddp(Rect(0,0,n,n),display.rect.min)

edit(b,m,file)
Bitmap *b,*m;
char *file;
{
	Event *e;
	Point p;
	Bitmap *res,*brush=0;
	int i,fd,size=25,wannago = 0;
	int mode=0;
	while (1) {
		switch ((e = getevent())->type) {
		case MOUSE:
			if (mouse.buttons&1) {
				p = sub(mouse.xy,Pt(size/2,size/2));
				docursor(0);
				bitblt(&display,p,&display,DR(size),F-mode);
				docursor(size);
				bitblt(m,sub(p,display.rect.min),m,Rect(0,0,size,size),mode);
			}
			else if (mouse.buttons&2) {
				if ((i = menuhit(&sizes,2)) == -1)
					break;
				size = atoi(fsizes[i]);
				docursor(size);
			}
			else if (mouse.buttons&4)
				switch (menuhit(&commands,3)) {
				case -1:
					break;
				case REMOVE:
					mode = 0;
					docursor(0);
					dispfs(b,m);
					docursor(size);
					break;
				case RESTORE:
					mode = F;
					docursor(0);
					bitblt(&display,display.rect.min,m,m->rect,notS);
					docursor(size);
					break;
				case WRITE:
					res = balloc(Rect(0,0,w,h),3);
/*
					bitblt(res,Pt(0,0),b,b->rect,S);
					bitblt(res,Pt(0,0),m,m->rect,S&D);
*/
					bitblt(res,Pt(0,0),m,m->rect,S);
					if ((fd = create("matte",1,0666)) < 0) {
						fprint("can't create matte file\n");
						continue;
					}
					mywrite(fd,res->base,w*h);
					close(fd);
					bfree(res);
					break;
				case EXIT:
					if (wannago)
						exit(0);
					else {
						wannago = 1;
						continue;
					}
					break;
				}
			wannago = 0;
			docursor(size);
		default:
			break;
		}
	}
}

docursor(n)
{
	static Point pt;
	static size;
	if (size != 0)
		cursoroff();
	bitblt(&display,sub(pt,Pt(size/2,size/2)),&display,DR(size),notD);
	pt = mouse.xy;
	size = n;
	bitblt(&display,sub(pt,Pt(size/2,size/2)),&display,DR(size),notD);
	if (size != 0)
		cursoron();
}	

ushort prop[1024];
uchar bright[] = {192,128,64,0};

fs(b)
Bitmap *b;
{
	int i,j,w,h,save,next,e,x;
	uchar *p;
	ushort *q;
	w = b->rect.max.x;
	h = b->rect.max.y;
	for (i = 0; i < h; i++) {
		p = (uchar *) addr(b,Pt(0,i));
		save = next = 0;
		q = prop;
		for (j = 0; j < w; j++) {
			x = *p + *q + next;
			if (x > 255)
				x = 255;
			e = x&0x3f;
			next = (e + (e<<1))>>3;
			*q++ = save + next;
			save = e>>2;
			*p++ = ~x;
		}
	}
}

myread(fd,buf,n)
uchar *buf;
{
	int i;
	do {
		i = read(fd,buf,n);
		if(i<=0)
			return;		/* let it overrun */
		buf += i;
		n -= i;
	} while (n > 0);
}

mywrite(fd,buf,n)
uchar *buf;
{
	int i;
	do {
		i = write(fd,buf,(n>4096)?4096:n);
		if (i<=0) {
			fprint(2,"bad write of %d\n",n);
			exit(1);
		}
		buf += i;
		n -= i;
	} while (n > 0);
}
