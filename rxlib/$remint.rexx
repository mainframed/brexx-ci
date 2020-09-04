/* ---------------------------------------------------------------------
 * Remove Internal Members from BREXX.$Install lib
 * $INTERNAL Will not delivered in BREXX.$RELEASE.RXLIB
 * ---------------------------------------------------------------------
 */
REMINT:
if abbrev(arg(1),'BREXX.$RELEASE')=0 then do
   say 'Cleanup Internal Member not permitted in 'arg(1)
   say '    must be 'arg(1)', processing terminated'
   exit 12    /* Don't  return, Exit immediately */
end
internal=0
num=readall(arg(1)'('arg(2)')',,'DSN',20)
num=min(num,20)
do i=1 to num
   if pos('$INTERNAL',readall.i)>0 then do
      internal=1
      leave
   end
end
if internal=0 then say left(arg(2),8)' will be delivered'
else do
   global.removed=global.removed+1
   say left(arg(2),8)' is an internal Member, will not be delivered'
   ADDRESS TSO
    "DELETE '"arg(1)"("arg(2)")'"
   do ixj=1 to 100
      axi=1
   end
end
return internal
