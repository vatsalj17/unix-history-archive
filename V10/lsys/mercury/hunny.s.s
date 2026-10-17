	.text
	.align 9
	.set INTSTACK,1
	.globl	scb
scb:
	.long straycatch+0x0+INTSTACK
	.long Xmachcheck+INTSTACK
	.long Xkspnotval+INTSTACK
	.long straycatch+0xc+INTSTACK
	.long Xprivinflt
	.long Xxfcflt
	.long Xresopflt
	.long Xresadflt
	.long Xprotflt
	.long Xtransflt
	.long Xtracep
	.long Xbptflt
	.long Xcompatflt
	.long Xarithtrap
	.long straycatch+0x38+INTSTACK
	.long straycatch+0x3c+INTSTACK
	.long Xsyscall
	.long Xchmx+INTSTACK
	.long Xchmx+INTSTACK
	.long Xchmx
	.long straycatch+0x50+INTSTACK
	.long straycatch+0x54+INTSTACK
	.long straycatch+0x58+INTSTACK
	.long Xnmiflt+INTSTACK
	.long straycatch+0x60+INTSTACK
	.long straycatch+0x64+INTSTACK
	.long straycatch+0x68+INTSTACK
	.long straycatch+0x6c+INTSTACK
	.long straycatch+0x70+INTSTACK
	.long straycatch+0x74+INTSTACK
	.long straycatch+0x78+INTSTACK
	.long straycatch+0x7c+INTSTACK
	.long straycatch+0x80+INTSTACK
	.long straycatch+0x84+INTSTACK
	.long Xastflt
	.long Xqsched
	.long straycatch+0x90+INTSTACK
	.long straycatch+0x94+INTSTACK
	.long straycatch+0x98+INTSTACK
	.long straycatch+0x9c+INTSTACK
	.long Xsoftclock+INTSTACK
	.long straycatch+0xa4+INTSTACK
	.long straycatch+0xa8+INTSTACK
	.long straycatch+0xac+INTSTACK
	.long straycatch+0xb0+INTSTACK
	.long straycatch+0xb4+INTSTACK
	.long straycatch+0xb8+INTSTACK
	.long straycatch+0xbc+INTSTACK
	.long Xhardclock+INTSTACK
	.long straycatch+0xc4+INTSTACK
	.long straycatch+0xc8+INTSTACK
	.long straycatch+0xcc+INTSTACK
	.long straycatch+0xd0+INTSTACK
	.long straycatch+0xd4+INTSTACK
	.long straycatch+0xd8+INTSTACK
	.long straycatch+0xdc+INTSTACK
	.long straycatch+0xe0+INTSTACK
	.long straycatch+0xe4+INTSTACK
	.long straycatch+0xe8+INTSTACK
	.long straycatch+0xec+INTSTACK
	.long straycatch+0xf0+INTSTACK
	.long straycatch+0xf4+INTSTACK
	.long Xcnrint+INTSTACK
	.long Xcnxint+INTSTACK
	.long straycatch+0x100+INTSTACK
	.long straycatch+0x104+INTSTACK
	.long straycatch+0x108+INTSTACK
	.long straycatch+0x10c+INTSTACK
	.long straycatch+0x110+INTSTACK
	.long straycatch+0x114+INTSTACK
	.long straycatch+0x118+INTSTACK
	.long straycatch+0x11c+INTSTACK
	.long Xnbia0+INTSTACK
	.long straycatch+0x124+INTSTACK
	.long straycatch+0x128+INTSTACK
	.long straycatch+0x12c+INTSTACK
	.long Xnbia1+INTSTACK
	.long straycatch+0x134+INTSTACK
	.long straycatch+0x138+INTSTACK
	.long straycatch+0x13c+INTSTACK
	.long straycatch+0x140+INTSTACK
	.long straycatch+0x144+INTSTACK
	.long Xmemerr+INTSTACK
	.long straycatch+0x14c+INTSTACK
	.long straycatch+0x150+INTSTACK
	.long straycatch+0x154+INTSTACK
	.long straycatch+0x158+INTSTACK
	.long straycatch+0x15c+INTSTACK
	.long straycatch+0x160+INTSTACK
	.long straycatch+0x164+INTSTACK
	.long straycatch+0x168+INTSTACK
	.long straycatch+0x16c+INTSTACK
	.long straycatch+0x170+INTSTACK
	.long straycatch+0x174+INTSTACK
	.long straycatch+0x178+INTSTACK
	.long straycatch+0x17c+INTSTACK
	.long straycatch+0x180+INTSTACK
	.long straycatch+0x184+INTSTACK
	.long straycatch+0x188+INTSTACK
	.long straycatch+0x18c+INTSTACK
	.long straycatch+0x190+INTSTACK
	.long straycatch+0x194+INTSTACK
	.long straycatch+0x198+INTSTACK
	.long straycatch+0x19c+INTSTACK
	.long straycatch+0x1a0+INTSTACK
	.long straycatch+0x1a4+INTSTACK
	.long straycatch+0x1a8+INTSTACK
	.long straycatch+0x1ac+INTSTACK
	.long straycatch+0x1b0+INTSTACK
	.long straycatch+0x1b4+INTSTACK
	.long straycatch+0x1b8+INTSTACK
	.long straycatch+0x1bc+INTSTACK
	.long straycatch+0x1c0+INTSTACK
	.long straycatch+0x1c4+INTSTACK
	.long straycatch+0x1c8+INTSTACK
	.long straycatch+0x1cc+INTSTACK
	.long straycatch+0x1d0+INTSTACK
	.long straycatch+0x1d4+INTSTACK
	.long straycatch+0x1d8+INTSTACK
	.long straycatch+0x1dc+INTSTACK
	.long straycatch+0x1e0+INTSTACK
	.long straycatch+0x1e4+INTSTACK
	.long straycatch+0x1e8+INTSTACK
	.long straycatch+0x1ec+INTSTACK
	.long straycatch+0x1f0+INTSTACK
	.long straycatch+0x1f4+INTSTACK
	.long straycatch+0x1f8+INTSTACK
	.long straycatch+0x1fc+INTSTACK
	.long straycatch+0x200+INTSTACK
	.long straycatch+0x204+INTSTACK
	.long straycatch+0x208+INTSTACK
	.long straycatch+0x20c+INTSTACK
	.long straycatch+0x210+INTSTACK
	.long straycatch+0x214+INTSTACK
	.long straycatch+0x218+INTSTACK
	.long straycatch+0x21c+INTSTACK
	.long straycatch+0x220+INTSTACK
	.long straycatch+0x224+INTSTACK
	.long straycatch+0x228+INTSTACK
	.long straycatch+0x22c+INTSTACK
	.long straycatch+0x230+INTSTACK
	.long straycatch+0x234+INTSTACK
	.long straycatch+0x238+INTSTACK
	.long straycatch+0x23c+INTSTACK
	.long straycatch+0x240+INTSTACK
	.long straycatch+0x244+INTSTACK
	.long straycatch+0x248+INTSTACK
	.long straycatch+0x24c+INTSTACK
	.long straycatch+0x250+INTSTACK
	.long straycatch+0x254+INTSTACK
	.long straycatch+0x258+INTSTACK
	.long straycatch+0x25c+INTSTACK
	.long straycatch+0x260+INTSTACK
	.long straycatch+0x264+INTSTACK
	.long straycatch+0x268+INTSTACK
	.long straycatch+0x26c+INTSTACK
	.long straycatch+0x270+INTSTACK
	.long straycatch+0x274+INTSTACK
	.long straycatch+0x278+INTSTACK
	.long straycatch+0x27c+INTSTACK
	.long straycatch+0x280+INTSTACK
	.long straycatch+0x284+INTSTACK
	.long straycatch+0x288+INTSTACK
	.long straycatch+0x28c+INTSTACK
	.long straycatch+0x290+INTSTACK
	.long straycatch+0x294+INTSTACK
	.long straycatch+0x298+INTSTACK
	.long straycatch+0x29c+INTSTACK
	.long straycatch+0x2a0+INTSTACK
	.long straycatch+0x2a4+INTSTACK
	.long straycatch+0x2a8+INTSTACK
	.long straycatch+0x2ac+INTSTACK
	.long straycatch+0x2b0+INTSTACK
	.long straycatch+0x2b4+INTSTACK
	.long straycatch+0x2b8+INTSTACK
	.long straycatch+0x2bc+INTSTACK
	.long Xdz00+INTSTACK
	.long Xdz01+INTSTACK
	.long Xdz10+INTSTACK
	.long Xdz11+INTSTACK
	.long Xkmc00+INTSTACK
	.long Xkmc01+INTSTACK
	.long Xom00+INTSTACK
	.long straycatch+0x2dc+INTSTACK
	.long straycatch+0x2e0+INTSTACK
	.long straycatch+0x2e4+INTSTACK
	.long straycatch+0x2e8+INTSTACK
	.long straycatch+0x2ec+INTSTACK
	.long straycatch+0x2f0+INTSTACK
	.long straycatch+0x2f4+INTSTACK
	.long straycatch+0x2f8+INTSTACK
	.long straycatch+0x2fc+INTSTACK
	.long straycatch+0x300+INTSTACK
	.long straycatch+0x304+INTSTACK
	.long straycatch+0x308+INTSTACK
	.long straycatch+0x30c+INTSTACK
	.long Xud10+INTSTACK
	.long straycatch+0x314+INTSTACK
	.long straycatch+0x318+INTSTACK
	.long straycatch+0x31c+INTSTACK
	.long straycatch+0x320+INTSTACK
	.long straycatch+0x324+INTSTACK
	.long straycatch+0x328+INTSTACK
	.long straycatch+0x32c+INTSTACK
	.long straycatch+0x330+INTSTACK
	.long straycatch+0x334+INTSTACK
	.long straycatch+0x338+INTSTACK
	.long straycatch+0x33c+INTSTACK
	.long straycatch+0x340+INTSTACK
	.long straycatch+0x344+INTSTACK
	.long straycatch+0x348+INTSTACK
	.long straycatch+0x34c+INTSTACK
	.long straycatch+0x350+INTSTACK
	.long straycatch+0x354+INTSTACK
	.long straycatch+0x358+INTSTACK
	.long straycatch+0x35c+INTSTACK
	.long straycatch+0x360+INTSTACK
	.long straycatch+0x364+INTSTACK
	.long straycatch+0x368+INTSTACK
	.long straycatch+0x36c+INTSTACK
	.long straycatch+0x370+INTSTACK
	.long straycatch+0x374+INTSTACK
	.long straycatch+0x378+INTSTACK
	.long straycatch+0x37c+INTSTACK
	.long straycatch+0x380+INTSTACK
	.long straycatch+0x384+INTSTACK
	.long straycatch+0x388+INTSTACK
	.long straycatch+0x38c+INTSTACK
	.long straycatch+0x390+INTSTACK
	.long straycatch+0x394+INTSTACK
	.long straycatch+0x398+INTSTACK
	.long straycatch+0x39c+INTSTACK
	.long straycatch+0x3a0+INTSTACK
	.long straycatch+0x3a4+INTSTACK
	.long straycatch+0x3a8+INTSTACK
	.long straycatch+0x3ac+INTSTACK
	.long straycatch+0x3b0+INTSTACK
	.long straycatch+0x3b4+INTSTACK
	.long straycatch+0x3b8+INTSTACK
	.long straycatch+0x3bc+INTSTACK
	.long straycatch+0x3c0+INTSTACK
	.long straycatch+0x3c4+INTSTACK
	.long straycatch+0x3c8+INTSTACK
	.long straycatch+0x3cc+INTSTACK
	.long straycatch+0x3d0+INTSTACK
	.long straycatch+0x3d4+INTSTACK
	.long straycatch+0x3d8+INTSTACK
	.long straycatch+0x3dc+INTSTACK
	.long straycatch+0x3e0+INTSTACK
	.long straycatch+0x3e4+INTSTACK
	.long straycatch+0x3e8+INTSTACK
	.long straycatch+0x3ec+INTSTACK
	.long straycatch+0x3f0+INTSTACK
	.long straycatch+0x3f4+INTSTACK
	.long straycatch+0x3f8+INTSTACK
	.long straycatch+0x3fc+INTSTACK
	.long Xbia20+INTSTACK
	.long straycatch+0x404+INTSTACK
	.long straycatch+0x408+INTSTACK
	.long straycatch+0x40c+INTSTACK
	.long Xbd00+INTSTACK
	.long Xbd01+INTSTACK
	.long straycatch+0x418+INTSTACK
	.long straycatch+0x41c+INTSTACK
	.long straycatch+0x420+INTSTACK
	.long straycatch+0x424+INTSTACK
	.long straycatch+0x428+INTSTACK
	.long straycatch+0x42c+INTSTACK
	.long Xuba00+INTSTACK
	.long straycatch+0x434+INTSTACK
	.long straycatch+0x438+INTSTACK
	.long straycatch+0x43c+INTSTACK
	.long Xbna00+INTSTACK
	.long straycatch+0x444+INTSTACK
	.long straycatch+0x448+INTSTACK
	.long straycatch+0x44c+INTSTACK
	.long straycatch+0x450+INTSTACK
	.long straycatch+0x454+INTSTACK
	.long straycatch+0x458+INTSTACK
	.long straycatch+0x45c+INTSTACK
	.long straycatch+0x460+INTSTACK
	.long straycatch+0x464+INTSTACK
	.long straycatch+0x468+INTSTACK
	.long straycatch+0x46c+INTSTACK
	.long straycatch+0x470+INTSTACK
	.long straycatch+0x474+INTSTACK
	.long straycatch+0x478+INTSTACK
	.long straycatch+0x47c+INTSTACK
	.long straycatch+0x480+INTSTACK
	.long straycatch+0x484+INTSTACK
	.long straycatch+0x488+INTSTACK
	.long straycatch+0x48c+INTSTACK
	.long straycatch+0x490+INTSTACK
	.long straycatch+0x494+INTSTACK
	.long straycatch+0x498+INTSTACK
	.long straycatch+0x49c+INTSTACK
	.long straycatch+0x4a0+INTSTACK
	.long straycatch+0x4a4+INTSTACK
	.long straycatch+0x4a8+INTSTACK
	.long straycatch+0x4ac+INTSTACK
	.long straycatch+0x4b0+INTSTACK
	.long straycatch+0x4b4+INTSTACK
	.long straycatch+0x4b8+INTSTACK
	.long straycatch+0x4bc+INTSTACK
	.long straycatch+0x4c0+INTSTACK
	.long straycatch+0x4c4+INTSTACK
	.long straycatch+0x4c8+INTSTACK
	.long straycatch+0x4cc+INTSTACK
	.long straycatch+0x4d0+INTSTACK
	.long straycatch+0x4d4+INTSTACK
	.long straycatch+0x4d8+INTSTACK
	.long straycatch+0x4dc+INTSTACK
	.long straycatch+0x4e0+INTSTACK
	.long straycatch+0x4e4+INTSTACK
	.long straycatch+0x4e8+INTSTACK
	.long straycatch+0x4ec+INTSTACK
	.long straycatch+0x4f0+INTSTACK
	.long straycatch+0x4f4+INTSTACK
	.long straycatch+0x4f8+INTSTACK
	.long straycatch+0x4fc+INTSTACK
	.long straycatch+0x500+INTSTACK
	.long straycatch+0x504+INTSTACK
	.long straycatch+0x508+INTSTACK
	.long straycatch+0x50c+INTSTACK
	.long straycatch+0x510+INTSTACK
	.long straycatch+0x514+INTSTACK
	.long straycatch+0x518+INTSTACK
	.long straycatch+0x51c+INTSTACK
	.long straycatch+0x520+INTSTACK
	.long straycatch+0x524+INTSTACK
	.long straycatch+0x528+INTSTACK
	.long straycatch+0x52c+INTSTACK
	.long straycatch+0x530+INTSTACK
	.long straycatch+0x534+INTSTACK
	.long straycatch+0x538+INTSTACK
	.long straycatch+0x53c+INTSTACK
	.long straycatch+0x540+INTSTACK
	.long straycatch+0x544+INTSTACK
	.long straycatch+0x548+INTSTACK
	.long straycatch+0x54c+INTSTACK
	.long straycatch+0x550+INTSTACK
	.long straycatch+0x554+INTSTACK
	.long straycatch+0x558+INTSTACK
	.long straycatch+0x55c+INTSTACK
	.long straycatch+0x560+INTSTACK
	.long straycatch+0x564+INTSTACK
	.long straycatch+0x568+INTSTACK
	.long straycatch+0x56c+INTSTACK
	.long straycatch+0x570+INTSTACK
	.long straycatch+0x574+INTSTACK
	.long straycatch+0x578+INTSTACK
	.long straycatch+0x57c+INTSTACK
	.long straycatch+0x580+INTSTACK
	.long straycatch+0x584+INTSTACK
	.long straycatch+0x588+INTSTACK
	.long straycatch+0x58c+INTSTACK
	.long straycatch+0x590+INTSTACK
	.long straycatch+0x594+INTSTACK
	.long straycatch+0x598+INTSTACK
	.long straycatch+0x59c+INTSTACK
	.long straycatch+0x5a0+INTSTACK
	.long straycatch+0x5a4+INTSTACK
	.long straycatch+0x5a8+INTSTACK
	.long straycatch+0x5ac+INTSTACK
	.long straycatch+0x5b0+INTSTACK
	.long straycatch+0x5b4+INTSTACK
	.long straycatch+0x5b8+INTSTACK
	.long straycatch+0x5bc+INTSTACK
	.long straycatch+0x5c0+INTSTACK
	.long straycatch+0x5c4+INTSTACK
	.long straycatch+0x5c8+INTSTACK
	.long straycatch+0x5cc+INTSTACK
	.long straycatch+0x5d0+INTSTACK
	.long straycatch+0x5d4+INTSTACK
	.long straycatch+0x5d8+INTSTACK
	.long straycatch+0x5dc+INTSTACK
	.long straycatch+0x5e0+INTSTACK
	.long straycatch+0x5e4+INTSTACK
	.long straycatch+0x5e8+INTSTACK
	.long straycatch+0x5ec+INTSTACK
	.long straycatch+0x5f0+INTSTACK
	.long straycatch+0x5f4+INTSTACK
	.long straycatch+0x5f8+INTSTACK
	.long straycatch+0x5fc+INTSTACK
	.long Xbia30+INTSTACK
	.long straycatch+0x604+INTSTACK
	.long straycatch+0x608+INTSTACK
	.long straycatch+0x60c+INTSTACK
	.long Xbd10+INTSTACK
	.long Xbd11+INTSTACK
	.long straycatch+0x618+INTSTACK
	.long straycatch+0x61c+INTSTACK
	.long Xuba10+INTSTACK
	.long straycatch+0x624+INTSTACK
	.long straycatch+0x628+INTSTACK
	.long straycatch+0x62c+INTSTACK
	.long straycatch+0x630+INTSTACK
	.long straycatch+0x634+INTSTACK
	.long straycatch+0x638+INTSTACK
	.long straycatch+0x63c+INTSTACK
	.long straycatch+0x640+INTSTACK
	.long straycatch+0x644+INTSTACK
	.long straycatch+0x648+INTSTACK
	.long straycatch+0x64c+INTSTACK
	.long straycatch+0x650+INTSTACK
	.long straycatch+0x654+INTSTACK
	.long straycatch+0x658+INTSTACK
	.long straycatch+0x65c+INTSTACK
	.long straycatch+0x660+INTSTACK
	.long straycatch+0x664+INTSTACK
	.long straycatch+0x668+INTSTACK
	.long straycatch+0x66c+INTSTACK
	.long straycatch+0x670+INTSTACK
	.long straycatch+0x674+INTSTACK
	.long straycatch+0x678+INTSTACK
	.long straycatch+0x67c+INTSTACK
	.long straycatch+0x680+INTSTACK
	.long straycatch+0x684+INTSTACK
	.long straycatch+0x688+INTSTACK
	.long straycatch+0x68c+INTSTACK
	.long straycatch+0x690+INTSTACK
	.long straycatch+0x694+INTSTACK
	.long straycatch+0x698+INTSTACK
	.long straycatch+0x69c+INTSTACK
	.long straycatch+0x6a0+INTSTACK
	.long straycatch+0x6a4+INTSTACK
	.long straycatch+0x6a8+INTSTACK
	.long straycatch+0x6ac+INTSTACK
	.long straycatch+0x6b0+INTSTACK
	.long straycatch+0x6b4+INTSTACK
	.long straycatch+0x6b8+INTSTACK
	.long straycatch+0x6bc+INTSTACK
	.long straycatch+0x6c0+INTSTACK
	.long straycatch+0x6c4+INTSTACK
	.long straycatch+0x6c8+INTSTACK
	.long straycatch+0x6cc+INTSTACK
	.long straycatch+0x6d0+INTSTACK
	.long straycatch+0x6d4+INTSTACK
	.long straycatch+0x6d8+INTSTACK
	.long straycatch+0x6dc+INTSTACK
	.long straycatch+0x6e0+INTSTACK
	.long straycatch+0x6e4+INTSTACK
	.long straycatch+0x6e8+INTSTACK
	.long straycatch+0x6ec+INTSTACK
	.long straycatch+0x6f0+INTSTACK
	.long straycatch+0x6f4+INTSTACK
	.long straycatch+0x6f8+INTSTACK
	.long straycatch+0x6fc+INTSTACK
	.long straycatch+0x700+INTSTACK
	.long straycatch+0x704+INTSTACK
	.long straycatch+0x708+INTSTACK
	.long straycatch+0x70c+INTSTACK
	.long straycatch+0x710+INTSTACK
	.long straycatch+0x714+INTSTACK
	.long straycatch+0x718+INTSTACK
	.long straycatch+0x71c+INTSTACK
	.long straycatch+0x720+INTSTACK
	.long straycatch+0x724+INTSTACK
	.long straycatch+0x728+INTSTACK
	.long straycatch+0x72c+INTSTACK
	.long straycatch+0x730+INTSTACK
	.long straycatch+0x734+INTSTACK
	.long straycatch+0x738+INTSTACK
	.long straycatch+0x73c+INTSTACK
	.long straycatch+0x740+INTSTACK
	.long straycatch+0x744+INTSTACK
	.long straycatch+0x748+INTSTACK
	.long straycatch+0x74c+INTSTACK
	.long straycatch+0x750+INTSTACK
	.long straycatch+0x754+INTSTACK
	.long straycatch+0x758+INTSTACK
	.long straycatch+0x75c+INTSTACK
	.long straycatch+0x760+INTSTACK
	.long straycatch+0x764+INTSTACK
	.long straycatch+0x768+INTSTACK
	.long straycatch+0x76c+INTSTACK
	.long straycatch+0x770+INTSTACK
	.long straycatch+0x774+INTSTACK
	.long straycatch+0x778+INTSTACK
	.long straycatch+0x77c+INTSTACK
	.long straycatch+0x780+INTSTACK
	.long straycatch+0x784+INTSTACK
	.long straycatch+0x788+INTSTACK
	.long straycatch+0x78c+INTSTACK
	.long straycatch+0x790+INTSTACK
	.long straycatch+0x794+INTSTACK
	.long straycatch+0x798+INTSTACK
	.long straycatch+0x79c+INTSTACK
	.long straycatch+0x7a0+INTSTACK
	.long straycatch+0x7a4+INTSTACK
	.long straycatch+0x7a8+INTSTACK
	.long straycatch+0x7ac+INTSTACK
	.long straycatch+0x7b0+INTSTACK
	.long straycatch+0x7b4+INTSTACK
	.long straycatch+0x7b8+INTSTACK
	.long straycatch+0x7bc+INTSTACK
	.long straycatch+0x7c0+INTSTACK
	.long straycatch+0x7c4+INTSTACK
	.long straycatch+0x7c8+INTSTACK
	.long straycatch+0x7cc+INTSTACK
	.long straycatch+0x7d0+INTSTACK
	.long straycatch+0x7d4+INTSTACK
	.long straycatch+0x7d8+INTSTACK
	.long straycatch+0x7dc+INTSTACK
	.long straycatch+0x7e0+INTSTACK
	.long straycatch+0x7e4+INTSTACK
	.long straycatch+0x7e8+INTSTACK
	.long straycatch+0x7ec+INTSTACK
	.long straycatch+0x7f0+INTSTACK
	.long straycatch+0x7f4+INTSTACK
	.long straycatch+0x7f8+INTSTACK
	.long straycatch+0x7fc+INTSTACK
	.long straycatch+0x800+INTSTACK
	.long straycatch+0x804+INTSTACK
	.long straycatch+0x808+INTSTACK
	.long straycatch+0x80c+INTSTACK
	.long straycatch+0x810+INTSTACK
	.long straycatch+0x814+INTSTACK
	.long straycatch+0x818+INTSTACK
	.long straycatch+0x81c+INTSTACK
	.long straycatch+0x820+INTSTACK
	.long straycatch+0x824+INTSTACK
	.long straycatch+0x828+INTSTACK
	.long straycatch+0x82c+INTSTACK
	.long straycatch+0x830+INTSTACK
	.long straycatch+0x834+INTSTACK
	.long straycatch+0x838+INTSTACK
	.long straycatch+0x83c+INTSTACK
	.long straycatch+0x840+INTSTACK
	.long straycatch+0x844+INTSTACK
	.long straycatch+0x848+INTSTACK
	.long straycatch+0x84c+INTSTACK
	.long straycatch+0x850+INTSTACK
	.long straycatch+0x854+INTSTACK
	.long straycatch+0x858+INTSTACK
	.long straycatch+0x85c+INTSTACK
	.long straycatch+0x860+INTSTACK
	.long straycatch+0x864+INTSTACK
	.long straycatch+0x868+INTSTACK
	.long straycatch+0x86c+INTSTACK
	.long straycatch+0x870+INTSTACK
	.long straycatch+0x874+INTSTACK
	.long straycatch+0x878+INTSTACK
	.long straycatch+0x87c+INTSTACK
	.long straycatch+0x880+INTSTACK
	.long straycatch+0x884+INTSTACK
	.long straycatch+0x888+INTSTACK
	.long straycatch+0x88c+INTSTACK
	.long straycatch+0x890+INTSTACK
	.long straycatch+0x894+INTSTACK
	.long straycatch+0x898+INTSTACK
	.long straycatch+0x89c+INTSTACK
	.long straycatch+0x8a0+INTSTACK
	.long straycatch+0x8a4+INTSTACK
	.long straycatch+0x8a8+INTSTACK
	.long straycatch+0x8ac+INTSTACK
	.long straycatch+0x8b0+INTSTACK
	.long straycatch+0x8b4+INTSTACK
	.long straycatch+0x8b8+INTSTACK
	.long straycatch+0x8bc+INTSTACK
	.long straycatch+0x8c0+INTSTACK
	.long straycatch+0x8c4+INTSTACK
	.long straycatch+0x8c8+INTSTACK
	.long straycatch+0x8cc+INTSTACK
	.long straycatch+0x8d0+INTSTACK
	.long straycatch+0x8d4+INTSTACK
	.long straycatch+0x8d8+INTSTACK
	.long straycatch+0x8dc+INTSTACK
	.long straycatch+0x8e0+INTSTACK
	.long straycatch+0x8e4+INTSTACK
	.long straycatch+0x8e8+INTSTACK
	.long straycatch+0x8ec+INTSTACK
	.long straycatch+0x8f0+INTSTACK
	.long straycatch+0x8f4+INTSTACK
	.long straycatch+0x8f8+INTSTACK
	.long straycatch+0x8fc+INTSTACK
	.long straycatch+0x900+INTSTACK
	.long straycatch+0x904+INTSTACK
	.long straycatch+0x908+INTSTACK
	.long straycatch+0x90c+INTSTACK
	.long straycatch+0x910+INTSTACK
	.long straycatch+0x914+INTSTACK
	.long straycatch+0x918+INTSTACK
	.long straycatch+0x91c+INTSTACK
	.long straycatch+0x920+INTSTACK
	.long straycatch+0x924+INTSTACK
	.long straycatch+0x928+INTSTACK
	.long straycatch+0x92c+INTSTACK
	.long straycatch+0x930+INTSTACK
	.long straycatch+0x934+INTSTACK
	.long straycatch+0x938+INTSTACK
	.long straycatch+0x93c+INTSTACK
	.long straycatch+0x940+INTSTACK
	.long straycatch+0x944+INTSTACK
	.long straycatch+0x948+INTSTACK
	.long straycatch+0x94c+INTSTACK
	.long straycatch+0x950+INTSTACK
	.long straycatch+0x954+INTSTACK
	.long straycatch+0x958+INTSTACK
	.long straycatch+0x95c+INTSTACK
	.long straycatch+0x960+INTSTACK
	.long straycatch+0x964+INTSTACK
	.long straycatch+0x968+INTSTACK
	.long straycatch+0x96c+INTSTACK
	.long straycatch+0x970+INTSTACK
	.long straycatch+0x974+INTSTACK
	.long straycatch+0x978+INTSTACK
	.long straycatch+0x97c+INTSTACK
	.long straycatch+0x980+INTSTACK
	.long straycatch+0x984+INTSTACK
	.long straycatch+0x988+INTSTACK
	.long straycatch+0x98c+INTSTACK
	.long straycatch+0x990+INTSTACK
	.long straycatch+0x994+INTSTACK
	.long straycatch+0x998+INTSTACK
	.long straycatch+0x99c+INTSTACK
	.long straycatch+0x9a0+INTSTACK
	.long straycatch+0x9a4+INTSTACK
	.long straycatch+0x9a8+INTSTACK
	.long straycatch+0x9ac+INTSTACK
	.long straycatch+0x9b0+INTSTACK
	.long straycatch+0x9b4+INTSTACK
	.long straycatch+0x9b8+INTSTACK
	.long straycatch+0x9bc+INTSTACK
	.long straycatch+0x9c0+INTSTACK
	.long straycatch+0x9c4+INTSTACK
	.long straycatch+0x9c8+INTSTACK
	.long straycatch+0x9cc+INTSTACK
	.long straycatch+0x9d0+INTSTACK
	.long straycatch+0x9d4+INTSTACK
	.long straycatch+0x9d8+INTSTACK
	.long straycatch+0x9dc+INTSTACK
	.long straycatch+0x9e0+INTSTACK
	.long straycatch+0x9e4+INTSTACK
	.long straycatch+0x9e8+INTSTACK
	.long straycatch+0x9ec+INTSTACK
	.long straycatch+0x9f0+INTSTACK
	.long straycatch+0x9f4+INTSTACK
	.long straycatch+0x9f8+INTSTACK
	.long straycatch+0x9fc+INTSTACK
	.long straycatch+0xa00+INTSTACK
	.long straycatch+0xa04+INTSTACK
	.long straycatch+0xa08+INTSTACK
	.long straycatch+0xa0c+INTSTACK
	.long straycatch+0xa10+INTSTACK
	.long straycatch+0xa14+INTSTACK
	.long straycatch+0xa18+INTSTACK
	.long straycatch+0xa1c+INTSTACK
	.long straycatch+0xa20+INTSTACK
	.long straycatch+0xa24+INTSTACK
	.long straycatch+0xa28+INTSTACK
	.long straycatch+0xa2c+INTSTACK
	.long straycatch+0xa30+INTSTACK
	.long straycatch+0xa34+INTSTACK
	.long straycatch+0xa38+INTSTACK
	.long straycatch+0xa3c+INTSTACK
	.long straycatch+0xa40+INTSTACK
	.long straycatch+0xa44+INTSTACK
	.long straycatch+0xa48+INTSTACK
	.long straycatch+0xa4c+INTSTACK
	.long straycatch+0xa50+INTSTACK
	.long straycatch+0xa54+INTSTACK
	.long straycatch+0xa58+INTSTACK
	.long straycatch+0xa5c+INTSTACK
	.long straycatch+0xa60+INTSTACK
	.long straycatch+0xa64+INTSTACK
	.long straycatch+0xa68+INTSTACK
	.long straycatch+0xa6c+INTSTACK
	.long straycatch+0xa70+INTSTACK
	.long straycatch+0xa74+INTSTACK
	.long straycatch+0xa78+INTSTACK
	.long straycatch+0xa7c+INTSTACK
	.long straycatch+0xa80+INTSTACK
	.long straycatch+0xa84+INTSTACK
	.long straycatch+0xa88+INTSTACK
	.long straycatch+0xa8c+INTSTACK
	.long straycatch+0xa90+INTSTACK
	.long straycatch+0xa94+INTSTACK
	.long straycatch+0xa98+INTSTACK
	.long straycatch+0xa9c+INTSTACK
	.long straycatch+0xaa0+INTSTACK
	.long straycatch+0xaa4+INTSTACK
	.long straycatch+0xaa8+INTSTACK
	.long straycatch+0xaac+INTSTACK
	.long straycatch+0xab0+INTSTACK
	.long straycatch+0xab4+INTSTACK
	.long straycatch+0xab8+INTSTACK
	.long straycatch+0xabc+INTSTACK
	.long straycatch+0xac0+INTSTACK
	.long straycatch+0xac4+INTSTACK
	.long Xud00+INTSTACK
	.long straycatch+0xacc+INTSTACK
	.long straycatch+0xad0+INTSTACK
	.long straycatch+0xad4+INTSTACK
	.long straycatch+0xad8+INTSTACK
	.long straycatch+0xadc+INTSTACK
	.long straycatch+0xae0+INTSTACK
	.long straycatch+0xae4+INTSTACK
	.long straycatch+0xae8+INTSTACK
	.long straycatch+0xaec+INTSTACK
	.long straycatch+0xaf0+INTSTACK
	.long straycatch+0xaf4+INTSTACK
	.long straycatch+0xaf8+INTSTACK
	.long straycatch+0xafc+INTSTACK
	.long straycatch+0xb00+INTSTACK
	.long straycatch+0xb04+INTSTACK
	.long straycatch+0xb08+INTSTACK
	.long straycatch+0xb0c+INTSTACK
	.long straycatch+0xb10+INTSTACK
	.long straycatch+0xb14+INTSTACK
	.long straycatch+0xb18+INTSTACK
	.long straycatch+0xb1c+INTSTACK
	.long straycatch+0xb20+INTSTACK
	.long straycatch+0xb24+INTSTACK
	.long straycatch+0xb28+INTSTACK
	.long straycatch+0xb2c+INTSTACK
	.long straycatch+0xb30+INTSTACK
	.long straycatch+0xb34+INTSTACK
	.long straycatch+0xb38+INTSTACK
	.long straycatch+0xb3c+INTSTACK
	.long straycatch+0xb40+INTSTACK
	.long straycatch+0xb44+INTSTACK
	.long straycatch+0xb48+INTSTACK
	.long straycatch+0xb4c+INTSTACK
	.long straycatch+0xb50+INTSTACK
	.long straycatch+0xb54+INTSTACK
	.long straycatch+0xb58+INTSTACK
	.long straycatch+0xb5c+INTSTACK
	.long straycatch+0xb60+INTSTACK
	.long straycatch+0xb64+INTSTACK
	.long straycatch+0xb68+INTSTACK
	.long straycatch+0xb6c+INTSTACK
	.long straycatch+0xb70+INTSTACK
	.long straycatch+0xb74+INTSTACK
	.long straycatch+0xb78+INTSTACK
	.long straycatch+0xb7c+INTSTACK
	.long straycatch+0xb80+INTSTACK
	.long straycatch+0xb84+INTSTACK
	.long straycatch+0xb88+INTSTACK
	.long straycatch+0xb8c+INTSTACK
	.long straycatch+0xb90+INTSTACK
	.long straycatch+0xb94+INTSTACK
	.long straycatch+0xb98+INTSTACK
	.long straycatch+0xb9c+INTSTACK
	.long straycatch+0xba0+INTSTACK
	.long straycatch+0xba4+INTSTACK
	.long straycatch+0xba8+INTSTACK
	.long straycatch+0xbac+INTSTACK
	.long straycatch+0xbb0+INTSTACK
	.long straycatch+0xbb4+INTSTACK
	.long straycatch+0xbb8+INTSTACK
	.long straycatch+0xbbc+INTSTACK
	.long straycatch+0xbc0+INTSTACK
	.long straycatch+0xbc4+INTSTACK
	.long straycatch+0xbc8+INTSTACK
	.long straycatch+0xbcc+INTSTACK
	.long straycatch+0xbd0+INTSTACK
	.long straycatch+0xbd4+INTSTACK
	.long straycatch+0xbd8+INTSTACK
	.long straycatch+0xbdc+INTSTACK
	.long straycatch+0xbe0+INTSTACK
	.long straycatch+0xbe4+INTSTACK
	.long straycatch+0xbe8+INTSTACK
	.long straycatch+0xbec+INTSTACK
	.long straycatch+0xbf0+INTSTACK
	.long straycatch+0xbf4+INTSTACK
	.long straycatch+0xbf8+INTSTACK
	.long straycatch+0xbfc+INTSTACK
	.globl	straycatch
straycatch:
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
	.align 2; bsbw stray
stray:	jmp Xstray
	.align 2
.globl _bia0int
Xbia20:
	pushr	$0x3f
	pushl	$2
	calls	$1,_bia0int
	popr	$0x3f
	rei
	.align 2
.globl _bia0int
Xbia30:
	pushr	$0x3f
	pushl	$3
	calls	$1,_bia0int
	popr	$0x3f
	rei
	.align 2
.globl _uba0int
Xuba00:
	pushr	$0x3f
	clrl	-(sp)
	calls	$1,_uba0int
	popr	$0x3f
	rei
	.align 2
.globl _uba0int
Xuba10:
	pushr	$0x3f
	pushl	$1
	calls	$1,_uba0int
	popr	$0x3f
	rei
	.align 2
.globl _ud0int
Xud00:
	pushr	$0x3f
	clrl	-(sp)
	calls	$1,_ud0int
	popr	$0x3f
	rei
	.align 2
.globl _ud0int
Xud10:
	pushr	$0x3f
	pushl	$1
	calls	$1,_ud0int
	popr	$0x3f
	rei
	.align 2
.globl _bna0int
Xbna00:
	pushr	$0x3f
	clrl	-(sp)
	calls	$1,_bna0int
	popr	$0x3f
	rei
	.align 2
.globl _bd0int
Xbd00:
	pushr	$0x3f
	clrl	-(sp)
	calls	$1,_bd0int
	popr	$0x3f
	rei
	.align 2
.globl _bd1int
Xbd01:
	pushr	$0x3f
	clrl	-(sp)
	calls	$1,_bd1int
	popr	$0x3f
	rei
	.align 2
.globl _bd0int
Xbd10:
	pushr	$0x3f
	pushl	$1
	calls	$1,_bd0int
	popr	$0x3f
	rei
	.align 2
.globl _bd1int
Xbd11:
	pushr	$0x3f
	pushl	$1
	calls	$1,_bd1int
	popr	$0x3f
	rei
	.align 2
.globl _kmc0int
Xkmc00:
	pushr	$0x3f
	clrl	-(sp)
	calls	$1,_kmc0int
	popr	$0x3f
	rei
	.align 2
.globl _kmc1int
Xkmc01:
	pushr	$0x3f
	clrl	-(sp)
	calls	$1,_kmc1int
	popr	$0x3f
	rei
	.align 2
.globl _om0int
Xom00:
	pushr	$0x3f
	clrl	-(sp)
	calls	$1,_om0int
	popr	$0x3f
	rei
	.align 2
.globl _dz0int
Xdz00:
	pushr	$0x3f
	clrl	-(sp)
	calls	$1,_dz0int
	popr	$0x3f
	rei
	.align 2
.globl _dz1int
Xdz01:
	pushr	$0x3f
	clrl	-(sp)
	calls	$1,_dz1int
	popr	$0x3f
	rei
	.align 2
.globl _dz0int
Xdz10:
	pushr	$0x3f
	pushl	$1
	calls	$1,_dz0int
	popr	$0x3f
	rei
	.align 2
.globl _dz1int
Xdz11:
	pushr	$0x3f
	pushl	$1
	calls	$1,_dz1int
	popr	$0x3f
	rei
