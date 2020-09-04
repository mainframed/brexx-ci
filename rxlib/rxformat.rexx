/* $INTERNAL Will not delivered in BREXX.INSTALL.RXLIB    */
 
/* ---------------------------------------------------------------------
 * value=FORMAT(value,
 *       +-------------------------------------------------------+---->
 *       '-,-+--------+-+--------------------------------------+-'
 *       '-before-' '-,-+-------+-+----------------------+-'
 *                      '-after-' '-,-+------+-+-------+-'
 *                                    '-expp-' '-,expt-'
 * ---------------------------------------------------------------------
 */
rxformat:
 parse arg inval,before,after,expp,expt
 if expp='' then return _noexp()
 return _isexp()
_noexp:
 if after='' then return right(_tstlen(inval,before),before)
 if after=0 then return right(_tstlen(_rndnodec(inval),before),before)
 rval=_round(inval,after)
 parse var rval bbb'.'ddd
 return right(_tstlen(bbb,before),before)'.'left(ddd,after,'0')
_isexp:
return inval
/* ---------------------------------------------------------------------
 * _RND  Round Decimal Number
 * result=_RND(number,digits-after-decimal-point)
 * ---------------------------------------------------------------------
 */
_round:
parse arg ppp'.'ddd,nnn
if ddd='' then return ppp
rdinc=pow10(-nnn)/2
if ppp>0 then return trunc(arg(1)+rdinc,nnn)
return trunc(arg(1)-rdinc,nnn)
/* ---------------------------------------------------------------------
 * _RND  Round Decimal Number to next integer
 * result=_RNDNoDec(decimal-number)
 * ---------------------------------------------------------------------
 */
_rndnodec:
parse arg ppp'.'ddd
if ddd='' then return ppp
if ppp>0 then return trunc(arg(1)+0.5,0)
return trunc(arg(1)-0.5,0)
_tstlen:
parse arg _val,_mlen
if length(_val)>_mlen then do
   say 'Format Error, Value '_val' does not fit in Format Length '_mlen
   exit 8
end
return _val
