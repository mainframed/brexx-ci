/* ---------------------------------------------------------------------
 * BAS64DEC returns decoded Base64 string
 *   decstring=BAS64DEC(data)
 *     adapted from coding in http://purl.net/xyzzy/src/md5.cmd
 * .............................. Created by PeterJ on 20. February 2019
 * ---------------------------------------------------------------------
 */
BAS64DEC: procedure              /* (unlimited) base64 to string: */
 !b64='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
 !istring=strip(translate(arg(1),'+/','-_' ))
 _tstr=''
 do while abbrev('==',!istring)=0
    parse var !istring #fill 2 !istring
    #fill=d2x( pos(#fill,!b64)-1)
    _tstr=_tstr||right(x2b(#fill),6,0)
 end
return x2c(b2x(left(_tstr,length(_tstr)-2*length(!istring))))
