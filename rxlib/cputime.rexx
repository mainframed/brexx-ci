cpuTime: Procedure
  _ascb=_MVSASCB()
  EJST=storage(d2x(_ascb+64),8)
  SRBT=storage(d2x(_ascb+200),8)
  WRK = x2d('0'left(c2x(EJST),13))
  CPUTIME = WRK/1000000
  WRK = x2d('0'left(c2x(SRBT),13))
  CPUTIME = CPUTIME + wrk/1000000
return Format(round(CPUTIME + wrk/1000000,6),,6)
_MVSASCB: return c2d(storage(224,4))
_MVSASSB: return c2d(storage(d2x(_MVSASCB()+336),4))
