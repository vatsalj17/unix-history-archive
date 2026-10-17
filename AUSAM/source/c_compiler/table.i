L1:<jmp>;.byte 301;<\n>
<\0>
L2:<GB>
<jmp>;.byte 243;<(I)\n>
<\0>
L3:<jsr>;.byte 360;<c,MA\n>
<\0>
L4:<GB>
<jsr>;.byte 360;<c,#(I)\n>
<\0>
L5:<GA>
<jsr>;.byte 360;<c,(I)\n>
<\0>
L6:<clr>;.byte 311;<\n>
<\0>
L7:<clrf>;.byte 311;<\n>
<\0>
L8:<movC>;.byte 301;<,I\n>
<\0>
L9:<movof>;.byte 301;<,I\n>
<\0>
L10:<GB>
<movC>;.byte 243;<(I),I\n>
<\0>
L11:<GB>
<movof>;.byte 243;<(I),I\n>
<\0>
L12:<mov>;.byte 301;<+,I+\n>
<mov>;.byte 301;<,I\n>
<\0>
L13:<GB>
<mov>;.byte 243;<+2(I),I+\n>
<mov>;.byte 243;<(I),I\n>
<\0>
L14:<movC>;.byte 301;<',I\n>
<M'C>;.byte 301;<''\n>
<\0>
L15:<mov>;.byte 301;<',I\n>
<M>;.byte 302;<,A''\n>
<\0>
L16:<GJ>
<movC>;.byte 243;<(J),I\n>
<M'C>;.byte 243;<(J)\n>
<\0>
L17:<GB>
<movC>;.byte 243;<(I),-(sp)\n>
<M'C>;.byte 243;<(I)\n>
<movC>;.byte 250;<sp)+,I\n>
<\0>
L18:<GJ>
<mov>;.byte 243;<(J),I\n>
<M>;.byte 302;<,#(J)\n>
<\0>
L19:<GB>
<mov>;.byte 243;<(I),-(sp)\n>
<M>;.byte 302;<,#(I)\n>
<mov>;.byte 250;<sp)+,I\n>
<\0>
L20:<GA>
<M>;.byte 244;<1,A+\n>
<V>;.byte 301;<\n>
<\0>
L21:<GJ>
<mov>;.byte 243;<+2(J),I+\n>
<mov>;.byte 243;<(J),I\n>
<M>;.byte 244;<1,#+2(J)\n>
<V>;.byte 243;<(J)\n>
<\0>
L22:<GB>
<mov>;.byte 243;<+2(I),-(sp)\n>
<mov>;.byte 243;<(I),-(sp)\n>
<add>;.byte 244;<1,#+2(I)\n>
<V>;.byte 243;<(I)\n>
<mov>;.byte 250;<sp)+,I\n>
<mov>;.byte 250;<sp)+,I+\n>
<\0>
L23:<GA>
<MP>;.byte 311;<\n>
<\0>
L24:<GA>
<M>;.byte 311;<\n>
<M>;.byte 311;<+\n>
<V>;.byte 311;<\n>
<\0>
L25:<KA>
<movC>;.byte 311;<,A\n>
<\0>
L26:<KA>
<movfo>;.byte 311;<,A\n>
<\0>
L27:<GB>
<KA>
<movf>;.byte 311;<,#(I)\n>
<\0>
L28:<GB>
<movC>;.byte 302;<,#(I)\n>
<movC>;.byte 243;<(I),I\n>
<\0>
L29:<GB>
<KA>
<movfo>;.byte 311;<,#(I)\n>
<\0>
L30:<GB>
<KI>
<movC>;.byte 312;<,#(I)\n>
<movC>;.byte 312;<,I\n>
<\0>
L31:<KA>
<GJ>
<movf>;.byte 311;<,#(J)\n>
<\0>
L32:<KA>
<GJ>
<movfo>;.byte 311;<,#(J)\n>
<\0>
L33:<GD>
<KA>
<movC>;.byte 311;<,*(sp)+\n>
<\0>
L34:<GD>
<KA>
<movfo>;.byte 311;<,*(sp)+\n>
<\0>
L35:<KA>
<mov>;.byte 311;<+,A+\n>
<mov>;.byte 311;<,A\n>
<\0>
L36:<KA>
<GJ>
<mov>;.byte 311;<+,2+#(J)\n>
<mov>;.byte 311;<,#(J)\n>
<\0>
L37:<GD>
<KA>
<mov>;.byte 311;<,*(sp)\n>
<add>;.byte 244;<2,(sp)\n>
<mov>;.byte 311;<+,*(sp)+\n>
<\0>
L38:<KA>
<bicC>;.byte 332;<,A'\n>
<bisC>;.byte 311;<,A''\n>
<\0>
L40:<KC>
<GB>
<bicC>;.byte 332;<,#(I)\n>
<bisC>;.byte 250;<sp),#(I)\n>
<mov>;.byte 250;<sp)+,I\n>
<\0>
L41:<GA>
<\0>
L42:<GA>
<M'>;.byte 311;<\n>
<\0>
add1:;L43:<GA>
<MD>;.byte 302;<,I\n>
<\0>
add2:;L44:<GA>
<KJ>
<MD>;.byte 242;<(J),I\n>
<\0>
add3:;L45:<GA>
<KI>
<MP>;.byte 312;<,I\n>
<\0>
add4:;L46:<KD>
<GA>
<MD>;.byte 252;<(sp)+,I\n>
<\0>
add5:;L47:<KC>
<GA>
<MP>;.byte 250;<sp)+,I\n>
<\0>
L48:<GA>
<M>;.byte 302;<,I+\n>
<V>;.byte 311;<\n>
<\0>
L49:<GA>
<KI>
<M>;.byte 312;<,I+\n>
<V>;.byte 311;<\n>
<\0>
L50:<GA>
<M>;.byte 302;<,I\n>
<M>;.byte 302;<+,I+\n>
<V>;.byte 311;<\n>
<\0>
addl1:;L51:<GA>
<KI>
<M>;.byte 312;<+,I+\n>
<V>;.byte 311;<\n>
<M>;.byte 312;<,I\n>
<\0>
addl2:;L52:<KC>
<GA>
<M>;.byte 250;<sp)+,I\n>
<M>;.byte 250;<sp)+,I+\n>
<V>;.byte 311;<\n>
<\0>
L54:<GC>
<KA>
<xor>;.byte 311;<,(sp)\n>
<mov>;.byte 250;<sp)+,I\n>
<\0>
L56:<KC>
<GA>
<M>;.byte 311;<,(sp)\n>
<mov>;.byte 250;<sp)+,I\n>
<M>;.byte 311;<+,(sp)\n>
<mov>;.byte 250;<sp)+,I+\n>
<\0>
L57:<GA>
<asr>;.byte 311;<\n>
<\0>
L62:<GA>
<T>
<M>;.byte 302;<,I-\n>
<\0>
L63:<GA>
<T>
<KJ>
<M>;.byte 242;<(J),I-\n>
<\0>
L64:<GA>
<T>
<KI>
<M>;.byte 312;<,I-\n>
<\0>
L65:<KC>
<GA>
<T>
<M>;.byte 250;<sp)+,I-\n>
<\0>
addq1:;L70:<M>;.byte 302;<,A'\n>
<mov>;.byte 301;<'',I\n>
<\0>
addq1a:;L71:<movC>;.byte 301;<',I\n>
<MP>;.byte 302;<,I\n>
<movC>;.byte 311;<,A''\n>
<\0>
addq2:;L72:<KB>
<M>;.byte 242;<(I),A'\n>
<mov>;.byte 301;<'',I\n>
<\0>
addq3:;L73:<KA>
<M>;.byte 311;<,A'\n>
<mov>;.byte 301;<'',I\n>
<\0>
addq4:;L74:<KB>
<GJ>
<M>;.byte 242;<(I),#(J)\n>
<mov>;.byte 243;<(J),I\n>
<\0>
addq4a:;L75:<movf>;.byte 301;<',I\n>
<KI>
<MP>;.byte 312;<,I\n>
<movf>;.byte 311;<,A''\n>
<\0>
addq5:;L76:<KC>
<movC>;.byte 301;<',I\n>
<MP>;.byte 250;<sp)+,I\n>
<movC>;.byte 311;<,A''\n>
<\0>
addq6:;L77:<KC>
<movof>;.byte 301;<',I\n>
<MP>;.byte 250;<sp)+,I\n>
<movfo>;.byte 311;<,A''\n>
<\0>
addq7:;L78:<KA>
<GJ>
<M>;.byte 311;<,#(J)\n>
<mov>;.byte 243;<(J),I\n>
<\0>
addq8:;L79:<KC>
<GB>
<M>;.byte 250;<sp)+,#(I)\n>
<mov>;.byte 243;<(I),I\n>
<\0>
addq9:;L80:<GD>
<KC>
<movC>;.byte 252;<2(sp),I\n>
<MP>;.byte 250;<sp)+,I\n>
<movC>;.byte 311;<,*(sp)+\n>
<\0>
addq9a:;L81:<KC>
<GB>
<movC>;.byte 243;<(I),I\n>
<MP>;.byte 250;<sp)+,I\n>
<movC>;.byte 311;<,#(I)\n>
<\0>
addq10:;L82:<KC>
<GB>
<movof>;.byte 243;<(I),J\n>
<MP>;.byte 250;<sp)+,J\n>
<movfo>;.byte 312;<,#(I)\n>
<movf>;.byte 312;<,I\n>
<\0>
L90:<movC>;.byte 301;<',I\n>
<V>;.byte 311;<-\n>
<MP>;.byte 302;<,I-\n>
<movC>;.byte 311;<-,A''\n>
<\0>
L91:<KC>
<movC>;.byte 301;<',I\n>
<V>;.byte 311;<-\n>
<M>;.byte 250;<sp)+,I-\n>
<movC>;.byte 311;<-,A''\n>
<\0>
L92:<KC>
<GJ>
<movC>;.byte 243;<(J),I\n>
<V>;.byte 311;<-\n>
<M>;.byte 250;<sp)+,I-\n>
<movC>;.byte 311;<-,#(J)\n>
<\0>
L93:<GD>
<KC>
<movC>;.byte 252;<2(sp),I\n>
<V>;.byte 311;<-\n>
<M>;.byte 250;<sp)+,I-\n>
<movC>;.byte 311;<-,*(sp)+\n>
<\0>
L100:<movC>;.byte 301;<',I\n>
<V>;.byte 311;<-\n>
<M>;.byte 302;<,I-\n>
<movC>;.byte 311;<,A''\n>
<\0>
L101:<KC>
<movC>;.byte 301;<',I\n>
<V>;.byte 311;<-\n>
<M>;.byte 250;<sp)+,I-\n>
<movC>;.byte 311;<,A''\n>
<\0>
L102:<KC>
<GJ>
<movC>;.byte 243;<(J),I\n>
<V>;.byte 311;<-\n>
<M>;.byte 250;<sp)+,I-\n>
<movC>;.byte 311;<,#(J)\n>
<\0>
L103:<GD>
<KC>
<movC>;.byte 252;<2(sp),I\n>
<V>;.byte 311;<-\n>
<M>;.byte 250;<sp)+,I-\n>
<mov>;.byte 311;<,*(sp)+\n>
<\0>
L105:<KC>
<movb>;.byte 301;<',I\n>
<xor>;.byte 311;<,(sp)\n>
<mov>;.byte 250;<sp)+,I\n>
<movb>;.byte 311;<,A''\n>
<\0>
L106:<GD>
<movC>;.byte 252;<(sp),-(sp)\n>
<KA>
<xor>;.byte 311;<,(sp)\n>
<movC>;.byte 250;<sp)+,I\n>
<movC>;.byte 311;<,*(sp)+\n>
<\0>
L107:<asrC>;.byte 301;<'\n>
<movC>;.byte 301;<'',I\n>
<\0>
L108:<GB>
<asrC>;.byte 243;<(I)\n>
<movC>;.byte 243;<(I),I\n>
<\0>
L114:<movif>;.byte 301;<,I\n>
<\0>
L115:<GB>
<movif>;.byte 243;<(I),I\n>
<\0>
L116:<GA>
<movif>;.byte 311;<,I\n>
<\0>
L117:<GA>
<movfi>;.byte 311;<,I\n>
<\0>
L118:<GA>
<setl\n>
<movfi>;.byte 311;<,-(sp)\n>
<mov>;.byte 250;<sp)+,I\n>
<mov>;.byte 250;<sp)+,I+\n>
<seti\n>
<\0>
L119:<setl\n>
<movif>;.byte 301;<,I\n>
<seti\n>
<\0>
L120:<GB>
<setl\n>
<movif>;.byte 243;<(I),I\n>
<seti\n>
<\0>
L121:<GC>
<setl\n>
<movif>;.byte 250;<sp)+,I\n>
<seti\n>
<\0>
L122:<GI>
<clr>;.byte 311;<\n>
<\0>
L123:<GI!>
<sxt>;.byte 311;<\n>
<\0>
L124:<mov>;.byte 301;<+,I\n>
<\0>
L125:<GB>
<mov>;.byte 243;<+2(I),I\n>
<\0>
L126:<KC>
<GC>
<jsr>;.byte 360;<c,M\n>
<add>;.byte 244;<10,sp\n>
<\0>
L127:<KC>
<GC>
<jsr>;.byte 360;<c,M\n>
<add>;.byte 244;<6,sp\n>
<\0>
move1:;L128:<M'C>;.byte 301;<\n>
<\0>
move2:;L129:<GB>
<M'C>;.byte 243;<(I)\n>
<\0>
move3:;L130:<ML>;.byte 302;<,A\n>
<\0>
move4:;L131:<KB>
<ML>;.byte 242;<(I),A\n>
<\0>
move5:;L132:<KA>
<MC>;.byte 311;<,A\n>
<\0>
move6:;L133:<GB>
<ML>;.byte 302;<,#(I)\n>
<\0>
move7:;L134:<GB>
<KJ>
<ML>;.byte 242;<(J),#(I)\n>
<\0>
move8:;L135:<GB>
<KI>
<MC>;.byte 312;<,#(I)\n>
<\0>
move9:;L136:<KB>
<GJ>
<ML>;.byte 242;<(I),#(J)\n>
<\0>
move10:;L137:<KA>
<GJ>
<MC>;.byte 311;<,#(J)\n>
<\0>
move11:;L138:<GD>
<KB>
<ML>;.byte 242;<(I),*(sp)+\n>
<\0>
move12:;L139:<GD>
<KA>
<MC>;.byte 311;<,*(sp)+\n>
<\0>
L140:<KA>
<movfi>;.byte 311;<,A\n>
<\0>
L141:<KA>
<GJ>
<movfi>;.byte 311;<,#(J)\n>
<\0>
L142:<clr>;.byte 301;<\n>
<clr>;.byte 301;<+\n>
<\0>
L143:<GB>
<clr>;.byte 243;<(I)\n>
<clr>;.byte 262;<+#(I)\n>
<\0>
move13a:;L144:<M>;.byte 302;<,A+\n>
<V>;.byte 301;<\n>
<\0>
L145:<KB>
<mov>;.byte 242;<(I),A+\n>
<V>;.byte 301;<\n>
<\0>
L146:<KA>
<mov>;.byte 311;<,A+\n>
<V>;.byte 301;<\n>
<\0>
L147:<KA>
<setl\n>
<movfi>;.byte 311;<,A\n>
<seti\n>
<\0>
L148:<KA>
<GJ>
<setl\n>
<movfi>;.byte 311;<,#(J)\n>
<seti\n>
<\0>
move13:;L149:<M>;.byte 302;<,A\n>
<M>;.byte 302;<+,A+\n>
<V>;.byte 301;<\n>
<\0>
move14:;L150:<KB>
<M>;.byte 242;<(I),A\n>
<M>;.byte 242;<+2(I),A+\n>
<V>;.byte 301;<\n>
<\0>
move15:;L151:<KA>
<M>;.byte 311;<,A\n>
<M>;.byte 311;<+,A+\n>
<V>;.byte 301;<\n>
<\0>
move14a:;L152:<GB>
<M>;.byte 302;<,#+2(I)\n>
<V>;.byte 243;<(I)\n>
<\0>
move16a:;L153:<GB>
<M>;.byte 302;<+,#+2(I)\n>
<V>;.byte 243;<(I)\n>
<M>;.byte 302;<,#(I)\n>
<\0>
move16:;L154:<KA>
<GJ>
<M>;.byte 311;<+,#+2(J)\n>
<V>;.byte 243;<(J)\n>
<M>;.byte 311;<,#(J)\n>
<\0>
L155:<KC>
<GB>
<mov>;.byte 250;<sp)+,#+2(I)\n>
<V>;.byte 243;<(I)\n>
<\0>
move17:;L156:<KC>
<GB>
<M>;.byte 250;<sp)+,#(I)\n>
<M>;.byte 250;<sp)+,#+2(I)\n>
<V>;.byte 243;<(I)\n>
<\0>
L176:<GD>
<KA>
<M>;.byte 311;<,*(sp)\n>
<mov>;.byte 250;<sp)+,I\n>
<M>;.byte 311;<+,2(I)\n>
<\0>
L177:<\0>
L178:<M'C>;.byte 301;<\n>
<\0>
L184:<KB>
<movC>;.byte 301;<',J\n>
<M>;.byte 242;<(I),J\n>
<movC>;.byte 312;<,A''\n>
<\0>
L185:<KA>
<movC>;.byte 301;<',J\n>
<M>;.byte 311;<,J\n>
<movC>;.byte 312;<,A''\n>
<\0>
L188:<KC>
<GB>
<movC>;.byte 243;<(I),J\n>
<M>;.byte 250;<sp)+,J\n>
<movC>;.byte 312;<,#(I)\n>
<\0>
L197:<asrC>;.byte 301;<\n>
<\0>
L198:<GB>
<asrC>;.byte 243;<(I)\n>
<\0>
L199:<aslC>;.byte 301;<\n>
<\0>
L200:<GB>
<aslC>;.byte 243;<(I)\n>
<\0>
L201:<ash>;.byte 302;<,A\n>
<\0>
L202:<KB>
<ash>;.byte 242;<(I),A\n>
<\0>
L203:<KA>
<ash>;.byte 311;<,A\n>
<\0>
L204:<GA>
<ashc>;.byte 302;<,I\n>
<mov>;.byte 311;<,A\n>
<mov>;.byte 311;<+,A+\n>
<\0>
L205:<KC>
<GA>
<ashc>;.byte 250;<sp)+,I\n>
<mov>;.byte 311;<,A\n>
<mov>;.byte 311;<+,A+\n>
<\0>
L206:<GD>
<KC>
<mov>;.byte 262;<(sp),I\n>
<mov>;.byte 262;<(I),I+\n>
<mov>;.byte 250;<I),I\n>
<ashc>;.byte 250;<sp)+,I\n>
<mov>;.byte 311;<,*(sp)\n>
<add>;.byte 244;<2,(sp)\n>
<mov>;.byte 311;<+,*(sp)+\n>
<\0>
L207:<bicC>;.byte 332;<,A'\n>
<bisC>;.byte 302;<,A''\n>
<\0>
L208:<KA>
<bicC>;.byte 332;<,A'\n>
<bisC>;.byte 311;<,A''\n>
<\0>
L209:<GB>
<bicC>;.byte 332;<,#(I)\n>
<bisC>;.byte 302;<,#(I)\n>
<\0>
fas1:;L210:<KA>
<GJ>
<bicC>;.byte 332;<,#(J)\n>
<bisC>;.byte 311;<,#(J)\n>
<\0>
L211:<GB>
<KI>
<bicC>;.byte 332;<,#(I)\n>
<bisC>;.byte 312;<,#(I)\n>
<\0>
L212:<KC>
<GB>
<bicC>;.byte 332;<,#(I)\n>
<bisC>;.byte 250;<sp)+,#(I)\n>
<\0>
L214:<movof>;.byte 301;<,I\n>
<\0>
L216:<GB>
<movof>;.byte 243;<(I),I\n>
<\0>
L217:<GE>
<\0>
L228:<tst>;.byte 301;<\n>
<X0>
<tst>;.byte 301;<+\n>
<X1>
<\0>
L229:<tst>;.byte 301;<\n>
<X0>
<cmp>;.byte 301;<+,B\n>
<X1>
<\0>
lcmp1:;L230:<M>;.byte 301;<,B\n>
<X0>
<M>;.byte 301;<+,B+\n>
<X1>
<\0>
L231:<GB>
<tst>;.byte 243;<(I)\n>
<X0>
<tst>;.byte 243;<+2(I)\n>
<X1>
<\0>
L232:<GB>
<tst>;.byte 243;<(I)\n>
<X0>
<cmp>;.byte 243;<+2(I),B\n>
<X1>
<\0>
lcmp2:;L233:<GB>
<M>;.byte 243;<(I),B\n>
<X0>
<M>;.byte 243;<+2(I),B+\n>
<X1>
<\0>
L234:<GA>
<tst>;.byte 311;<\n>
<X0>
<tst>;.byte 311;<+\n>
<X1>
<\0>
L235:<GA>
<tst>;.byte 311;<\n>
<X0>
<cmp>;.byte 311;<+,B\n>
<X1>
<\0>
lcmp3:;L236:<GA>
<M>;.byte 311;<,B\n>
<X0>
<M>;.byte 311;<+,B+\n>
<X1>
<\0>
lcmp4:;L237:<GB>
<KJ>
<M>;.byte 243;<(I),"(J)\n>
<X0>
<M>;.byte 243;<+2(I),"+2(J)\n>
<X1>
<\0>
lcmp5:;L238:<GA>
<KJ>
<M>;.byte 311;<,"(J)\n>
<X0>
<M>;.byte 311;<+,"+2(J)\n>
<X1>
<\0>
L244:</bit>;.byte 244;<0,A\n>
</X0>
<bit>;.byte 302;<,A+\n>
<X1>
<\0>
L245:<GB>
</bit>;.byte 244;<0,"(I)\n>
</X0>
<bit>;.byte 302;<,"+2(I)\n>
<X1>
<\0>
L251:<GA>
</bit>;.byte 244;<0,I\n>
</X0>
<bit>;.byte 302;<,I+\n>
<X1>
<\0>
L252:<HA>
<\0>
L253:<clrC>;.byte 255;<(sp)\n>
<\0>
L254:<mov>;.byte 301;<,-(sp)\n>
<\0>
L255:<GB>
<mov>;.byte 243;<(I),-(sp)\n>
<\0>
L256:<mov>;.byte 301;<+,-(sp)\n>
<mov>;.byte 301;<,-(sp)\n>
<\0>
L257:<GC>
<M'>;.byte 250;<sp)\n>
<\0>
L258:<GC>
<M>;.byte 302;<,(sp)\n>
<\0>
L259:<GC>
<KB>
<M>;.byte 242;<(I),(sp)\n>
<\0>
L260:<GC>
<KA>
<M>;.byte 311;<,(sp)\n>
<\0>
L261:<GC>
<clr>;.byte 255;<(sp)\n>
<\0>
L262:<GC>
<sxt>;.byte 255;<(sp)\n>
<\0>
L263:<GA>
<setl\n>
<movfi>;.byte 311;<,-(sp)\n>
<seti\n>
<\0>
.even
/
/ c code tables-- compile to register
/

.globl	_regtab

.data
_regtab=.
	106.;	cr106
	30.;	cr70
	31.;	cr70
	32.;	cr32
	33.;	cr32
	37.;	cr37
	38.;	cr37
	98.;	cr100
	99.;	cr100
	80.;	cr80
	40.;	cr40
	41.;	cr40	/ - like +
	42.;	cr42
	43.;	cr43
	14.;	cr43
	44.;	cr43
	45.;	cr45
	46.;	cr40
	55.; 	cr40
	48.;	cr40
	49.;	cr49
	70.;	cr70
	71.;	cr70
	72.;	cr72
	73.;	cr73
	74.;	cr74
	75.;	cr75
	76.;	cr72
	78.;	cr70
	85.;	cr70
	79.;	cr79
	102.;	cr102
	51.;	cr51
	52.;	cr52
	56.;	cr56
	57.;	cr57
	58.;	cr58
	59.;	cr59
	91.;	cr91
	82.;	cr82
	83.;	cr82
	84.;	cr82
	86.;	cr86
	87.;	cr86
	88.;	cr86
	16.;	cr16
	92.;	cr92
	17.;	cr43
	18.;	cr74
	0
.text

/ goto
cr102=.+2; 0
.byte 20,0;.byte 77,0
L1
.byte 177,0;.byte 77,0
L2
/ call
cr100=.+2; 0
.byte 20,0;.byte 77,0
L3
.byte 177,0;.byte 77,0
L4
.byte 77,0;.byte 77,0
L5
/ addressible
cr106=.+2; 0
.byte 4,0;.byte 77,0
L6
.byte 4,4;.byte 77,0
L7
.byte 20,0;.byte 77,0
L8
.byte 20,5;.byte 77,0
L8
.byte 20,4;.byte 77,0
L9
.byte 177,0;.byte 77,0
L10
.byte 177,5;.byte 77,0
L10
.byte 177,4;.byte 77,0
L11
.byte 20,10;.byte 77,0
L12
.byte 177,10;.byte 77,0
L13
/ ++,-- postfix
cr32=.+2; 0
.byte 20,0;.byte 5,0
L14
.byte 20,1;.byte 77,0
L15
.byte 124,0;.byte 5,0
L16
.byte 177,0;.byte 5,0
L17
.byte 124,1;.byte 77,0
L18
.byte 177,1;.byte 77,0
L19
.byte 20,10;.byte 5,0
L20
.byte 124,10;.byte 5,0
L21
.byte 177,10;.byte 5,0
L22
/ - unary, ~
cr37=.+2; 0
.byte 77,0;.byte 77,0
L23
.byte 77,4;.byte 77,0
L23
.byte 77,10;.byte 77,0
L24
/ =
cr80=.+2; 0
.byte 20,0;.byte 77,0
L25
.byte 20,5;.byte 77,4
L25
.byte 20,4;.byte 77,4
L26
.byte 177,5;.byte 20,4
L27
.byte 177,0;.byte 20,1
L28
.byte 177,4;.byte 20,4
L29
.byte 177,0;.byte 24,0
L30
.byte 124,5;.byte 77,4
L31
.byte 124,4;.byte 77,4
L32
.byte 177,0;.byte 77,0
L33
.byte 177,5;.byte 77,4
L33
.byte 177,4;.byte 77,4
L34
.byte 20,10;.byte 77,10
L35
.byte 124,10;.byte 77,10
L36
.byte 177,10;.byte 77,10
L37
/ field assign, value in reg.
cr16=.+2; 0
.byte 20,0;.byte 77,0
L38
.byte 124,0;.byte 77,0
L39
L39=fas1

.byte 177,0;.byte 77,0
L40
/ +, -, |, &~, <<
cr40=.+2; 0
.byte 77,0;.byte 4,0
L41
.byte 77,0;.byte 5,0
L42
.byte 77,0;.byte 20,1
L43
.byte 77,4;.byte 20,5
L43
.byte 77,0;.byte 124,1
L44
.byte 77,4;.byte 124,5
L44
.byte 77,0;.byte 24,0
L45
.byte 77,4;.byte 24,4
L45
.byte 77,0;.byte 177,1
L46
.byte 77,4;.byte 177,5
L46
.byte 77,0;.byte 77,0
L47
.byte 77,4;.byte 77,4
L47
.byte 77,10;.byte 10,0
L48
.byte 77,10;.byte 20,11
L48
.byte 77,10;.byte 24,11
L49
.byte 77,10;.byte 20,10
L50
.byte 77,10;.byte 24,10
L51
.byte 77,10;.byte 77,10
L52
/ ^ -- xor
cr49=.+2; 0
.byte 77,0;.byte 24,0
L53
L53=add3

.byte 77,0;.byte 77,0
L54
.byte 77,10;.byte 24,10
L55
L55=addl1

.byte 77,10;.byte 77,10
L56
/ >> (all complicated cases taken care of by << -)
cr45=.+2; 0
.byte 77,0;.byte 5,0
L57
/ * -- I must be odd on integers
cr42=.+2; 0
.byte 77,0;.byte 20,1
L58
.byte 77,4;.byte 20,5
L58
L58=add1

.byte 77,0;.byte 124,1
L59
.byte 77,4;.byte 124,5
L59
L59=add2

.byte 77,0;.byte 24,0
L60
.byte 77,4;.byte 24,4
L60
L60=add3

.byte 77,0;.byte 77,0
L61
.byte 77,4;.byte 77,4
L61
L61=add5

/ / I must be odd on integers
cr43=.+2; 0
.byte 77,0;.byte 20,1
L62
.byte 77,0;.byte 124,1
L63
.byte 77,0;.byte 24,0
L64
.byte 77,0;.byte 77,0
L65
.byte 77,4;.byte 20,5
L66
L66=add1

.byte 77,4;.byte 124,5
L67
L67=add2

.byte 77,4;.byte 24,4
L68
L68=add3

.byte 77,4;.byte 77,4
L69
L69=add5

/ =+, =-, =|, =&~
cr70=.+2; 0
.byte 20,1;.byte 20,1
L70
.byte 20,0;.byte 20,1
L71
.byte 20,5;.byte 20,5
L71
.byte 20,1;.byte 177,1
L72
.byte 20,1;.byte 77,0
L73
.byte 124,1;.byte 177,1
L74
.byte 20,5;.byte 24,4
L75
.byte 20,0;.byte 77,0
L76
.byte 20,5;.byte 77,4
L76
.byte 20,4;.byte 77,4
L77
.byte 124,1;.byte 77,0
L78
.byte 177,1;.byte 77,0
L79
.byte 177,0;.byte 77,0
L80
.byte 177,5;.byte 77,4
L81
.byte 177,4;.byte 77,4
L82
/ =*, =<< (for integer multiply, I must be odd)
cr72=.+2; 0
.byte 20,0;.byte 20,1
L83
.byte 20,5;.byte 20,5
L83
L83=addq1a

.byte 20,4;.byte 77,4
L84
L84=addq6

.byte 20,5;.byte 24,4
L85
L85=addq4a

.byte 20,0;.byte 77,0
L86
.byte 20,5;.byte 77,4
L86
L86=addq5

.byte 177,0;.byte 77,0
L87
L87=addq9

.byte 177,5;.byte 77,4
L88
L88=addq9a

.byte 177,4;.byte 77,4
L89
L89=addq10

/ =/ ;  I must be odd on integers
cr73=.+2; 0
.byte 20,0;.byte 20,1
L90
.byte 20,0;.byte 77,0
L91
.byte 124,0;.byte 77,0
L92
.byte 177,0;.byte 77,0
L93
.byte 20,5;.byte 20,5
L94
L94=addq1a

.byte 20,5;.byte 24,4
L95
L95=addq4a

.byte 20,5;.byte 77,4
L96
L96=addq5

.byte 20,4;.byte 77,4
L97
L97=addq6

.byte 177,5;.byte 77,4
L98
L98=addq9a

.byte 177,4;.byte 77,4
L99
L99=addq10

/ =mod; I must be odd on integers
cr74=.+2; 0
.byte 20,0;.byte 20,1
L100
.byte 20,0;.byte 77,0
L101
.byte 124,0;.byte 77,0
L102
.byte 177,0;.byte 77,0
L103
/ =^ -- =xor
cr79=.+2; 0
.byte 20,1;.byte 77,0
L104
L104=addq3

.byte 20,3;.byte 77,0
L105
.byte 177,0;.byte 77,0
L106
/ =>> (all complicated cases done by =<< -)
cr75=.+2; 0
.byte 20,0;.byte 5,0
L107
.byte 177,0;.byte 5,0
L108
/ << for longs
cr91=.+2; 0
.byte 77,10;.byte 20,1
L109
L109=add1

.byte 77,10;.byte 124,1
L110
L110=add2

.byte 77,10;.byte 24,0
L111
L111=add3

.byte 77,10;.byte 177,1
L112
L112=add4

.byte 77,10;.byte 77,0
L113
L113=add5

/ int -> float
cr51=.+2; 0
.byte 20,1;.byte 77,0
L114
.byte 177,1;.byte 77,0
L115
.byte 77,0;.byte 77,0
L116
/ float, double -> int
cr52=.+2; 0
.byte 77,4;.byte 77,0
L117
/ double (float) to long
cr56=.+2; 0
.byte 77,4;.byte 77,0
L118
/ long to double
cr57=.+2; 0
.byte 20,10;.byte 77,0
L119
.byte 177,10;.byte 77,0
L120
.byte 77,10;.byte 77,0
L121
/ integer to long
cr58=.+2; 0
.byte 77,11;.byte 77,0
L122
.byte 77,0;.byte 77,0
L123
/ long to integer
cr59=.+2; 0
.byte 20,10;.byte 77,0
L124
.byte 177,10;.byte 77,0
L125
/ *, /, remainder for longs.
cr82=.+2; 0
.byte 77,10;.byte 77,10
L126
/ =*, =/, =rem for longs
/ Operands of the form &x op y, so stack space is known.
cr86=.+2; 0
.byte 77,0;.byte 77,10
L127
/
/ c code tables -- compile for side effects.
/ Olso set condition codes properly (except for ++, --)
/

.globl	_efftab

.data
_efftab=.
	30.;	ci70
	31.;	ci70
	32.;	ci70
	33.;	ci70
	80.;	ci80
	70.;	ci70
	71.;	ci70	/ - like +
	78.;	ci78
	79.;	ci79
	85.;	ci78
	75.;	ci75
	76.;	ci76
	16.;	ci16
	0
.text

/ =
ci80=.+2; 0
.byte 20,0;.byte 4,0
L128
.byte 20,5;.byte 4,4
L128
.byte 177,0;.byte 4,0
L129
.byte 177,5;.byte 4,4
L129
.byte 20,0;.byte 20,1
L130
.byte 20,3;.byte 20,0
L130
.byte 20,3;.byte 177,0
L131
.byte 20,0;.byte 177,1
L131
.byte 20,0;.byte 77,0
L132
.byte 177,0;.byte 20,1
L133
.byte 177,3;.byte 20,0
L133
.byte 177,0;.byte 124,1
L134
.byte 177,3;.byte 124,0
L134
.byte 177,0;.byte 24,0
L135
.byte 124,0;.byte 177,1
L136
.byte 124,3;.byte 177,0
L136
.byte 124,0;.byte 77,0
L137
.byte 177,0;.byte 177,1
L138
.byte 177,3;.byte 177,0
L138
.byte 177,0;.byte 77,0
L139
.byte 20,1;.byte 77,4
L140
.byte 124,1;.byte 77,4
L141
.byte 20,10;.byte 4,0
L142
.byte 177,10;.byte 4,0
L143
.byte 20,10;.byte 20,1
L144
.byte 20,10;.byte 177,1
L145
.byte 20,10;.byte 77,0
L146
.byte 20,10;.byte 77,4
L147
.byte 124,10;.byte 77,4
L148
.byte 20,10;.byte 20,10
L149
.byte 20,10;.byte 177,10
L150
.byte 20,10;.byte 77,10
L151
.byte 177,10;.byte 20,1
L152
.byte 124,10;.byte 20,10
L153
.byte 124,10;.byte 77,10
L154
.byte 177,10;.byte 77,0
L155
.byte 177,10;.byte 77,10
L156
/ =| and =& ~
ci78=.+2; 0
.byte 20,0;.byte 20,0
L157
L157=move3

.byte 20,0;.byte 77,0
L158
L158=move5

.byte 177,0;.byte 20,0
L159
L159=move6

.byte 177,0;.byte 124,0
L160
L160=move7

.byte 177,0;.byte 24,0
L161
L161=move8

.byte 124,0;.byte 177,0
L162
L162=move9

.byte 124,0;.byte 77,0
L163
L163=move10

.byte 177,0;.byte 177,0
L164
L164=move11

.byte 177,0;.byte 77,0
L165
L165=move12

.byte 20,10;.byte 10,0
L166
.byte 20,10;.byte 20,11
L166
L166=move13a

.byte 20,10;.byte 20,10
L167
L167=move13

.byte 20,10;.byte 177,10
L168
L168=move14

.byte 20,10;.byte 77,10
L169
L169=move15

.byte 177,10;.byte 10,0
L170
L170=move14a

.byte 124,10;.byte 20,10
L171
L171=move16a

.byte 124,10;.byte 77,10
L172
L172=move16

.byte 177,10;.byte 77,10
L173
L173=move17

/ =^
ci79=.+2; 0
.byte 20,10;.byte 77,10
L174
L174=move15

.byte 124,10;.byte 77,10
L175
L175=move16

.byte 177,10;.byte 77,10
L176
/ =+
ci70=.+2; 0
.byte 177,0;.byte 4,0
L177
.byte 20,0;.byte 4,0
L177
.byte 20,0;.byte 5,0
L178
.byte 20,1;.byte 20,1
L179
L179=move3

.byte 20,1;.byte 177,1
L180
L180=move4

.byte 20,1;.byte 77,0
L181
L181=move5

.byte 177,0;.byte 5,0
L182
L182=move2

.byte 124,1;.byte 177,1
L183
L183=move9

.byte 20,0;.byte 177,1
L184
.byte 20,0;.byte 77,0
L185
.byte 124,1;.byte 77,0
L186
L186=move10

.byte 177,1;.byte 77,0
L187
L187=move12

.byte 177,0;.byte 77,0
L188
.byte 20,10;.byte 10,0
L189
.byte 20,10;.byte 20,11
L189
L189=move13a

.byte 20,10;.byte 20,10
L190
L190=move13

.byte 20,10;.byte 177,10
L191
L191=move14

.byte 20,10;.byte 77,10
L192
L192=move15

.byte 177,10;.byte 10,0
L193
.byte 177,10;.byte 20,11
L193
L193=move14a

.byte 124,10;.byte 20,10
L194
L194=move16a

.byte 124,10;.byte 77,10
L195
L195=move16

.byte 177,10;.byte 77,10
L196
L196=move17

/ =>> (all harder cases handled by =<< -)
ci75=.+2; 0
.byte 20,0;.byte 5,0
L197
.byte 177,0;.byte 5,0
L198
/ =<<
ci76=.+2; 0
.byte 20,0;.byte 5,0
L199
.byte 177,0;.byte 5,0
L200
.byte 11,0;.byte 20,1
L201
.byte 11,0;.byte 177,1
L202
.byte 11,0;.byte 77,0
L203
/ =<< for longs
cr92=.+2; 0
.byte 20,10;.byte 20,1
L204
.byte 20,10;.byte 77,0
L205
.byte 177,10;.byte 77,0
L206
/ field = ...
ci16=.+2; 0
.byte 20,0;.byte 20,0
L207
.byte 20,0;.byte 77,0
L208
.byte 177,0;.byte 20,0
L209
.byte 124,0;.byte 77,0
L210
.byte 177,0;.byte 24,0
L211
.byte 177,0;.byte 77,0
L212
/
/ c code tables-- set condition codes
/

.globl	_cctab

.data
_cctab=.
	106.;	cc60
	28.;	rest
	55.;	rest
	34.;	rest
	35.;	rest
	36.;	rest
	37.;	rest
	40.;	rest
	41.;	rest
	42.;	rest
	43.;	rest
	45.;	rest
	46.;	rest
	81.;	cc81	/ & as in "if ((a&b)==0)"
	48.;	rest
	60.;	cc60
	61.;	cc60
	62.;	cc60
	63.;	cc60
	64.;	cc60
	65.;	cc60
	66.;	cc60
	67.;	cc60
	68.;	cc60
	69.;	cc60
	72.;	rest
	73.;	rest
	79.;	rest
	0
.text

/ relationals
cc60=.+2; 0
.byte 20,0;.byte 4,0
L213
.byte 20,5;.byte 4,4
L213
L213=move1

.byte 20,4;.byte 4,0
L214
.byte 177,0;.byte 4,0
L215
.byte 177,5;.byte 4,4
L215
L215=move2

.byte 177,4;.byte 4,0
L216
.byte 77,0;.byte 4,0
L217
.byte 77,4;.byte 4,4
L217
.byte 20,1;.byte 20,1
L218
.byte 20,3;.byte 20,3
L218
L218=move3

.byte 177,1;.byte 20,1
L219
.byte 177,3;.byte 20,3
L219
L219=move6

.byte 77,0;.byte 20,1
L220
.byte 77,4;.byte 20,5
L220
L220=add1

.byte 177,1;.byte 124,1
L221
.byte 177,3;.byte 124,3
L221
L221=move7

.byte 177,1;.byte 24,0
L222
L222=move8

.byte 77,0;.byte 124,1
L223
.byte 77,4;.byte 124,5
L223
L223=add2

.byte 77,0;.byte 24,0
L224
.byte 77,4;.byte 24,4
L224
L224=add3

.byte 177,1;.byte 177,1
L225
.byte 177,3;.byte 177,3
L225
L225=move11

.byte 177,1;.byte 77,0
L226
L226=move12

.byte 77,0;.byte 77,0
L227
.byte 77,4;.byte 77,4
L227
L227=add5

.byte 20,10;.byte 4,0
L228
.byte 20,10;.byte 10,0
L229
.byte 20,10;.byte 20,11
L229
.byte 20,10;.byte 20,10
L230
.byte 177,10;.byte 4,0
L231
.byte 177,10;.byte 10,0
L232
.byte 177,10;.byte 20,11
L232
.byte 177,10;.byte 20,10
L233
.byte 77,10;.byte 4,0
L234
.byte 77,10;.byte 10,0
L235
.byte 77,10;.byte 20,11
L235
.byte 77,10;.byte 20,10
L236
.byte 177,10;.byte 124,10
L237
.byte 77,10;.byte 124,10
L238
/ & as in "if ((a&b) ==0)"
cc81=.+2; 0
.byte 20,0;.byte 20,0
L239
L239=move3

.byte 177,0;.byte 20,0
L240
L240=move6

.byte 77,0;.byte 20,0
L241
L241=add1

.byte 77,0;.byte 24,0
L242
L242=add3

.byte 77,0;.byte 77,0
L243
L243=add5

.byte 20,10;.byte 10,0
L244
.byte 20,10;.byte 20,11
L244
.byte 177,10;.byte 10,0
L245
.byte 177,10;.byte 20,11
L245
.byte 20,10;.byte 20,10
L246
L246=lcmp1

.byte 177,10;.byte 20,10
L247
L247=lcmp2

.byte 77,10;.byte 20,10
L248
L248=lcmp3

.byte 177,10;.byte 124,10
L249
L249=lcmp4

.byte 77,10;.byte 124,10
L250
L250=lcmp5

.byte 77,10;.byte 10,0
L251
.byte 77,10;.byte 20,11
L251
/ set codes right
rest=.+2; 0
.byte 77,0;.byte 77,0
L252
.byte 77,4;.byte 77,4
L252
/
/ c code tables-- expression to -(sp)
/

.globl	_sptab

.data
_sptab=.
	106.;	cs106
	40.;	cs40
	41.;	cs40
	55.;	cs40
	48.;	cs40
	58.;	cs58
	56.;	cs56
	0
.text

/ name
cs106=.+2; 0
.byte 4,0;.byte 77,0
L253
.byte 4,4;.byte 77,0
L253
.byte 20,1;.byte 77,0
L254
.byte 177,1;.byte 77,0
L255
.byte 20,10;.byte 77,0
L256
/ +, -, |, &~
cs40=.+2; 0
.byte 20,0;.byte 5,0
L257
.byte 20,0;.byte 20,1
L258
.byte 20,0;.byte 177,1
L259
.byte 20,0;.byte 77,0
L260
/ integer to long
cs58=.+2; 0
.byte 77,11;.byte 77,0
L261
.byte 77,0;.byte 77,0
L262
/ float to long
cs56=.+2; 0
.byte 77,4;.byte 77,0
L263
0
