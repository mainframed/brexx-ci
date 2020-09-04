/* ---------------------------------------------------------------------
 * Modulo function to make sure integer is returned
 *  the remainder division (//) returns decimal number not an integer
 *      for example 5//2  results in 1.0000000000003
 * .............................. Created by PeterJ on 25. February 2020
 * $INTERNAL Will not delivered in BREXX.$RELEASE.RXLIB
 * ---------------------------------------------------------------------
 */
Modulo:  return (arg(1)//arg(2))%1
