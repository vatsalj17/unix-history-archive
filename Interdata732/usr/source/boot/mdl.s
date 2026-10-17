         NORX3
         TITLE REGISTER DEFINITIONS
R0       EQU   0
R1       EQU   1
R2       EQU   2
R3       EQU   3
R4       EQU   4
R5       EQU   5
R6       EQU   6
R7       EQU   7
R8       EQU   8
R9       EQU   9
RA       EQU  10
RB       EQU  11
RC       EQU  12
RD       EQU  13
RE       EQU  14
RF       EQU  15
         TITLE MULTI MEDIA DIAGNOSTIC COMMON DISC BOOT LOADER
*
*
*
*   BEFORE THIS BOOT LOADER IS EXECUTED THE DEVICE DEFINITION
*   TABLE MUST BE SET UP AS FOLLOWS.
*
*                     LOCATIONS
*     DEVICE      X'78'    X'7A'    X'7C'
*
*   MAG TAPE&     DDA1     0000     0000
*   CASSETTE
*
*
*     DISC        YYZZ     DDTT     CCSS
*
*
*         WHERE: DD= THE DEVICE WITH THE MAGNETIC MEDIA
*                CC= THE CONTROLLER ADDRESS IF DD IS A DISC
*                TT= DISC TYPE INDICATOR
*                    OO=2.5 MB DISC
*                    01=10 MB  DISC
*                
*                SS= THE DISC SELCH ADDRESS
*                YY= THE DEVICE ADDRESS OF THE DISC BOOT
*                    LOADING DEVICE.
*                ZZ= THE READ COMMAND FOR THE BOOT LOAD DEVICE
*
*                      99= READER PUNCH
*                          94= TTY
*
*
*
*
low	equ	*
	dc	y'8000',disbt	start psw
	org	low+x'60'	'OS' entry point
	lpsw	low		halt at entry
	org	low+x'100'
DISBT    EQU   *
         LB    R1,X'7D'            GET SELCH ADDRESS
         LIS   R4,8                LOAD THE SELCH STOP COMMAND
         OCR   R1,R4               STOP THE SELCH
         LB    R2,X'7A'            GET THE DEVICE ADDRESS
         LB    R3,X'7C'            GET THE CONTROLLER ADDRESS
         LHI   R8,X'C1'            LOAD THE RESTORE COMMAND
         OCR   R2,R8               RESTORE THE FILE
         BAL   RF,FRSRWN           WAIT FOR COMPLETE
         LIS   R7,0                SET THE CYLINDER ADDRESS
         LIS   RD,0                SET THE SECTOR OR HEAD NUMBER
         AIS   R8,1                MAKE THE RESTORE COMMAND A SEEK COMMAND
         BAL   RE,READ             READ THE DIRECTORY
*
         LIS   R5,8                SET POINTER
*
*         LH    RE,X'7E'            GET THE LOADER INDICATOR
*         BZS   LD16                16 BIT BRANCH
         AIS   R5,8                32 BIT-BUMP POINTER
LD16     LH    R7,PDB+4(R5)        LOAD THE CYLINDER NUMBER
         LB    R9,PDB+7(R5)        GET THE HEAD NUMBER
         SLHL  R9,5
         LB    RD,PDB+6(R5)        GET THE SECTOR NUMBER
         OAR   RD,R9
         BAL   RE,READ             READ THE PROGRAM DEFINITION BLOCK
*
         LB    R5,PDB+19
         SLHL  R5,8
         LB    R6,PDB+20
         OAR   R5,R6               R5=THE STARTING ADDRESS
         LH    R6,PDB+22           R6= THE
         CLHI  RD,23               END OF TRACK?
         BNES  CONT1
         LHI   RD,X'1F'            SET HEAD1 SECTOR 0
CONT1    AIS   RD,1                BUMP TO NEXT SECTOR
         BAL   RE,READ1            GO READ THE LOADER
*
*CALCULATE CHKSUM
*
         LDAR  R7,R5
         XAR   RA,RA               ZERO THE CHKSUM ACCUMULATOR
CKLP     LB    RB,0(R7)            
         XAR   RA,RB
         AIS   R7,1
         CLAR  R7,R6
         BLS   CKLP
         BES   CKLP
         LB    R7,PDB+24           GET THE CHKSUM BYTE
         CLAR  R7,RA
         BER   R5                  CHKSUM OK PASS CONTROL TO THE PROGRAM
*
         LHI   R1,X'EE00'
         LIS   RA,1                DISPLAY
         LHI   RB,X'40'
         OCR   RA,RB               THE CHKSUM
         WHR   RA,R1
         XAR   RE,RE               ERROR CODE
         WHR   RA,RE
         LHI   RB,X'80'
         OCR   RA,RB               
HERE     B     HERE
*
*
READ     LA    R5,PDB              SET UP THE SELCH            ***
         LA    R6,PDB+30           ADDRESSES                   ***
READ1    WHR   R2,R7               SET UP THE FILE
         OCR   R2,R8               SEEK
         BAL   RF,FRSRWN
         LIS   RB,1                LOAD THE DISC READ COMMAND
         LHI   RA,X'30'            LOAD THE SELCH READ COMMAND
         WHR   R2,R7               SET UP THE FILE
         WDR   R3,RD               SET UP THE CONTROLLER
         ST    R5,SELCHAD          START ADDRESS               ***
         WD    R1,SELCHAD+1        SET UP THE SELCH            ***
         WH    R1,SELCHAD+2                                    ***
         ST    R6,SELCHAD          END ADDRESS                 ***
         WD    R1,SELCHAD+1        SET UP THE SELCH            ***
         WH    R1,SELCHAD+2                                    ***
*        WHR   R1,R5               SET UP THE SELCH            ***
*        WHR   R1,R6               SET UP THE SELCH            ***
         OCR   R3,RB               START THE CONTROLLER
         OCR   R1,RA               START THE SELCH
         SSR   R1,R0               WAIT FOR SELCH
         BTBS  8,1                 NOT BUSY
         OCR   R1,R4               STOP THE SELCH
         SSR   R3,R0               WAIT FOR CONTROLLER
         BFBS  2,1                 IDLE
         BR    RE
*
   
FRSRWN   EQU   *
         SSR   R3,R0               WAIT FOR CONTROLLER
         BFBS  2,1                 IDLE
         SSR   R2,R0               WAIT FOR FILE
         BTBS  15,1                STATUS = 0
         BR    RF
*
         ALIGN 4
SELCHAD  DC    A(*-*)              FOR SELCH ADDRESS           ***
PDB      DS    32
DISBTE   EQU   *
*
         END   DISBT
