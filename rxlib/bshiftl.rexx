/* ---------------------------------------------------------------------
 * BSHIFTL  Left shift Bits of a Decimal Number
 * $INTERNAL Will not delivered in BREXX.$RELEASE.RXLIB
 *  number=BSHIFTL(decimal-number,shift-bit-amount)
 * .............................. Created by PeterJ on 19. February 2019
 * ---------------------------------------------------------------------
 */
bShiftL:
return b2d(d2b(arg(1))copies(0,arg(2)))+0 /* +0 to make sure it's NUM */
