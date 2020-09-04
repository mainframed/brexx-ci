/* ---------------------------------------------------------------------
 * GETToken  Get a Unique Token
 * token=GetToken('MVS')       the token is unique within an IPLed MVS
 * token=GetToken('CENTURY')   the token is unique within the century
 * .............................. Created by PeterJ on 18. February 2019
 * ---------------------------------------------------------------------
 */
GetToken:
  if abbrev('CENTURY',translate(arg(1)),1)=1 then do
     call wait 1 /* wait a bit to avoid duplicates if called in a row */
     __DATX=date('sorted')
     __DATX=substr(__DATX,2,2)+365*substr(__DATX,4)
     return __DATX''changestr(' ',translate(time('L'),,'.:'),'')
  end
/* else  MVS  time in milliseconds MVS is up and running */
return @ADR(@ADR(@ADR(16)+604)+124)
@ADR: return c2d(storage(d2x(arg(1)),4))
