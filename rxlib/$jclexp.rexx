/* ---------------------------------------------------------------------
 * EXPAND JCL (Resolve INCLUDE, SET statements in JCL
 *
 * $INTERNAL Will not delivered in BREXX.$RELEASE.RXLIB
 * ---------------------------------------------------------------------
 */
lib='BREXX.$RELEASE.INSTLIB'
say '------------------------------------------------'
say 'Expand JCL in 'lib
say '------------------------------------------------'
global.changed=0
count=Perform(lib,'$EXPAND',1)
say right(count,3)' Members in 'lib
say right(global.changed,3)' JCL Members expanded'
say '------------------------------------------------'
say 'Expand JCL in 'lib
say '------------------------------------------------'
lib='BREXX.$RELEASE.PROCLIB'
global.changed=0
count=Perform(lib,'$EXPAND',1)
say right(count,3)' Members in 'lib
say right(global.changed,3)' JCL Members expanded'
say '------------------------------------------------'
say 'Expand Sample CNTL in 'lib
say '------------------------------------------------'
lib='BREXX.$RELEASE.CNTL'
global.changed=0
count=Perform(lib,'$EXPAND',1)
say right(count,3)' Members in 'lib
say right(global.changed,3)' JCL Members expanded'
