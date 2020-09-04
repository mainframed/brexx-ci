/* REXX */
/* ---------------------------------------------------------------------
 * Encrypt and Decrypt dataset
 *   CRYPTDSN('ENCRYPT',dsn-to-encrypt,enrypted-dsn,passphrase)
 *           dsn-to-encrypt any sequential file, any recfm and lrecl
 *           passphrase     string which encrypts the dsn
 *   CRYPTDSN('DECRYPT',dsn-to-decrypt,target-dsn,passphrase)
 *           dsn-to-encrpyt must be a fixed (blocked) file which was
 *                          encrypted prior to decryption
 * .................................... Created by PeterJ on 22.May 2020
 * $INTERNAL Will not delivered in BREXX.INSTALL.RXLIB
 * ---------------------------------------------------------------------
 */
CryptDSN: Procedure
parse arg mode,idsn,odsn,passw,replace
  crc=8
  ft1=-1
  ft2=-1
  deleted=-1
  if substr(translate(replace),1,1)='R' then deleted=remove("'"odsn"'")
  if abbrev('ENCRYPT',mode,2)=1 then crc=encryptf(idsn,odsn,passw)
  else if abbrev('DECRYPT',mode,2)=1 then crc=decryptf(idsn,odsn,passw)
  else say 'Invalid CRYPTDSN Mode 'mode
  if ft1>=0 then call close(ft1)
  if ft2>=0 then call close(ft2)
return crc
/* ---------------------------------------------------------------------
 * Encrypt entire File
 * ---------------------------------------------------------------------
 */
encryptf:
parse upper arg indsn,outdsn
  password=arg(3)
  call inputDSN('E')
  if encinit()>0 then do
     say 'Encryption of 'indsn' failed'
     say copies('-',72)
     return 8
  end
  i=1   /* Header record is already written */
  total=80
  do until EOF(ft1)
     oline=read(ft1,80)
     reclen=length(oline)
     if EOF(ft1) & reclen=0 then leave
     if reclen<80 then do          /* is only true for last Record */
        oline=left(oline,80,'01'x) /* fill up last Record */
     end
     notwritten=encrypt(oline,password)
     call writeNow
  end
  say 'Encrypted   'i' Records'
  say 'Total Bytes 'total
  say copies('-',72)
return 0
/* ---------------------------------------------------------------------
 * Decrypt entire File
 * ---------------------------------------------------------------------
 */
decryptF:
parse upper arg indsn,outdsn
  password=arg(3)
  call inputDSN('D')
  if decinit()>0 then do
     say 'Decryption of 'indsn' failed'
     say copies('-',72)
     return 8
  end
  total=0
  i=1        /* header record has already been read */
/* don't write line immediately, before we know if it caused EOF*/
  notWritten=''
  do until EOF(ft1)
     oline=read(ft1,80)
     if EOF(ft1) & length(oline)=0 then leave
     if notwritten<>'' then call writeNow
     notwritten=decrypt(oline,password)
  end
  notwritten=strip(notwritten,'t','01'x)
  call writeNow
  say 'Decrypted   'i' Records'
  say 'Total Bytes 'total
  say copies('-',72)
return 0
/* ---------------------------------------------------------------------
 * write hanging records
 * ---------------------------------------------------------------------
 */
writeNow:
  call write(ft2,notwritten)
  i=i+1
  total=total+length(notwritten)
return
/* ---------------------------------------------------------------------
 * Prepare Encrypting of file
 * ---------------------------------------------------------------------
 */
encinit:
  ft1=open("'"indsn"'",'rb')
  if ft1<1 then say 'Input DSN 'indsn' cannot be opened'
  ft2=open("'"outdsn"'",'rb')
  if ft2>0 then return outDSNpresent(outdsn,'Encryption')
  ft2=open("'"outdsn"'","wb,recfm=fb,lrecl=80,blksize=8000,
           ,unit=sysda,pri=100,sec=100,rlse")
  if ft2<1 then say 'Output DSN 'outdsn' cannot be opened'
  if ft1>0 & ft2>0 then nop
  else do
     say 'Encryption Terminated'
     return 8
  end
  ft1len=GetSize()
  header=header' FILESIZE='ft1len
  header=right(header,80)
  eheader=encrypt(header,'thisisNOTthePassword')
  call write ft2,eheader
  call outputDSN('E','FB',80,8000)
return 0
/* ---------------------------------------------------------------------
 * Prepare Encrypting of file
 * ---------------------------------------------------------------------
 */
decinit:
  ft1=open("'"indsn"'",'rb')
  if ft1<1 then do
     say 'Input DSN 'indsn' cannot be opened'
     return 8
  end
  ft2=open("'"outdsn"'",'rb')
  if ft2>0 then return outDSNpresent(outdsn,'Decryption')
  ft1len=GetSize()
  header=read(ft1,80)
  header=decrypt(header,'thisisNOTthePassword')
  lrecl=-1
  blksize=-1
  size=-1
  parse value header with ,
    1 "RECFM="recfm" ",
    1 "LRECL="lrecl" ",
    1 "BLKSIZE="blksize" ",
    1 "FILESIZE="size" "
/* .....................................................................
 * Check if it is really a Encrypted File
 * .....................................................................
 */
  if lrecl<0 | blksize<0 | size<0 then do
     say "Invalid Input DSN '"indsn
     say "  DSN is either not Encrypted or corrupted"
     return 8
  end
/* Proceed with target DSN calculations                        */
  btrk=20000%blksize         /* Blocks per Track (fictous 20K) */
  blk =Ceil(size/blksize)    /* number of blocks needed        */
  pri=ceil(1.5*blk/btrk)
  sec=pri
  ft2=open("'"outdsn"'",
      ,"wb,recfm="recfm",lrecl="lrecl",blksize="blksize" ,
      ,unit=sysda,pri="pri",sec="sec",rlse")
  if ft2<1 then say 'Output DSN 'outdsn' cannot be opened'
  if ft1>0 & ft2>0 then do
     call outputDSN('D',recfm,lrecl,blksize)
     return 0
  end
  say 'Encryption Terminated'
return 8
/* ---------------------------------------------------------------------
 * Fetch File Size of Input File
 * ---------------------------------------------------------------------
 */
GetSize:
  flen=seek(ft1,0,'EOF')
  call seek(ft1,0)
  say ' File Size  'flen
return flen
/* ---------------------------------------------------------------------
 * Output DSN must not be present
 * ---------------------------------------------------------------------
 */
outDSNpresent:
  say arg(1)' already present'
  say arg(2)' Terminated'
return 8
/* ---------------------------------------------------------------------
 * Report Input DSN
 * ---------------------------------------------------------------------
 */
InputDSN:
parse arg cmode
  say copies('-',72)
  if cmode='E' then say "Encrypting '"indsn"'"
     else say "Decrypting '"indsn"'"
  call LISTDSI("'"indsn"'")
  say '     RECFM  'sysrecfm
  say '     LRECL  'syslrecl
  say '   BLKSIZE  'sysblksize
  header='RECFM='sysrecfm' LRECL='syslrecl' BLKSIZE='sysblksize
return
/* ---------------------------------------------------------------------
 * Report Output DSN
 * ---------------------------------------------------------------------
 */
OutputDSN:
parse arg cmode,recfm,lrecl,blksize
  if deleted=0 then say "Target DSN '"outdsn"' *** old Version deleted"
     else say "Target DSN '"outdsn"'"
  say '     RECFM  'recfm
  say '     LRECL  'lrecl
  say '   BLKSIZE  'blksize
  if cmode='E' then say "Password   '"password"' REQUIRED TO DECRYPT!"
     else say "Password   '"password"'"
return
