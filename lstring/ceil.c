//
// Created by PeterJ on 01.05.2020.
//
#include "lstring.h"

/* ------------------- Ceil ------------------ */
void __CDECL
Lceil( const PLstr to, const PLstr num )
{
    double    s;
    int       nlen;
    char      fnull;
  // round number to one fraction digit and create string number
    snprintf(LSTR(*to), LMAXLEN(*to), "%.*f", (int)1,Lrdreal(num));
    nlen=STRLEN(LSTR(*to));
    fnull=LSTR(*to)[nlen-1];     // fetch first fraction digit
    LLEN(*to)  = nlen-2;         // set string length to represent integer

    LTYPE(*to) = _Lisnum(to);    // convert it to integer and set type (lLastScannedNumber is also set)
    s=lLastScannedNumber;           // s is created integer
    LLEN(*to)  = sizeof(long);
    if (fnull != '0' && s > 0) { s++; } // +1 if first fraction was <> 0 and number is positive (CEILING)
    LINT(*to)  = (long)s;
  } /* Ceil */