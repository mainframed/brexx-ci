/* REXX */
/* $INTERNAL Will not delivered in BREXX.$RELEASE.RXLIB */
parse upper arg type
  buffer.0=0
  if abbrev('USER',type,3)=1 then call quser
  else if abbrev('IPL',type,3)=1  then call qipl
  else if abbrev('JOB',type,3)=1  then call qjob
  else if abbrev('DATE',type,2)=1  then call qdate
  else if abbrev('BUFFER',type,2)=1  then call qbuffer
  else if abbrev('SYSTEM',type,2)=1  then call qsystem
  else if abbrev('STORAGE',type,3)=1  then call qStorage
  else do
     call quser
     call qbuffer
     call qdate
     call qjob
     call qipl
     call qsystem
  end
  if _screen.FMTLIST=1 then return 0
  do i=1 to buffer.0
     say buffer.i
  end
return 4
/* ---------------------------------------------------------------------
 * Active Users   (created by Moshix)
 * ---------------------------------------------------------------------
 */
quser:
  usr=0
  CALL PUSHBUFFER  'Currently Active Users'
  CALL PUSHBUFFER  '----------------------'
  ASVT=qPTR(CVT()+556)+512            /* GET ASVT                     */
  ASVTMAXU=qPTR(ASVT+4)               /* GET MAX ASVT ENTRIES         */
  DO IX=0 TO ASVTMAXU - 1
     ASCB=qSTG(ASVT+16+IX*4,4)        /* GET PTR TO ASCB (SKIP Master)*/
     IF BITAND(ASCB,'80000000'X)='00000000'X THEN do /* IF IN USE     */
        ASCB=C2D(ASCB)                  /* GET ASCB ADDRESS           */
        CSCB=qPTR(ASCB+56)              /* GET CSCB ADDRESS           */
        CHTRKID=qSTG(CSCB+28,1)         /* CHECK ADDR SPACE TYPE      */
        IF CHTRKID='01'X THEN do        /* IF TSO USER                */
           ASCBJBNS=qPTR(ASCB+176)      /* GET ASCBJBNS               */
           ASCBSRBT=qPTR(ASCB+200)      /* GET ASCBEATT               */
           usr=usr+1
           as=right(ASCBSRBT,2,'0')
           CALL PUSHBUFFER ' 'RIGHT(usr,2,'0')' 'AS' 'qSTG(ASCBJBNS,8)
        end
     end
  end
  CALL PUSHBUFFER usr' User(s) logged on'
  CALL PUSHBUFFER ' '
return
/* ---------------------------------------------------------------------
 * Active Users   (created by Moshix)
 * ---------------------------------------------------------------------
 */
qipl:
  CALL PUSHBUFFER  'IPL Monitor'
  CALL PUSHBUFFER  '-----------'
  tickfactor=1.024                /* RMCT time ticks a bit slower     */
  rmcttod=QPTR(rmct()+124)        /* pick start time                  */
 $iplsec=RMCTTOD%1000*tickfactor /* Convert into secs and mult. factor*/
  $ipldays=$iplsec%86400          /* days MVS is running              */
  $iplrem=$iplsec//86400%1        /* remaining seconds                */
  days1900=Rxdate('b')-$ipldays   /* calculate days since 1.1.1900    */
  $iplsec=time('s')-$iplrem
  do while $iplsec<0
     $iplsec=$iplsec+86400
     days1900=days1900-1
  end
  $ipldate=Rxdate(,days1900,'B')  /* convert it back normal date      */
  $iplwday=Rxdate('WEEKDAY',days1900,'B')   /* convert it normal date */
  $iplsec=sec2time($iplsec)
  $iplrem=sec2time($iplrem)
  $Time=time('l')
  CALL PUSHBUFFER ' Current Time '$TIME
  CALL PUSHBUFFER ' IPL on  '$iplwday $ipldate' at '$iplsec
  CALL PUSHBUFFER ' MVS up  for '$ipldays' days '$iplrem' hours'
  CALL PUSHBUFFER ' '
return
/* ---------------------------------------------------------------------
 * Return Job Info
 * ---------------------------------------------------------------------
 */
qJob:
  call Qjobnfo
  CALL PUSHBUFFER 'JOB Information'
  CALL PUSHBUFFER '---------------'
  CALL PUSHBUFFER ' Job Number 'jobNumber
  CALL PUSHBUFFER ' Job        'jobName
  CALL PUSHBUFFER ' Step       'stepName
  CALL PUSHBUFFER ' Program    'programname
  CALL PUSHBUFFER ' '
return
/* ---------------------------------------------------------------------
 * Return System Info
 * ---------------------------------------------------------------------
 */
qSystem:
  CALL PUSHBUFFER 'System Information'
  CALL PUSHBUFFER '------------------'
  CALL PUSHBUFFER ' CPU Model 'd2x(c2d(storage(d2x(cvt()-6),2)))
  CALL PUSHBUFFER ' '
return
/* ---------------------------------------------------------------------
 * Return Storage Fragmentation
 * ---------------------------------------------------------------------
 */
qStorage:
 Address TSO
  "RXSTOR"
  ct=value('IRXVAR_000000')
  call PushBuffer 'Storage Fragmentation     '
  call PushBuffer '--------------------------'
  do i=1 to ct
     call PushBuffer value('IRXVAR_'right(i,6,'0'))
  end
  CALL PUSHBUFFER ' '
return
/* ---------------------------------------------------------------------
 * Query Buffer
 * ---------------------------------------------------------------------
 */
qBuffer:
  call pushBuffer 'Active Buffers'
  call pushBuffer copies('-',72)
  call pushBuffer 'Buffer Line  Active       Command    Token   Current Output Line'
  call pushBuffer 'Number Count   Line'
  bufmax=#$_BUF.$stack
  do bufno=1 to bufmax
     applid=right(word(#$_BUF.bufno.$APPLID,2),7)
     if symbol('#$_BUF.bufno.$command')='VAR' then ,
        _crexx=left(#$_BUF.bufno.$COMMAND,10)' '
        else _crexx=copies(' ',11)
     parse var #$_BUF.bufno.0 blinc'/'blino
     bline='   'right(bufno,3,'0')right(blinc,6,)right(blino,7,)
     if symbol('#$_BUF.bufno.1')='VAR' & blinc>0 then ,
        cline=substr(#$_BUF.bufno.blino,1,40)
     else cline='.....'
     if bufno=_#bno then call pushBuffer bline' *Act* '_crexx''applid' 'cline
     else call pushBuffer bline''copies(' ',7)_crexx''applid' 'cline
  end
  call pushBuffer bufmax' Buffers are active'
  CALL PUSHBUFFER ' '
return
/* ---------------------------------------------------------------------
 * Query Date
 * ---------------------------------------------------------------------
 */
qdate:
  CALL PUSHBUFFER 'Date Information'
  CALL PUSHBUFFER '------------------'
  CALL PUSHBUFFER 'Date            'rxdate()
  CALL PUSHBUFFER 'Time            'Time()
  call PUSHBUFFER rxdate('base')'          days since 01.01.0001 '
 CALL PUSHBUFFER RXDATE('JDN')'         days since 24. November 4714 BC'
  CALL PUSHBUFFER RXDATE('Julian')'         Julian Date'
  CALL PUSHBUFFER RXDATE('Days')'             days in this year'
  CALL PUSHBUFFER left(RXDATE('Weekday'),16)'weekday'
  CALL PUSHBUFFER RXDATE('Century')'            dddd days in is century'
  CALL PUSHBUFFER RXDATE('European')'      European format'
  CALL PUSHBUFFER RXDATE('German')'      German format'
  CALL PUSHBUFFER RXDATE('USA')'      US format'
  CALL PUSHBUFFER RXDATE('SHEurope')'        short European format'
  CALL PUSHBUFFER RXDATE('SHGerman')'        short German format'
  CALL PUSHBUFFER RXDATE('SHUSA')'        short US format'
  CALL PUSHBUFFER RXDATE('STANDARD')'        standard format'
  CALL PUSHBUFFER RXDATE('ORDERED')'      ordered format'
  CALL PUSHBUFFER RXDATE('SHORT')'     short format'
  CALL PUSHBUFFER RXDATE('LONG')' long format'
  CALL PUSHBUFFER ' '
return
/* ---------------------------------------------------------------------
 * Push Content into FMTLIST Buffer
 * ---------------------------------------------------------------------
 */
PushBuffer:
  if datatype(buffer.0)<>'NUM' then buffer.0=0
PushBuffer:
  ixl=buffer.0+1
  buffer.ixl=arg(1)
  buffer.0=ixl
return
/* ---------------------------------------------------------------------
 * Returns Information of the running JOB
 * ............................... Created by PeterJ on 04. January 2019
 * ---------------------------------------------------------------------
 */
QJobnfo:
  jobname=strip(STORAGE(d2x(__TIOT()),8))   /* Jobname in TIOT + 0 */
/* ....... Fetch Job Number via JCB ........*/
  _ssib=QPTR(__JSCB()+316)           /* ssib address at JSCB + 316 */
  _jobn=storage(d2x(_ssib+12),8)     /* job number at ssib + 12    */
  _jobn=translate(_jobn,'0',' ')     /* fix leading zeros for tso  */
  jobNumber=strip(_jobn)
/* ....... Fetch Step Name from TIOT via TCB .......*/
  _proc=strip(STORAGE(d2x(__TIOT()+8),8))  /* STEP/PROCNAME          */
  _step=strip(STORAGE(d2x(__TIOT()+16),8)) /* STEP Name(if PROCNAME )*/
  if _step='' then stepname=_proc
     else stepname=_proc'.'_step
/* ....... Fetch Program Name from the EXEC Statement ..... */
  programName=STORAGE(d2x(__JSCB()+360),8)
return 0
/* ---------------------------------------------------------------------
 * Address some MVS Control Blocks
 * ---------------------------------------------------------------------
 */
qSTG: RETURN STORAGE(D2X(ARG(1)),ARG(2))     /* RETURN STORAGE       */
cvt:  return QPTR(16)
rmct: return QPTR(cvt()+604)
qptr: return c2d(storage(d2x(arg(1)),4))  /* return pointer (decimal) */
__TCB: return QPTR(540)         /* TCB address at 540 = '21C'X of PSA*/
__TIOT: return QPTR(__TCB()+12) /* __TIOT address at TCB+12 */
__JSCB: return QPTR(__TCB()+180)
