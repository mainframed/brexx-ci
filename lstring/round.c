//
// Created by PeterJ on 05.05.2020.
//

#include "lstring.h"

/* ---------------- Lround ----------------- */
void __CDECL
Lround( const PLstr to, const PLstr from, long n) {
    int i;
    L2REAL(from);
    if (LREAL(*from)<0) ;
    Lfx(to,n+15);

    snprintf(LSTR(*to), LMAXLEN(*to), "%.*f", (int)n, LREAL(*from));
    LTYPE(*to) = LSTRING_TY;
    LLEN(*to)  = STRLEN(LSTR(*to));
} /* Lround */
