/* ---------------------------------------------------------------------
 * DUMPVAR   Dumps Content of Variable at its storage address
 * .............................. added by Peter Jacob   06. August 2020
 * ---------------------------------------------------------------------
 */
DUMPVAR: Procedure
  parse upper arg vname
  xname=value(vname,,0)
  vaddr=addr('xname')
  vlen=length(xname)
  say 'Core Dump of 'vname', length 'vlen', length displayed 'vlen+16
  say copies('-',77)
  vlen=vlen+16
  call dumpit d2x(vaddr),vlen
return 0
