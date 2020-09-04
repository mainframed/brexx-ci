RXWAIT   TITLE 'WRAP CALL TO IKJ441 FROM C'
         PRINT GEN
* ---------------------------------------------------------------------
*   WAIT FOR HUNDREDS OF A SECOND, CALLED FROM C (FOR BREXX)
*   AUTHOR  : PETER JACOB (PEJ)
*   CREATED : 02.11.2018  PEJ
*   JCC PROLOGUE : JUERGEN WINKELMANN, ETH ZUERICH.
* ---------------------------------------------------------------------
RXWAIT   MRXSTART A2PLIST=YES
* --------------------------------------------------------------------
*   RXWAIT CODE: WAIT IN HUNDREDS OF A SECOND
* --------------------------------------------------------------------
         USING WTCB,RB       ENABLE ADDRESSIBILTY OF C INPUT AREA
         L     RA,WTRWKADR   LOAD ADDRESS OF WORKAREA IN INPUT AREA
         USING WAITWRK,RA    ENABLE ADDRESSIBILTY OF WORKAREA
         L     R1,WTSECADR   LOAD ADDRESS OF WAIT
         MVC   WAITS,0(R1)   LOAD AMOUNT OF HH SECONDS
         STIMER WAIT,BINTVL=WAITS
*        L     R1,WTRRCADR   LOAD ADDRESS OF RETURN CODE
*        MVC   0(4,R1),=A(0) LOAD ADDRESS OF WAIT
* --------------------------------------------------------------------
*   EXIT PROGRAM
* --------------------------------------------------------------------
EXIT     MRXEXIT
         LTORG
* --------------------------------------------------------------------
*    INCOMING STORAGE DEFINITION (FROM C PROGRAM)
* --------------------------------------------------------------------
*
WTCB     DSECT               CB ADDRESSING THE C PROGRAM WORK AREA
WTSECADR DS    A             WAIT IN HUNDREDS OF A SECOND
WTRRCADR DS    A             WAIT RETURN CODE
WTRWKADR DS    A             ADDRESS OF  WORKAREA ALLOCATED IN CPROGRAM
WTCBEND  DS    0A
*
WAITWRK  DSECT               WORKAREA BY C PROGRAM (ADR IN WTRWKADR)
WAITS    DS    A
EOWORK   EQU   WAITWRK+255   WORKAREA CONSISTS OF 255 BYTES
* --------------------------------------------------------------------
*    REGISTER DEFINITIONS
* --------------------------------------------------------------------
*
         COPY  MRXREGS
         END   RXWAIT
