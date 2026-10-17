#
# define APPEND 257
# define COPY 258
# define CREATE 259
# define DEFINE 260
# define DELETE 261
# define DESTROY 262
# define HELP 263
# define INDEX 264
# define MODIFY 265
# define PRINT 266
# define INTEGRITY 267
# define RANGE 268
# define REPLACE 269
# define RETRIEVE 270
# define SAVE 271
# define UNIQUE 272
# define PERMIT 273
# define VIEW 274
# define INGRES 275
# define EXIT 276
# define PARAM 277
# define TYPE 278
# define ALLOC 279
# define STRUCT 280
# define STRUCT_VAR 281
# define ALL 282
# define BY 283
# define FROM 284
# define IN 285
# define INTO 286
# define IS 287
# define OF 288
# define ON 289
# define ONTO 290
# define TO 291
# define WHERE 292
# define UNTIL 293
# define AT 294
# define NAME 295
# define SCONST 296
# define I2CONST 297
# define I4CONST 298
# define F8CONST 299
# define C_CODE 300
# define COMMA 301
# define LPAREN 302
# define RPAREN 303
# define PERIOD 304
# define QUOTE 305
# define BGNCMNT 306
# define ENDCMNT 307
# define LBRACE 308
# define RBRACE 309
# define LBRKT 310
# define RBRKT 311
# define NONREF 312
# define SEMICOL 313
# define POINTER 314
# define COLON 315
# define UOP 316
# define BOP 317
# define BDOP 318
# define EOP 319
# define LBOP 320
# define LUOP 321
# define FOP 322
# define FBOP 323
# define AOP 324
# define unaryop 325
	/* STANDARD SCANNER & PARSER GLOBALS */

# include	"constants.h"
# include	"../fileio.h"
# include	"globals.h"

#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar, yyerrflag;

int yyval 0;
int *yypv;
int yylval 0;
yyactr(__np__){
	register struct cvar	*cvarp;

switch(__np__){

case 1: 
		{
			/* for each "statement", free the symbol space
			 * used by that query (lookahed space must not
			 * be freed), and catch up on output lines
			 */
			symspfree();
			equate_lines();
		} break;
case 3: 
		{
			w_sync();
			/* the purpose of the actions for startquel
			 * and this action is to make each query
			 * a single compound C statement (if (x) "{query}")
			 */
			w_op("}");
		} break;
case 4:
		{
			end_quote();
			w_op("}");
		} break;
case 8:
			w_op("{"); break;
case 10:
			Block_level =+ 1; break;
case 11:
		{
			if (Block_level == 0)
				yyserror("extra '}'", yypv[1]);
			else if ((Block_level =- 1) == 0)
			{
				freecvar(&C_locals);
				freecvar(&F_locals);
			}
		} break;
case 12:
			w_op(yypv[1]->d_elm); break;
case 13:
			w_op(yypv[1]->d_elm); break;
case 76:
			w_con(I2CONST, yypv[1]->d_elm); break;
case 77:
		{
			if (yypv[1])
			{
				if (!Cvarp)
					w_key(yypv[1]->d_elm);
				else if (Fieldp && Fieldp->c_type == opINT
					|| Cvarp->c_type == opINT)
						w_var(Cv_display, opINT);
				else if (Fieldp && Fieldp->c_type == opSTRING 
					|| Cvarp->c_type == opSTRING)
						w_var(Cv_display, opIDSTRING);
				else
					yyserror("in MODIFY, qual var must be in or string",
					yypv[1]);
			}
			else
				yyserror("bad modify qualification", 0);
			free_display(Cv_display);
			Cvarp = Fieldp = 0;
		} break;
case 82:
			w_key(yypv[1]->d_elm); break;
case 83:
			w_key(yypv[1]->d_elm); break;
case 84:
			w_key(yypv[1]->d_elm); break;
case 85: 
			w_key(yypv[1]->d_elm); break;
case 114:
		{
			if (yypv[1] && Cvarp)
			{
				if (Fieldp && Fieldp->c_type != opSTRING
				   || !Fieldp && Cvarp->c_type != opSTRING)
					yyserror("string var expected for from/into in COPY",
					yypv[1]);
				else
					w_var(Cv_display, opIDSTRING);
			}
			else
				yyserror("into/from expected in COPY", yypv[1]);
			free_display(Cv_display);
			Fieldp = Cvarp = 0;
		} break;
case 116: 
		{ 
			Opflag = mdEXIT;
			w_new("IIexit();"); 
		} break;
case 117: 
			w_op(");"); break;
case 118:
			w_op("0"); break;
case 120:
			w_op(","); break;
case 121:
		{
			w_string(yypv[1]->d_elm, 0);
			w_op(",");
		} break;
case 124:
			w_flush(); break;
case 125:
			w_flush(); break;
case 126:
		{
			w_new("IIsetup();");
			w_sync();
		} break;
case 127:
		{
			Opflag = mdTUPRET;
			w_key(yypv[1]->d_elm);
		} break;
case 128:
			Opflag = mdTUPRET; break;
case 133:
			Opflag = mdCTLELM; break;
case 134:
			w_op("}"); break;
case 135:
			w_op("}"); break;
case 136:
		{
			w_new("while(IIn_get(");
			w_file();
			w_op(")){");
			w_ret();
			free_ret();
			w_op("if(IIerrtest())continue;");
			equate_lines();
		} break;
case 137:
			w_op("}"); break;
case 138:
			w_op("}"); break;
case 139:
		{
			w_new("while(IIgettup(");
			w_file();
			w_op(")){");
			equate_lines();
		} break;
case 140:
		{
			w_new("IIsetup();");
			w_sync();
		} break;
case 142:
		{
			w_op(yypv[3]->d_elm);
			Type_spec = 0;
		} break;
case 143: 
		{ 
			w_op(yypv[2]->d_elm);
			Type_spec = 0;
		} break;
case 148:
		{
			Struct_flag = 0;
			Type_spec = opSTRUCT;
		} break;
case 149:
		{
			Struct_flag = 0;
			Type_spec = opSTRUCT;
		} break;
case 150:
		{
			Opflag = mdDECL;
			w_key(yypv[1]->d_elm);
			/* in case the default "int" should be assumed,
			 * the Type_spec is set up for it, if a previous
			 * type hasn't been given
			 */
			if (!Type_spec)
				Type_spec = opINT;
		} break;
case 151:
		{
			Opflag = mdDECL;
			w_key(yypv[1]->d_elm);
			Type_spec = Opcode;
		} break;
case 155:
			w_key(yypv[2]->d_elm); break;
case 156:
		{
			w_op(yypv[3]->d_elm);
			Type_spec = yypv[3];
		} break;
case 159:
		{
			w_op(yypv[3]->d_elm);
			Type_spec = 0;
		} break;
case 163:
		{
			if (Type_spec == opSTRING)
				Indir_level =- 1;
			if (Struct_flag)
				decl_field(yypv[1]->d_elm, Type_spec,
					Indir_level, Block_level);
			else
				decl_cvar(yypv[1]->d_elm, Type_spec,
					Indir_level, Block_level);
			free_display(Cv_display);
			Indir_level = Field_indir = 0;
			Fieldp = Cvarp = 0;
		} break;
case 164:
		{
			yyval = yypv[1];
			if (Cvarp && Cvarp->c_indir != Indir_level)
			{
				yyserror("bad indirection on a C variable", yypv[1]);
				yyval = 0;
			}
			Indir_level = Field_indir = 0;
		} break;
case 165:
		{
			enter_display(Cv_display, salloc(yypv[1]->d_elm));
			Cvarp = Fieldp = 0;
			yyval = yypv[2];
		} break;
case 166:
		{
			enter_display(Cv_display, salloc(yypv[1]->d_elm));
			Cvarp = Fieldp = 0;
			yyval = yypv[2];
		} break;
case 167:
		{
			if (!Fieldp)
			{
				yyserror("undeclared field", yypv[1]);
				yyval = yypv[0];
			}
			else if (Fieldp->c_indir != Field_indir)
			{
				yyserror("bad indirection on a structure's field",
				yypv[1]);
				yyval = 0;
			}
			if (Cvarp->c_indir != Indir_level)
			{
				yysemerr("bad indirection a structure variable",
				Cvarp->c_indir);
				yyval = 0;
			}
			Indir_level = Field_indir = 0;
		} break;
case 168:
		{
			if (yypv[1]->d_elm [1] == '*')
				Field_indir =+ 1;
			Field_indir =+ 1;
			yyval = yypv[2];
		} break;
case 169:
			Field_indir =+ 1; break;
case 171:
		{
			Cvarp = getcvar(yypv[1]->d_elm);
			enter_display(Cv_display, yypv[1]->d_elm);
		} break;
case 172:
		{
			Indir_level =+ 1;
			yyval = yypv[2];
		} break;
case 173:
		{
			enter_display(Cv_display, yypv[2]->d_elm);
			Fieldp = getfield(yypv[2]->d_elm);
			yyval = yypv[2];
		} break;
case 174:
			enter_display(Cv_display, yypv[1]->d_elm); break;
case 175:
		{
			enter_display(Cv_display, yypv[1]->d_elm);
			Indir_level =+ 1;
		} break;
case 176:	
		{
			if (yypv[1])
			{
				if (!Fieldp && ! Cvarp)
				{
					if (!Field_indir && !Indir_level
					  && (sequal(yypv[1]->d_elm, "dba")
					    || sequal(yypv[1]->d_elm, "usercode")))
						/* constant operator COP */
						w_key(yypv[1]->d_elm);
					else
						yyserror("C var expected", yypv[1]);
				}
				else if (Opflag == mdCTLELM)
				{
					w_con(NAME,
					  Fieldp ? Fieldp->c_id: Cvarp->c_id);
					enter_ret(Cv_display,
					  Fieldp ? Fieldp->c_type: Cvarp->c_type);
				}
				else
					w_var(Cv_display,
					  Fieldp ? Fieldp->c_type: Cvarp->c_type);
			}
			free_display(Cv_display);
			Fieldp = Cvarp = 0;
			Indir_level = Field_indir = 0;
		} break;
case 177:
		{
			if (Opflag == mdDECL)
				w_con(NAME, yypv[1]->d_elm);
			else
			{
				Cvarp = getcvar(yypv[1]->d_elm);
				enter_display(Cv_display, salloc(yypv[1]->d_elm));
			}
		} break;
case 178:
		{
			if (yypv[1]->d_elm [1] == '*')
				Indir_level =+ 1;
			Indir_level =+ 1;
			yyval = yypv[2];
		} break;
case 179:
		{
			Indir_level =+ 1;
		} break;
case 180:
		{
			if (!sequal(yypv[1]->d_elm, "*") && !sequal(yypv[1]->d_elm, "**"))
				yyserror(Opflag == mdDECL ?
				"invalid operator in declaration":
				"invalid operator in C variable",
				yypv[1]);
			if (Opflag == mdDECL)
				w_op(yypv[1]->d_elm);
			else
				enter_display(Cv_display, salloc(yypv[1]->d_elm));
		} break;
case 181:
		{
			if (Opflag == mdDECL)
				eat_display(0, '[', ']');
			else
				eat_display(Cv_display, '[', ']');
		} break;
case 182:
		{
			if (yypv[1])
			{
				if (Cvarp)
				{
					if (Fieldp && Fieldp->c_type != opSTRING
					   || !Fieldp && Cvarp->c_type != opSTRING)
						yyserror("string var expected", yypv[1]);
					else if (Opflag == mdFILENAME)
						w_var(Cv_display, opSTRING);
					else if (Opflag == mdINGRES)
						w_display(Cv_display);
					else
						w_var(Cv_display, opIDSTRING);
				}
				else if (Opflag == mdINGRES)
					w_string(yypv[1]->d_elm, 0);
				else if (Opflag == mdFILENAME)
					yyserror("file for a COPY must be a string or string variable",
					yypv[1]);
				else
					w_key(yypv[1]->d_elm);
			}
			free_display(Cv_display);
			Fieldp = Cvarp = 0;
		} break;
case 185:
			w_con(I2CONST, yypv[1]->d_elm); break;
case 186:
		{
			if (yypv[1])
			{
				if (Cvarp)
					if (Fieldp && Fieldp->c_type == opINT
					   || Cvarp->c_type == opINT)
						w_var(Cv_display, opINT);
					else
						yyserror("integer variable required",
						yypv[1]);
				else
					yyserror("integer variable required", yypv[1]);
			}
			free_display(Cv_display);
		} break;
case 189:
		{
			w_op("(");
			end_quote();
			if (Opflag == mdTUPRET)
				w_key("IIw_left");
			else
				w_key("IIw_right");
			eat_display(0, '(', ')');
			w_op(";");
			begin_quote();
			w_op(")");

		} break;
case 191:
		{
			if (!yypv[2] || !Cvarp)
				yyserror("C var (string) expected", yypv[2]);
			else if (Fieldp && Fieldp->c_type == opSTRING
				|| Cvarp->c_type == opSTRING)
			{
				w_key("");	/* put out blank after
						 * "where"
						 */
				end_quote();
				w_op("IIwrite(");
				w_display(Cv_display);
				w_op(");");
			}
			else
				yyserror("var must be string valued for qualification",
				yypv[2]);
			free_display(Cv_display);
			Cvarp = Fieldp = 0;
		} break;
case 202: 
			w_con(SCONST, yypv[1]->d_elm); break;
case 218: 
			w_con(I2CONST, yypv[1]->d_elm); break;
case 219: 
			w_con(I4CONST, yypv[1]->d_elm); break;
case 220: 
			w_con(F8CONST, yypv[1]->d_elm); break;
case 221: 
			w_con(SCONST, yypv[1]->d_elm); break;
case 227:
		{
			if (Opflag != mdVIEW && Opflag != mdRETRIEVE
			   && Opflag != mdAPPEND)
				yyserror(
				"'all' applied to this range variable illegal in this kind of statement",
				yypv[1]);
		} break;
case 228: 
			w_key(yypv[1]->d_elm); break;
case 229: 
			w_key(yypv[1]->d_elm); break;
case 230: 
			w_op(yypv[1]->d_elm); break;
case 231: 
			w_op(yypv[1]->d_elm); break;
case 232: 
			w_op(yypv[1]->d_elm); break;
case 233: 
			w_op("="); break;
case 234: 
			w_op(yypv[1]->d_elm); break;
case 235: 
			w_key(yypv[1]->d_elm); break;
case 236: 
			w_key(yypv[1]->d_elm); break;
case 237: 
			w_op(yypv[1]->d_elm); break;
case 238: 
			w_op(yypv[1]->d_elm); break;
case 239:
			w_key(yypv[1]->d_elm); break;
case 240: 
			w_key(yypv[1]->d_elm); break;
case 241:
		{
			begin_quote();
			w_key(yypv[2]->d_elm);
			Opflag = mdAPPEND;
		} break;
case 242:
		{
			Opflag = mdAPPEND;
			begin_quote();
			w_key(yypv[1]->d_elm);
		} break;
case 243:
		{
			Opflag = mdCOPY;
			begin_quote();
			w_key(yypv[1]->d_elm);
		} break;
case 244:
		{
			Opflag = mdCOPY;
			begin_quote();
			w_key(yypv[2]->d_elm);
		} break;
case 245: 
		{
			w_key(yypv[1]->d_elm);
			Opflag = mdFILENAME;
		} break;
case 246: 
		{
			w_key(yypv[1]->d_elm);
			Opflag = mdFILENAME;
		} break;
case 247:
		{
			Opflag = mdCREATE;
			begin_quote();
			w_key(yypv[1]->d_elm);
		} break;
case 248:
		{
			Opflag = mdCREATE;
			begin_quote();
			w_key(yypv[2]->d_elm);
		} break;
case 249:
		{
			Opflag = mdDEFINE;
			begin_quote();
			w_key(yypv[1]->d_elm);
		} break;
case 250:
		{
			Opflag = mdDELETE;
			begin_quote();
			w_key(yypv[1]->d_elm);
		} break;
case 251:
		{
			Opflag = mdDESTROY;
			begin_quote();
			w_key(yypv[1]->d_elm);
		} break;
case 252:
		{
			Opflag = mdHELP;
			begin_quote();
			w_key(yypv[1]->d_elm);
		} break;
case 253:
		{
			Opflag = mdINDEX;
			begin_quote();
			w_key(yypv[1]->d_elm);
			w_key(yypv[2]->d_elm);
		} break;
case 254:
		{
			Opflag = mdINGRES;
			w_new("IIingres(");
		} break;
case 255:
		{
			if (Opflag == mdDEFINE)
				Opflag = mdINTEGRITY;
			w_key(yypv[1]->d_elm);
		} break;
case 256:
		{ 
			if (Opflag == mdCTLELM)
				Opflag = mdTUPRET;
			w_op("=");
		} break;
case 257: 
			w_key(yypv[1]->d_elm); break;
case 258:
		{
			Opflag = mdMODIFY;
			begin_quote();
			w_key(yypv[1]->d_elm);
		} break;
case 259:
		{
			if (Opflag == mdDEFINE)
				Opflag = mdINTEGRITY;
			w_key(yypv[1]->d_elm);
		} break;
case 260:  
		{ 
			Opflag = mdPRINT;
			begin_quote();
			w_key(yypv[1]->d_elm);
		} break;
case 261: 
		{
			Opflag = mdRANGE;
			begin_quote();
			w_key(yypv[1]->d_elm);
			w_key(yypv[2]->d_elm);
		} break;
case 262:
		{
			Opflag = mdREPLACE;
			begin_quote();
			w_key(yypv[1]->d_elm);
		} break;
case 263:
		{
			begin_quote();
			Opflag = mdREPLACE;
			w_key(yypv[2]->d_elm);
		} break;
case 264:
		{
			Opflag = mdRETRIEVE;
			begin_quote();
			w_key(yypv[1]->d_elm);
		} break;
case 265:
		{
			Opflag = mdRETRIEVE;
			begin_quote();
			w_key(yypv[2]->d_elm);
		} break;
case 266:
		{
			Opflag = mdSAVE;
			begin_quote();
			w_key(yypv[1]->d_elm);
		} break;
case 267:
		{
			Opflag = mdDECL;
			Struct_flag = 1;
			w_key(yypv[1]->d_elm);
		} break;
case 268:
		{
			begin_quote();
			w_key(yypv[2]->d_elm);
			Opflag = mdTUPRET;
		} break;
case 269:
		{
			if (Opflag == mdDEFINE)
				Opflag = mdVIEW;
			w_key(yypv[1]->d_elm);
		} break;
case 270:
			w_key(yypv[1]->d_elm); break;
case 276:
			w_key(yypv[1]->d_elm); break;
case 277:
			w_op(yypv[1]->d_elm); break;
case 278: 
			w_op(yypv[1]->d_elm); break;
case 283:
			w_key(yypv[1]->d_elm); break;
case 288: 
			w_op("="); break;
case 291:
			w_op(yypv[1]->d_elm); break;
case 292: 
			w_op(yypv[1]->d_elm); break;
case 293:
			w_key(yypv[1]->d_elm); break;
case 294:
			w_key(yypv[1]->d_elm); break;
case 295:
			w_op(yypv[1]->d_elm); break;
case 300:
			w_op(yypv[1]->d_elm); break;
case 301: 
			w_key(yypv[1]->d_elm); break;
case 305: 
			w_key(yypv[1]->d_elm); break;
case 306: 
			w_key(yypv[1]->d_elm); break;
}
return(0);
}
int yyerrval = 256;


# include	"tokens.y"

int yyact[] = {0,12290,4096,16384,4352,8198,4374,8208,4375,8209
,4376,8212,4396,8199,4404,8203,4405,8204,12296,12289
,4353,8265,4354,8266,4355,8267,4356,8272,4357,8268
,4358,8269,4359,8270,4360,8271,4361,8273,4362,8274
,4364,8275,4365,8276,4366,8277,4367,8278,4371,8279
,4372,8258,4373,8264,0,12293,12294,12295,12297,12298
,12299,4391,8285,4409,8282,4413,8287,0,12300,12301
,4375,8209,12432,4374,8208,4376,8212,12433,12436,12439
,12438,4404,8293,12441,4391,8295,4404,8293,0,12555
,12291,12292,12302,12303,12304,12305,12306,12307,12308,12309
,12310,12311,12312,12313,12314,12315,12316,12317,12318,12319
,12320,12321,12322,12323,12324,12325,12326,4382,8298,4385
,8301,4386,8299,4387,8300,12563,4377,8309,4391,8285
,4408,8305,4413,8287,0,4380,8315,4381,8313,4385
,8314,12570,4363,8321,4369,8322,4377,8309,4391,8285
,4408,8305,4413,8287,0,4363,8321,4369,8322,4370
,8332,4377,8309,4378,8326,4391,8285,4392,8333,4408
,8305,4413,8287,12342,4363,8321,4369,8322,4370,8332
,0,4353,8342,4357,8343,4365,8344,4366,8341,4378
,8326,0,4377,8309,4391,8285,4408,8305,4413,8287
,0,4381,8351,4382,8350,4385,8352,12587,4368,8356
,4382,8357,4387,8358,4398,12416,12592,12404,4377,8309
,4391,8285,4392,8366,4408,8305,4413,8287,0,4382
,8357,4387,8358,12592,4404,8370,12424,4404,8373,12427
,4353,8376,4354,8377,4355,8378,4356,8272,4365,8379
,4366,8380,0,12530,12531,12535,12538,12539,12540,4385
,8381,0,12537,12546,12548,4384,8382,0,12550,12552
,12554,12542,4368,8356,12416,4397,8386,4409,8384,0
,12431,12449,4406,8388,12451,12465,4391,8285,4413,8287
,0,12468,12434,12435,12437,12440,12446,12579,12442,12443
,4398,8393,0,12559,12560,12561,12562,4398,8393,0
,12470,4406,8388,12452,4377,8397,4391,8396,0,4406
,8388,12455,4377,8309,4391,8285,4413,8287,0,4400
,8403,4406,8388,4410,8404,0,12459,4388,8408,12480
,12567,12568,12569,4397,8386,12336,12471,12338,12339,12543
,12547,12343,4397,8386,12344,12558,12346,12350,12351,12348
,12349,12557,12490,4383,8415,0,4381,8419,4384,8420
,4385,8417,4386,8418,0,12367,4387,8423,0,4384
,8430,4385,8429,4387,8423,4397,8386,0,12368,12370
,12371,12372,12373,12374,4397,8386,12390,12584,12585,12586
,4398,8393,0,12415,12590,12591,4389,8439,0,4398
,8441,0,12405,4377,8309,4391,8285,4392,8366,4408
,8305,4413,8287,12406,12408,12409,12412,12424,12423,12413
,12427,12426,4370,8332,0,12529,12532,12536,12551,4368
,12556,4398,12556,12553,12541,12549,12430,4391,8285,4413
,8287,0,12566,12467,12469,12466,4374,8208,4396,8455
,4405,8452,0,4377,8309,4391,8285,4408,8305,4413
,8287,0,12580,12328,4377,8309,4391,8285,4399,8464
,4408,8305,4413,8287,0,12453,12454,12457,12456,12458
,4391,8466,0,12462,12463,4377,8309,4391,8285,4408
,8305,4413,8287,0,12334,4377,8309,4391,8285,4392
,8485,4393,8482,4394,8483,4395,8484,4398,8393,4408
,8305,4412,8488,4413,8287,4417,8473,4418,8489,4419
,8490,4420,8491,12485,12594,12335,4377,8309,4378,8326
,4391,8285,4393,8498,4408,8305,4413,8287,0,4377
,8309,4391,8285,4392,8333,4408,8305,4413,8287,0
,4397,8386,12345,12576,12572,12573,12574,12575,12589,4353
,8342,4357,8343,4365,8344,4366,8341,4378,8326,0
,12375,12376,12377,12582,12581,12393,12421,12395,4377,8309
,4391,8285,4408,8305,4413,8287,0,12593,12477,4377
,8309,4380,8521,4382,8520,4391,8285,4408,8305,4413
,8287,0,12403,12407,4396,8199,4404,8203,4405,8204
,0,12450,12444,12445,4391,8285,4413,8287,0,12448
,12327,4399,8464,0,4397,8386,12492,4379,8534,4383
,8533,4400,8535,0,12495,4397,8386,4399,8464,0
,4380,8521,4382,8520,0,12588,12460,12461,4397,8386
,4399,8464,0,4416,8542,12478,4383,12464,4400,12470
,4412,12464,4413,12464,4414,12464,4415,12464,12479,4377
,8309,4391,8285,4392,8485,4393,8482,4394,8483,4395
,8484,4398,8393,4408,8305,4412,8488,4413,8287,4417
,8473,4418,8489,4419,8490,4420,8491,12485,4377,8309
,4391,8285,4392,8485,4393,8482,4394,8483,4395,8484
,4398,8393,4408,8305,4412,8488,4413,8287,4417,8473
,4418,8489,4419,8490,4420,8491,12485,12484,12517,4383
,8551,4412,8553,4413,8552,4414,8550,4415,8549,0
,12496,12497,12498,4377,8309,4391,8285,4392,8485,4393
,8482,4394,8483,4395,8484,4398,8393,4408,8305,4412
,8488,4413,8287,4418,8489,4419,8490,4420,8491,0
,12506,12507,12508,12509,12510,12511,12522,12523,12524,12528
,4400,8535,0,12472,12337,4397,8386,12340,12341,12475
,12473,12474,12347,4383,8562,12578,4385,8429,12355,4398
,8393,12379,12369,12391,12392,12394,12414,4397,8386,12418
,4377,8309,4391,8285,4408,8305,4413,8287,0,12396
,4377,8309,4391,8285,4393,8498,4408,8305,4413,8287
,0,12399,4377,8309,4391,8285,4392,8333,4408,8305
,4413,8287,0,12401,12402,12533,12534,12410,12411,4405
,8575,0,4405,8576,0,12428,4397,8386,4409,8578
,0,12491,4377,8309,4378,8326,4391,8285,4408,8305
,4413,8287,0,12544,12545,12583,12333,12516,4399,8464
,4416,8542,0,4383,8551,4399,8464,4412,8553,4413
,8552,4414,8550,4415,8549,0,4400,12470,12464,12482
,12519,12520,12521,12525,12526,12501,12577,12398,4377,8309
,4391,8285,4408,8305,4413,8287,0,4378,8326,4387
,8423,12382,12417,4379,8534,4383,8533,0,12464,12400
,12331,12332,12422,12425,12447,12493,4412,8553,4413,8552
,12494,12514,12515,12330,12488,12483,12481,12500,4412,8553
,4413,8552,4414,8613,12486,4412,8553,12499,4399,8464
,4412,8553,4413,8552,0,4399,8464,4412,8553,4413
,8552,0,4397,8386,4412,8553,4413,8552,0,4379
,8618,4388,8408,4412,8553,4413,8552,12480,12476,4397
,8386,4399,8464,0,4416,8542,12353,4388,8408,4397
,8386,12361,12356,4411,8624,12358,4390,8627,12385,12381
,4397,8386,4399,8464,0,12419,12397,12429,12329,12518
,12502,4377,8309,4391,8285,4392,8485,4393,8482,4394
,8483,4395,8484,4398,8393,4408,8305,4412,8488,4413
,8287,4418,8489,4419,8490,4420,8491,0,12527,12352
,12354,4377,8309,4391,8285,4408,8305,4413,8287,0
,4377,8309,4391,8285,4408,8305,4413,8287,0,12565
,4380,8643,12387,4377,8309,4378,8326,4391,8285,4408
,8305,4413,8287,0,12564,12380,12378,4412,8553,4413
,8552,12420,12489,4412,8553,4413,8552,12487,4399,8464
,4412,8553,4413,8552,0,12504,4388,8408,4397,8386
,12480,4412,8553,4413,8552,12513,12357,4397,8386,12360
,12359,4385,8429,12389,12571,12383,12384,12503,4377,8309
,4391,8285,4393,8658,4408,8305,4413,8287,0,4411
,8624,0,12505,4412,8553,4413,8552,12512,12362,12364
,12365,12366,12363,12388,12386,-1};

int yypact[] = {0,1,2,19,20,55,56,57,58,59
,60,61,68,69,70,73,78,79,80,81
,84,89,90,91,92,93,94,95,96,97
,98,99,100,101,102,103,104,105,106,107
,108,109,110,111,112,113,114,115,116,117
,126,126,135,142,155,126,174,126,181,192
,126,201,208,126,117,126,126,217,218,201
,229,234,237,240,253,254,255,256,257,258
,259,262,263,264,265,268,269,270,271,272
,275,280,281,282,285,286,291,292,293,294
,295,296,297,298,299,300,126,303,304,305
,306,307,310,311,314,319,322,329,336,307
,337,126,340,341,342,343,126,346,347,348
,349,350,351,352,192,355,356,357,358,359
,360,361,362,363,366,375,126,376,379,388
,389,390,391,392,393,394,363,300,126,397
,398,399,300,400,126,403,404,405,406,409
,409,409,412,413,424,425,409,409,426,427
,428,429,430,431,432,435,436,437,438,439
,444,445,409,446,447,452,453,454,455,456
,337,463,472,473,474,485,486,487,488,489
,329,490,493,494,495,504,505,534,535,126
,536,549,560,126,563,126,564,565,566,567
,300,126,568,126,569,580,581,582,583,584
,126,337,585,337,337,586,587,588,597,337
,598,599,612,613,337,337,614,614,126,337
,621,622,623,624,629,630,631,634,637,644
,645,650,363,655,656,657,658,663,666,679
,708,737,738,739,750,751,752,753,307,307
,307,780,781,782,783,784,785,786,787,788
,789,790,793,794,795,798,799,800,801,802
,307,803,337,806,809,812,813,814,815,816
,631,817,820,829,830,841,842,853,854,855
,856,857,858,859,862,409,865,866,871,463
,753,872,883,884,885,650,126,842,126,886
,708,887,888,893,906,909,753,753,910,911
,912,913,914,915,753,753,753,753,830,192
,708,916,917,918,927,192,932,586,933,938
,830,939,940,941,942,943,337,944,945,946
,951,952,842,363,953,954,955,956,957,958
,965,968,975,982,989,998,999,1004,1007,1012
,1013,1016,126,1019,1020,1025,753,1026,1027,1028
,126,753,1029,1030,753,631,1031,1058,1059,1060
,1061,1070,126,1079,1080,1083,1094,1095,1096,1097
,1102,1103,1108,1115,1116,1121,1126,1127,363,1130
,1131,830,1134,1135,1136,1137,631,753,126,1138
,337,126,1149,1152,1153,363,1158,1159,1160,1161
,376,830,1138,126,376,1162,1163,830,1149,830
,1164,-1};

int yyr1[] = {0,1,1,2,2,2,2,2,3,6
,6,6,8,9,4,4,4,4,4,4
,4,4,4,4,4,4,4,4,4,4
,5,5,5,5,5,5,5,5,5,10
,36,11,11,46,46,12,13,50,14,14
,54,54,55,55,15,15,15,15,61,61
,63,63,62,62,16,17,18,18,75,75
,77,77,76,76,80,80,81,81,19,83
,84,84,91,91,91,91,91,85,85,85
,86,86,87,87,87,88,88,88,89,89
,90,90,20,21,22,99,23,102,24,106
,25,26,27,110,110,28,29,30,113,113
,114,114,31,32,33,33,117,121,121,122
,123,123,124,125,118,118,129,120,120,130
,119,34,7,7,132,132,132,132,132,132
,135,134,136,136,136,137,138,141,141,142
,142,133,133,143,82,82,82,82,145,145
,145,148,149,149,150,150,126,144,144,144
,146,147,40,53,53,95,95,58,58,108
,38,38,38,71,71,71,71,71,153,153
,43,43,47,37,156,156,157,157,128,128
,128,128,128,128,128,128,160,159,161,161
,161,161,161,161,168,168,158,158,152,151
,155,154,154,154,163,164,165,162,162,167
,166,107,35,41,109,45,45,48,111,68
,49,52,60,66,112,56,127,127,72,57
,96,97,98,115,101,116,104,139,131,65
,59,39,39,39,39,39,93,78,64,51
,51,51,51,94,69,69,69,69,67,70
,70,140,42,92,74,169,100,100,100,100
,44,73,103,103,103,105,79,-1};

int yyr2[] = {0,2,0,2,2,1,1,1,0,1
,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,4
,2,7,6,1,1,5,3,2,2,4
,1,1,1,1,1,2,2,3,1,3
,1,1,1,1,7,6,7,4,1,3
,1,3,2,0,3,5,1,1,10,2
,1,3,1,1,1,1,1,1,1,1
,3,0,2,1,0,2,2,0,8,0
,4,0,2,4,4,2,4,2,4,3
,5,4,5,1,1,3,1,2,1,2
,1,1,4,4,2,2,4,1,0,3
,1,3,4,0,4,1,0,4,1,0
,4,6,3,2,1,1,2,2,1,2
,1,1,2,1,2,2,3,2,0,3
,1,1,3,1,1,2,2,1,2,2
,2,1,2,2,1,1,1,1,2,2
,1,1,1,1,3,1,1,1,3,1
,2,2,0,3,2,3,1,0,3,5
,3,5,1,3,1,3,3,1,1,1
,1,3,3,2,4,6,5,7,1,1
,1,1,1,1,3,1,3,3,1,1
,1,1,1,1,1,1,1,1,1,1
,1,2,1,1,2,1,1,1,2,1
,1,1,1,2,1,1,1,1,1,1
,1,2,1,2,1,2,1,1,2,1
,1,1,1,1,1,0,1,1,1,1
,1,1,0,1,1,1,1,1,1,1
,0,1,1,1,1,1,1,1,1,0
,1,1,1,1,0,1,1,-1};

int yygo[] = {0,-1,1,-1,2,-1,3,-1,21,-1
,22,255,332,256,333,-1,4,-1,5,-1
,8,-1,9,-1,23,-1,24,-1,25,-1
,26,-1,27,-1,28,-1,29,-1,30,-1
,31,-1,32,-1,33,-1,34,-1,35,-1
,36,-1,37,-1,38,-1,39,-1,40,-1
,41,-1,42,-1,43,-1,44,-1,45,-1
,46,-1,47,-1,48,63,168,-1,104,156
,240,161,242,229,311,-1,199,199,264,240
,316,242,317,243,318,248,324,253,330,254
,331,258,335,311,371,385,417,403,424,443
,455,459,468,-1,214,-1,105,49,110,50
,118,52,126,53,138,54,142,56,146,58
,126,59,155,62,167,64,169,65,170,67
,173,105,202,120,217,125,219,133,126,145
,229,157,241,163,245,172,173,200,267,203
,271,213,271,218,301,220,138,222,309,224
,310,230,312,232,313,239,315,246,323,257
,334,325,382,338,267,340,389,345,392,346
,382,347,394,368,126,372,409,374,126,391
,382,411,436,419,439,429,409,430,447,431
,448,434,452,457,464,460,469,472,475,-1
,300,-1,49,104,200,110,203,118,213,156
,200,161,200,162,244,215,278,229,200,278
,278,279,278,287,364,288,365,289,366,309
,368,313,374,349,278,369,278,-1,363,213
,275,-1,269,203,270,265,337,269,344,275
,348,319,375,351,396,401,422,405,427,413
,437,424,442,441,454,455,462,-1,397,270
,346,344,391,-1,326,346,393,391,418,-1
,380,53,139,220,139,-1,381,-1,50,-1
,51,-1,119,-1,120,-1,52,58,154,133
,221,368,405,374,413,-1,124,53,136,-1
,125,-1,302,55,143,-1,127,55,144,-1
,128,-1,303,53,131,219,304,340,390,373
,412,434,453,-1,153,-1,53,-1,132,-1
,133,220,308,-1,135,89,193,132,220,147
,233,266,338,269,345,275,345,303,367,320
,376,336,193,402,423,407,429,443,456,446
,457,-1,218,55,145,183,257,-1,137,-1
,54,155,239,271,347,392,419,447,458,464
,471,-1,222,72,183,-1,55,-1,224,-1
,369,278,351,279,354,349,395,369,406,-1
,276,-1,56,147,236,373,411,469,472,473
,476,-1,230,312,372,449,460,-1,234,-1
,407,-1,428,429,445,-1,408,461,470,477
,478,-1,431,407,430,-1,215,-1,446,471
,474,-1,465,215,277,219,307,250,327,278
,353,279,353,286,353,321,378,323,307,339
,353,349,353,355,353,356,353,363,353,364
,353,365,353,366,353,367,307,369,353,379
,307,415,353,420,353,423,353,425,353,450
,307,456,353,458,467,470,307,471,467,476
,307,478,307,-1,111,-1,57,-1,147,-1
,232,-1,373,-1,410,-1,433,-1,449,-1
,459,233,314,-1,148,-1,235,-1,434,-1
,450,323,379,367,404,379,416,450,461,470
,473,476,477,478,479,-1,305,-1,58,-1
,59,-1,60,68,175,-1,156,-1,157,-1
,61,69,176,-1,161,-1,163,-1,62,-1
,246,-1,322,-1,63,169,250,170,251,175
,253,176,254,191,258,334,385,-1,248,-1
,64,-1,325,-1,65,-1,67,172,252,-1
,171,-1,172,-1,68,-1,69,-1,70,-1
,177,-1,71,-1,180,88,191,-1,162,-1
,243,376,414,-1,319,-1,320,-1,321,321
,377,-1,294,377,415,-1,339,278,352,286
,362,339,388,355,398,356,399,363,400,364
,401,365,402,366,403,415,438,420,440,423
,441,425,444,456,463,-1,282,178,255,-1
,179,181,256,-1,182,-1,88,-1,10,262
,336,-1,89,14,97,198,262,-1,13,13
,96,-1,14,14,98,-1,15,-1,18,19
,102,-1,99,-1,19,-1,100,-1,198,-1
,261,193,259,-1,91,10,92,94,197,115
,197,193,92,262,92,-1,112,115,207,-1
,114,10,94,94,94,193,94,262,94,-1
,115,114,206,116,209,207,206,209,209,-1
,195,-1,116,209,273,-1,208,-1,210,-1
,279,-1,349,-1,280,-1,355,-1,420,338
,387,-1,265,-1,266,200,268,338,268,-1
,295,-1,283,-1,284,-1,285,-1,356,-1
,286,-1,287,-1,288,-1,289,-1,425,-1
,443,-1,340,-1};

int yypgo[] = {0,1,3,5,7,9,11,17,19,21
,23,25,27,29,31,33,35,37,39,41
,43,45,47,49,51,53,55,57,59,61
,63,65,67,69,71,73,75,79,87,115
,117,221,223,259,263,289,295,301,307,309
,311,313,315,317,327,331,333,337,341,343
,355,357,359,361,365,393,399,401,413,417
,419,421,431,433,443,449,451,453,457,463
,467,469,473,535,537,539,541,543,545,547
,549,551,555,557,559,561,577,579,581,583
,587,589,591,595,597,599,601,603,605,619
,621,623,625,627,631,633,635,637,639,641
,643,645,649,651,655,657,659,663,667,697
,701,705,707,709,713,719,723,727,729,733
,735,737,739,741,745,757,761,771,781,783
,787,789,791,793,795,797,799,803,805,811
,813,815,817,819,821,823,825,827,829,831
,-1};

int nterms = 71;
int nnonter = 169;
int nstate = 480;
char *yysterm[] = {
"error",
"APPEND",
"COPY",
"CREATE",
"DEFINE",
"DELETE",
"DESTROY",
"HELP",
"INDEX",
"MODIFY",
"PRINT",
"INTEGRITY",
"RANGE",
"REPLACE",
"RETRIEVE",
"SAVE",
"UNIQUE",
"PERMIT",
"VIEW",
"INGRES",
"EXIT",
"PARAM",
"TYPE",
"ALLOC",
"STRUCT",
"STRUCT_VAR",
"ALL",
"BY",
"FROM",
"IN",
"INTO",
"IS",
"OF",
"ON",
"ONTO",
"TO",
"WHERE",
"UNTIL",
"AT",
"NAME",
"SCONST",
"I2CONST",
"I4CONST",
"F8CONST",
"C_CODE",
"COMMA",
"LPAREN",
"RPAREN",
"PERIOD",
"QUOTE",
"BGNCMNT",
"ENDCMNT",
"LBRACE",
"RBRACE",
"LBRKT",
"RBRKT",
"NONREF",
"SEMICOL",
"POINTER",
"COLON",
"UOP",
"BOP",
"BDOP",
"EOP",
"LBOP",
"LUOP",
"FOP",
"FBOP",
"AOP",
"unaryop",
0 };

char *yysnter[] = {
"$accept",
"program",
"statement",
"startquel",
"quel_statement",
"equel_statement",
"c_code",
"declaration",
"beginblock",
"endblock",
"append",
"copy",
"create",
"delete",
"destroy",
"help",
"index",
"integrity",
"modify",
"permit",
"print",
"range",
"replace",
"retrieve",
"save",
"view",
"append_p",
"copy_p",
"create_p",
"exit",
"ingres",
"replace_p",
"retrieve_p",
"tupret",
"view_p",
"append_key",
"apclause",
"tlclause",
"qualclause",
"apkword",
"id",
"copy_key",
"lparen",
"ctl",
"rparen",
"cp_kword",
"filename",
"sconst",
"create_key",
"delete_key",
"delclause",
"delnoise",
"destroy_key",
"idlist",
"integ_permit",
"int_list_all",
"integ_key",
"permit_key",
"int_list",
"all",
"help_key",
"hlist",
"int_perm_view",
"hparam",
"comma",
"view_key",
"index_key",
"is",
"define_key",
"integnoise",
"isnoise",
"qual",
"modify_key",
"to",
"on",
"modkeylist",
"density",
"modkey",
"colon",
"where",
"modfill",
"mod_var",
"c_variable",
"def_perm",
"permit_list",
"on_of_to",
"perm_tl",
"perm_who",
"perm_term",
"perm_time",
"perm_day",
"permlistelm",
"of",
"at",
"from",
"integer",
"print_key",
"range_of",
"replace_key",
"repclause",
"repkword",
"retrieve_key",
"resclause",
"retkword",
"save_key",
"until",
"date",
"append_p_key",
"param_tl",
"copy_p_key",
"fr_in_id",
"create_p_key",
"ingres_key",
"param_list",
"param",
"replace_p_key",
"retrieve_p_key",
"tupret_keyw",
"xc_code",
"tupret_p",
"o_xc_code",
"unique",
"c_tlclause",
"c_tlist",
"c_tlelm",
"reduc",
"cvar",
"is_key",
"afcn",
"gen_while",
"o_gen_while",
"tupret_p_key",
"decl_specifer",
"declarator_list",
"type_specifier",
"sc_specifier",
"struct_dec",
"struct_name",
"field_declaration",
"struct_key",
"lbrace",
"field_seq",
"field",
"cvar_dec",
"cvarx",
"struct_var",
"ptr",
"arraysub",
"str_var_key",
"selector_part",
"select_op",
"luop",
"lbop",
"clause",
"rop",
"bdop",
"tlist",
"tlelm",
"attrib",
"aggrfcn",
"aggr",
"attribfcn",
"bop",
"uop",
"fop",
"fbop",
"aop",
"by",
"aseq",
"period" };
