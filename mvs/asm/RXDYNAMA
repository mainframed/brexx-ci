* --------------------------------------------------------------------
* RXWDYNAMA REXX WRAPPER MODULE FOR PROGRAM DYNAM
* --------------------------------------------------------------------
*   DYNAM IS AN DYNALLOC API FOR PROGRAMMING LANGUAGES
*   DYNAM IS PART OF THE CBT FILE089
*   INSTALLATION: UNIVERSITY OF MANITOBA COMPUTER CENTRE
*   AUTHOR:       GERRY DUECK
*   DATE WRITTEN: SUMMER 1978
*   MODS:         ADDITION OF ALLOCR VERB.
* --------------------------------------------------------------------
         MACRO
         GETVAR &VAR,&FEX=
         LA    R4,&VAR         SET TO OPCODE VAR
         LA    R3,&VAR.LN
         BAL   RE,SUBPARM      FIND PARM, REMAINING LENGTH (R2) CHANGED
         L     R1,PARMCNT      INCREASE PARM COUNT +1
         LA    R1,4(R1)        +1
         ST    R1,PARMCNT
         LTR   RF,RF
         BH    &FEX
         MEND
         MACRO
         SCALL &ENTRY,&PARM
         LCLC  &ADDR
         LCLA  &IX
         LCLC  &LBL
&LBL     SETC  'L&SYSNDX'
&IX      SETA  1
         B     C&SYSNDX
.NXTPARM AIF   ('&PARM(&IX)' EQ '').ALLPARM
         AIF   ('&PARM(&IX)'(1,1) NE '''').ITER
&LBL&IX  DC    C&PARM(&IX)
.ITER    ANOP
&IX      SETA  &IX+1
         AGO   .NXTPARM
.ALLPARM ANOP
.* ----------------------------------------------------
&IX      SETA  1
         DC    C'***PARM***'
P&SYSNDX DS    0A
.NXTPRM2 AIF   ('&PARM(&IX)' EQ '').ENDPARM
         AIF   ('&PARM(&IX)'(1,1) NE '''').VARPARM
         DC    A(&LBL&IX)
         AGO   .ITER2
.VARPARM ANOP
         DC    A(&PARM(&IX))
.ITER2   ANOP
&IX      SETA  &IX+1
         AGO   .NXTPRM2
.ENDPARM ANOP
         ORG   *-4
         DC    X'80'
         ORG   *+3
.* ----------------------------------------------------
C&SYSNDX LA    R1,P&SYSNDX
         AIF   ('&ENTRY'(1,1) EQ '(').ISREG
         AIF   ('&ENTRY'(1,1) EQ '#').ISADDR
         BAL   RE,&ENTRY
         AGO   .BALSET
.ISREG   ANOP
         BALR  RE,&ENTRY(1)
         AGO   .BALSET
.ISADDR  ANOP
&ADDR    SETC  '&ENTRY'(2,7)
         L     RF,&ADDR
         BALR  RE,RF
.BALSET  ANOP
         MEND
         COPY  REGS
         GBLA  &PARMMAX        DEFINE MAXIMUM PARAMETERS TO PICK UP
&PARMMAX SETA  10
DYNALCWR PPROC TITLE='DYNALCWR',                                       X
               WORK=WORKAREA,WORKREG=RB
* ====================================================================
*   MAIN PROGRAM
* ====================================================================
         BAL   RE,FTCHPARM
         MVC   WACOMM,=C'+++COMAREA+++'
         XC    WAREA,WAREA   CLEAR OUT WORKAREA
         IF    (CLC,PRINT(7),EQ,=C'NOPRINT')
           MVI   WAREA,X'42'
         ENDIF
         LA    R1,WAREA      LOAD CALL LIST
         ST    R1,PARMADR
         L     R2,CALLCT
         LA    R2,1(R2)
         ST    R2,CALLCT
         L     RF,=V(DYNAM)
         ST    RF,CALLADR
         IF    (CLC,PRINT(6),EQ,=C'XPRINT')
           DC    A(0)
         ENDIF
CALLIT   BALR  RE,RF         CALL DYNAM
         ST    RF,CCOD
         B     DYNEND
* ===================================================================
*   ERROR RC 8
* ====================================================================
ERROR8   MVI   CCOD+3,X'8'
* ===================================================================
*   END DYNALLOC WRAPPER
* ====================================================================
DYNEND   L     RF,CCOD
         SRETURN RC=(RF)
* ---------------------------------------------------------------------
*  FETCH INPUT PARAMETER OF CALLING LIST AND ITS LENGTH
* ---------------------------------------------------------------------
FTCHPARM DS    0H
         ST    RE,SAVELV2
* ---------------------------------------------------------------------
         LA    R1,0
         LCLA  &CT
&CT      SETA  0
         XC    PARMCNT,PARMCNT
.LOOPA   ANOP
         BLANK P&CT
         MVC   P&CT.HDR(10),=CL10'#***P&CT***#'
         ST    R1,P&CT.LN
         LA    R2,P&CT
         ST    R2,PADR&CT
&CT      SETA  &CT+1
         AIF   ('&CT' LE '&PARMMAX').LOOPA
* ......
         L     R9,JCLPARM      LOAD LENGTH OF INPUT PARMS
         LTR   R9,R9           IS IT PRESENT?
         BZ    ERROR8
FCONT1   LH    R2,0(R9)        LOAD LENGTH OF PARMS
         LTR   R2,R2           IS IT PRESENT?
         BZ    ERROR8
FCONT2   LA    R9,2(R9)        SET TO PARM STRING
         MVI    EODEL,C' '
         GETVAR PRINT,FEX=ENDPARM  FECH PRINT/NOPRINT REQUEST
         XC    PARMCNT,PARMCNT
* ... FETCH VARIABLE NAME
         GETVAR P0,FEX=ENDPARM
         MVI    EODEL,C';'
         GETVAR P1,FEX=ENDPARM
         GETVAR P2,FEX=ENDPARM
         GETVAR P3,FEX=ENDPARM
         GETVAR P4,FEX=ENDPARM
         GETVAR P5,FEX=ENDPARM
         GETVAR P6,FEX=ENDPARM
         GETVAR P7,FEX=ENDPARM
         GETVAR P8,FEX=ENDPARM
         GETVAR P9,FEX=ENDPARM
         GETVAR P10,FEX=ENDPARM
ENDPARM  DS    0H
         L     R1,PARMCNT
         SH    R1,=AL2(4)
         LA    R2,PARMAREA
         LA    R2,0(R1,R2)
         MVI   0(R2),X'80'
         L     RE,SAVELV2
         BR    RE
* ---------------------------------------------------------------------
*  EXTRACT SUB PARAMETER OF CALLING LIST
* ---------------------------------------------------------------------
SUBPARM  DS    0H
         ST    RE,SAVE02
         LA    RE,0
         LA    RF,0                STILL CHARS AVAILABLE
NEXTCHR  CLI   0(R9),C','
         BE    ENDSUB
         CLI   0(R9),C'"'
         BE    DROPCHR
         MVC   0(1,R4),0(R9)
         LA    R4,1(R4)
         LA    RE,1(RE)            INCREASE LENGTH OF VARIABLE
DROPCHR  LA    R9,1(R9)            SET TO NEXT CHAR OF PARM STRING
         BCT   R2,NEXTCHR
         LA    RF,4                NOTHING LEFT
         B     SUBEXIT
ENDSUB   LA    R9,1(R9)            SET TO NEXT CHAR OF CALLING PARMS
         BCTR  R2,0                -1 FOR REMAINING LENGTH
SUBEXIT  MVC   0(1,R4),EODEL       INSERT EOF STRING DELIMETER
         LA    RE,1(RE)            INCREASE LENGTH OF VARIABLE +1
         ST    RE,0(R3)            SAVE PARAMETER LENGTH
         L     RE,SAVE02
         BR    RE
* --------------------------------------------------------------------
*   WORK AREA
* --------------------------------------------------------------------
         LTORG
         DC    C'+++ STATS +++'
CALLCT   DS    A
CALLADR  DS    A
PARMADR  DS    A
         WORKAREA
EODEL    DS    CL1
PARMCNT  DS    F
SAVELV1  DS    F
SAVELV2  DS    F
ENTRY    DS    A
CCOD     DS    A
&CT      SETA  0
.LOOPC   ANOP
P&CT.HDR DS    CL10
P&CT.LN  DS    F
P&CT     DS    CL100
&CT      SETA  &CT+1
         AIF   ('&CT' LE '&PARMMAX').LOOPC
PRINT    DS    CL12
PRINTLN  DS    A
WACOMM   DC    C'+++COMAREA+++'
WAREA    DC    A(0)
PARMAREA DS    0A
&CT      SETA  0
.LOOPB   ANOP
PADR&CT  DS    A
&CT      SETA  &CT+1
         AIF   ('&CT' LE '&PARMMAX').LOOPB
         WORKEND
         END
