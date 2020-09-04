/* ---------------------------------------------------------------------
 * $INTERNAL Will not delivered in BREXX.$RELEASE.RXLIB
 * D2B  Translate Decimal to Bit representation
 * bitstring=D2B(number)
 *      number must be between -2,147,483,648 and +2,147,483,647
 * .............................. Created by PeterJ on 19. February 2019
 * ---------------------------------------------------------------------
 */
d2b:  return x2b(d2x(arg(1)%1)) /* %1 to be sure it's a whole number  */
