/* ---------------------------------------------------------------------
 * SINE plots the SINE function
 *   this is a test function for the BREXX Interactive System
 * ............................... Created by PeterJ on 20. January 2020
 * $INTERNAL Will not delivered in BREXX.$RELEASE.RXLIB
 * ---------------------------------------------------------------------
 */
li=0
do x=0 to 12.56 by 0.229
    y = trunc(27 * (sin(x) + 1.1)) + 1
    out = copies(" ",y) || "*"
    l = left(out,31," ")
    r = substr(out,41)
    li=li+1
    buffer.li=l||substr("|*",1+pos("*",substr(out,40,1)),1)||r
end
buffer.0=li
