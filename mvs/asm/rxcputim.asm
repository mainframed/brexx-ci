RXCPUTIM TITLE 'RETURN TIME IN HUNDREDS OF A SECOND'
* ---------------------------------------------------------------------
*   RETURN USED CPU TIME IN MICRO SECONDS (SINCE JOB BEGIN)
*   AUTHOR  : PETER JACOB (PEJ)
*   CREATED : 01.09.2020  PEJ
*   JCC PROLOGUE : JUERGEN WINKELMANN, ETH ZUERICH.
* ---------------------------------------------------------------------
         PRINT GEN
* --------------------------------------------------------------------
*   RXCPUTIME CODE: RETURN USED CPU TIME SINCE JOB BEGIN
* --------------------------------------------------------------------
RXCPUTIM MRXSTART A2PLIST=YES
         USING STIMPARM,RB   ENABLE ADDRESSIBILTY OF C INPUT AREA
         L     RA,WPTWKADR   LOAD WORK AREA OF INPUT PARM
* --------------------------------------------------------------------
*   EXTRACT CPU TIME
* --------------------------------------------------------------------
         USING PSA,R0
         USING LCCA,R6
         USING ASCB,R7
*
         L     R6,PSALCCAV        get LCCA ptr
         L     R7,PSAAOLD         get ASCB ptr
         LA    R10,9              init retry loop count
*
CPUTIMR  LM    R8,R9,LCCADTOD     get initial LCCADTOD
         STM   R8,R9,SAVDTOD      and save it
*
         STCK  CKBUF              store TOD
         LM    R0,R1,CKBUF
         SLR   R1,R9              low order:  sum=TOD-LCCADTOD
         BC    3,*+4+4            check for borrow
         SL    R0,=F'1'           and correct if needed
         SLR   R0,R8              high order: sum=TOD-LCCADTOD
*
         LM    R8,R9,ASCBEJST     load ASCBEJST
         ALR   R1,R9              low order:  sum+=ASCBEJST
         BC    12,*+4+4           check for carry
         AL    R0,=F'1'           and correct if needed
         ALR   R0,R8              high order: sum+=ASCBEJST
*
         LM    R8,R9,ASCBSRBT     load ASCBSRBT
         ALR   R1,R9              low order:  sum+=ASCBSRBT
         BC    12,*+4+4           check for carry
         AL    R0,=F'1'           and correct if needed
         ALR   R0,R8              high order: sum+=ASCBSRBT
*
         LM    R8,R9,LCCADTOD     get final LCCADTOD
         C     R9,SAVDTOD+4       check low order
         BNE   CPUTIMN            if ne, dispatch detected
         C     R8,SAVDTOD         check high order
         BE    CPUTIMX            if eq, all fine
*
CPUTIMN  BCT   R10,CPUTIMR        retry in case dispatch detected
*
CPUTIMX  STM   R0,R1,SAVSUM       save full sum
         SRDL  R0,12              shift to convert to microsec
         ST    R1,CPUTIML         SAVE TIME IN USEC
         BIN2CHR CPUTIME,CPUTIML
         LA    RA,WPTWKADR
         L     RA,0(RA)
         MVC   0(15,RA),CPUTIME+1
*
* --------------------------------------------------------------------
*   EXIT PROGRAM
* --------------------------------------------------------------------
         LA    RF,0          SET RC=0
EXIT     MRXEXIT
         LTORG
         DC    C'###CPU###'
CPUTIME  DS    CL16          BIN2CHR DESTINATION FIELD
CPUTIML  DS    A             CPU TIME IN BINARY
CKBUF    DS    D
SAVDTOD  DS    D
SAVSUM   DS    D
STRPACK  DS    PL8           BIN2PCK TEMP MAXIMUM 999,999,999,999,999
* --------------------------------------------------------------------
*    INCOMING STORAGE DEFINITION (FROM C PROGRAM)
* --------------------------------------------------------------------
*    INPUT PARM DSECT, PROVIDED AS INPUT PARAMETER BY THE C PROGRAM
STIMPARM DSECT               INPUT PARM DSECT
WPTWKADR DS    A             ADDRESS RESULT RETURNED FROM PGM
* --------------------------------------------------------------------
*    OTHER DEFS
* --------------------------------------------------------------------
         IHAPSA
         IHALCCA
         IHAASCB
* --------------------------------------------------------------------
*    REGISTER DEFINITIONS
* --------------------------------------------------------------------
         COPY  MRXREGS
         END   RXCPUTIM
