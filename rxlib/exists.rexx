/* REXX */
/* ---------------------------------------------------------------------
 * $INTERNAL Will not delivered in BREXX.INSTALL.RXLIB
 * ---------------------------------------------------------------------
 */
/* ---------------------------------------------------------------------
 * Check existence of Dataset
 *   EXISTS(ds-name)         check existence of dataset, if quotes the
 *                           DSN will be prefixed by the userid
 *   EXISTS('ds-name')       check existence of fully qualified DSN
 * .................................... Created by PeterJ on 29.May 2020
 * ---------------------------------------------------------------------
 */
parse upper arg udsn
rc=SYSDSN(udsn)
if rc=='OK' then return 1
return 0
