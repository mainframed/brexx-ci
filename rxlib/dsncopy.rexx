/* ---------------------------------------------------------------------
 * $INTERNAL Will not delivered in BREXX.INSTALL.RXLIB
 * ---------------------------------------------------------------------
 */
DSNcopy: procedure expose _msglimit
  parse upper arg dsnfrom,dsnto,replace
  call _msg '','COPY 'dsnfrom' TO 'dsnto
  if abbrev('REPLACE',replace,1)=1 then replace=1
     else replace=0
  if init()       >0 then signal cleanup
  if testsc()     >0 then signal cleanup
  if replaceopt() >0 then signal cleanup
  if alloctarget()<>0 then do
     call _MSG(8,dsnto" cannot be allocated")
     signal cleanup
  end
  if recordmode=0 then rc=BlockCopy()
     else rc=RecordCopy()
/* ..... Cleanup and return ..... */
cleanup:
  if f1>0 then call close(f1)
  if f2>0 then call close(f2)
  if maxrc<8 then call _msg('','COPY completed in 'round(time('e'),3)' seconds')
     else call _msg('','COPY failed in 'round(time('e'),3)' seconds')
  if _msglimit>=0 then say ' '
return maxrc
/* ---------------------------------------------------------------------
 * Copy Source File to Target File by BLKsize
 * ---------------------------------------------------------------------
 */
BlockCopy:
  flen=0
  if f1 =0 then f1=open(dsnfrom,'RB')
  if f1<=0 then return _msg(8,'unable to open source file 'dsnfrom)
  f2=open(dsnto,'WB')
  if f2<=0 then return _msg(8,'unable to open target file 'dsnto)
  call _msg(0,'Copying in Block Mode, block size 'blksize1' bytes')
  do until eof(f1)
     buffer=read(f1,blksize1)
     if length(buffer)=0 then leave
     flen=flen+write(f2,buffer)
  end
return _MSG(0,flen' Bytes copied from 'dsnfrom' to 'dsnto)
/* ---------------------------------------------------------------------
 * Copy Source File to Target File by Records
 * ---------------------------------------------------------------------
 */
RecordCopy:
  flen=0
  reci=0
  if f1>0 then call close(f1)  /* close file to re-open it in RT mode */
  f1=open(dsnfrom,'RT')
  if f1<=0 then return _msg(8,'unable to open source file 'dsnfrom)
  f2=open(dsnto,'WT')
  if f2<=0 then return _msg(8,'unable to open target file 'dsnto)
  call _msg(0,'Copying in Record Mode')
  if recfm2='V' | recfm2='VB' then lrecl2=lrecl2-4
  if recordMode=1 then do  /* records must be truncated */
     do until eof(f1)
        buffer=read(f1)
        if length(buffer)=0 then leave
        reci=reci+1
        flen=flen+write(f2,left(buffer,lrecl2),'NL')
     end
  end
  else do    /* recordMode=2, records fit in target format */
     do until eof(f1)
        buffer=read(f1)
        if length(buffer)=0 then leave
        reci=reci+1
        flen=flen+write(f2,buffer,'NL')
     end
  end
  call _MSG(0, reci' Records copied from 'dsnfrom' to 'dsnto)
return _MSG(0, flen' Bytes copied')
/* ---------------------------------------------------------------------
 * Calculate Target File size
 * ---------------------------------------------------------------------
 */
alloctarget:
  if pdsn2=1 then return 0  /* no allocation if target is PDS */
  blockT=(19000/sysblksize)%1    /* blocks per track */
  blocks=(1+size1/sysblksize)
  trk=(blocks/blockT)%1
  pri=(1+trk*1.5)%1
  sec=(1+trk/2)%1
  dcb1='recfm='SYSRECFM',lrecl='SYSLRECL',blksize='SYSBLKSIZE
  dcb2='unit=sysda,pri='pri',sec='sec
  rc=create(dsnto,dcb1','dcb2)
  if rc<>0 then return _MSG(8,dsnto' cannot be allocated, rc='rc)
  call _MSG(0,dsnto' has been allocated')
  call _MSG(0,'     DCB   'dcb1)
return _MSG(0,'     SPACE 'dcb2)
/* ---------------------------------------------------------------------
 * Test Copy Scenarios
 * ---------------------------------------------------------------------
 */
testsc:
  if pdsn1=1 & dsorg1 <> 'PO' then  ,
    return _MSG(8,dsnfrom' is not a partitioned, member notation invalid')
  if pdsn2=1 & dsorg2 <> 'PO' & cat2=1 then  ,
    return _MSG(8,dsnto' is not a partitioned, member notation invalid')
  if pdsn2=1 & cat2=0 then  ,
    return _MSG(8,dsnto' partitioned DSN not catalogued')
  if pdsn1=0 & dsorg1 == 'PO' then  ,
     return _MSG(8,dsnfrom' a partitioned DSN cannot be copied')
/* .... DSORG match only relevant for PS DSNs */
  if pdsn2=0 & dsorg2 == 'PO' then ,
     return _MSG(8,dsnto' is partitioned, needs a member clause')
  recordmode=0
/* .... RECFM match only relevant for PO DSNs */
  if substr(recfm1,1,1)<>substr(recfm2,1,1) & if dsorg2='PO' then do
     CALL _MSG 4,'RECFMs do not match: 'recfm1'/'recfm2', convert lines'
     recordmode=2
  end
  if substr(recfm1,1,1)='V' then slrecl=lrecl1-4
     else slrecl=lrecl1
  if substr(recfm2,1,1)='V' then tlrecl=lrecl2-4
     else tlrecl=lrecl2
/* .... LRECL match only relevant for PO DSNs */
  if slrecl>tlrecl & dsorg2='PO' then do
   CALL _MSG 4,"LRECLs do not match: "slrecl'/'tlrecl', records will be truncated'
     recordmode=1
  end
  if tlrecl>slrecl & dsorg2='PO' then do
   CALL _MSG 4,"LRECLs do not match: "slrecl'/'tlrecl', records increased'
     recordmode=2
  end
return 0
/* ---------------------------------------------------------------------
 * Check Target File and Remove if necessary
 * ---------------------------------------------------------------------
 */
ReplaceOpt:
  if cat2=0 then do
     dsorg2='PS'
     return 0
  end
  if dsorg2='PS' then do     /* additional check, if cat2=1 */
     if replace=0 then  ,
       return _MSG(8,dsnto' is already catalogued, no REPLACE option specified')
     else do
        if remove(dsnto)=0 then nop
          else return _MSG(8,dsnto' cannot be removed as requested by REPLACE option')
        call _msg 0, dsnto' removed as requested by REPLACE option'
     end
  end
  else if dsorg2='PO' then do
     if poexist=0 then CALL _MSG 0, dsnto' new Member will be added'
     else do
        if replace=0 then ,
        return _MSG(8,dsnto' already exists, no REPLACE option specified')
        else CALL _MSG 0, dsnto' will be replaced due REPLACE option'
     end
  end
return 0
/* ---------------------------------------------------------------------
 * Check Dataset
 * ---------------------------------------------------------------------
 */
checkDSN:
  interpret 'pdsn'arg(2)'=0'
  interpret 'member'arg(2)'=""'
  if pos('(',arg(1))=0 then isdsn=arg(1)
  else do
     parse value arg(1) with 1  isdsn"("member")"
     quote=substr(isdsn,1,1)
     if quote="'" then isdsn=isdsn"'"
     else if quote='"' then isdsn=isdsn'"'
     interpret 'pdsn'arg(2)'=1'
     interpret 'member'arg(2)'=member'
  end
  interpret 'dsn'arg(2)'=isdsn'
  interpret 'dsncat'arg(2)'=isdsn'
  interpret 'cat'arg(2)'=0'
  rc=listdsi(isdsn)
  if rc>0 then return rc
  interpret 'cat'arg(2)'=1'
  fullqualDSN="'"sysdsname /* don't add closing delim due member name */
  interpret 'dsncat'arg(2)'=fullqualDSN'
  interpret 'volser'arg(2)'=sysvolume'
  interpret 'dsorg'arg(2)'=sysdsorg'
  interpret 'recfm'arg(2)'=sysrecfm'
  interpret 'blksize'arg(2)'=sysblksize'
  interpret 'lrecl'arg(2)'=syslrecl'
  fx=open(arg(1),'RB')
  if fx<=0 then return 4
  interpret 'size'arg(2)'=seek(fx,0,"EOF")'
  call close(fx)
  return 0
/* ---------------------------------------------------------------------
 * Set Message and Error Code
 * ---------------------------------------------------------------------
 */
_msg:
parse arg _mslv
  if _msglimit<0 then return _mslv
  if datatype(_mslv)<>'NUM' then do
     say time()'      'arg(2)
     return 0
  end
  if _mslv<_msglimit then return _mslv
  say time()'  'right(_mslv,2,'0')'  'arg(2)
  if _mslv>maxrc then maxrc=_mslv
  return _mslv
/* ---------------------------------------------------------------------
 * INIT Test Source File and Target File
 * ---------------------------------------------------------------------
 */
init:
  maxrc=0
  call time('R')
  f1=0
  f2=0
  if _msglimit='_MSGLIMIT' then _msglimit=0
/* ..... 1. Check Source DSN ..... */
  rc=checkDSN(dsnfrom,1)
  if rc>0 then return _msg(8, dsnfrom' is not available')
  if dsorg1='PS' | dsorg1='PO' then nop
  else  ,
   return _MSG(8,dsorg1' invalid DSORG, ONLY PS AND PO Datasets are allowed')
  if length(member1)>8 then ,
     return _msg(8, dsnfrom' Member Name exceeds length of 8')
  if dsorg1='PO' then osize=''
     else osize=', size='size1'B'
  call _MSG(0,dsncat1' is 'sysdsorg', 'sysrecfm', BLK='sysblksize', LRECL='syslrecl''osize)
/* ..... 2. Check Target DSN ..... */
  rc=checkDSN(dsnto,2)
  if rc=4 then poExist=0
     else poExist=1
  if length(member2)>8 then ,
     return _msg(8, dsnto' Member Name exceeds length of 8')
/* ..... 3. Check Target DSN, if not catalogued take source infos ... */
  if cat2=0 then do
     size2=size1
     volser2=volser1
     dsorg2=dsorg1
     recfm2=recfm1
     lrecl2=lrecl1
     blksize2=blksize1
  end
  if dsorg2='PO' then osize=''
     else osize=', size='size2'B'
  if cat2=1 then  ,
  call _MSG(0,dsncat2' is 'sysdsorg', 'sysrecfm', BLK='sysblksize', LRECL='syslrecl''osize)
  if member1='' then dsnfrom=dsncat1"'"
     else dsnfrom=dsncat1'('member1")'"
  if cat2=1 then do
     if member2='' then dsnto=dsncat2"'"
        else dsnto=dsncat2'('member2")'"
  end
  else do
     if member2='' then dsnto=dsncat2
        else dsnto=dsncat2'('member2")"
  end
return 0
