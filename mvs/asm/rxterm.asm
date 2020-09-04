RXTERM  TITLE 'TERMINATE AND CLEANUP THE BREXX/370 ENVIRONMENT'
* ---------------------------------------------------------------------
*   TERMINATE AND CLEANUP THE BREXX/370 ENVIRONMENT
*   AUTHOR     : MIKE GROSSMANN (MIG)
*   CREATED    : 10.03.2020  MIG
*   C PROLOGUE : JUERGEN WINKELMANN, ETH ZUERICH.
* ---------------------------------------------------------------------
         PRINT   GEN
* =====================================================================
* RXTERM
*
*     MAIN ENTRY POINT USED BY BREXX/370
*
*     INPUT:
*              R1    PARAMS
*
*     OUTPUT:
*              R15   RETURN CODE
*
*     REGISTER USAGE:
*              R4    ENVIRONMENT AREA
*              R5    USER AREA
*              R12   BASE REGISTER
*
* =====================================================================
RXTERM   MRXSTART A2PLIST=YES  START OF PROGRAM
*
         USING PARAMS,RB
         L     R4,ENVPTR       GET PTR TO ENVIRONMENT AREA
         GETMAIN R,LV=USRLEN   GET STORAFE FOR USER AREA
         LR    R5,R1           SAVE GETMAIN POINTER
         USING ENVCTX,R4
         USING USER,R5
         DROP  RB
*
         CALL  UNALLOC         FREE ALLOCATION MADE BY US
*
         DROP  R5
*
         FREEMAIN R,LV=USRLEN,A=(5)
*
         MRXEXIT
         LTORG
*
         EJECT
* =====================================================================
* UNALLOC
*
*     PERFORM UNALLOCATION OF ALLOCATIONS MADE BY OURSELVES
*
*     INPUT:
*              R4    ENVIRONMENT AREA
*              R5    USER AREA
*
*     OUTPUT:
*
*     REGISTER USAGE:
*              R5    USERREA
*              R12   BASE REGISTER
*
* =====================================================================
         USING ENVCTX,R4
         USING USER,R5
UNALLOC  CSECT ,
*
* ---------------------------------------------------------------------
* ENTRY CODING
* ---------------------------------------------------------------------
         SAVE  (14,12),,UNALLOC SAVE CALLER'S REGISTERS
         BALR  R12,R0          ESTABLISH ADDRESSABILITY
         USING *,R12           SET BASE REGISTER
*
         LA    R11,USRSA1      GET SAVE AREA POINTER
         ST    R13,4(R11)      STORE BACKWARD POINTER
         ST    R11,8(R13)      STORE FORWARD POINTER
*
         LR    R13,R11         GET SAVE AREA ADDRESS
* ---------------------------------------------------------------------
* UNALLOCATE STDIN / STDOUT / STDERR
* ---------------------------------------------------------------------
         IF (TM,EFLAGS2,EF2TSOFG,O) ONLY FOR TSOFG
           IF (TM,EFLAGS3,EF3IN,O)
             CALL DYNAFREE,(=CL8'STDIN'),MF=(E,ULCALL2) FREE STDIN
           ENDIF
 
           IF (TM,EFLAGS3,EF3OUT,O)
             CALL DYNAFREE,(=CL8'STDOUT'),MF=(E,ULCALL2) FREE STDOUT
           ENDIF
 
           IF (TM,EFLAGS3,EF3ERR,O)
             CALL DYNAFREE,(=CL8'STDERR'),MF=(E,ULCALL2) FREE STDERR
           ENDIF
         ENDIF
* ---------------------------------------------------------------------
* EXIT CODING
* ---------------------------------------------------------------------
         L     R13,4(,R13)     PICK UP CALLER'S SAVE AREA
         L     R14,12(,R13)    GET RETURN ADDRESS
         RETURN (0,12)
*
         LTORG
*
         EJECT
* =====================================================================
* DYNAFREE
*
*     PERFORM UNALLOCATION FOR GIVEN DD NAME
*
*     INPUT:
*              R1    PARAMS
*              R5    USERAREA
*
*     OUTPUT:
*
*     REGISTER USAGE:
*              R5    USERREA
*              R12   BASE REGISTER
*
* =====================================================================
         USING USER,R5
DYNAFREE CSECT ,
*
* ---------------------------------------------------------------------
* ENTRY CODING
* ---------------------------------------------------------------------
         SAVE  (14,12),,DYNAFREE SAVE CALLER'S REGISTERS
         BALR  R12,R0          ESTABLISH ADDRESSABILITY
         USING *,R12           SET BASE REGISTER
*
         LA    R11,USRSA2      GET SAVE AREA POINTER
         ST    R13,4(R11)      STORE BACKWARD POINTER
         ST    R11,8(R13)      STORE FORWARD POINTER
*
         LR    R13,R11         GET SAVE AREA ADDRESS ..
* ---------------------------------------------------------------------
* GET DD NAME FROM PARAMETERS
* ---------------------------------------------------------------------
         L     R10,0(,R1)
         LA    R10,0(,R10)
* ---------------------------------------------------------------------
* PREPARE REQUEST BLOCK
* ---------------------------------------------------------------------
         XC    UARBP(UALEN),UARBP CLEAR
*
         LA    R9,UARBP
         USING S99RBP,R9
         LA    R4,S99RBPTR+L'S99RBPTR
         USING S99RB,R4
         ST    R4,S99RBPTR
         OI    S99RBPTR,S99RBPND
         DROP  R9
* ---------------------------------------------------------------------
* BUILD REQUEST BLOCK
* ---------------------------------------------------------------------
         MVI   S99RBLN,S99RBLEN
         MVI   S99VERB,S99VRBAL
         LA    R2,UATUPL
         ST    R2,S99TXTPP
         DROP  R4
*
         USING S99TUPL,R2
* ---------------------------------------------------------------------
* ADD DALDDNAM TEXT UNIT POINTER TO LIST
* ---------------------------------------------------------------------
         LA    R6,UADDNAMU
         ST    R6,S99TUPTR
* ---------------------------------------------------------------------
* MARK LAST ENTRY IN TEXT UNIT POINTER LIST
* ---------------------------------------------------------------------
         OI    S99TUPTR,S99TUPLN
* ---------------------------------------------------------------------
* BUILD TEXT UNITS
* ---------------------------------------------------------------------
         MVC   UADDNAMU(UADDNAML),MADDNAMU
* ---------------------------------------------------------------------
* FILL TEXT UNITS WITH VALUES
* ---------------------------------------------------------------------
         MVC   UADDNAM(L'UADDNAM),0(R10) DDNAME
*
         LA    R1,UARBP
         DYNALLOC
* ---------------------------------------------------------------------
* EXIT CODING
* ---------------------------------------------------------------------
         L     R13,4(,R13)     PICK UP CALLER'S SAVE AREA
         L     R14,12(,R13)    GET RETURN ADDRESS
         RETURN (0,12)
*
         LTORG
*
MADDNAMU DC    AL2(DALDDNAM),X'0001',X'0008'    DDNAME
         EJECT
* =====================================================================
* PARAMETER AREA
* =====================================================================
PARAMS   DSECT
ENVPTR   DS    A
WORKPTR  DS    A
* =====================================================================
* USER AREA DUMMY SECTION
* =====================================================================
USER     DSECT
USREYE   DS    0CL4            THE EYE CATCHER
         DC    CL4'USER'
USRSA1   DS    18F             SAVE AREA DEPTH 1
USRSA2   DS    18F             SAVE AREA DEPTH 2
* --- CALL MACROS
ULCALL1  CALL ,(0,0,0),MF=L    CALL PARAMETER LIST W 3 PARMS DEPTH 1
ULCALL2  CALL ,(0,0,0),MF=L    CALL PARAMETER LIST W 3 PARMS DEPTH 2
* --- DYNALLOC REQUEST BLOCK
UARBP    DS    F,CL20          REQUEST BLOCK POINTER AND REQUEST BLOCK
UATUPL   DS    4A              TEXT UNIT POINTER LIST
* --- TEXT UNITS
UADDNAMU DC    H'1,1,8'        KEY,VALCOUNT,LEN
UADDNAML EQU   *-UADDNAMU
UADDNAM  DS    CL8
UALEN    EQU *-UARBP
USRLEN   EQU *-USER
         EJECT
* =====================================================================
* EVIRONMENT CONTEXT
* =====================================================================
         #ENVCTX                  BREXX ENVIRONMENT CONTEXT
* =====================================================================
* OTHER DUMMY SECTIONS
* =====================================================================
*        CVT      DSECT=YES       COMMON VECTOR TABLE
*        IHAPSA   DSECT=YES       PREFIXED SAVE AREA
*        IHAASCB  DSECT=YES       ADDRESS SPACE CONTOL BLOCK
*        IHAASXB  DSECT=YES       ADDRESS SPACE EXTENSION BLOCK
*        IHAACEE  ,               ACCESSOR ENVIRONMENT ELEMENT
*        IKJEFLWA ,               LOGON WORK AREA
*        IKJECT   ,               ENVIRONMENT CONTROL TABLE
*        IKJPSCB  ,               PROTECTED STEP CONTROL BLOCK
*        IKJTCB   LIST=YES        TASK CONTROL BLOCK
*        IKJUPT   ,               USER PROFILE TABLE
*        IEFTIOT1 ,               TASK INPUT OUTPUT TABLE
         IEFZB4D0 ,               DYNALLOC PARAMETER LIST
         IEFZB4D2 ,               DYNALLOC TEXT UNIT KEYS
* --- MISSING LENGTH EQUATE
S99RBLEN EQU   (S99RBEND-S99RB)
* --- I/O SERVICE ROUTINE WORK AREA
         EJECT
         COPY  MRXREGS
         END   RXTERM
