//
// Created by PeterJ on 05.05.2020.
//

#include "lstring.h"

/* ---------------- Lround ----------------- */
void __CDECL
Lround( const PLstr to, const PLstr from, long n) {
    double add=1;
    int i;
    L2REAL(from);
    if (LREAL(*from)<0) add=-1;
    Lfx(to,n+15);

    if (n<0) n=0;
    for (i=0;i<=n;i++) add=add/10;
   // snprintf does round!, but due the conversion of real numbers a 1.50 might appear as 1.499999
   // and therefore snprintf will round it to 1.4. To increase the reliability of the rounding we add
   // 0.01 if we round to n=0.
    snprintf(LSTR(*to), LMAXLEN(*to), "%.*f", (int)n, LREAL(*from)+add);
    LTYPE(*to) = LSTRING_TY;
    LLEN(*to)  = STRLEN(LSTR(*to));
} /* Lround */
