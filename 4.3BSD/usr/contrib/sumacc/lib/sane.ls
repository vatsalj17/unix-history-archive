   1                                                        |-----------------------------------------------------------
   2                                                        | 28aug82: written by jerome coonen
   3                                                        | 13oct82: remove uses of d0 across fp calls (jtc).
   4                                                        | 12dec82: fix bugs in class fcns (jtc).
   5                                                        | 28dec82: split nan class to snan, qnan; scalb, logb (jtc).
   6                                                        | 29dec82: i2x, l2x pass by value here. (jtc)
   7                                                        | 27apr83: neg as core function added. (jtc)
   8                                                        | 25may84: C version, uses macros to save my fingers. (croft)
   9                                                        |-----------------------------------------------------------
 572                                                        
 573                                                        __fp68k = /a9eb		| avoids including toolmacs.h
 574                                                        __elems68k = /a9ec
 575                                                        
 576                                                        	.text
 577                                                        
 578                                                        	.macro cfp,c
 579                                                        	movw	#c,sp@-
 580                                                        	.word	__fp68k
 581                                                        	.endm
 582                                                        	.macro cel,c
 583                                                        	movw	#c,sp@-
 584                                                        	.word	__elems68k
 585                                                        	.endm
 586                                                        	.macro c1,l,c
 587                                                        	.globl l
 588                                                        l:	movw	#c,d0
 589                                                        	bra	call1
 590                                                        	.endm
 591                                                        	.macro c2,l,c
 592                                                        	.globl l
 593                                                        l:	movw	#c,d0
 594                                                        	bra	call2
 595                                                        	.endm
 596                                                        	.macro c3,l,c
 597                                                        	.globl l
 598                                                        l:	movw	#c,d0
 599                                                        	bra	call3
 600                                                        	.endm
 601                                                        
 602                                                        	c2,AddS,fadds
 603*                                                       	.globl AddS
 604*   00 0000  303C  1000                                 AddS:	movw	#fadds,d0
 605*   00 0004  6000  018C                                 	bra	call2
 606                                                        	c2,AddD,faddd
 607*                                                       	.globl AddD
 608*   00 0008  303C  0800                                 AddD:	movw	#faddd,d0
 609*   00 000C  6000  0184                                 	bra	call2
 610                                                        	c2,AddC,faddc
 611*                                                       	.globl AddC
 612*   00 0010  303C  3000                                 AddC:	movw	#faddc,d0
 613*   00 0014  6000  017C                                 	bra	call2
 614                                                        	c2,AddX,faddx
 615*                                                       	.globl AddX
 616*   00 0018  303C  0000                                 AddX:	movw	#faddx,d0
 617*   00 001C  6000  0174                                 	bra	call2
 618                                                        	c2,SubS,fsubs
 619*                                                       	.globl SubS
 620*   00 0020  303C  1002                                 SubS:	movw	#fsubs,d0
 621*   00 0024  6000  016C                                 	bra	call2
 622                                                        	c2,SubD,fsubd
 623*                                                       	.globl SubD
 624*   00 0028  303C  0802                                 SubD:	movw	#fsubd,d0
 625*   00 002C  6000  0164                                 	bra	call2
 626                                                        	c2,SubC,fsubc
 627*                                                       	.globl SubC
 628*   00 0030  303C  3002                                 SubC:	movw	#fsubc,d0
 629*   00 0034  6000  015C                                 	bra	call2
 630                                                        	c2,SubX,fsubx
 631*                                                       	.globl SubX
 632*   00 0038  303C  0002                                 SubX:	movw	#fsubx,d0
 633*   00 003C  6000  0154                                 	bra	call2
 634                                                        	c2,MulS,fmuls
 635*                                                       	.globl MulS
 636*   00 0040  303C  1004                                 MulS:	movw	#fmuls,d0
 637*   00 0044  6000  014C                                 	bra	call2
 638                                                        	c2,MulD,fmuld
 639*                                                       	.globl MulD
 640*   00 0048  303C  0804                                 MulD:	movw	#fmuld,d0
 641*   00 004C  6000  0144                                 	bra	call2
 642                                                        	c2,MulC,fmulc
 643*                                                       	.globl MulC
 644*   00 0050  303C  3004                                 MulC:	movw	#fmulc,d0
 645*   00 0054  6000  013C                                 	bra	call2
 646                                                        	c2,MulX,fmulx
 647*                                                       	.globl MulX
 648*   00 0058  303C  0004                                 MulX:	movw	#fmulx,d0
 649*   00 005C  6000  0134                                 	bra	call2
 650                                                        	c2,DivS,fdivs
 651*                                                       	.globl DivS
 652*   00 0060  303C  1006                                 DivS:	movw	#fdivs,d0
 653*   00 0064  6000  012C                                 	bra	call2
 654                                                        	c2,DivD,fdivd
 655*                                                       	.globl DivD
 656*   00 0068  303C  0806                                 DivD:	movw	#fdivd,d0
 657*   00 006C  6000  0124                                 	bra	call2
 658                                                        	c2,DivC,fdivc
 659*                                                       	.globl DivC
 660*   00 0070  303C  3006                                 DivC:	movw	#fdivc,d0
 661*   00 0074  6000  011C                                 	bra	call2
 662                                                        	c2,DivX,fdivx
 663*                                                       	.globl DivX
 664*   00 0078  303C  0006                                 DivX:	movw	#fdivx,d0
 665*   00 007C  6000  0114                                 	bra	call2
 666                                                        
 667                                                        	c2,I2X,fi2x
 668*                                                       	.globl I2X
 669*   00 0080  303C  200E                                 I2X:	movw	#fi2x,d0
 670*   00 0084  6000  010C                                 	bra	call2
 671                                                        	c2,L2X,fl2x
 672*                                                       	.globl L2X
 673*   00 0088  303C  280E                                 L2X:	movw	#fl2x,d0
 674*   00 008C  6000  0104                                 	bra	call2
 675                                                        	c2,S2X,fs2x
 676*                                                       	.globl S2X
 677*   00 0090  303C  100E                                 S2X:	movw	#fs2x,d0
 678*   00 0094  6000  00FC                                 	bra	call2
 679                                                        	c2,D2X,fd2x
 680*                                                       	.globl D2X
 681*   00 0098  303C  080E                                 D2X:	movw	#fd2x,d0
 682*   00 009C  6000  00F4                                 	bra	call2
 683                                                        	c2,C2X,fc2x
 684*                                                       	.globl C2X
 685*   00 00A0  303C  300E                                 C2X:	movw	#fc2x,d0
 686*   00 00A4  6000  00EC                                 	bra	call2
 687                                                        	c2,X2X,fx2x
 688*                                                       	.globl X2X
 689*   00 00A8  303C  000E                                 X2X:	movw	#fx2x,d0
 690*   00 00AC  6000  00E4                                 	bra	call2
 691                                                        	c2,X2I,fx2i
 692*                                                       	.globl X2I
 693*   00 00B0  303C  2010                                 X2I:	movw	#fx2i,d0
 694*   00 00B4  6000  00DC                                 	bra	call2
 695                                                        	c2,X2L,fx2l
 696*                                                       	.globl X2L
 697*   00 00B8  303C  2810                                 X2L:	movw	#fx2l,d0
 698*   00 00BC  6000  00D4                                 	bra	call2
 699                                                        	c2,X2S,fx2s
 700*                                                       	.globl X2S
 701*   00 00C0  303C  1010                                 X2S:	movw	#fx2s,d0
 702*   00 00C4  6000  00CC                                 	bra	call2
 703                                                        	c2,X2D,fx2d
 704*                                                       	.globl X2D
 705*   00 00C8  303C  0810                                 X2D:	movw	#fx2d,d0
 706*   00 00CC  6000  00C4                                 	bra	call2
 707                                                        	c2,X2C,fx2c
 708*                                                       	.globl X2C
 709*   00 00D0  303C  3010                                 X2C:	movw	#fx2c,d0
 710*   00 00D4  6000  00BC                                 	bra	call2
 711                                                        	c2,Dec2S,fdec2s
 712*                                                       	.globl Dec2S
 713*   00 00D8  303C  1009                                 Dec2S:	movw	#fdec2s,d0
 714*   00 00DC  6000  00B4                                 	bra	call2
 715                                                        	c2,Dec2D,fdec2d
 716*                                                       	.globl Dec2D
 717*   00 00E0  303C  0809                                 Dec2D:	movw	#fdec2d,d0
 718*   00 00E4  6000  00AC                                 	bra	call2
 719                                                        	c2,Dec2C,fdec2c
 720*                                                       	.globl Dec2C
 721*   00 00E8  303C  3009                                 Dec2C:	movw	#fdec2c,d0
 722*   00 00EC  6000  00A4                                 	bra	call2
 723                                                        	c2,Dec2X,fdec2x
 724*                                                       	.globl Dec2X
 725*   00 00F0  303C  0009                                 Dec2X:	movw	#fdec2x,d0
 726*   00 00F4  6000  009C                                 	bra	call2
 727                                                        	c3,S2Dec,fs2dec
 728*                                                       	.globl S2Dec
 729*   00 00F8  303C  100B                                 S2Dec:	movw	#fs2dec,d0
 730*   00 00FC  6000  00A8                                 	bra	call3
 731                                                        	c3,D2Dec,fd2dec
 732*                                                       	.globl D2Dec
 733*   00 0100  303C  080B                                 D2Dec:	movw	#fd2dec,d0
 734*   00 0104  6000  00A0                                 	bra	call3
 735                                                        	c3,C2Dec,fc2dec
 736*                                                       	.globl C2Dec
 737*   00 0108  303C  300B                                 C2Dec:	movw	#fc2dec,d0
 738*   00 010C  6000  0098                                 	bra	call3
 739                                                        	c3,X2Dec,fx2dec
 740*                                                       	.globl X2Dec
 741*   00 0110  303C  000B                                 X2Dec:	movw	#fx2dec,d0
 742*   00 0114  6000  0090                                 	bra	call3
 743                                                        
 744                                                        	c1,SqrtX,fsqrtx
 745*                                                       	.globl SqrtX
 746*   00 0118  303C  0012                                 SqrtX:	movw	#fsqrtx,d0
 747*   00 011C  6000  006A                                 	bra	call1
 748                                                        	c1,RintX,frintx
 749*                                                       	.globl RintX
 750*   00 0120  303C  0014                                 RintX:	movw	#frintx,d0
 751*   00 0124  6000  0062                                 	bra	call1
 752                                                        	c1,NegX,fnegx
 753*                                                       	.globl NegX
 754*   00 0128  303C  000D                                 NegX:	movw	#fnegx,d0
 755*   00 012C  6000  005A                                 	bra	call1
 756                                                        	c1,AbsX,fabsx
 757*                                                       	.globl AbsX
 758*   00 0130  303C  000F                                 AbsX:	movw	#fabsx,d0
 759*   00 0134  6000  0052                                 	bra	call1
 760                                                        	c2,CpySgnX,fcpysgnx
 761*                                                       	.globl CpySgnX
 762*   00 0138  303C  0011                                 CpySgnX:	movw	#fcpysgnx,d0
 763*   00 013C  6000  0054                                 	bra	call2
 764                                                        	c2,ScalbX,fscalbx
 765*                                                       	.globl ScalbX
 766*   00 0140  303C  2018                                 ScalbX:	movw	#fscalbx,d0
 767*   00 0144  6000  004C                                 	bra	call2
 768                                                        	c1,LogbX,flogbx
 769*                                                       	.globl LogbX
 770*   00 0148  303C  001A                                 LogbX:	movw	#flogbx,d0
 771*   00 014C  6000  003A                                 	bra	call1
 772                                                        	c2,NextS,fnexts
 773*                                                       	.globl NextS
 774*   00 0150  303C  1013                                 NextS:	movw	#fnexts,d0
 775*   00 0154  6000  003C                                 	bra	call2
 776                                                        	c2,NextD,fnextd
 777*                                                       	.globl NextD
 778*   00 0158  303C  0813                                 NextD:	movw	#fnextd,d0
 779*   00 015C  6000  0034                                 	bra	call2
 780                                                        	c2,NextX,fnextx
 781*                                                       	.globl NextX
 782*   00 0160  303C  0013                                 NextX:	movw	#fnextx,d0
 783*   00 0164  6000  002C                                 	bra	call2
 784                                                        
 785                                                        	c1,SetEnv,fsetenv
 786*                                                       	.globl SetEnv
 787*   00 0168  303C  0001                                 SetEnv:	movw	#fsetenv,d0
 788*   00 016C  6000  001A                                 	bra	call1
 789                                                        	c1,GetEnv,fgetenv
 790*                                                       	.globl GetEnv
 791*   00 0170  303C  0003                                 GetEnv:	movw	#fgetenv,d0
 792*   00 0174  6000  0012                                 	bra	call1
 793                                                        	c1,ProcEntry,fprocentry
 794*                                                       	.globl ProcEntry
 795*   00 0178  303C  0017                                 ProcEntry:	movw	#fprocentry,d0
 796*   00 017C  6000  000A                                 	bra	call1
 797                                                        	c1,ProcExit,fprocexit
 798*                                                       	.globl ProcExit
 799*   00 0180  303C  0019                                 ProcExit:	movw	#fprocexit,d0
 800*   00 0184  6000  0002                                 	bra	call1
 801                                                        
 802    00 0188  205F                                       call1:	movl	sp@+,a0
 803    00 018A  3F00                                       	movw	d0,sp@-
 804    00 018C  A9EB                                       	.word	__fp68k
 805    00 018E  598F                                       	subql	#4,sp
 806    00 0190  4ED0                                       	jmp	a0@
 807                                                        
 808    00 0192  205F                                       call2:	movl	sp@+,a0
 809    00 0194  2217                                       	movl	sp@,d1
 810    00 0196  2EAF  0004                                 	movl	sp@(4),sp@
 811    00 019A  2F41  0004                                 	movl	d1,sp@(4)
 812    00 019E  3F00                                       	movw	d0,sp@-
 813    00 01A0  A9EB                                       	.word	__fp68k
 814    00 01A2  518F                                       	subql	#8,sp
 815    00 01A4  4ED0                                       	jmp	a0@
 816                                                        
 817    00 01A6  205F                                       call3:	movl	sp@+,a0
 818    00 01A8  2217                                       	movl	sp@,d1
 819    00 01AA  2EAF  0008                                 	movl	sp@(8),sp@
 820    00 01AE  2F41  0008                                 	movl	d1,sp@(8)
 821    00 01B2  3F00                                       	movw	d0,sp@-
 822    00 01B4  A9EB                                       	.word	__fp68k
 823    00 01B6  9EFC  000C                                 	subw	#12,sp
 824    00 01BA  4ED0                                       	jmp	a0@
 825                                                        
 826                                                        
 827                                                        
 828                                                                .globl    CmpX
 829    00 01BC                                             CmpX:
 830    00 01BC  205F                                               movl   sp@+,a0        | ret address
 831    00 01BE  222F  0004                                         movl    sp@(4),d1        | relop to be tested
 832    00 01C2  5201                                               addqb  #1,d1           | handy to increment
 833                                                        
 834    00 01C4  2017                                               movl    sp@,d0         | swap addresses (C already swapped)
 835    00 01C6  548F                                               addql  #2,sp           | kill slot for relop
 836    00 01C8  2E80                                       	movl	d0,sp@
 837                                                        
 838    00 01CA  0C01  0004                                         cmpb  #4,d1           | equal
 839    00 01CE  670E                                               beqs   .L1
 840    00 01D0  0C01  0008                                         cmpb  #8,d1           | unordered
 841    00 01D4  6708                                               beqs   .L1
 842                                                        
 843                                                                cfp fcpxx
 844*   00 01D6  3F3C  000A                                 	movw	#fcpxx,sp@-
 845*   00 01DA  A9EB                                       	.word	__fp68k
 846    00 01DC  6006                                               bras   .L10
 847    00 01DE                                             .L1:
 848                                                                cfp fcmpx
 849*   00 01DE  3F3C  0008                                 	movw	#fcmpx,sp@-
 850*   00 01E2  A9EB                                       	.word	__fp68k
 851    00 01E4                                             .L10:
 852                                                                fbne    .L12
 853*   00 01E4  6600  0008                                         bne     .L12
 854    00 01E8  0801  0002                                         btst    #2,d1           | #2 is equal bit
 855    00 01EC  6018                                               bras   .L50
 856    00 01EE                                             .L12:
 857                                                                fbo     .L14
 858*   00 01EE  6800  0008                                         bvc     .L14
 859    00 01F2  0801  0003                                         btst    #3,d1           | #3 is unordered bit
 860    00 01F6  600E                                               bras   .L50
 861    00 01F8                                             .L14:
 862                                                                fbgt    .L16
 863*   00 01F8  6E00  0008                                         bgt     .L16
 864    00 01FC  0801  0001                                         btst    #1,d1           | #1 is less bit
 865    00 0200  6004                                               bras   .L50
 866    00 0202                                             .L16:
 867    00 0202  0801  0000                                         btst    #0,d1           | #0 is greater bit
 868    00 0206                                             .L50:
 869    00 0206  4280                                       	clrl	d0
 870    00 0208  56C0                                               sne     d0
 871    00 020A  4400                                               negb   d0
 872    00 020C  9EFC  000C                                 	subw	#12,sp		| C wants to pop his own args
 873    00 0210  4ED0                                               jmp     a0@
 874                                                        
 875                                                        
 876                                                        
 877                                                                .globl    RelX
 878    00 0212                                             RelX:				| C args already swapped
 879    00 0212  205F                                               movl   sp@+,a0
 880                                                        
 881    00 0214  7207                                               moveq   #7,d1           | assume unordered
 882                                                                cfp fcmpx
 883*   00 0216  3F3C  0008                                 	movw	#fcmpx,sp@-
 884*   00 021A  A9EB                                       	.word	__fp68k
 885                                                                fbu     .L9              | unordered
 886*   00 021C  6900  0014                                         bvs     .L9
 887                                                                fbne    .L7
 888*   00 0220  6600  0006                                         bne     .L7
 889    00 0224  7203                                               moveq   #3,d1           | equal
 890    00 0226  600A                                               bras   .L9
 891    00 0228                                             .L7:
 892                                                                fbgt    .L3
 893*   00 0228  6E00  0006                                         bgt     .L3
 894    00 022C  7201                                               moveq   #1,d1           | less
 895    00 022E  6002                                               bras   .L9
 896    00 0230                                             .L3:
 897    00 0230  7200                                               moveq   #0,d1           | greater
 898    00 0232                                             .L9:
 899    00 0232  2001                                       	movl	d1,d0
 900    00 0234  9EFC  0008                                 	subw	#8,sp
 901    00 0238  4ED0                                               jmp     a0@
 902                                                        
 903                                                        
 904                                                                .globl    RemX
 905    00 023A                                             RemX:
 906    00 023A  205F                                               movl   sp@+,a0        | return address
 907    00 023C  226F  0008                                 	movl	sp@(8),a1	| addr of quotient
 908    00 0240  201F                                       	movl	sp@+,d0		| swap args
 909    00 0242  2F40  0004                                 	movl	d0,sp@(4)
 910                                                                cfp fremx                   | returns with quo in d0
 911*   00 0246  3F3C  000C                                 	movw	#fremx,sp@-
 912*   00 024A  A9EB                                       	.word	__fp68k
 913    00 024C  2280                                               movl    d0,a1@         | deliver integer result
 914    00 024E  9EFC  000C                                 	subw	#12,sp
 915    00 0252  4ED0                                               jmp     a0@
 916                                                        
 917                                                        
 918                                                        |-----------------------------------------------------------
 919                                                        | the class functions use a common startup routine to fetch
 920                                                        | the leading 32 bits and store the sign in its var param.
 921                                                        | a set of finishup sequences set the return value to the
 922                                                        | appropriate integer value.
 923                                                        |-----------------------------------------------------------
 924                                                        | pascal: function class?(var x: ???????; var sgn: integer):
 925                                                        |                                            numberclass
 926                                                        | stack = ret < sgn < x < result
 927                                                        |-----------------------------------------------------------
 928                                                        
 929                                                        	.macro	class,l,c
 930                                                        	.globl	l
 931                                                        l:	movw	#c,d0
 932                                                        	bras	classcom
 933                                                        	.endm
 934                                                        
 935                                                        	class,ClassS,fclasss
 936*                                                       	.globl	ClassS
 937*   00 0254  303C  101C                                 ClassS:	movw	#fclasss,d0
 938*   00 0258  6012                                       	bras	classcom
 939                                                        	class,ClassD,fclassd
 940*                                                       	.globl	ClassD
 941*   00 025A  303C  081C                                 ClassD:	movw	#fclassd,d0
 942*   00 025E  600C                                       	bras	classcom
 943                                                        	class,ClassC,fclassc
 944*                                                       	.globl	ClassC
 945*   00 0260  303C  301C                                 ClassC:	movw	#fclassc,d0
 946*   00 0264  6006                                       	bras	classcom
 947                                                        	class,ClassX,fclassx
 948*                                                       	.globl	ClassX
 949*   00 0266  303C  001C                                 ClassX:	movw	#fclassx,d0
 950*   00 026A  4E71                                       	bras	classcom
 951                                                        
 952    00 026C                                             classcom:
 953    00 026C  205F                                       	movl	sp@+,a0		| ret addrs
 954    00 026E  226F  0004                                 	movl	sp@(4),a1	| swap args and save sgn var
 955    00 0272  2F57  0004                                 	movl	sp@,sp@(4)
 956    00 0276  2E89                                       	movl	a1,sp@
 957    00 0278  3F00                                       	movw	d0,sp@-
 958    00 027A  A9EB                                       	.word	__fp68k
 959    00 027C  3011                                               movw    a1@,d0         | ret code
 960    00 027E  48C0                                       	extl	d0
 961    00 0280  4291                                               clrl   a1@            | 0 for +, 1 for -
 962    00 0282  4A40                                               tstw   d0
 963    00 0284  6A04                                               bpls   .L71
 964                                                        
 965    00 0286  5291                                               addql  #1,a1@
 966    00 0288  4480                                               negl   d0
 967    00 028A                                             .L71:
 968    00 028A  5380                                               subql  #1,d0
 969    00 028C  9EFC  0008                                 	subw	#8,sp		| C wants to pop his own stack
 970    00 0290  4ED0                                               jmp     a0@
 971                                                        
 972                                                        
 973                                                        
 974                                                        |-----------------------------------------------------------
 975                                                        |-----------------------------------------------------------
 976                                                        | the environment parameter is an integer passed by address
 977                                                        | for convenience in later routines, set/getenv leave a1
 978                                                        | unchanged but touch d0.
 979                                                        |-----------------------------------------------------------
 980                                                        |-----------------------------------------------------------
 981                                                        
 982                                                        	.globl	SetRnd
 983    00 0292                                             SetRnd:
 984    00 0292  225F                                               movl   sp@+,a1        | ret adrs
 985                                                        
 986    00 0294  2217                                               movl    sp@,d1         | new rnd mode
 987    00 0296  203C  0000  000D                           	movl	#13,d0
 988    00 029C  E1B9                                               roll   d0,d1           | align round bits
 989    00 029E  0281  0000  6000                                   andl  #/6000,d1       | mask just those
 990                                                        
 991    00 02A4  4857                                               pea     sp@            | overwrite input operand
 992                                                        	cfp	fgetenv
 993*   00 02A6  3F3C  0003                                 	movw	#fgetenv,sp@-
 994*   00 02AA  A9EB                                       	.word	__fp68k
 995    00 02AC  0257  9FFF                                         andw  #/9fff,sp@     | kill old mode
 996    00 02B0  8357                                               orw    d1,sp@         | new mode
 997    00 02B2  4857                                               pea     sp@
 998                                                        	cfp	fsetenv
 999*   00 02B4  3F3C  0001                                 	movw	#fsetenv,sp@-
1000*   00 02B8  A9EB                                       	.word	__fp68k
1001    00 02BA  4ED1                                               jmp     a1@
1002                                                        
1003                                                        
1004                                                        	.globl	GetRnd
1005    00 02BC                                             GetRnd:
1006    00 02BC  225F                                               movl   sp@+,a1
1007    00 02BE  4267                                       	clrw	sp@-
1008    00 02C0  4857                                               pea     sp@
1009                                                        	cfp	fgetenv
1010*   00 02C2  3F3C  0003                                 	movw	#fgetenv,sp@-
1011*   00 02C6  A9EB                                       	.word	__fp68k
1012                                                        
1013    00 02C8  203C  0000  6000                                   movl    #/6000,d0       | isolate round bits
1014    00 02CE  C05F                                               andw   sp@+,d0
1015    00 02D0  223C  0000  000D                           	movl	#13,d1
1016    00 02D6  E278                                               rorw   d1,d0           | in hi byte
1017    00 02D8  4ED1                                               jmp     a1@
1018                                                        
1019                                                        
1020                                                        |-----------------------------------------------------------
1021                                                        | pascal: procedure testxcp(x: exception): boolean;
1022                                                        | so stack = ret < x < result.   hi byte (0) is set of flags
1023                                                        | and low byte (1) is halts.
1024                                                        |-----------------------------------------------------------
1025                                                        
1026                                                        	.globl	TestXcp
1027    00 02DA                                             TestXcp:
1028    00 02DA  4281                                               clrl   d1
1029    00 02DC  6002                                               bras   gentest
1030                                                        	.globl	TestHlt
1031    00 02DE                                             TestHlt:
1032    00 02DE  7201                                               moveq   #1,d1
1033                                                        
1034    00 02E0                                             gentest:
1035    00 02E0  225F                                               movl   sp@+,a1        | ret adrs, safe from getenv
1036    00 02E2  2017                                       	movl	sp@,d0
1037    00 02E4  3F00                                       	movw	d0,sp@-
1038    00 02E6  486F  0002                                         pea     sp@(2)
1039                                                        	cfp	fgetenv
1040*   00 02EA  3F3C  0003                                 	movw	#fgetenv,sp@-
1041*   00 02EE  A9EB                                       	.word	__fp68k
1042    00 02F0  301F                                               movw    sp@+,d0        | bit index in hi byte
1043    00 02F2  4880                                               extw   d0              | make it word index
1044    00 02F4  0101                                               btst    d0,sp@(0,d1)
1045    00 02F6  56C0                                               sne     d0              | d0 = 0000 or 00ff
1046    00 02F8  4400                                               negb   d0              | d0 = 0000 or ffff
1047                                                        
1048    00 02FA  4ED1                                               jmp     a1@
1049                                                        
1050                                                        |-----------------------------------------------------------
1051                                                        | pascal: procedure setxcp(x: exception; onoff: boolean);
1052                                                        | stack = ret < onoff < x
1053                                                        | this procedure was originally set up to just set or clear
1054                                                        | bits in the control word.  nowadays, setting an exception
1055                                                        | flag entails a possible halt, so a special dummy floating
1056                                                        | point routine is used.
1057                                                        |-----------------------------------------------------------
1058                                                        	.globl	SetXcp,SetHlt
1059    00 02FC  4E56  0000                                 SetXcp:	link	a6,#0
1060    00 0300  202E  0008                                 	movl	a6@(8),d0
1061    00 0304  1F00                                       	movb	d0,sp@-
1062    00 0306  202E  000C                                 	movl	a6@(12),d0
1063    00 030A  1F00                                       	movb	d0,sp@-
1064    00 030C  611A                                       	bsrs	sxcp
1065    00 030E  4E5E                                       	unlk	a6
1066    00 0310  4E75                                       	rts
1067                                                        
1068    00 0312  4E56  0000                                 SetHlt:	link	a6,#0
1069    00 0316  202E  0008                                 	movl	a6@(8),d0
1070    00 031A  1F00                                       	movb	d0,sp@-
1071    00 031C  202E  000C                                 	movl	a6@(12),d0
1072    00 0320  1F00                                       	movb	d0,sp@-
1073    00 0322  6124                                       	bsrs	shlt
1074    00 0324  4E5E                                       	unlk	a6
1075    00 0326  4E75                                       	rts
1076                                                        
1077    00 0328                                             sxcp:
1078    00 0328  4A2F  0004                                         tstb   sp@(4)           | check boolean
1079    00 032C  6716                                               beqs   clrxcp
1080                                                        
1081    00 032E  205F                                               movl   sp@+,a0        | save return address
1082    00 0330  548F                                               addql  #2,sp           | kill boolean
1083    00 0332  1017                                               movb    sp@,d0         | get error index
1084    00 0334  4880                                               extw   d0              | extend to word
1085    00 0336  3E80                                               movw    d0,sp@         | replace as word
1086    00 0338  4857                                               pea     sp@            | save address
1087                                                        	cfp	fsetxcp
1088*   00 033A  3F3C  0015                                 	movw	#fsetxcp,sp@-
1089*   00 033E  A9EB                                       	.word	__fp68k
1090    00 0340  548F                                               addql  #2,sp           | kill index word
1091    00 0342  4ED0                                               jmp     a0@
1092                                                        
1093    00 0344                                             clrxcp:
1094    00 0344  4241                                               clrw   d1
1095    00 0346  6002                                               bras   gensetx
1096    00 0348                                             shlt:
1097    00 0348  7201                                               moveq   #1,d1
1098    00 034A                                             gensetx:
1099    00 034A  2257                                               movl   sp@,a1         | ret address, leave slot
1100                                                        
1101    00 034C  4857                                               pea     sp@		
1102                                                        	cfp	fgetenv
1103*   00 034E  3F3C  0003                                 	movw	#fgetenv,sp@-
1104*   00 0352  A9EB                                       	.word	__fp68k
1105    00 0354  102F  0006                                         movb    sp@(6),d0        | bit index in hi byte
1106    00 0358  4880                                               extw   d0              | make it word index
1107                                                        
1108    00 035A  4A2F  0004                                         tstb   sp@(4)           | test input boolean
1109    00 035E  6604                                               bnes   .L81
1110                                                        
1111    00 0360  0181                                               bclr    d0,sp@(0,d1)
1112    00 0362  6002                                               bras   .L83
1113    00 0364                                             .L81:
1114    00 0364  01C1                                               bset    d0,sp@(0,d1)
1115    00 0366                                             .L83:
1116    00 0366  4857                                               pea     sp@
1117                                                        	cfp	fsetenv
1118*   00 0368  3F3C  0001                                 	movw	#fsetenv,sp@-
1119*   00 036C  A9EB                                       	.word	__fp68k
1120                                                        
1121    00 036E  508F                                               addql  #8,sp           | kill ret and inputs
1122    00 0370  4ED1                                               jmp     a1@
1123    00 0372                                             .end
