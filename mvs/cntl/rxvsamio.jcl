//PEJASM1 JOB CLASS=A,MSGCLASS=H,REGION=2048K,NOTIFY=&SYSUID
//*
//ASM      EXEC PROC=ASMFCL,
//         MAC='SYS2.MACLIB',
//         MAC1='BREXX.MACLIB',
//         SOUT=H
//ASM.SYSIN    DD DISP=SHR,DSN=BREXX.ASM(RXVSAM)
//LKED.SYSLMOD DD DISP=SHR,DSN=SYS2.LINKLIB(RXVSAM)
