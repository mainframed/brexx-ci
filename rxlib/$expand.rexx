/* ---------------------------------------------------------------------
 * Replace in JCL JCL-SET and JCL-INCLUDE Commands
 * $INTERNAL Will not delivered in BREXX.INSTALL.RXLIB
 * ---------------------------------------------------------------------
 */
parse upper arg pds,file,suppress
varct=0
_changed=0
_drop=0
if suppress='' then suppress=0
subx=READALL(pds'('file')','jclin.','DSN',,'//')
if subx<0 then _drop=1
if substr(jclin.1,1,2)<>'//' then _drop=1
if _drop=1 then do
   say file' no JCL Member '
   return -4
end
/* Add some fixed JCL Variables   */
brexxVersion=version('FULL')
call addJclVar 'brexxVersion'
InstallerDate=date()' 'time()
if upper(date())='29 MAR 2020' then InstallerDate=01 MAR 2020' 'time()
call addJclVar 'InstallerDate'
do lino=1 until lino>=jclin.0
   jclin.lino=strip(substr(jclin.lino,1,72),'T') /* clear Line number */
   if varct>0 then call replacevar
   if substr(jclin.lino,1,3)='//*' then call searchVar
end
if _changed=0 then say file' nothing changed'
else do
   subx=WRITEALL(pds'('file')','jclin.','DSN')
   say file' changes made '_changed
   global.changed=global.changed+1
   if suppress=0 then do
      say '*** Final JCL '
      do i=1 to jclin.0
         say right(i,3) substr(JCLIN.i,1,72)
      end
   end
end
return 0
/* ---------------------------------------------------------------------
 * Search in JCL comment line for SET or INLCUDE Commands
 * ---------------------------------------------------------------------
 */
searchVar:
 setx=upper(strip(substr(JCLIN.LINO,4,68)))
 wsi=wordpos('INCLUDE',setx)
 wss=wordpos('SET',setx)
 if wss=2 then nop
 else if wsi>0 then do
    call jclincl lino,wsi
    return
 end
 else return
/* ... SET Statement found, assigne value, keep it for substitution  */
 setv=word(setx,1)             /* Variable Name is 1. Word */
 iwi=wordindex(setx,2)+4       /* set behind SET */
 val=strip(substr(setx,iwi))
 sq=substr(val,1,1)
 if sq="'" then nop
 else if sq='"' then nop  /* Test for " as string delimeter */
 else val="'"val"'"       /* test for ' as string delimeter */
 interpret setv'='val
 call addjclvar setv
return
/* ---------------------------------------------------------------------
 * Replace in JCL line Variables
 * ---------------------------------------------------------------------
 */
ReplaceVar:
 do k=1 to varct
    kvar=substr(sortin.k,5)
    jclorg=jclin.lino
    jclin.lino=changestr('&'kvar,jclin.lino,value(kvar))
    if jclin.lino<>jclorg then _changed=_changed+1
 end
return
/* ---------------------------------------------------------------------
 * New Variable Found, add it Variable Stack
 * ---------------------------------------------------------------------
 */
addJclVar:
 parse upper arg varname
 varct=varct+1
 sortin.varct=right(length(varname),4,'0')varname
 sortin.0=varct
 if varct>3 then call rxsort 'quicksort','descending'
return
/* ---------------------------------------------------------------------
 * New Include found, insert JCL after Include
 * ---------------------------------------------------------------------
 */
jclincl:
 parse arg insert,wpos
 iwi=wordindex(setx,wpos)+8         /* set behind INCLUDE */
 incfile=strip(substr(setx,iwi))
 incfile=unquote(incfile)
 RXMSLV='N'
 if suppress=1 then jclin.insert='//*  '
 insert=insert+1
 ct=READALL(incfile,'INCFILE.','DSN')
 if ct<=0 then do
    incfile.0=1
    incfile.1='//* !!! 'incfile' not found !!!'
 end
 if suppress=1 then nop
 else do    /* suppress=0   */
    incmax=incfile.0+1
    incfile.0=incmax                             /* add trailing line */
    incfile.incmax='//* ... END OF 'incfile      /* add trailing line */
 end
 ct=STEMINS('INCFILE.','JCLIN.',insert)
return
