#!/bin/bash

# Dual mode: clean and Upload/Assemble/Link

CLASS="A"


if [ $1 = "clean" ]; then

if [ $# = 2 ]; then
    CLASS=$2
fi

cat << JCLCLEAN
//BRCLEAN1 JOB CLASS=A,MSGCLASS=$CLASS,MSGLEVEL=(1,1),
//         USER=HERC01,PASSWORD=CUL8TR
//*********************************************************************
//* DELETE PRIOR VERSIONS OF SOURCE AND OBJECT DATASETS               *
//*********************************************************************
//*
//BRDELETE EXEC PGM=IDCAMS,REGION=1024K
//SYSPRINT DD  SYSOUT=A
//SYSIN    DD  *
    DELETE BREXX.ASM NONVSAM SCRATCH PURGE
    DELETE BREXX.LINKLIB NONVSAM SCRATCH PURGE
    DELETE BREXX.MACLIB NONVSAM SCRATCH PURGE
    DELETE BREXX.RXMVSEXT NONVSAM SCRATCH PURGE
    DELETE BREXX.OBJ NONVSAM SCRATCH PURGE
 /* IF THERE WAS NO DATASET TO DELETE, RESET CC           */
 IF LASTCC = 8 THEN
   DO
       SET LASTCC = 0
       SET MAXCC = 0
   END
/*
//* ------------------------------------------------------------------
//* Delete sys2.linklib members
//* ------------------------------------------------------------------
//BRDLINKL EXEC PGM=IKJEFT01,REGION=8192K
//SYSTSPRT DD   SYSOUT=*
//SYSTSIN  DD   *
  DELETE 'SYS2.LINKLIB(BREXX)'
  DELETE 'SYS2.LINKLIB(REXX)'
  DELETE 'SYS2.LINKLIB(RX)'
  COMPRESS 'SYS2.LINKLIB'
/*
JCLCLEAN
exit
fi

if [ $# = 4 ]; then
    CLASS=$4
fi

cat <<END_HEREDOC
//BRUPASLN JOB CLASS=A,MSGCLASS=$CLASS,MSGLEVEL=(1,1),
//         USER=HERC01,PASSWORD=CUL8TR
//*********************************************************************
//* DELETE PRIOR VERSIONS OF SOURCE AND OBJECT DATASETS               *
//*********************************************************************
//*
//BRDELETE EXEC PGM=IDCAMS,REGION=1024K
//SYSPRINT DD  SYSOUT=A
//SYSIN    DD  *
    DELETE BREXX.RXMVSEXT NONVSAM SCRATCH PURGE
    DELETE BREXX.OBJ NONVSAM SCRATCH PURGE
 /* IF THERE WAS NO DATASET TO DELETE, RESET CC           */
 IF LASTCC = 8 THEN
   DO
       SET LASTCC = 0
       SET MAXCC = 0
   END
/*

cat << JCLLINKOBJ
//BRLNAUTH EXEC PGM=IEWL,
//         PARM='AC=1,NCAL,MAP,LIST,XREF,NORENT,SIZE=(999424,65536)'
//SYSUT1   DD UNIT=SYSDA,SPACE=(CYL,(5,2))
//SYSPRINT DD SYSOUT=*
//SYSLIN   DD DATA,DLM=\$\$
::E $3
\$\$
//SYSLMOD  DD DSN=SYS2.LINKLIB(BREXX),DISP=SHR
//* -----------------------------------------------------------------
//* !!!!! APF Version
//* Link Aliases separately to avoid interference with BREXX LINK
//*      Use Fake Aliases as there are External names with the Aliases
//* ------------------------------------------------------------------
//LINKAUTH EXEC  PGM=IEWL,
//         PARM='AC=1,NCAL,MAP,LIST,XREF,NORENT,SIZE=(999424,65536)'
//SYSLMOD  DD  DSN=SYS2.LINKLIB,DISP=SHR
//SYSUT1   DD  UNIT=SYSDA,SPACE=(1024,(100,10))
//SYSPRINT DD  SYSOUT=*
//SYSLIN   DD  *
 INCLUDE SYSLMOD(BREXX)
 ALIAS RX1
 ALIAS RX2
 NAME BREXX(R)
/*
//* ------------------------------------------------------------------
//* Rename Fake Aliases into real Aliases
//* ------------------------------------------------------------------
//ALIASES  EXEC PGM=IKJEFT01,REGION=8192K
//SYSTSPRT DD   SYSOUT=*
//SYSTSIN  DD   *
  DELETE 'SYS2.LINKLIB(REXX)'
  DELETE 'SYS2.LINKLIB(RX)'
  RENAME 'SYS2.LINKLIB(RX1)' 'SYS2.LINKLIB(REXX)'
  RENAME 'SYS2.LINKLIB(RX2)' 'SYS2.LINKLIB(RX)'
/*
//
JCLLINKOBJ
