//
// Created by PeterJ on 05.05.2020.
//
/*
 * $Id: trunc.c,v 1.9 2011/06/29 08:33:09 bnv Exp $
 * $Log: trunc.c,v $
 * Revision 1.9  2011/06/29 08:33:09  bnv
 * char to unsigned
 *
 * Revision 1.8  2011/06/20 08:31:19  bnv
 * removed the FCVT and GCVT replaced with sprintf
 *
 * Revision 1.7  2010/01/27 13:21:03  bnv
 * Use of fcvt
 *
 * Revision 1.6  2008/07/15 07:40:54  bnv
 * #include changed from <> to ""
 *
 * Revision 1.5  2008/07/14 13:08:16  bnv
 * MVS,CMS support
 *
 * Revision 1.4  2002/06/11 12:37:15  bnv
 * Added: CDECL
 *
 * Revision 1.3  2001/06/25 18:49:48  bnv
 * Header changed to Id
 *
 * Revision 1.2  1999/11/26 12:52:25  bnv
 * Changed: To use the fcvt()
 *
 * Revision 1.1  1998/07/02 17:18:00  bnv
 * Initial Version
 *
 */

#include "lstring.h"

/* ---------------- Lround ----------------- */
void __CDECL
Lround( const PLstr to, const PLstr from, long n) {
    char *buf[50];
    int i;
    double r;
    double f = 1.0;
    char lastdig;

    // to avoid pow function (not included) use loop)
    for (i = 1; i <= n; i++) f = f / 10;
    if (n<0) n = 0;

    L2REAL(from);
    Lfx(to,n+15);

/* trunc doesn't round. therefore add one extra digit */
    snprintf(LSTR(*to), LMAXLEN(*to), "%.*f", (int)n+1, LREAL(*from));
    lastdig=LSTR(*to)[STRLEN(LSTR(*to))-1];
/* ... and remove it later */
    i=STRLEN(LSTR(*to))-1;
    if (n==0) i--;               // if no fraction digits required, remove also decimal dot
    LTYPE(*to) = LSTRING_TY;
    LLEN(*to)  = i;
    if (lastdig>='5')     {
       _Lisnum(to);
       r = lLastScannedNumber+f;
       snprintf(LSTR(*to), LMAXLEN(*to), "%.*f", (int)n, r);
       LLEN(*to)  = STRLEN(LSTR(*to));
    }
} /* R_round */
