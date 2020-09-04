RXDYNALC TITLE 'SIMPLE DYNAMIC ALLOCATIONS'
* CHECK IF IN BREXX OR STUB MODE ON MMRSTART
* ---------------------------------------------------------------------
*   DYNALLOC EXISTING DSN  (CALLED FROM BREXX C-ROUTINE)
*   AUTHOR     : PETER JACOB (PEJ)
*   CREATED    : 30.05.2020  PEJ
*   C PROLOGUE : JUERGEN WINKELMANN, ETH ZUERICH.
* ---------------------------------------------------------------------
         PRINT   GEN
         MACRO
&NAME    S99ENTRY &MODE,&TYPE,&LEN=,&LENVAR=,&VALUE=,&VALVAR=,&UNIT=DAL
         AIF   ('&MODE' EQ 'SET').SET
&NAME    DC    AL2(&UNIT&TYPE)
         DC    X'0001'
         AIF   ('&VALUE' EQ '').VALDEF
         DC    AL2(&LEN)
N&SYSNDX DC    &VALUE
         ORG   N&SYSNDX
         DS    CL&LEN
         AGO   .VALDEFD
.VALDEF  ANOP
         DC    AL2(100)
N&SYSNDX DC    CL100' '
.VALDEFD ANOP
         MEXIT
.SET     ANOP
         MVC   &NAME,=AL2(&UNIT&TYPE)
         MVC   &NAME+2(2),=AL2(1)
         AIF   ('&LEN' EQ '').LENVAR
         LA    R1,&LEN
         AGO   .LENSET
.LENVAR  ANOP
         REGOP L,R1,&LENVAR
.LENSET  ANOP
         STH   R1,&NAME+4
.*  NOW SET VALUE
         MVI   &NAME+6,C' '
         MVC   &NAME+7(52),&NAME+6
         AIF   ('&VALUE' EQ '').VALVAR
         EXMVC &NAME+6,=&VALUE,LEN=0(R1)
         AGO   .VALSET
.VALVAR  ANOP
         EXMVC &NAME+6,&VALVAR,LEN=0(R1)
.VALSET  ANOP
         MEND
.* ENTRY LIST
         MACRO
         ENTRYLST &ENTRY
         LCLA  &I
         LCLA  &OFFS
&I       SETA  1
&OFFS    SETA  0
         LA    R5,S99RB+RBLEN        -> PAST 'RB' TO START OF TUP LIST.
.LOOP    ANOP
         LA    R5,S99TUPL+&OFFS      GET ADDRESS OF NEXT TUP LIST ENTRY
         AIF   (&I GT 1).NXT
         ST    R5,S99TXTPP           STORE ADDRESS OF TUP LIST IN THE R
.NXT     ANOP
         LA    R6,&ENTRY(&I)         GET ADDRESS OF FIRST TEXT UNIT
         ST    R6,S99TUPTR           AND STORE IN TUP LIST.
&I       SETA  &I+1
&OFFS    SETA  &OFFS+4
         AIF   ('&ENTRY(&I)' NE '').LOOP
         LA    R6,S99TUPL+&OFFS-4    POINT PAST END OF TUP LIST.
         LA    R5,S99TUPL+&OFFS-8    GET ADDRESS OF NEXT TUP LIST ENTRY
         ST    R6,S99TUPTR           STORE ADDRESS OF TEXT UNIT TUP LST
         OI    S99TUPTR,S99TUPLN    TURN ON HIGH-ORDER BIT IN LAST LST
         MEND
* =====================================================================
* RXDYNALC
* =====================================================================
RXDYNALC MRXSTART A2PLIST=YES,BREXX=YES  START OF PROGRAM
         GETMAIN R,LV=WORKLEN       GET STORAFE FOR WORK AREA
         LR    RA,R1                SAVE GETMAIN ADDRESS
         USING WORKAREA,RA
         USING ALCCOM,RB
* --------------------------------------------------------------------
*   MAIN PROGRAM
* --------------------------------------------------------------------
         USING S99RB,R4      EST. ADDRESSABILITY RB DSECT (R4 IN INIT)
         BAL   RE,RXDINIT
         CLI   ALCFUNC,C'A'  ALLOCATE REQUESTED?
         BNE   TSTNEXT
         CLI   ALCMEM,C' '   ALLOCATE WITH MEMBER REQUESTED?
         BNE   ALMEMBER
         B     ALLOC
TSTNEXT  CLI   ALCFUNC,C'F'  FREE REQUESTED (UN-ALLOCATE)
         BE    FREE
         MVC   CCOD,=A(16)
         B     DYNEND
* --------------------------------------------------------------------
*   FREE ALLOCATION
* --------------------------------------------------------------------
FREE     BAL   RE,FREE99
         BAL   RE,SVC99
         B     DYNEND
* --------------------------------------------------------------------
*   ALLOCATE MEMBER
* --------------------------------------------------------------------
ALMEMBER BAL   RE,ALLOCM
         BAL   RE,SVC99
         B     DYNEND
* --------------------------------------------------------------------
*   ALLOCATE DSN
* --------------------------------------------------------------------
ALLOC    BAL   RE,ALLOCD
         BAL   RE,SVC99
* -------------------------------------------------------------
*   EXIT PROGRAM
* -------------------------------------------------------------
DYNEND   L     RF,CCOD
         LR    R5,RF                SAVE RETURN CODE
         FREEMAIN R,LV=WORKLEN,A=(RA)
         LR    RF,R5                RESTORE RETURN CODE
         MRXEXIT
* ====================================================================
*   PERFORM SVC 99, DYNALLOC MACRO DOES NOT EXPAND ...
* ====================================================================
SVC99    ST    RE,SAVE01
         LA    R1,S99WRK     SAVE THE ADDRESS OF THE RETURNED STORAG
         SVC   99            DIRECT CALL OF SVC99
         ST    RF,CCOD
         LTR   RF,RF
         BZ    SVC99OK
* --------------------------------------------------------------------
*   SVC 99 CHECK FOR SUCCESS                     ...
* --------------------------------------------------------------------
         LH    RF,S99ERROR
         ST    RF,CCOD
SVC99OK  L     RE,SAVE01
         BR    RE
* --------------------------------------------------------------------
*   RELEASE (FREE) FILE
* --------------------------------------------------------------------
FREE99   DS    0H
         ST    RE,SAVE01
         MVI   DYNTYPE,S99VRBUN  SET THE VERB CODE FIELD TO UNALLOC
         BAL   RE,ALCINIT
         USING S99TUPL,R5        ESTABLISH ADDRESSABILITY FOR TEXT UNIT
         USING S99TUNIT,R6       ESTABLISH ADDRESSABILITY TO TEXT UNIT
TUDDNAME S99ENTRY SET,DDNAM,LENVAR=ALCDDNLN,VALVAR=ALCDDN,UNIT=DUN
TUUNALC  S99ENTRY SET,UNALC,LEN=1,VALUE=X'07',UNIT=DUN
* TUUNREMV S99ENTRY SET,REMOV,LEN=1,VALUE=X'08',UNIT=DUN
* --------------------------------------------------------------------
*   CREATE TEXT UNITS FROM ABOVE CBS
* --------------------------------------------------------------------
         ENTRYLST (TUDDNAME,TUUNALC)
         L     RE,SAVE01
         BR    RE
* ====================================================================
*   DYNALLOC PDS FILE WITH MEMBER
* ====================================================================
ALLOCM   DS    0H
         ST    RE,SAVE01
         MVI   DYNTYPE,S99VRBAL SET THE VERB CODE FIELD TO ALLOC
         BAL   RE,ALCINIT
         USING S99TUPL,R5    ESTABLISH ADDRESSABILITY FOR TEXT UNIT
         USING S99TUNIT,R6   ESTABLISH ADDRESSABILITY TO TEXT UNIT
TUDDNAME S99ENTRY SET,DDNAM,LENVAR=ALCDDNLN,VALVAR=ALCDDN
TUDSNAME S99ENTRY SET,DSNAM,LENVAR=ALCDSNLN,VALVAR=ALCDSN
TUDSMEMB S99ENTRY SET,MEMBR,LENVAR=ALCMEMLN,VALVAR=ALCMEM
TUSTATUS S99ENTRY SET,STATS,LEN=1,VALUE=X'08'
* --------------------------------------------------------------------
*   CREATE TEXT UNITS FROM ABOVE CBS
* --------------------------------------------------------------------
         ENTRYLST (TUDDNAME,TUDSNAME,TUDSMEMB,TUSTATUS)
         L     RE,SAVE01
         BR    RE
* ====================================================================
*   DYNALLOC FILE
* ====================================================================
ALLOCD   DS    0H
         ST    RE,SAVE01
         MVI   DYNTYPE,S99VRBAL  SET THE VERB CODE FIELD TO ALLOC
         BAL   RE,ALCINIT
         USING S99TUPL,R5        ESTABLISH ADDRESSABILITY FOR TEXT UNIT
         USING S99TUNIT,R6       ESTABLISH ADDRESSABILITY TO TEXT UNIT
TUDDNAME S99ENTRY SET,DDNAM,LENVAR=ALCDDNLN,VALVAR=ALCDDN
TUDSNAME S99ENTRY SET,DSNAM,LENVAR=ALCDSNLN,VALVAR=ALCDSN
TUSTATUS S99ENTRY SET,STATS,LEN=1,VALUE=X'08'
* --------------------------------------------------------------------
*   CREATE TEXT UNITS FROM ABOVE CBS
* --------------------------------------------------------------------
         ENTRYLST (TUDDNAME,TUDSNAME,TUSTATUS)
         L     RE,SAVE01
         BR    RE
* ====================================================================
*   INIT S99RBP
*   ESTABLISH ADDRESSABILITY OF S99RBP
* ====================================================================
ALCINIT  ST    RE,SAVE02
         LA    R8,S99WRK     LOAD S99WORK
         XC    S99WRK,S99WRK ZERO THE RB
         USING S99RBP,R8     ESTABLISH ADDRESSABILITY FOR S99RBP DSECT.
         LA    R4,S99RBPTR+4 POINT 4 BYTES BEYOND START OF S99R
* --------------------------------------------------------------------
*   ESTABLISH ADDRESSABILITY OF S99RB
* --------------------------------------------------------------------
         ST    R4,S99RBPTR        MAKE 'RBPTR' POINT TO RB.
         OI    S99RBPTR,S99RBPND  TURN ON THE HIGH-ORDER BIT IN RBPTR
         XC    S99RB(RBLEN),S99RB ZERO OUT 'RB' ENTIRELY.
         MVI   S99RBLN,RBLEN      PUT THE LENGTH OF 'RB' IN ITS LENGTH
         MVC   S99VERB,DYNTYPE    SET THE CODE FIELD TO ALLOC/FREE
         L     RE,SAVE02
         BR    RE
* ====================================================================
*  CALCULATE LENGTH OF INPUT PARAMETERS
* ====================================================================
RXDINIT  DS    0H
         ST    RE,SAVE01
         MVC   EYECATCH,=CL12'**LENGTHS**'
         MVC   EYE2,=CL12'**S99RECS**'
         LA    R1,ALCDDN
         LA    R2,8
         BAL   RE,PARMLEN
         ST    RF,ALCDDNLN
*  DDN PARM
         LA    R1,ALCDSN
         LA    R2,44
         BAL   RE,PARMLEN
         ST    RF,ALCDSNLN
*  DSN PARM
         LA    R1,ALCMEM
         LA    R2,8
         BAL   RE,PARMLEN
         ST    RF,ALCMEMLN
         L     RE,SAVE01
         BR    RE
* ====================================================================
*  EXTRACT SUB PARAMETER OF CALLING LIST
* ====================================================================
PARMLEN  DS    0H
         ST    RE,SAVE02
         LA    RF,0
NEXTCHR  CLI   0(R1),C' '
         BE    ENDSUB
         CLI   0(R1),X'0'
         BE    ENDSUB
         LA    RF,1(RF)            INCREASE LENGTH OF VARIABLE
         LA    R1,1(R1)            INCREASE ADDRESS
         BCT   R2,NEXTCHR
ENDSUB   L     RE,SAVE02
         BR    RE
         LTORG
**********************************************************************
*        REXX DYNALC CONTROL BLOCK                                  *
*    INCOMING STORAGE DEFINITION (FROM C PROGRAM)
**********************************************************************
ALCCOM   DSECT                     DYNALLOC IO CONTROL BLOCK
ALCFUNC  DC    CL8'ALLOC'          VSAM FUNCTION TO CALL
ALCDDN   DC    CL8'VSIN'           DD NAME OF VSAM FILE
ALCDSN   DC    CL44' '             DS NAME MAX LENGTH IS 44
ALCMEM   DC    CL8' '              MEMBER NAME MAX LENGTH 8
ALCDISP  DC    CL4'SHR'            DISPOSITION
ALCRCODE DS    A                   ALC RETURN CODE
ALCSPARE DS    CL16                SOME SPARE BYTES
ALCLEN   EQU   *-ALCCOM            LENGTH OF DUMMY SECTION
* --------------------------------------------------------------------
*   WORK AREA
* --------------------------------------------------------------------
         WORKAREA
SAVE01   DS    A                      BALR SAVE REGISTER LEVEL 1
SAVE02   DS    A                      BALR SAVE REGISTER LEVEL 2
CCOD     DS    A
EYECATCH DS    CL12
ALCDDNLN DS    A
ALCDSNLN DS    A
ALCMEMLN DS    A
DYNTYPE  DS    CL1
EYE2     DS    CL12
         DS    0D
TUDDNAME S99ENTRY DEFINE,DDNAM,8,C'RXINCL'
TUDSNAME S99ENTRY DEFINE,DSNAM,44,C'PEJ.TEMP'
TUSTATUS S99ENTRY DEFINE,STATS,1,X'08'
TUUNALC  S99ENTRY DEFINE,UNALC,1,X'07',UNIT=DUN
TUUNREMV S99ENTRY DEFINE,REMOV,1,X'08',UNIT=DUN
TUDSMEMB S99ENTRY DEFINE,MEMBR,8,C'SPROC'
S99EYE   DS    CL9
         DS    0F
S99WRK   DS    CL256
         WORKEND
         IEFZB4D0
         IEFZB4D2
RBLEN    EQU   (S99RBEND-S99RB)
         COPY  MRXREGS
         END   RXDYNALC
