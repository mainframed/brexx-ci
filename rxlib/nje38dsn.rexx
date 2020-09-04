/* ---------------------------------------------------------------------
 * Determine Netspool DSN
 * ---------------------------------------------------------------------
 */
RXNJE38DSN:
  parse arg mode
  if mode='ALLOC' then do
     dsn=_netspool()
     if dsn='' then do
        zerrsm='NJE38 Application not active'
        zerrlm='NJE38 Application not active or Allocation failed'
        return 8
     end
     alc=allocate('NETSPOOL',"'"dsn"'")
     if alc<>0 then do
        zerrsm='NJE38 NETSpool allocation error'
        zerrlm='NJE38 NETSpool allocation error'
        return 8
     end
     return alc
  end
  if mode='FREE' then return free('NETSPOOL')
  say mode' unknown mode'
return 8
/* ---------------------------------------------------------------------
 * Determine Netspool DSN
 * ---------------------------------------------------------------------
 */
_NETSPOOL: procedure
  r2=LOADC(16)+640   /*   CVT +offset 280  -> cvtfqcb */
/* run through all Major Queue Names */
  do forever
     r2=LOADC(r2)                             /* Load next Major QCB  */
     if r2=0 then return ''
     if PEEKS(r2+16,5)='NJE38' then leave     /* ddname is offset +16 */
  end
/* run through Minor Queue Names associated with Major Queue of NJE38 */
  r3=LOADC(r2+8)                            /* Pointer to Minor Queue */
  do for 64
     r4=r3+20                               /* load content of addr   */
     if PEEKS(r4,7)='NJEINIT' then return strip(PEEKS(R4+12,44))
     if r3=LOADC(r2+12) then return ''      /* end of chain reached   */
     r3=LOADC(r3)
  end
return ''
PEEKS: return storage(d2x(arg(1)),arg(2))
LOADC: return c2d(storage(d2x(arg(1)),4)) /* return pointer (decimal) */
