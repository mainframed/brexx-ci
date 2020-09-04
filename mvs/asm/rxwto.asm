RXWTO    TITLE 'SEND MESSAGE TO MVS CONSOLE'
* ---------------------------------------------------------------------
*   SEND MESSAGE TO OPERATOR'S CONSOLE, CALLED FROM C (FOR BREXX)
*   AUTHOR  : PETER JACOB (PEJ)
*   CREATED : 03.11.2018  PEJ
*   JCC PROLOGUE : JUERGEN WINKELMANN, ETH ZUERICH.
* ---------------------------------------------------------------------
         PRINT GEN
* --------------------------------------------------------------------
*   RXWTO CODE: SEND MESSAGE TO CONSOLE
* --------------------------------------------------------------------
RXWTO    MRXSTART A2PLIST=YES
         USING WTOPARM,RB    ENABLE ADDRESSIBILTY OF C INPUT AREA
RXWTOGO  L     RA,WMSWKADR   LOAD WORK AREA OF INPUT PARM
         USING WTOCB,RA      ENABLE ADDRESSIBILTY OF C INPUT AREA
* ... PICK UP AND VERIFY LENGTH PARAMETER
         L     RF,WMSLADR    LOAD LENGTH ADDRESS
         L     RF,0(RF)      ... AND REAL LENGTH
         CH    RF,=AL2(80)   MUST NOT EXCEED 120 CHARS
         BNH   LENOK
         LA    RF,80         TOO HIGH, LOAD MAX LENGTH
LENOK    LR    R3,RF         CALCULATE LENGTH OF ENTIRE CB
         LA    R3,4(R3)      ... +2 LENGTH +2 BYTES FILLER
         STH   R3,WTOMSGLN   SAVE ENTIRE LENGTH IN CB
* ... PICK UP MESSAGE AND MOVE IT INTO WTO CB (ACCORDING ITS LENGTH)
         L     R5,WMSGADR    LOAD MESSAGE ADDRESS
         CLC   =C'$LINKMVS:',0(R5)
         BNE   WTOC
         LOAD  EP=RXFUNC,ERRET=WTOC    PRE LOAD RXFUNC
         LR    RF,R0            LOAD ENTRY POINT OF PROGRAM
         LA    RF,0(RF)         LOAD AND CLEAR HIGH ORDER BYTE
         LTR   RF,RF            LOAD AND TEST ENTRY POINT
         BZ    WTOC             ENTRY POINT ADDRESS IS ZERO, EXIT PGM
         BALR  RE,RF            CALL RXFUNC
         B     EXIT
WTOC     L     R5,WMSGADR    LOAD MESSAGE ADDRESS
         EXMVC WTOMSG,0(R5),LEN=0(RF)  AND SAVE IT IN WTO CB
         MVC   WTOFILLR,=AL2(0)  CLEAR NEXT 2 BYTES
         WTO   MF=(E,WTOCB)  SEND MESSAGE TO CONSOLE
* --------------------------------------------------------------------
*   EXIT PROGRAM
* --------------------------------------------------------------------
EXIT     MRXEXIT
         LTORG
* --------------------------------------------------------------------
*    INCOMING STORAGE DEFINITION (FROM C PROGRAM)
* --------------------------------------------------------------------
*
* INPUT PARM DSECT, PROVIDED AS INPUT PARAMETER BY THE C PROGRAM
WTOPARM  DSECT               INPUT PARM DSECT
WMSGADR  DS    A             MESSAGE ADDRESS
WMSLADR  DS    A             MESSAGE LENGHT ADDRESS
WMSRCADR DS    A             ADDRESS OF  RC
WMSWKADR DS    A             ADDRESS OF  WORKAREA ALLOCATED IN CPROGRAM
* WTO CONTROL BLOCK (WILL BE PLACED INTO WTO WORK AREA)
WTOCB    DSECT
WTOMSGLN DS    AL2
WTOFILLR DS    CL2
WTOMSG   DS    CL80
WTOMSEND DS    0H
* --------------------------------------------------------------------
*    REGISTER DEFINITIONS
* --------------------------------------------------------------------
         COPY  MRXREGS
         END   RXWTO
