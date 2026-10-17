\{#
#
int	fromflg	0;
\}
%term	CONTIG QTEXT  SPACE THIN TAB
%term	MATRIX LCOL CCOL RCOL COL
%term	MARK LINEUP
%term	SUM INT PROD UNION INTER
%term	LPILE PILE CPILE RPILE ABOVE
%term	DEFINE TDEFINE NDEFINE DELIM  GSIZE GFONT
%right	FROM TO
%left	OVER SQRT
%right	SUP SUB
%right	SIZE FONT ROMAN ITALIC BOLD FAT
%right	UP DOWN BACK FWD
%left	MQ MQ1
%left	LEFT RIGHT
%right	DOT DOTDOT HAT TILDE BAR UNDER VEC DYAD

%%

stuff	: eqn 	={ putout($1); }
	| error	={ error(!FATAL, "syntax error in equation %d", first);  }
	|	={ eqnreg = 0; }
	;

eqn	: box	={ if(dbg)printf(".\teqn: S%d\n",$1); }
	| eqn box	={ eqnbox($1,$2); }
	| eqn MARK	={ mark($1); }
	| MARK	={ mark(0); }
	| eqn LINEUP	={ lineup($1); }
	| LINEUP	={ lineup(0); }
	;

matrix	: MATRIX	={ $$ = ct; } ;

collist	: column
	| collist column
	;

column	: lcol MQ list MQ1	={ column('L',$1,$3); }
	| ccol MQ list MQ1	={ column('C',$1,$3); }
	| rcol MQ list MQ1	={ column('R',$1,$3); }
	| col MQ list MQ1	={ column('-',$1,$3); }
	;

lcol	: LCOL	={ $$ = ct++; } ;
ccol	: CCOL	={ $$ = ct++; } ;
rcol	: RCOL	={ $$ = ct++; } ;
col	: COL	={ $$ = ct++; } ;

sbox	: sup box	%prec SUP	={ $$ = $2; }
	;

tbox	: to box	%prec TO	={ $$ = $2; }
	|	%prec FROM	={ $$ = 0; }
	;

box	: box OVER box	={ boverb($1,$3); }
	| size box	%prec SIZE	={ size($1, $2); }
	| font box	%prec FONT	={ font($1, $2); }
	| FAT box	={ $$ = $2; }
	| SQRT box	={ sqrt($2); }
	| lpile MQ list MQ1	={ lpile('L', $1, ct); ct = $1; }
	| cpile MQ list MQ1	={ lpile('C', $1, ct); ct = $1; }
	| rpile MQ list MQ1	={ lpile('R', $1, ct); ct = $1; }
	| pile MQ list MQ1	={ lpile('-', $1, ct); ct = $1; }
	| box sub box sbox	%prec SUB
		={ if(dbg)printf(".\t sub box %d %d %d\n",$1,$3,$4);
			shift2($1, $3, $4);
		}
	| box sub box	%prec SUB	={ bshiftb($1, $2, $3); }
	| box sup box	%prec SUP	={ bshiftb($1, $2, $3); }
	| left eqn right	={ paren($1, $2, $3); }
	| pbox
	| box from box tbox	%prec FROM
		={ fromto($1,$3,$4); fromflg=0; }
	| box to box	%prec TO	={ fromto($1, 0, $3); }
	| box diacrit	={ diacrit($1,$2); }
	| fwd box	%prec UP	={ move(0,$1,$2); }
	| up box	%prec UP	={ move(1,$1,$2); }
	| back box	%prec UP	={ move(2,$1,$2); }
	| down box	%prec UP	={ move(3,$1,$2); }
	| matrix MQ collist MQ1	={ matrix($1,$3); }
	;

fwd	: FWD text	={ $$ = numb($1); } ;
up	: UP text	={ $$ = numb($1); } ;
back	: BACK text	={ $$ = numb($1); } ;
down	: DOWN text	={ $$ = numb($1); } ;

diacrit	: HAT	={ $$ = 'H'; }
	| VEC	={ $$ = 'V'; }
	| DYAD	={ $$ = 'Y'; }
	| BAR	={ $$ = 'B'; }
	| UNDER	={ $$ = 'N'; }	/* under bar */
	| DOT	={ $$ = 'D'; }
	| TILDE	={ $$ = 'T'; }
	| DOTDOT	={ $$ = 'U'; } /* umlaut = double dot */
	;

from	: FROM
	;
to	: TO
	;

left	: LEFT text	={ $$ = $2->c1; }
	| LEFT MQ	={ $$ = '{'; }
	;

right	: RIGHT text	={ $$ = $2->c1; }
	| RIGHT MQ1	={ $$ = '}'; }
	|		={ $$ = 0; }
	;

list	: eqn	={ lp[ct++] = $1; }
	| list ABOVE eqn	={ lp[ct++] = $3; }
	;

lpile	: LPILE	={ $$=ct; } ;
cpile	: CPILE	={ $$=ct; } ;
pile	: PILE	={ $$=ct; } ;
rpile	: RPILE	={ $$=ct; } ;

size	: SIZE text
	;

font	: ROMAN
	| ITALIC
	| BOLD
	| FONT text
	;

sub	: SUB	={ shift(1); } ;

sup	: SUP	={ shift(-1); } ;

pbox	: MQ eqn MQ1	={ $$ = $2; }
	| QTEXT	={ text('q',$1); }
	| CONTIG	={ text('c',$1); }
	| SPACE	={ text('~', $1); }
	| THIN	={ text('^', $1); }
	| TAB	={ text('\t', $1); }
	| SUM	={ funny('S'); }
	| PROD	={ funny('P'); }
	| INT	={ funny('I'); }
	| UNION	={ funny('U'); }
	| INTER	={ funny('A'); }	/* intersection */
	;

text	: CONTIG
	| QTEXT
	| SPACE	={ $$ = & "\\|\\|"; }
	| THIN	={ $$ = & "\\|"; }
	;

%%
