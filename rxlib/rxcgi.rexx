/* ---------------------------------------------------------------------
 * $INTERNAL Will not delivered in BREXX.INSTALL.RXLIB
 * ---------------------------------------------------------------------
 */
parse arg httpPage
_$server._html=httpPage
fk=open("'BREXX.HTTPD.HTML("httpPage")'",'RT')
if fk<=0 then do
   fk=open("'BREXX.HTTPD.HTML(HTMLERR)'",'RT')
end
i=0
_$pl=0
do until eof(fk)
   i=i+1
   line=read(fk)
   pi=pos('<?RX',line)
   if pi=0 then call pushLine line
   else if analyseBREXX()=64 then leave
end
_$PSHL.0=_$pl
call close(fk)
return 0
/* ---------------------------------------------------------------------
 * BREXX Code found in HTML Definition
 * ---------------------------------------------------------------------
 */
analyseBREXX:
  parse value line with '<?RX'brxl'?>'
  if pos('?>',line)=0 then do until eof(fk)  /* multi line REXX */
     line=strip(read(fk))
     pi=pos('?>',line)
     if pi=0 then brxl=brxl';'strip(line)
     else do
        brxl=brxl';'strip(substr(line,1,pi-1))
        leave
     end
  end
  if exec(brxl)=64 then return 64
return 0
/* ---------------------------------------------------------------------
 * Run BREXX Code
 * ---------------------------------------------------------------------
 */
exec: procedure expose _$server. _$pl _$PSHL.
  signal on syntax name htmlERR
  exc=changestr('SAY',arg(1),'call pushline')
  exc=changestr('say',exc,'call pushline')
  interpret strip(exc)
return 0
htmlErr:
  signal off syntax
  rtag='<p style="color:tomato;">'
  call pushline rtag'// Error occurred during HTML generation in REXX Code</p>'
  call pushline rtag'// Check REXX Code in HTML Definition</p>'
  call pushline rtag'// Look  for paired quotes</p>'
  call pushline rtag'// Look  for paired parentheses</p>'
  call pushline rtag'// Look  for arithmetic errors</p>'
return 64
/* ---------------------------------------------------------------------
 * Push Output line into Stem
 * ---------------------------------------------------------------------
 */
PushLine:
  _$pl=_$pl+1
  _$PSHL._$pl=arg(1)
return
