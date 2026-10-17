/*	ansi.c	1.4	83/10/04	*/

/* ******************************************************************
 * ANSI standard tape handler
 * 
 *   Usage:    ansi [cxrt][012][d][v][s rec_siz]  files....
 *   Default:  t1   (lists all files on device /dev/rmt1)
 *   Compile:  cc -O ansi.c -o ansi
 *
 *   Writes the ansi standard two header format.
 *   Assumes 80 character maximum record length when writing tapes.
 *
 *   Warren Gee  
 *   University of California, San Francisco
 *   April, 1981
 *
 *   Revision History
 *	...		Many mods by Laurie Jarvis and Tom Ferrin
 *	5aug83		Changed Extract to allocate larger input buffer
 *			when buffer size exceeded (sizeof Buffer) - Conrad Huang
 * **************************************************************** */
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/stat.h>

#define	NRMT0		"/dev/nrmt0"	/* 800 bpi */
#define	NRMT1		"/dev/nrmt1"	/* 1600 bpi */
#define	RMT0		"/dev/rmt0"
#define	RMT1		"/dev/rmt1"
#define TESTFILE	"ansi.out"

#define	Fill(A, B, C, D)	strncpy(&A[B - 1], C, D)
#define	NULLCHAR	'\0'
#define	BLANK		' '
#define	TRUE		1
#define	FALSE		0
#define	HEAD_SIZE	81
#define	BLOCK		2048
#define REC_SIZ		84	/* max record size assumed when writing
				   including 4 byte label */

#define	READ		0
#define	WRITE		1
#define	READ_WRITE	2

#define	APPEND		0
#define	CREATE		1
#define	EXTRACT		2
#define	TABLE		3

#define	USAGE		"Usage: %s [cxrt][01][dv][s rec_size] [files]\n"
#define	VMUNIX		/* Define this if the gethostname call exists */

char		*Command;
char		*Device;
char		*Nrmt0;
char		*Nrmt1;
char		*Testfile;
char		*Rmt0;
char		*Rmt1;
char		*TapeName;
char		TNBuffer[10];
char		Buffer[4096];
char		File[20];
char		Hdr1[HEAD_SIZE];
char		Hdr2[HEAD_SIZE];
char		OBuffer[BLOCK];
char		OpChar;
char		Temp[BLOCK];
char		Vol[HEAD_SIZE];
extern		errno;
int		filecnt;
int		CharCnt;
int		Chars;
int		Cnt;
int		Debug;
int		Eof;
int		Finish;
int		InFD;
int		InitHead;
int		InitVol;
int		Intrd;
int		Mode;
int		N;
int		OutFD;
int		Pntr;
int		RecSiz;
int		StartCnt;
int		Stop;
int		ThisLen;
int		Verbose;
int		Writing;
long		ByteTol;
long		lseek();

main(Argc, Argv)
int	Argc;
char	**Argv;
{
    int i;
    struct mtop TapeOp;
    int Close();
    int StopAppend();

    RecSiz = REC_SIZ;
    if(GetArgs(Argc, Argv) != Argc) {
	Argc--; Argv++;
    }
    switch (Mode) {
	case APPEND :
	    if (Argc == 2)
		Usage();
	    signal(SIGINT, StopAppend);
	    signal(SIGHUP, StopAppend);
	    InFD = OpenDev(READ);
	    Extract((char **)0, 0);
	    TapeOp.mt_op = MTBSR;
	    TapeOp.mt_count = 2;
	    ioctl(InFD, MTIOCTOP, &TapeOp);
	    close(InFD);
	    InitVol = TRUE;
	/* falls through */
	case CREATE :
	    signal(SIGINT, Close);
	    signal(SIGHUP, Close);
	    if (Argc == 2)
		Usage();
	    OutFD = OpenDev(WRITE);
	    filecnt = 1;
	    for (i = 2; i < Argc; i++) {
		TapeWrite(Argv[i]);
		filecnt++;
	    }
	    close(OutFD);
	    if(Device != Testfile)
	    	Device = (Device == Nrmt0) ? Rmt0 : Rmt1;
	    OutFD = OpenDev(WRITE);
	    close(OutFD);
	    break;
	case EXTRACT :
	case TABLE :
	    Device = (Device == Nrmt0) ? Rmt0 : Rmt1;
	    InFD = OpenDev(READ);
	    Extract(&Argv[2], Argc - 2);
	    break;
	default :
	    if (Debug)
		fprintf(stderr, "Bad Mode in main switch\n");
	    break;
    }
}

/*
 * FixTapeName - make Tape name buffer 6 characters long
 */
FixTapeName(buf)
	char *buf;
{
	int done = 0, i;
	
	for (i=0; i<=6; i++) {
		if (done) {
			buf[i]=' ';
		}
		else if (buf[i]=='\0') {
			done=1;
			buf[i]=' ';
		}
	}
	buf[7]='\0';
}

/*
 * GetArgs - get param line arguments and initialize status flags
 */
GetArgs(Argc, Argv)
int	Argc;
char	**Argv;
{
    char *Chr;
    char *ModeStr;

    Writing = FALSE;
    Rmt0 = RMT0;
    Rmt1 = RMT1;
    Nrmt0 = NRMT0;
    Nrmt1 = NRMT1;
    Testfile = TESTFILE;
    Mode = TABLE;
    ModeStr = "TABLE";
    Verbose = FALSE;
    InitVol = FALSE;
    Debug = FALSE;
    Device = Nrmt1;
    Command = *Argv;
    Eof = FALSE;
    Stop = FALSE;
    Intrd = FALSE;
    if (Argc < 2)
	Usage();
    Argv++;
    Chr = *Argv;
    OpChar = 't';
    while (*Chr != NULLCHAR) {
	switch (*Chr) {
	    case 's' :
		Argv++;
		Argc--;
		sscanf(*Argv,"%d",&RecSiz);
		RecSiz += 4;
		break;
	    case 'c' :
		Mode = CREATE;
		OpChar = *Chr;
#ifdef	VMUNIX
		if (gethostname(TNBuffer, 6) == -1)
		    strcpy(TNBuffer, "UCSF  ");
		else {
		    register char *cp;
		    register i;

		    for (i = 0, cp = TNBuffer; i < 6; i++, cp++) {
			if (*cp == '\0')
			    break;
			if (islower(*cp))
			    *cp = toupper(*cp);
		    }
		    while (i++ < 6)
			*cp++ = ' ';
		    *cp = '\0';
		}
#else	VMUNIX
	        strcpy(TNBuffer, "UCSF  ");
#endif	VMUNIX
		TapeName = TNBuffer;
		ModeStr = "CREATE";
		break;
	    case 'l' :
		Argv++;
		Argc--;
		sscanf(*Argv,"%s",TNBuffer);
		TapeName = TNBuffer;
		FixTapeName(TapeName);
		break;
	    case 'x' :
		Mode = EXTRACT;
		OpChar = *Chr;
		ModeStr = "EXTRACT";
		break;
	    case 't' :
		Mode = TABLE;
		OpChar = *Chr;
		ModeStr = "TABLE";
		break;
	    case 'r' :
		Mode = APPEND;
		OpChar = *Chr;
		ModeStr = "APPEND";
		break;
	    case '1' :
		Device = Nrmt1;
		break;
	    case '0' :
		Device = Nrmt0;
		break;
	   case '2' :
		Device = Testfile;
		break;
	    case 'v' :
		Verbose = TRUE;
		break;
	    case 'd' :
		Debug = TRUE;
		break;
	    default :
		fprintf(stderr, "unknown option '");
		PrintChr(*Chr, stderr);
		fprintf(stderr, "'\n");
		Usage();
		break;
	}
	Chr++;
    }
    if (Debug) {
	fprintf(stdout,"Debug   : set\n");
	if (Verbose)
	    fprintf(stdout,"Verbose : Set\n");
	fprintf(stdout,"Mode    = %s\n", ModeStr);
	fprintf(stdout,"Device  = %s\n", Device);
	fprintf(stdout,"Max record size = %d\n",RecSiz);
    }
    return(Argc);
}

/*
 * PrintChar - print ascii character
 */
PrintChr(Chr, Stream)
char	Chr;
FILE	*Stream;
{
    if (isprint(Chr) || Chr == BLANK)
	fprintf(Stream, "%c", Chr);
    else if (0 <= Chr && Chr < BLANK)
	fprintf(Stream, "^%c", Chr + 'A' - 1);
    else
	fprintf(Stream, "\\0%o", Chr);
}

/*
 * Usage - print usage diagnostic and exit
 */
Usage()
{
    fprintf(stderr, USAGE, Command);
    exit(-1);
}

/*
 * TapeWrite - write a file to tape
 */
TapeWrite(FileName)
char	*FileName;
{
    int i;
    char StrTmp[10];
    int OverFlow;
    int Written;
    int blkcnt;	/* block count */
    struct stat Buf;

    InFD = open(FileName, READ);
    if (InFD == -1) {
	perror(FileName);
	return;
    }
    fstat(InFD, &Buf);
    if (Buf.st_mode & S_IFDIR) {
	fprintf(stderr, "%s: directory\n", FileName);
	return;
    }
    if (Buf.st_mode & S_IEXEC) {
	fprintf(stderr, "%s: executable\n", FileName);
	return;
    }
    if(strlen(FileName) > 17)
	fprintf(stderr,"Warning: Filename %s exceeds 17 characters : truncated\n",FileName);
    sprintf(File, "%-17.17s", FileName);
    ByteTol = 0;
    Finish = FALSE;
    N = 4;		/* current position in output buffer */
    StartCnt = 0;	/* start of current record in output buffer */
    Cnt = 4;		/* byte count relative to StartCnt */
    CharCnt = 0;	/* set by GetChar() */
    Chars = -1;		/* set by GetChar() */
    blkcnt = 0;		/* initialize block count */
    OBuffer[N] = GetChar();
    OverFlow = FALSE;
    Written = FALSE;
    InitHead = FALSE;
    while (!Finish) {
	if (OBuffer[N] == '\n') {
	    sprintf(StrTmp, "%04d", Cnt); /* zero-filled byte count */
	    if(Cnt > RecSiz)
		fprintf(stdout,"Warning: Record exceeds %d characters [file %s]; use -s to increase limit\n",RecSiz - 4,FileName);
	    strncpy(&OBuffer[StartCnt], StrTmp, 4);
	    StartCnt = N;
	    N += 4;
	    Cnt = 4;
	    if (N >= BLOCK) {
		N = 4;
		OverFlow = TRUE;
	    }
	} else {
	    N++;
	    Cnt++;
	    if (N >= BLOCK) {
		N = BLOCK - StartCnt;
		OverFlow = TRUE;
	    }
	}
	if (OverFlow) {
	    if (StartCnt == 0) {
		fprintf(stderr,
		    "Line too long, must be less than %d characters\n",
		    BLOCK);
		Close();
		break;
	    }
	    strncpy(Temp,&OBuffer[StartCnt + 4],N - 4);
	    for (i = StartCnt; i < BLOCK; i++)
		OBuffer[i] = '^';
	    if (!InitHead)
		Headers();
	    Write(OutFD, OBuffer, BLOCK);
	    blkcnt++;
	    Written = TRUE;
	    strncpy(&OBuffer[4], Temp, N - 4);
	    StartCnt = 0;
	    Cnt = N;
	    OverFlow = FALSE;
	}
	OBuffer[N] = GetChar();
    }
    if (N > 4 || !Written) {
	while (StartCnt < BLOCK) {
	    OBuffer[StartCnt] = '^';
	    StartCnt++;
	}
	if (!InitHead)
	    Headers();
	Write(OutFD, OBuffer, BLOCK);
 	blkcnt++;
    }
    close(InFD);
    TapeMark();
    Fill(Hdr1, 1, "EOF", 3);
    sprintf(StrTmp,"%06d",blkcnt);
    Fill(Hdr1,55, StrTmp , 6);
    Write(OutFD, Hdr1, 80);
    Fill(Hdr2, 1, "EOF", 3);
    Fill(Hdr2, 51, "00", 2);
    Write(OutFD, Hdr2, 80);
    Writing = FALSE;
    if (Verbose)
	fprintf(stdout,"%c - %7D bytes \"%s\"\n", OpChar, ByteTol, FileName);
    TapeMark();
    if (Intrd)
	Close(); /* close output device and exit */
}

/*
 * Headers - write ansi file headers to output device
 */
Headers()
{
    char StrTmp[10];

    Writing = TRUE;
    if (!InitVol) {
	blk_fil(Vol, 1, 80);			/*** VOLUMN LABEL #1	***/
	Fill(Vol, 1, "VOL", 3);
	Fill(Vol, 4, "1", 1);
	Fill(Vol, 5, TapeName, 6);
	Fill(Vol, 11, " ", 1);
	blk_fil(Vol, 12, 26);
	Fill(Vol, 38, "D%B          1", 14);
	blk_fil(Vol, 52, 28);
	Fill(Vol, 80, "3", 1);
	Write(OutFD, Vol, 80);
	if (Debug)
	    Write(1, Vol, 80);
	InitVol = TRUE;
    }
    blk_fil(Hdr1, 1, 80);	/*** HEADER LABEL #1	***/
    Fill(Hdr1, 1, "HDR", 3);
    Fill(Hdr1, 4, "1", 1);
    Fill(Hdr1, 5, File, 17);
    Fill(Hdr1, 22, TapeName, 6);
    Fill(Hdr1, 28, "0001", 4);
    sprintf(StrTmp, "%04d", filecnt);
    Fill(Hdr1, 32, StrTmp, 4);
    Fill(Hdr1, 36, "0001", 4);
    Fill(Hdr1, 40, "00", 2);
    Fill(Hdr1, 42, "000000", 6);
    Fill(Hdr1, 48, "000000", 6);
    blk_fil(Hdr1, 54, 1);
    Fill(Hdr1, 55, "000000", 6);
    Fill(Hdr1, 61, "DECUNIX      ", 13);
    blk_fil(Hdr1, 74, 7);
    Write(OutFD, Hdr1, 80);
    if (Debug)
	Write(1, Hdr1, 80);
    blk_fil(Hdr2, 1, 80);	/*** HEADER LABEL #2	***/
    Fill(Hdr2, 1, "HDR", 3);
    Fill(Hdr2, 4, "2", 1);
    Fill(Hdr2, 5, "D", 1);
    sprintf(StrTmp, "%05d", BLOCK);
    Fill(Hdr2, 6, StrTmp, 5);
    sprintf(StrTmp, "%05d", RecSiz);
    Fill(Hdr2, 11, StrTmp, 5);
    blk_fil(Hdr2, 16, 21);
    Fill(Hdr2, 37, " ", 1);
    blk_fil(Hdr2, 38, 13);
    Fill(Hdr2, 51,"00", 2);
    blk_fil(Hdr2, 53, 28);
    Write(OutFD, Hdr2, 80);
    if (Debug)
	Write(1, Hdr2, 80);
    TapeMark();
    InitHead = TRUE;
}

/*
 * GetChar - returns next character from input
 */
GetChar()
{
    CharCnt++;
    if (CharCnt >= Chars) {
	Chars = read(InFD, Buffer, 4096);
	if (Chars == -1) {
	    perror(Device);
	    exit(-1);
	} else if (Chars == 0) {
	    Finish = TRUE;
	    return (-1);
	}
	ByteTol += Chars;
	CharCnt = 0;
    }
    return (Buffer[CharCnt]);
}

/*
 * blk_fil - blank fill str beginning at start for count bytes
 */
blk_fil(str,start,count)
char *str;
{
	char *s;
	if(start <= 0) return;
	s = &str[start - 1];
	for(;count > 0;count--)
		*s++ = BLANK;
}

/*
 * Extract - scans ansi input for FileCnt files specified in **File
 *           if FileCnt == 0, extracts all files
 *           does not write files to disk if mode is APPEND or TABLE
 */
Extract(File, FileCnt)
char	**File;
int	FileCnt;
{
    char FileName[20];
    char RecFrmt;
    char FrmtCntl;
    char *InputBuf;
    int RecLen;
    int BlckSize;
    int ByteCnt;
    long ByteTol;
    int WriteIt;
    int i;
    char *malloc();

    GetLabel("VOL1");
    GetLabel("HDR1");
    while (!Eof) {
	sscanf(&Buffer[4], "%17s", FileName);
	for (i = 16; (i >= 0) && (FileName[i] == ' '); i--)
	    ;
	FileName[i + 1] = NULLCHAR;
	if (Mode == TABLE || Mode == APPEND)
	    WriteIt = FALSE;
	else if (FileCnt == 0)
	    WriteIt = TRUE;
	else {
	    WriteIt = FALSE;
	    for (i = 0; i < FileCnt; i++)
		if (!strcmp(File[i], FileName))
		    WriteIt = TRUE;
	}
	if (WriteIt) {
	    if( (OutFD = creat(FileName, 0666)) == -1) {
		perror(FileName);
		exit(-1);
	    }
	    /* CheckStat(OutFD, FileName); */
	}
	RecFrmt = 'D';
	FrmtCntl = BLANK;
	RecLen = BLOCK;
	BlckSize = BLOCK;
	GetLabel("HDR2");
	sscanf(&Buffer[5], "%5d", &BlckSize);
	sscanf(&Buffer[10], "%5d", &RecLen);
	RecFrmt = Buffer[4];
	if (RecFrmt == 'S' || RecFrmt == 'U') {
	    fprintf(stderr, "Format '%c' is not supported\n", RecFrmt);
	    exit(-1);
	}
	FrmtCntl = Buffer[36];
	GetEof();
	if (BlckSize <= sizeof Buffer)
	    InputBuf = Buffer;
	else {
	    InputBuf = malloc(BlckSize);
	    if (InputBuf == NULL) {
		fprintf(stderr, "Not enough memory for buffering %d bytes\n",
		    BlckSize);
		exit(1);
	    }
	}

	ThisLen = RecLen;
	ByteTol = 0;
	while ((ByteCnt = read(InFD, InputBuf, BlckSize)) > 0) {
	    if(ByteCnt != BlckSize)
		fprintf(stderr,"Unexpected data record: asked for %d, got %d\n",
			BlckSize,ByteCnt);
	    ByteTol += ByteCnt;
	    Pntr = 0;
	    if (ThisLen != RecLen) {
		Pntr = RecLen - ThisLen;
		if (WriteIt)
		    Write(OutFD, InputBuf, Pntr);
		Pntr += (RecLen % 2);
		if (FrmtCntl != 'M') {
		    ByteTol -= 3;
		    if (WriteIt)
			Write(OutFD, "\n", 1);
		}
	    }
	    while (Pntr < ByteCnt) {
		if (RecFrmt == 'D') {
		    if (InputBuf[Pntr] == '^') {
			Pntr++;
			ByteTol--;
			continue;
		    }
		    sscanf(&InputBuf[Pntr], "%4d", &RecLen);
		    RecLen -= 4;
		    Pntr += 4;
		    if (RecLen == -1 || RecLen == -257)
			break;
		    if (RecLen > 2 * BlckSize || RecLen < 0) {
			fprintf(stdout,"Bad variable record length %d\n",RecLen);
			fprintf(stdout,"Switching over to fixed length\n");
			RecFrmt = 'F';
			RecLen = BlckSize;
			FrmtCntl = 'M';
			Pntr -= 4;
		    }
		}
		ThisLen = (Pntr + RecLen > BlckSize) ? BlckSize - Pntr : RecLen;
		if (WriteIt)
		    Write(OutFD, &InputBuf[Pntr], ThisLen);
		if (FrmtCntl != 'M' && ThisLen == RecLen) {
		    ByteTol -= 3;
		    if (WriteIt)
			Write(OutFD, "\n", 1);
		}
		Pntr += ThisLen;
	    }
	}
	if(ByteTol == 0 && (Verbose || Debug) )
		fprintf(stderr,"Null file found: %s\n",FileName);
	close(OutFD);
	if ((WriteIt && (Verbose || Debug))
	|| (Verbose && Mode == TABLE && FileCnt == 0))
	    fprintf(stdout,"%c - %7D bytes \"%s\"\n", OpChar, ByteTol, FileName);
	else if (Mode == TABLE && FileCnt == 0 && !Verbose)
	    fprintf(stdout,"\"%s\"\n", FileName);
	else if (Mode == TABLE && FileCnt != 0)
	    for (i = 0; i < FileCnt; i++)
		if (!strcmp(File[i], FileName))
		    if (Verbose)
			fprintf(stdout,"%c - %7D bytes \"%s\"\n",
			    OpChar, ByteTol, FileName);
		    else
			fprintf(stdout,"\"%s\"\n", FileName);
	GetLabel("EOF1");
	GetLabel("EOF2");
	if (Stop) {
	    close(InFD);
	    Device = (Device == Nrmt0) ? Rmt0 : Rmt1;
	    InFD = OpenDev(READ);
	    close(InFD);
	    exit(0);
	}
	GetEof();
	GetLabel("HDR1");
	if (InputBuf != Buffer)
	    free(InputBuf);
    }
}

/*
 * GetLabel - read next record from input; check for Key record type
 */
GetLabel(Key)
char	*Key;
{
    int ByteCnt;

    ByteCnt = read(InFD, Buffer, 80);
    if ((ByteCnt == 0) && !strncmp(Key, "HDR1", 4)) {
	if ((Debug || Verbose) && OpChar != 'r')
	    fprintf(stdout,"End of Tape\n");
	Eof = TRUE;
    } else if (ByteCnt != 80 || strncmp(Buffer, Key, 4)) {
	fprintf(stderr, "Unsuccessful attempt to find %s record\n", Key);
	exit(-1);
    }
}

/*
 * Write - write BCount bytes of Str to file descriptor FD
 */
Write(FD, Str, BCount)
int	FD;
char	*Str;
int	BCount;
{
    if (write(FD, Str, BCount) != BCount) {
	perror(Device);
	fprintf(stderr, "Unable to write to %s\n", Device);
    }
}

/*
 * Close - close output device and exit
 */
Close()
{
    signal(SIGINT, SIG_IGN);
    Intrd = TRUE;
    if (Writing)
	return;
    close(OutFD);
    if(Device != Testfile) { /* if not a testfile, rewind */
	Device = (Device == Nrmt0) ? Rmt0 : Rmt1;
    	OutFD = OpenDev(WRITE);
    	close(OutFD);
    }
    exit(-1);
}

/*
 * StopAppend - ignore signals
 */
StopAppend()
{
    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    Stop = TRUE;
}

OpenDev(OpenMode)
int OpenMode;
{
    register FD;
    register Count;

    FD = open(Device, OpenMode);
    for (Count = 0; FD == -1 && Count < 3; Count++) {
	if(Device == Testfile) 	/* if device is a file  */
	   FD = creat(Device, 0666);
	else
	   FD = open(Device, OpenMode);
    }
    if (FD == -1) {
	if (errno == EIO)
	    fprintf(stderr, "%s offline or needs write ring\n", Device);
	else if (errno == ENXIO)
	    fprintf(stderr, "%s already in use\n", Device);
	else
	    perror(Device);
	exit(-1);
    }
    return (FD);
}

TapeMark()
{
	static int mtdevice = 1;
	static struct mtop mtop = {MTWEOF, 1L};
	struct mtget mtget;

	if (mtdevice == 1)
		mtdevice = ioctl(OutFD, MTIOCGET, &mtget);
	if (mtdevice == 0) {
		if (ioctl(OutFD, MTIOCTOP, &mtop) < 0) {
			fprintf(stderr, "ansi: error writing EOF\n");
			exit(1);
		}
	}
}

GetEof()
{
	static int mtdevice = 1;
	static struct mtop mtop = {MTFSF, 1L};
	struct mtget mtget;

	/*
	while (read(InFD, Buffer, 80) != 0)
	    ;
	*/
	if (mtdevice == 1)
		mtdevice = ioctl(InFD, MTIOCGET, &mtget);
	if (mtdevice == 0) {
		if (ioctl(InFD, MTIOCTOP, &mtop) < 0) {
			fprintf(stderr, "ansi: error positioning tape\n");
			exit(1);
		}
	}
	else
		fprintf(stderr,"ansi: GetEOF ioctl fails\n");
}
