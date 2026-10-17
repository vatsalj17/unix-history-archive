%start		input

%{

#include	"scsi.h"

%}

%token		DISK ID RELEASE HELP SET EJECT INQUIRY READ CAPACITY SENSE
%token		NUMBER EXT SIDEA SIDEB ALL RESET SLEEP START STOP STATUS
%token		ECHO ERROR MEDIA WORM WRITE DIAGNOSTIC STRING COPY

%type <str>	STRING
%type <num>	NUMBER drive side shelf

%union {
	char *str;
	long num;
}

%%

input	:	/* nothing */
	|	input line error '\n'
	|	input line '\n'
	|	input line ';'
	|	input line error ';'
	;

	/*
		help can be done explicitly by :help info::
	*/
line	:		/* nothing */
	|	CAPACITY drive   { s_capacity($2, (long *)0, (long *)0); }
	|	COPY drive NUMBER NUMBER drive NUMBER {/*:COPY sdrive sstart nblocks ddrive dstart:: */
			s_copy($2, $3, $4, $5, $6);
		}
	|	DIAGNOSTIC drive { s_drdiag($2); }
	|	DISK EJECT drive  { s_diskeject($3); }
	|	DISK RELEASE drive shelf side  { s_diskrelease($4, $5, $3, 1); }
	|	DISK RELEASE drive  { s_diskrelease(-1, A, $3, 1); }
	|	DISK SET shelf side drive  { s_diskset($3, $4, $5, 1); }
	|	ECHO NUMBER  { print(">> %d <<\n", $2); }
	|	HELP  { help(); }
	|	ID drive  { scsiid($2); }
	|	INQUIRY {
			int i;
			for(i = 0; i<8; i++)
				s_inquiry(scsi_id, i);
		}
	|	INQUIRY drive  { s_inquiry(scsi_id, $2); }
	|	MEDIA drive NUMBER NUMBER  { s_media($2, $3, $4, 0); }
	|	MEDIA drive NUMBER NUMBER STRING  { s_mediaf($2, $3, $4, $5); } /*:MEDIA drive start nblocks \"filename\"::*/
	|	EXT MEDIA drive NUMBER NUMBER  { s_media($3, $4, $5, 1); }
	|	READ drive NUMBER  {
			struct scsi_o output;
			s_read($2, $3, 1, &output);
			scsiodump(output.data, 1024);
		}
	|	READ ID drive  {
			struct scsi_o output;
			s_read($3, 1, 1, &output);
			print("id='%s'\n", &output.data[42]);
		}
	|	RESET  { s_reset(); }
	|	SENSE drive { s_sense($2, 0); }
	|	EXT SENSE drive { s_sense($3, 1); }
	|	SLEEP NUMBER  { sleep($2); }
	|	START drive { s_start($2); }
	|	STATUS drive { s_status($2, (struct scsi_o *)0); }
	|	STATUS { s_status(0, (struct scsi_o *)0); }
	|	STOP drive  { s_stop($2); }
	|	WORM drive { s_worm($2, 1); }
	|	WORM drive NUMBER { s_worm($2, $3); } /*:WORM drive start:: */
	|	WRITE drive NUMBER { s_write($2, $3, 1); }
	|	WRITE drive NUMBER NUMBER { s_write($2, $3, $4); } /*:WRITE drive start n:: */
	;

drive	:	NUMBER
	;

shelf	:	NUMBER
	;

side	:	SIDEA  { $$ = 0; }
	|	SIDEB   { $$ = 1; }
	;

%%

yywrap()
{
	return(1);
}

yyerror(s1, s2)
	char *s1, *s2;
{
	fprint(2, s1, s2);
	fprint(2, "\n");
}
