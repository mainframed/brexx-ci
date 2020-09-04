BRXSVC  TITLE 'BREXX EXTERNAL SVC INTERFACE'
         PRINT   GEN
BRXSVC   START 0                  START MAIN CODE CSECT AT BASE 0
*
         SAVE  (14,12)            SAVE INPUT REGISTERS
         LR    R12,R15            BASE REGISTER := ENTRY ADDRESS
         USING BRXSVC,R12         DECLARE BASE REGISTER
         ST    R13,SAVE+4         SET BACK POINTER IN CURRENT SAVE AREA
         LR    R2,R13             REMEMBER CALLERS SAVE AREA
         LA    R13,SAVE           SETUP CURRENT SAVE AREA
         ST    R13,8(R2)          SET FORW POINTER IN CALLERS SAVE AREA
*
         LR    R11,R1             SAVE PARM LIST
*
         L     R10,0(,R11)        SVC NUMBER
         L     R9,4(,R11)         A(SVCREGS)
*
         L     R0,0(,R9)          R0
         L     R1,4(,R9)          R1
         L     R15,8(,R9)         R15
*
         EX    R10,DOSVC
*
         ST    R15,8(,R9)         UPDATE R15
         ST    R1,4(,R9)          UPDATE R1
         ST    R0,0(,R9)          UPDATE R0
*
*
         L     R13,4(0,R13)
         L     R14,12(0,R13)
         LM    R1,12,24(R13)
         BR    R14
*
DOSVC    SVC   0                  Executed Instruction
*
         EJECT
SAVE     DS    18F                LOCAL SAVE AREA
*
* =====================================================================
* OTHER DUMMY SECTIONS
* =====================================================================
         YREGS    ,
         END   BRXSVC
