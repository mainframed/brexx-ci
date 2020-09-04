/* ---------------------------------------------------------------------
 * Remove Internal Members from BREXX.$RELEASE.RXLIB
 *
 * $INTERNAL Will not delivered in BREXX.$RELEASE.RXLIB
 * ---------------------------------------------------------------------
 */
lib='BREXX.$RELEASE.SAMPLES'
say '------------------------------------------------'
say 'Remove Internal Members from 'lib
say '------------------------------------------------'
global.removed=0
count=Perform(lib,'$REMINT')
say right(count,3)' Members in 'lib
say right(global.removed,3)' Internal Members removed'
