/* REXX */
/* $INTERNAL Will not delivered in BREXX.$RELEASE.RXLIB */
/* ---------------------------------------------------------------------
 * Encrypt and Decrypt dataset
 *   CRYPTDSN('ENCRYPT',dsn-to-encrypt,enrypted-dsn,passphrase)
 *           dsn-to-encrypt any sequential file, any recfm and lrecl
 *           passphrase     string which encrypts the dsn
 *   CRYPTDSN('DECRYPT',dsn-to-decrypt,target-dsn,passphrase)
 *           dsn-to-encrpyt must be a fixed (blocked) file which was
 *                          encrypted prior to decryption
 * .................................... Created by PeterJ on 22.May 2020
 * $INTERNAL Will not delivered in BREXX.$RELEASE.RXLIB
 * ---------------------------------------------------------------------
 */
MatchDSN: Procedure
 
parse arg idsn,odsn
  ft1=open("'"idsn"'",'rt')
  if ft1<1 then say 'First DSN 'indsn' cannot be opened'
  fs1=seek(ft1,0,'EOF')
  call seek(ft1,0)
  ft2=open("'"odsn"'",'rt')
  if ft2<1 then say 'Second DSN 'outdsn' cannot be opened'
  fs2=seek(ft2,0,'EOF')
  call seek(ft2,0)
  if ft1<0 & ft2<0 then say 'DSN Compare terminated'
  i=0
  j=0
  neq=0
  eq=0
  do until EOF(ft1)
     rec1=read(ft1)
     i=i+1
     rec2=read(ft2)
     j=j+1
     diff=compare(rec1,rec2,' ')
     if diff=0 then do
        eq=eq+1
        iterate
     end
     neq=neq+1
     say 'F1: 'right(i,4,'0')' 'rec1
     say '--- 'copies(' ',diff+5)'/'
     say 'F2: 'right(i,4,'0')' 'rec2
     say '  '
  end
  if ft1>=0 then call close(ft1)
  if ft2>=0 then call close(ft2)
  say i' records read from 'idsn
  say j' records read from 'odsn
  say eq' records are equal'
  say neq' records are different'
return 0
