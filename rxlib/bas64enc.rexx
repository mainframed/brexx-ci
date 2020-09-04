/* ---------------------------------------------------------------------
 * BAS64ENC returns encoded (binary)-data in BASE64 Format
 *   encstring=BAS64ENC(data)
 *     adapted from coding in http://purl.net/xyzzy/src/md5.cmd
 * .............................. Created by PeterJ on 20. February 2019
 * ---------------------------------------------------------------------
 */
B64ENC: Procedure            /* string to (unlimited) base64: */
 !b64='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
 !istring=x2b(c2x(arg(1)))
 _tstr=''
 #fill=((length(!istring)/4)//3)%1
 !istring=!istring||copies('00',#fill)
 do while !istring<>''
    parse var !istring n 7 !istring
    n=x2d(b2x(n))
    _tstr=_tstr||substr(!b64,n+1,1)
 end
return _tstr||copies('=',#fill )
