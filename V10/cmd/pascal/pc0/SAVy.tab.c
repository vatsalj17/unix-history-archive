/* Copyright (c) 1979 Regents of the University of California */

static	char sccsid[] = "@(#)pas.y 1.4 3/18/81";

#include "whoami.h"
#include "0.h"
#include "yy.h"
#include "tree.h"

#ifdef PI
#define	lineof(l)	l
#define	line2of(l)	l
#endif

extern yyerrflag;

int *yyval;
int *yypv;
yyactr(__np__){
	register int **yyYpv;
	register int *p, *q;
	yyYpv = (int**)yypv;

switch(__np__){

case 1: funcend(yyYpv[1], yyYpv[3], lineof(yyYpv[4])); break;
case 2: segend(); break;
case 3: yyval = (int*)funcbody(funchdr(tree5(T_PROG, lineof(yyYpv[1]), yyYpv[2], fixlist(yyYpv[4]), NIL))); break;
case 4: {
			yyPerror("Malformed program statement", PPROG);
			/*
			 * Should make a program statement
			 * with "input" and "output" here.
			 */
			yyval = (int*)funcbody(funchdr(tree5(T_PROG, lineof(yyYpv[1]), NIL, NIL, NIL)));
		  } break;
case 5: {
			yyval = (int*)tree3(T_BSTL, lineof(yyYpv[1]), fixlist(yyYpv[2]));
			if ((int)yyYpv[3] < 0)
				brerror(yyYpv[1], "begin");
		  } break;
case 6: trfree(); break;
case 7: {
Derror:
			constend(), typeend(), varend(), trfree();
			yyPerror("Malformed declaration", PDECL);
		  } break;
case 8: trfree(); break;
case 10: constend(); break;
case 11: typeend(); break;
case 12: varend(); break;
case 14: label(fixlist(yyYpv[2]), lineof(yyYpv[1])); break;
case 15: yyval = (int*)newlist(yyYpv[1] == NIL ? NIL : *hash(yyYpv[1], 1)); break;
case 16: yyval = (int*)addlist(yyYpv[1], yyYpv[3] == NIL ? NIL : *hash(yyYpv[3], 1)); break;
case 17: constbeg(yyYpv[1], line2of(yyYpv[2])), const(lineof(yyYpv[3]), yyYpv[2], yyYpv[4]); break;
case 18: const(lineof(yyYpv[3]), yyYpv[2], yyYpv[4]); break;
case 19: {
			constbeg(yyYpv[1], line2of(yyYpv[1]));
Cerror:
			yyPerror("Malformed const declaration", PDECL);
		  } break;
case 20: goto Cerror; break;
case 21: typebeg(yyYpv[1], line2of(yyYpv[2])), type(lineof(yyYpv[3]), yyYpv[2], yyYpv[4]); break;
case 22: type(lineof(yyYpv[3]), yyYpv[2], yyYpv[4]); break;
case 23: {
			typebeg(yyYpv[1], line2of(yyYpv[1]));
Terror:
			yyPerror("Malformed type declaration", PDECL);
		  } break;
case 24: goto Terror; break;
case 25: varbeg(yyYpv[1], line2of(yyYpv[3])), var(lineof(yyYpv[3]), fixlist(yyYpv[2]), yyYpv[4]); break;
case 26: var(lineof(yyYpv[3]), fixlist(yyYpv[2]), yyYpv[4]); break;
case 27: {
			varbeg(yyYpv[1], line2of(yyYpv[1]));
Verror:
			yyPerror("Malformed var declaration", PDECL);
		  } break;
case 28: goto Verror; break;
case 29: funcfwd(yyYpv[1]); break;
case 30: funcext(yyYpv[1]); break;
case 31: funcend(yyYpv[1], yyYpv[3], lineof(yyYpv[4])); break;
case 32: funcbody(yyYpv[1]); break;
case 33: yyval = (int*)funchdr(tree5(yyYpv[1], lineof(yyYpv[5]), yyYpv[2], yyYpv[3], yyYpv[4])); break;
case 34: yyval = (int*)T_PDEC; break;
case 35: yyval = (int*)T_FDEC; break;
case 36: yyval = (int*)fixlist(yyYpv[2]); break;
case 37: yyval = (int*)NIL; break;
case 38: yyval = (int*)tree3(T_PVAL, fixlist(yyYpv[1]), yyYpv[3]); break;
case 39: yyval = (int*)tree3(T_PVAR, fixlist(yyYpv[2]), yyYpv[4]); break;
case 40: yyval = (int*)tree5(T_PFUNC, fixlist(yyYpv[2]), yyYpv[4], yyYpv[3], lineof(yyYpv[1])); break;
case 41: yyval = (int*)tree5(T_PPROC, fixlist(yyYpv[2]), yyYpv[4], yyYpv[3], lineof(yyYpv[1])); break;
case 42: yyval = (int*)yyYpv[2]; break;
case 43: yyval = (int*)NIL; break;
case 44: yyval = (int*)newlist(yyYpv[1]); break;
case 45: yyval = (int*)addlist(yyYpv[1], yyYpv[3]); break;
case 46: yyval = (int*)tree2(T_CSTRNG, yyYpv[1]); break;
case 48: yyval = (int*)tree2(T_PLUSC, yyYpv[2]); break;
case 49: yyval = (int*) tree2(T_MINUSC, yyYpv[2]); break;
case 50: yyval = (int*) tree2(T_ID, yyYpv[1]); break;
case 51: yyval = (int*) tree2(T_CINT, yyYpv[1]); break;
case 52: yyval = (int*) tree2(T_CBINT, yyYpv[1]); break;
case 53: yyval = (int*) tree2(T_CFINT, yyYpv[1]); break;
case 54: yyval = (int*) newlist(yyYpv[1]); break;
case 55: yyval = (int*) addlist(yyYpv[1], yyYpv[3]); break;
case 57: yyval = (int*) tree3(T_TYPTR, lineof(yyYpv[1]), tree2(T_ID, yyYpv[2])); break;
case 59: yyval = (int*) tree3(T_TYPACK, lineof(yyYpv[1]), yyYpv[2]); break;
case 61: yyval = (int*) tree3(T_TYSCAL, lineof(yyYpv[1]), fixlist(yyYpv[2])); break;
case 62: yyval = (int*) tree4(T_TYRANG, lineof(yyYpv[2]), yyYpv[1], yyYpv[3]); break;
case 63: yyval = (int*) tree4(T_TYARY, lineof(yyYpv[1]), fixlist(yyYpv[3]), yyYpv[6]); break;
case 64: yyval = (int*) tree3(T_TYFILE, lineof(yyYpv[1]), yyYpv[3]); break;
case 65: yyval = (int*) tree3(T_TYSET, lineof(yyYpv[1]), yyYpv[3]); break;
case 66: {
			yyval = (int*) setuptyrec( lineof( yyYpv[1] ) , yyYpv[2] );
			if ((int)yyYpv[3] < 0)
				brerror(yyYpv[1], "record");
		  } break;
case 67: yyval = (int*) newlist(yyYpv[1]); break;
case 68: yyval = (int*) addlist(yyYpv[1], yyYpv[3]); break;
case 69: yyval = (int*) tree4(T_FLDLST, lineof(NIL), fixlist(yyYpv[1]), yyYpv[2]); break;
case 70: yyval = (int*) newlist(yyYpv[1]); break;
case 71: yyval = (int*) addlist(yyYpv[1], yyYpv[3]); break;
case 72: yyPerror("Malformed record declaration", PDECL); break;
case 73: yyval = (int*) NIL; break;
case 74: yyval = (int*) tree4(T_RFIELD, lineof(yyYpv[2]), fixlist(yyYpv[1]), yyYpv[3]); break;
case 75: yyval = (int*) NIL; break;
case 76: yyval = (int*) tree5(T_TYVARPT, lineof(yyYpv[1]), NIL, yyYpv[2], fixlist(yyYpv[4])); break;
case 77: yyval = (int*) tree5(T_TYVARPT, lineof(yyYpv[1]), yyYpv[2], yyYpv[4], fixlist(yyYpv[6])); break;
case 78: yyval = (int*) newlist(yyYpv[1]); break;
case 79: yyval = (int*) addlist(yyYpv[1], yyYpv[3]); break;
case 80: yyPerror("Malformed record declaration", PDECL); break;
case 81: yyval = (int*) NIL; break;
case 82: yyval = (int*) tree4(T_TYVARNT, lineof(yyYpv[2]), fixlist(yyYpv[1]), yyYpv[4]); break;
case 83: yyval = (int*) tree4(T_TYVARNT, lineof(yyYpv[2]), fixlist(yyYpv[1]), NIL); break;
case 84: yyval = (int*) newlist(yyYpv[1]); break;
case 85: {
			if ((p = yyYpv[1]) != NIL && (q = (int*)p[1])[0] == T_IFX) {
				q[0] = T_IFEL;
				q[4] = (int)yyYpv[2];
			} else
				yyval = (int*) addlist(yyYpv[1], yyYpv[2]);
		  } break;
case 86: if ((q = yyYpv[1]) != NIL && (p = (int*)q[1]) != NIL && p[0] == T_IF) {
			if (yychar < 0)
				yychar = yylex();
			if (yyshifts >= 2 && yychar == YELSE) {
				recovered();
				copy(&Y, &OY, sizeof Y);
				yerror("Deleted ';' before keyword else");
				yychar = yylex();
				p[0] = T_IFX;
			}
		  } break;
case 87: yyval = (int*) newlist(yyYpv[1]); break;
case 88: yyval = (int*) addlist(yyYpv[1], yyYpv[3]); break;
case 89: {
			yyval = (int*) NIL;
Kerror:
			yyPerror("Malformed statement in case", PSTAT);
		  } break;
case 90: goto Kerror; break;
case 91: yyval = (int*) tree4(T_CSTAT, lineof(yyYpv[2]), fixlist(yyYpv[1]), yyYpv[3]); break;
case 92: yyval = (int*) tree4(T_CSTAT, lineof(yyYpv[1]), NIL, yyYpv[2]); break;
case 93: yyval = (int*) tree4(T_OTHERCASE, lineof(yyYpv[2]), NIL, yyYpv[3]); break;
case 94: yyval = (int*) NIL; break;
case 95: yyval = (int*) NIL; break;
case 96: yyval = (int*) tree4(T_LABEL, lineof(yyYpv[2]), yyYpv[1] == NIL ? NIL : *hash(yyYpv[1], 1), yyYpv[3]); break;
case 97: yyval = (int*) tree4(T_PCALL, lineof(yyline), yyYpv[1], NIL); break;
case 98: yyval = (int*) tree4(T_PCALL, lineof(yyYpv[2]), yyYpv[1], fixlist(yyYpv[3])); break;
case 99: goto NSerror; break;
case 101: {
			yyval = (int*) tree3(T_BLOCK, lineof(yyYpv[1]), fixlist(yyYpv[2]));
			if ((int)yyYpv[3] < 0)
				brerror(yyYpv[1], "begin");
		  } break;
case 102: {
			yyval = (int*) tree4(T_CASE, lineof(yyYpv[1]), yyYpv[2], fixlist(yyYpv[4]));
			if ((int)yyYpv[5] < 0)
				brerror(yyYpv[1], "case");
		  } break;
case 103: yyval = (int*) tree4(T_WITH, lineof(yyYpv[1]), fixlist(yyYpv[2]), yyYpv[4]); break;
case 104: yyval = (int*) tree4(T_WHILE, lineof(yyYpv[1]), yyYpv[2], yyYpv[4]); break;
case 105: yyval = (int*) tree4(T_REPEAT, lineof(yyYpv[3]), fixlist(yyYpv[2]), yyYpv[4]); break;
case 106: yyval = (int*) tree5(T_FORU, lineof(yyYpv[1]), yyYpv[2], yyYpv[4], yyYpv[6]); break;
case 107: yyval = (int*) tree5(T_FORD, lineof(yyYpv[1]), yyYpv[2], yyYpv[4], yyYpv[6]); break;
case 108: yyval = (int*) tree3(T_GOTO, lineof(yyYpv[1]), *hash(yyYpv[2], 1)); break;
case 109: yyval = (int*) tree5(T_IF, lineof(yyYpv[1]), yyYpv[2], yyYpv[4], NIL); break;
case 110: yyval = (int*) tree5(T_IFEL, lineof(yyYpv[1]), yyYpv[2], yyYpv[4], yyYpv[6]); break;
case 111: yyval = (int*) tree5(T_IFEL, lineof(yyYpv[1]), yyYpv[2], yyYpv[4], NIL); break;
case 112: yyval = (int*) tree3(T_ASRT, lineof(yyYpv[1]), yyYpv[3]); break;
case 113: {
NSerror:
			yyval = (int*) NIL;
Serror:
			yyPerror("Malformed statement", PSTAT);
		  } break;
case 114: yyval = (int*) tree4(T_ASGN, lineof(yyYpv[2]), yyYpv[1], yyYpv[4]); break;
case 115: {
NEerror:
			yyval = (int*) NIL;
Eerror:
			yyPerror("Missing/malformed expression", PEXPR);
		  } break;
case 116: yyval = (int*) tree4(yyYpv[2], yyYpv[1][1] == SAWCON ? yyYpv[3][1] : yyYpv[1][1], yyYpv[1], yyYpv[3]); break;
case 117: yyval = (int*) tree3(T_PLUS, yyYpv[2][1], yyYpv[2]); break;
case 118: yyval = (int*) tree3(T_MINUS, yyYpv[2][1], yyYpv[2]); break;
case 119: yyval = (int*) tree4(yyYpv[2], yyYpv[1][1] == SAWCON ? yyYpv[3][1] : yyYpv[1][1], yyYpv[1], yyYpv[3]); break;
case 120: yyval = (int*) tree4(yyYpv[2], yyYpv[1][1] == SAWCON ? yyYpv[3][1] : yyYpv[1][1], yyYpv[1], yyYpv[3]); break;
case 121: yyval = (int*) tree2(T_NIL, NOCON); break;
case 122: yyval = (int*) tree3(T_STRNG, SAWCON, yyYpv[1]); break;
case 123: yyval = (int*) tree3(T_INT, NOCON, yyYpv[1]); break;
case 124: yyval = (int*) tree3(T_BINT, NOCON, yyYpv[1]); break;
case 125: yyval = (int*) tree3(T_FINT, NOCON, yyYpv[1]); break;
case 127: goto NEerror; break;
case 128: yyval = (int*) tree4(T_FCALL, NOCON, yyYpv[1], fixlist(yyYpv[3])); break;
case 129: yyval = (int*) yyYpv[2]; break;
case 130: yyval = (int*) tree3(T_NOT, NOCON, yyYpv[2]); break;
case 131: yyval = (int*) tree3(T_CSET, SAWCON, fixlist(yyYpv[2])); break;
case 132: yyval = (int*) tree3(T_CSET, SAWCON, NIL); break;
case 133: yyval = (int*) newlist(yyYpv[1]); break;
case 134: yyval = (int*) addlist(yyYpv[1], yyYpv[3]); break;
case 136: yyval = (int*) tree3(T_RANG, yyYpv[1], yyYpv[3]); break;
case 137: {
			yyval = (int*) setupvar(yyYpv[1], NIL);
		  } break;
case 138: yyYpv[1][3] = fixlist(yyYpv[1][3]); break;
case 139: yyval = (int*) setupvar(yyYpv[1], tree2(T_ARY, fixlist(yyYpv[3]))); break;
case 140: yyYpv[1][3] = addlist(yyYpv[1][3], tree2(T_ARY, fixlist(yyYpv[3]))); break;
case 141: yyval = (int*) setupvar(yyYpv[1], setupfield(yyYpv[3], NIL)); break;
case 142: yyYpv[1][3] = addlist(yyYpv[1][3], setupfield(yyYpv[3], NIL)); break;
case 143: yyval = (int*) setupvar(yyYpv[1], tree1(T_PTR)); break;
case 144: yyYpv[1][3] = addlist(yyYpv[1][3], tree1(T_PTR)); break;
case 146: yyval = (int*) tree4(T_WEXP, yyYpv[1], yyYpv[3], NIL); break;
case 147: yyval = (int*) tree4(T_WEXP, yyYpv[1], yyYpv[3], yyYpv[5]); break;
case 148: yyval = (int*) tree4(T_WEXP, yyYpv[1], NIL, yyYpv[2]); break;
case 149: yyval = (int*) tree4(T_WEXP, yyYpv[1], yyYpv[3], yyYpv[4]); break;
case 150: yyval = (int*) OCT; break;
case 151: yyval = (int*) HEX; break;
case 152: yyval = (int*) newlist(yyYpv[1]); break;
case 153: yyval = (int*) addlist(yyYpv[1], yyYpv[3]); break;
case 154: yyval = (int*) newlist(yyYpv[1]); break;
case 155: yyval = (int*) addlist(yyYpv[1], yyYpv[3]); break;
case 156: yyval = (int*) T_EQ; break;
case 157: yyval = (int*) T_LT; break;
case 158: yyval = (int*) T_GT; break;
case 159: yyval = (int*) T_NE; break;
case 160: yyval = (int*) T_LE; break;
case 161: yyval = (int*) T_GE; break;
case 162: yyval = (int*) T_IN; break;
case 163: yyval = (int*) T_ADD; break;
case 164: yyval = (int*) T_SUB; break;
case 165: yyval = (int*) T_OR; break;
case 166: yyval = (int*) T_OR; break;
case 167: yyval = (int*) T_MULT; break;
case 168: yyval = (int*) T_DIVD; break;
case 169: yyval = (int*) T_DIV; break;
case 170: yyval = (int*) T_MOD; break;
case 171: yyval = (int*) T_AND; break;
case 172: yyval = (int*) T_AND; break;
case 175: yyval = (int*) newlist(yyYpv[1]); break;
case 176: yyval = (int*) addlist(yyYpv[1], yyYpv[3]); break;
case 177: yyval = (int*) newlist(yyYpv[1]); break;
case 178: yyval = (int*) addlist(yyYpv[1], yyYpv[3]); break;
case 180: {
			yyval = (int*) tree3(T_TYID, lineof(yyline), yyYpv[1]);
		  } break;
}
}

int yyact[] = {0,-286,8196,12296,0,16384,4096,12296,0,12290
,-256,8199,-261,8206,-271,8213,-277,8205,-285,8212
,-293,8207,-295,8208,4096,-256,8215,-273,8214,4096
,-256,8199,-259,8217,-261,8206,-271,8213,-277,8205
,-285,8212,-293,8207,-295,8208,4096,12294,12295,12297
,0,12298,-256,8219,-259,12298,-261,12298,-271,12298
,-273,8218,-277,12298,-285,12298,-293,12298,-295,12298
,4096,0,12299,-256,8221,-259,12299,-261,12299,-271
,12299,-273,8220,-277,12299,-285,12299,-293,12299,-295
,12299,4096,0,12300,-256,8223,-259,12300,-261,12300
,-271,12300,-273,8224,-277,12300,-285,12300,-293,12300
,-295,12300,4096,12301,-276,8226,4096,-256,8228,-273
,8227,4096,-256,8230,-273,8229,4096,-256,8232,-273
,8224,4096,-270,8233,-304,8234,12320,-273,8236,4096
,12322,12323,-40,8237,4096,12292,-46,8238,4096,-256
,8255,-259,8246,-260,8247,-267,12383,-269,8251,-272
,8252,-273,8244,-274,8253,-276,8242,-288,8250,-296
,8249,-297,8248,-301,8254,-59,12383,4096,-61,8261
,4096,12308,-61,8262,4096,12312,-44,8264,-58,8263
,4096,12316,12465,-59,8265,-44,8266,4096,12303,-61
,8267,4096,12307,-61,8268,4096,12311,-44,8264,-58
,8269,4096,12315,-59,8270,4096,-59,8271,4096,-256
,8199,-259,8217,-261,8206,-271,8213,-277,8205,-285
,8212,-293,8207,-295,8208,4096,-40,8274,12325,-273
,8224,4096,12289,-267,8276,-59,8277,4096,12372,-256
,8255,-259,8246,-260,8247,-267,12383,-269,8251,-272
,8252,-273,8244,-274,8253,-276,8242,-288,8250,-294
,12383,-296,8249,-297,8248,-301,8254,-59,12383,4096
,-58,8279,4096,-40,8280,12385,-256,8281,-266,12474
,-267,12474,-294,12474,-46,12472,-40,12474,-59,12474
,-58,12425,-94,12471,-91,12470,4096,12388,-256,8284
,-273,8293,-276,8289,-279,8298,-280,8291,-284,8287
,-290,8288,-298,8290,-43,8285,-45,8286,-40,8295
,-91,8297,-126,8299,4096,-273,8302,4096,-273,8302
,4096,-276,8306,4096,-40,8308,4096,12401,-58,8309
,4096,-46,8311,-94,8312,-91,8310,12426,-91,8313
,4096,-46,8314,4096,-94,8315,4096,-273,8325,-276
,8322,-280,8324,-290,8317,-298,8323,-43,8319,-45
,8320,4096,-258,8334,-268,8335,-273,8338,-276,8322
,-280,8324,-283,8330,-287,8337,-289,8336,-290,8317
,-298,8323,-43,8319,-45,8320,-40,8332,-94,8328
,4096,-273,8340,4096,12302,-276,8341,4096,12317,12318
,-59,8345,4096,-58,8347,12331,-271,8352,-273,8224
,-285,8353,-295,8351,4096,-41,8354,-44,8264,4096
,12293,12374,12373,-256,8284,-273,8293,-276,8289,-279
,8298,-280,8291,-284,8287,-290,8288,-298,8290,-43
,8285,-45,8286,-40,8295,-91,8297,-126,8299,4096
,12387,-267,8359,-59,8277,4096,-257,8376,-262,8374
,-275,8367,-278,8375,-281,8360,-282,8370,-60,8365
,-61,8364,-62,8366,-43,8368,-45,8369,-124,8371
,-42,8372,-47,8373,-38,8377,4096,12403,12409,12410
,12411,12412,12413,12414,-256,8380,-257,12425,-262,12425
,-263,12425,-264,12425,-265,12425,-266,12425,-267,12425
,-275,12425,-278,12425,-281,12425,-282,12425,-291,12425
,-292,12425,-294,12425,-299,12425,-300,12425,-60,12425
,-61,12425,-62,12425,-43,12425,-45,12425,-124,12425
,-42,12425,-47,12425,-38,12425,-46,12472,-40,12475
,-41,12425,-59,12425,-44,12425,-58,12425,-94,12471
,-91,12470,-93,12425,4096,-40,8381,4096,-256,8284
,-273,8293,-276,8289,-279,8298,-280,8291,-284,8287
,-290,8288,-298,8290,-43,8285,-45,8286,-40,8295
,-91,8297,-93,8385,-126,8299,4096,12461,12462,-263
,8388,-44,8389,4096,12463,-46,12472,-94,12471,-91
,12470,12425,-257,8376,-262,8374,-263,8390,-275,8367
,-278,8375,-282,8370,-60,8365,-61,8364,-62,8366
,-43,8368,-45,8369,-124,8371,-42,8372,-47,8373
,-38,8377,4096,-294,8391,-59,8277,4096,-265,8392
,-292,8393,4096,12396,-257,8376,-262,8374,-275,8367
,-278,8375,-282,8370,-291,8394,-60,8365,-61,8364
,-62,8366,-43,8368,-45,8369,-124,8371,-42,8372
,-47,8373,-38,8377,4096,-61,8396,4096,-256,8284
,-273,8293,-276,8289,-279,8298,-280,8291,-284,8287
,-290,8288,-298,8290,-43,8285,-45,8286,-40,8295
,-91,8297,-126,8299,4096,-273,8400,4096,12432,12431
,-59,8403,4096,12334,12335,-273,8325,-276,8322,-280
,8324,-298,8323,4096,12338,12339,12340,12341,12467,-59
,8406,4096,12344,-273,8407,4096,12346,-258,8334,-268
,8335,-287,8337,-289,8336,4096,12348,-264,8410,4096
,-91,8411,4096,-281,8412,4096,-281,8413,4096,-273
,8224,12361,-264,12467,12468,-59,8418,4096,12466,12304
,-59,8419,4096,-59,8420,4096,-59,8421,4096,12319
,-59,8422,4096,-41,8424,-59,8425,4096,12332,-44
,8264,-58,8426,4096,-59,8430,4096,12384,-41,8431
,-44,8432,4096,12442,-257,8376,-262,8374,-275,8367
,-278,8375,-282,8370,-299,8435,-300,8436,-60,8365
,-61,8364,-62,8366,-43,8368,-45,8369,-124,8371
,-42,8372,-47,8373,-38,8377,-58,8433,12433,12389
,-256,8439,-267,12382,-273,8325,-276,8322,-280,8324
,-290,8317,-298,8323,-302,8441,-306,8442,-43,8319
,-45,8320,-59,12382,4096,12444,-61,8448,-62,8447
,12445,-61,8449,12446,12450,12451,12452,12453,12454,12455
,12456,12457,12458,12459,12460,-257,8376,-262,8374,-278
,8375,-42,8372,-47,8373,-38,8377,12405,-257,8376
,-262,8374,-278,8375,-42,8372,-47,8373,-38,8377
,12406,12415,-257,8376,-262,8374,-275,8367,-278,8375
,-282,8370,-60,8365,-61,8364,-62,8366,-43,8368
,-45,8369,-124,8371,-42,8372,-47,8373,-38,8377
,-41,8451,4096,12418,-44,8453,-93,8452,4096,12420
,12421,-257,8376,-262,8374,-264,8454,-275,8367,-278
,8375,-282,8370,-60,8365,-61,8364,-62,8366,-43
,8368,-45,8369,-124,8371,-42,8372,-47,8373,-38
,8377,12423,-273,8302,4096,-257,8376,-262,8374,-275
,8367,-278,8375,-282,8370,-60,8365,-61,8364,-62
,8366,-43,8368,-45,8369,-124,8371,-42,8372,-47
,8373,-38,8377,-41,8462,4096,-44,8465,-93,8464
,4096,-257,8376,-262,8374,-275,8367,-278,8375,-282
,8370,-60,8365,-61,8364,-62,8366,-43,8368,-45
,8369,-124,8371,-42,8372,-47,8373,-38,8377,12440
,12430,12473,-44,8465,-93,8466,4096,12429,12306,12336
,12337,12310,12345,12347,-41,8467,-44,8264,4096,-273
,8338,-276,8322,-280,8324,-290,8317,-298,8323,-43
,8319,-45,8320,-40,8332,4096,-273,8338,-276,8322
,-280,8324,-290,8317,-298,8323,-43,8319,-45,8320
,-40,8332,4096,-267,8473,4096,-256,8476,-260,8477
,-267,12363,-41,12363,-59,8475,4096,12358,-44,8264
,-58,8478,4096,12314,12305,12309,12313,12321,12330,12324
,-271,8352,-273,8224,-285,8353,-295,8351,4096,-44
,8264,-58,8481,4096,-40,8274,-44,8264,12325,-40
,8274,-44,8264,12325,12291,12386,-256,8284,-273,8293
,-276,8289,-279,8298,-280,8291,-284,8287,-290,8288
,-298,8290,-43,8285,-45,8286,-40,8295,-91,8297
,-126,8299,4096,12436,12438,12439,-256,8487,-267,8488
,-59,8486,4096,12375,12377,-44,8489,-58,8490,4096
,-58,8492,4096,12342,-257,8376,-262,8374,-275,4096
,-278,8375,-282,8370,-60,4096,-61,4096,-62,4096
,-43,8368,-45,8369,-124,8371,-42,8372,-47,8373
,-38,8377,12404,-257,8376,-262,8374,-278,8375,-42
,8372,-47,8373,-38,8377,12407,12408,12447,12448,12449
,-41,8493,-44,8432,4096,12417,12419,-256,8284,-273
,8293,-276,8289,-279,8298,-280,8291,-284,8287,-290
,8288,-298,8290,-43,8285,-45,8286,-40,8295,-91
,8297,-126,8299,4096,12391,12464,12392,-257,8376,-262
,8374,-275,8367,-278,8375,-282,8370,-60,8365,-61
,8364,-62,8366,-43,8368,-45,8369,-124,8371,-42
,8372,-47,8373,-38,8377,12393,-257,8376,-262,8374
,-263,8496,-275,8367,-278,8375,-282,8370,-60,8365
,-61,8364,-62,8366,-43,8368,-45,8369,-124,8371
,-42,8372,-47,8373,-38,8377,4096,-257,8376,-262
,8374,-263,8497,-275,8367,-278,8375,-282,8370,-60
,8365,-61,8364,-62,8366,-43,8368,-45,8369,-124
,8371,-42,8372,-47,8373,-38,8377,4096,-266,8498
,12397,12400,-257,8376,-262,8374,-275,8367,-278,8375
,-282,8370,-60,8365,-61,8364,-62,8366,-43,8368
,-45,8369,-124,8371,-42,8372,-47,8373,-38,8377
,12402,12428,12427,12349,12350,-44,8501,-93,8500,4096
,12355,12352,12353,12354,12357,-273,8224,12361,12360,-273
,8504,4096,12333,12326,12443,-257,8376,-262,8374,-275
,8367,-278,8375,-282,8370,-299,8435,-300,8436,-60
,8365,-61,8364,-62,8366,-43,8368,-45,8369,-124
,8371,-42,8372,-47,8373,-38,8377,-58,8509,12434
,-273,8325,-276,8322,-280,8324,-290,8317,-298,8323
,-302,8441,-306,8442,-43,8319,-45,8320,12382,12378
,12390,12380,12416,12422,-257,8376,-262,8374,-275,8367
,-278,8375,-282,8370,-60,8365,-61,8364,-62,8366
,-43,8368,-45,8369,-124,8371,-42,8372,-47,8373
,-38,8377,12424,-256,8255,-259,8246,-260,8247,-266
,12383,-267,12383,-269,8251,-272,8252,-273,8244,-274
,8253,-276,8242,-288,8250,-294,12383,-296,8249,-297
,8248,-301,8254,-59,12383,4096,-257,8376,-262,8374
,-275,8367,-278,8375,-282,8370,-60,8365,-61,8364
,-62,8366,-43,8368,-45,8369,-124,8371,-42,8372
,-47,8373,-38,8377,12441,-281,8518,4096,12359,-281
,8520,4096,-58,8521,12468,12362,12327,12328,12329,12437
,12376,12343,12379,12381,12394,12395,12398,12356,-273,8325
,-276,8322,-280,8324,-290,8317,-298,8323,-43,8319
,-45,8320,12369,-273,8528,4096,-257,8376,-262,8374
,-275,8367,-278,8375,-282,8370,-60,8365,-61,8364
,-62,8366,-43,8368,-45,8369,-124,8371,-42,8372
,-47,8373,-38,8377,12435,12351,-256,8530,-267,12364
,-41,12364,-59,8529,4096,12366,-44,8489,-58,8531
,4096,-281,8532,4096,12468,-273,8325,-276,8322,-280
,8324,-290,8317,-298,8323,-43,8319,-45,8320,12369
,12368,-40,8534,4096,12367,-273,8224,-41,8537,12361
,-256,8530,-267,12365,-41,12365,-59,8529,4096,-41
,8538,4096,12371,12370,-1};

int yypact[] = {0,1,4,7,8,25,30,47,48,49
,50,71,92,113,114,117,122,127,132,7
,137,140,141,142,145,146,149,178,181,182
,185,186,191,192,193,198,199,202,203,206
,207,212,213,216,219,236,239,242,243,248
,249,280,283,286,307,149,308,335,308,149
,338,341,308,344,347,348,351,358,361,364
,367,382,382,411,414,415,367,382,382,418
,419,420,423,426,435,440,441,442,249,443
,470,471,476,507,308,308,508,509,510,511
,512,513,514,585,308,308,588,617,618,619
,624,625,632,663,668,673,674,308,705,708
,735,738,708,735,739,740,743,744,745,745
,754,755,756,757,758,759,762,763,766,767
,776,239,777,780,783,786,789,792,795,798
,799,800,803,806,809,810,382,813,818,819
,239,239,239,824,827,828,833,834,869,870
,308,308,308,895,896,901,904,905,906,907
,908,909,910,911,912,913,914,915,928,941
,443,942,973,974,979,980,981,249,1012,249
,308,308,308,249,1015,308,1046,1051,1080,1081
,1082,1087,1088,1089,1090,1091,1092,1093,1094,367
,1099,382,1116,1133,1136,1147,1148,1153,1154,1155
,1156,1157,1158,1159,1160,382,1169,1174,1179,1184
,1185,1186,308,1213,1214,1215,1216,1223,1224,1225
,249,1230,1233,1234,1263,1276,1277,1278,1279,1280
,1285,1286,1287,308,1314,1315,1316,1317,1346,1377
,1408,1411,1412,1441,308,1442,1443,1444,1445,1450
,1451,1452,1453,1454,1455,1458,1459,382,1462,1463
,382,423,423,1464,1465,1500,1519,1520,367,249
,1521,249,1522,1523,1524,249,249,1553,1586,1615
,1116,1618,1619,1622,1625,1626,1627,1628,308,1629
,1630,1631,1632,1633,1634,1635,1636,382,1637,1638
,1653,1656,1685,1686,1695,1696,1701,1704,1705,1720
,1721,1638,1724,1725,1730,1739,1742,1743,-1};

int yyr1[] = {0,1,1,2,2,4,3,3,3,7
,7,7,7,7,8,13,13,9,9,9
,9,10,10,10,10,11,11,11,11,12
,12,12,17,16,18,18,19,19,22,22
,22,22,20,20,21,21,14,14,14,14
,23,23,23,23,25,25,15,15,15,15
,26,26,26,27,27,27,27,29,29,30
,31,31,31,33,33,32,32,32,34,34
,34,35,35,35,6,6,37,38,38,38
,38,39,39,39,39,36,36,36,36,36
,36,36,36,36,36,36,36,36,36,36
,36,36,36,36,42,43,43,43,43,43
,43,43,43,43,43,43,43,43,43,43
,43,43,43,51,51,52,52,45,45,53
,53,53,53,53,53,59,59,59,59,59
,60,60,55,55,41,41,46,46,46,46
,46,46,46,47,47,47,47,48,48,48
,48,48,48,50,50,44,44,5,5,24
,28,61,54,58,56,57,40,49,-1};

int yyr2[] = {0,4,1,6,2,3,2,2,0,1
,1,1,1,1,3,1,3,5,5,2
,2,5,5,2,2,5,5,2,2,3
,3,4,1,5,1,1,3,0,3,4
,4,4,2,0,1,3,1,1,2,2
,1,1,1,1,1,3,1,2,1,2
,1,3,3,6,3,3,3,1,3,2
,1,3,2,0,3,0,4,6,1,3
,2,0,5,4,1,2,2,1,3,1
,2,3,2,3,0,0,3,1,4,2
,1,3,5,4,4,4,6,6,2,4
,6,5,4,1,4,1,3,2,2,3
,3,1,1,1,1,1,1,2,4,3
,2,3,2,1,3,1,3,1,1,4
,4,3,3,2,2,1,3,5,2,4
,1,1,1,3,1,3,1,1,1,2
,2,2,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,3,1,3,1
,1,1,1,1,1,1,1,1,-1};

int yygo[] = {0,-1,1,-1,2,2,5,18,43,-1
,3,43,80,-1,24,11,30,16,39,45
,83,82,158,140,217,159,235,160,236,161
,237,233,158,-1,225,54,90,58,112,-1
,47,-1,6,-1,8,-1,9,-1,10,-1
,11,-1,12,-1,33,69,124,75,150,168
,251,218,276,294,251,297,320,328,251,337
,251,340,251,-1,141,71,147,76,151,77
,152,155,231,220,279,234,288,286,313,289
,314,326,331,-1,134,-1,17,-1,18,-1
,19,236,290,237,291,-1,81,290,315,291
,316,-1,154,-1,156,233,287,-1,157,127
,212,128,213,-1,126,-1,129,168,248,294
,248,-1,334,219,278,221,280,309,327,-1
,135,138,216,-1,137,285,311,329,335,-1
,139,-1,277,342,344,-1,222,-1,223,-1
,282,283,310,-1,224,340,343,-1,332,337
,341,-1,333,49,86,87,163,196,263,198
,265,202,269,249,299,298,321,300,322,304
,323,305,324,306,325,-1,48,-1,49,-1
,245,294,319,-1,246,-1,51,189,258,-1
,164,59,113,-1,53,55,91,57,111,61
,115,93,186,94,187,103,190,104,191,105
,195,116,203,118,206,121,206,169,252,170
,253,171,254,199,266,200,267,201,268,204
,271,241,293,261,195,262,303,273,307,317
,330,-1,166,-1,108,25,64,49,64,54
,64,56,109,58,64,59,64,87,64,196
,64,197,264,198,64,202,64,249,64,298
,64,300,64,304,64,305,64,306,64,-1
,100,-1,169,-1,170,-1,171,-1,102,-1
,104,-1,192,261,302,-1,194,-1,65,-1
,66,121,209,-1,205,-1,67,122,210,-1
,207,-1,68,240,292,-1,165,293,318,-1
,242,-1,-1,-1};

int yypgo[] = {0,1,3,5,11,15,35,41,43,45
,47,49,51,53,55,75,95,97,99,101
,107,113,115,119,125,127,133,141,145,151
,153,157,159,161,165,169,173,197,199,201
,205,207,211,215,263,265,301,303,305,307
,309,311,313,317,319,321,325,327,331,333
,337,341,-1};


yyEactr(__np__, var)
int __np__;
char *var;
{
switch(__np__) {
default:
return (1);
case 179:  return (identis(var, CONST)); break;
case 181:  return (identis(var, VAR)); break;
case 182:  return (identis(var, ARRAY)); break;
case 183:  return (identis(var, PTRFILE)); break;
case 184:  return (identis(var, RECORD)); break;
case 185:  return (identis(var, FIELD)); break;
case 186:  return (identis(var, PROC)); break;
case 187:  return (identis(var, FUNC)); break;
case 137: {
			 return (identis(var, VAR));
}
break;
case 180: {
			 return (identis(var, TYPE));
}
break;
}
}
