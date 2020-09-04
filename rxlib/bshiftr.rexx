/* ---------------------------------------------------------------------
 * $INTERNAL Will not delivered in BREXX.$RELEASE.RXLIB
 * BSHIFTR  Right shift Bits of a Decimal Number
 *  number=BSHIFTL(decimal-number,shift-bit-amount)
 * .............................. Created by PeterJ on 19. February 2019
 * ---------------------------------------------------------------------
 */
bShiftR:
  parse arg !bsn,!bsh
  !bsn=d2b(!bsn)
  !bsln=length(!bsn)
  !bsn=copies('0',!bsh)substr(!bsn,1,!bsln-!bsh)
return b2d(!bsn)
