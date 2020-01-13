*
***************
*
* ****
* *****         void EXSVC(int svc, struct SVCREGS *regs);
* ****
*               struct SVCREGS
*				{
*					unsigned int R0;
*					unsigned int R1;
*					unsigned int R15;
*				};
*
***************
EXSVC    CSECT ,
@REGION  DS    0H
         STM   14,12,12(13)
         L     2,8(0,13)
         LA    14,92(0,2)
         L     12,0(0,13)
         CL    14,4(0,12)
         BL    @F1-@REGION+4(0,15)
         L     10,0(0,12)
         BALR  11,10
         CNOP  0,4
@F1      DS    0H
         DC    F'92'
         STM   12,14,0(2)
         LR    13,2
         LR    12,15
         USING @REGION,12
*
         LR    11,1              Save Parm List Address
*
*
*
         L     10,0(,11)         SVC Number
		 L     9,4(,11)          A(SVCREGS)
*
         L     0,0(,9)           Reg 0
         L     1,4(,9)           Reg 1
         L     15,8(,9)          Reg 15
*
         EX    10,DOSVC 
*
         ST    15,8(,9)          Update R15
         ST    1,4(,9)           Update R1
         ST    0,0(,9)           Update R0
*
*
         L     13,4(0,13)
         L     14,12(0,13)
         LM    1,12,24(13)
         BR    14
*
DOSVC    SVC   0                 Executed Instruction
*
*
*
         END
