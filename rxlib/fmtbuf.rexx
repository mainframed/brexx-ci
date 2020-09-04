/* ---------------------------------------------------------------------
 * Display Buffer
 *   Content must be stored in STEM BUFFER.
 *   Buffer.0    contains number of lines
 *   Buffer.n    each line in stem as single entry n=1,2,...,maximum
 * .................................. Created by PeterJ on 25. July 2019
 * $INTERNAL Will not delivered in BREXX.$RELEASE.RXLIB
 * ---------------------------------------------------------------------
 */
fmtbuf: procedure expose LastCommand. ,
     buffer. #$_BUF. help. _screen. _fssinit _FSSFuncKeys. (public)
parse arg lineareaLen,LineareaChar,header,header2 ,
          ,LinecommandAPPL
  traceCount=1          /* Suppress instruction count display */
  signal off syntax
  call import FSSAPI
  Address FSS
  if _screen.Ftrace=1 then call fssFtrace 'Entering FMTLIST'
  call fmtLinit         /* init and set size of 3270 screen  */
  call fetchBuffer      /* Copy Buffer to internal buffer    */
/* ..... Display first Buffer ..... */
  lino=display(1,1)
/* .....................................................................
 *  Call Screen Handler to manage Buffer until PF3/PF4 */
  _rc=screenHandler()   /* Screen Handler manages input keys */
/* .....................................................................
 */
  drop #$_BUF._#bno.    /* second level drop, doesn't seem to work */
  #$_BUF._#bno.0=-1
  _fssInit=_fssInit-1   /* buffer number -1 */
  #$_BUF.$stack=#$_BUF.$stack-1
  if botcolor>0 & botindx>0 then call resetBotColor botindx
  if topcolor>0 then call resetTopColor 'FORCE'
/* ..... If Last Buffer is about to close, perform cleanup  */
  if _fssInit=0 then  , /* if initial buffer terminate      */
     call fssclose
  if _screen.Ftrace=1 then call fssFtrace 'Leaving FMTLIST',100
/* ..... Return to previous Buffer, or Final end of FMTLIST */
  if _fssInit=0 then return 0
  if _rc=-16 then return _rc /* -16: end next buffer too PF4*/
return _#bno   /* return to previous buffer, which is _#bno */
/* ---------------------------------------------------------------------
 * Screen Handler
 *   handles ENTER and PF Keys
 * ---------------------------------------------------------------------
 */
screenHandler:
  do forever
     _pfkey=fssrefresh('CHAR') /* Display Screen, return PF-key/Enter */
    call wait 2
    if msgset=1 then do
       call statspart
       if msglong=1 then call ZERRLM ' '
       msgset=0
    end
    if traceCount>0 then call InstructionLogA
    if _pfkey='PF12' then do
       call LastCommand
       iterate
    end
    call getFields
    if _pfkey='ENTER' then do   /* action command from enter key */
       $erc=enterKey()
       if $erc=-4  then if returnok('QUIT')=0 then leave; else nop
       else if $erc=-16 then return -16 ; else nop
     end
     else if _pfkey='PF03' then if returnok()=0 then leave; else nop
     else if _pfkey='PF15' then if returnok()=0 then leave; else nop
     else if _pfkey='PF04' then return -16
     else if _pfkey='PF16' then return -16
     else if _pfkey='PF08' then lino=display(lino+scroll(command),scol)
     else if _pfkey='PF07' then lino=display(lino-scroll(command),scol)
     else if _pfkey='PF11' then lino=display(lino,scol+50)
     else if _pfkey='PF10' then lino=display(lino,scol-50)
     else if _pfkey='PF09' then call swapx
     else if _pfkey='PF02' then if startx()=-16 then return -16;else nop
     else if _pfkey='PF01' then do
            call DisplayHelp     /* Load Help in new Buffer */
         /* Display Help in new Buffer, if PF04  trigger final end  */
            fmbuf=FMTLIST(1,'.')=-16 then return -16
            call recoverScreen fmbuf,1   /* recover old Buffer */
     end  /* enter, cmd+line cmd, -16 means fast quit of buffer */
    if traceCount>0 then call InstructionLogB
  end
return 0
/* ---------------------------------------------------------------------
 * Test if requested Return is on last buffer
 * ---------------------------------------------------------------------
 */
returnok:
parse arg qcode
  if qcode='' then qcode='RETURN (PF3/PF15)'
  if _#bno=#$_BUF.$stack then return 0
  call zerrsm word(qcode,1)' only on last buffer'
  $bnum='B'right(#$_BUF.$stack,2,'0')
  $bxa ='B'right(_#bno,2,'0')
  call zerrLM qcode' only available on last buffer '$bnum' not on '$bxa
return 8
/* ---------------------------------------------------------------------
 * Display Screen
 * ---------------------------------------------------------------------
 */
Display:
  parse arg lino,scol,forcedisp
  if lino<1 then lino=1
     else if lino>linc then lino=linc
  if scol<1 then scol=1
     else if scol>255 then scol=LastScol
  if lastLino=lino & lastScol=scol & forcedisp='' then return lino
  lini=lino
  if botcolor>0 & botindx>0 then call resetBotColor botindx
  botindx=0   /* reset bottom line index, will be newly determined */
 /* Display top lines and Header lines, if any */
  if lini<=1 & #$_BUF._#BNO.$HDR1=0 then do
     i=setLine(1,#lastar,center(' Top of Data ',#LSTWIDTH,'*'))
     if colortop1<>'' then call SetTopColor 1
  end
  else if #$_BUF._#BNO.$HDR1=1 then i=headerAll()
  else i=0
/* Display Buffer Content of curent screen page  */
  if topcolor=1 then call resetTopColor lini
  if topcolor=100 then topcolor=1
  do i=i+1 to #lstheight
     if lini<=linc &linc>0 then call bufline /* display content lines */
     else if lini+lino=0 then call endline   /* Display END OF DATA   */
     else if lini=linc+1 then call endline   /* Display END OF DATA   */
     else call dummyLine                     /* Display emtpy line    */
  end
  lastLino=lino
  lastScol=scol
  if msgset=0 then call statsPart
return lino
/* ---------------------------------------------------------------------
 * Refresh Statistics at Top of Screen
 * ---------------------------------------------------------------------
 */
statsPart:
/* Update other parts of the List Screen   */
  _fs='ROWS 'right(lino,5,'0')'/'right(linc,5,'0'),
      'COL 'right(scol,3,'0'),
      'B'right(_#BNO,2,'0')
  'SET FIELD STATS _fs'
  'SET CURSOR CMD'
return
mycolor:
parse arg slino p0 p1 p2 p3
parse var p2 xy'.'blino
if p3=61696 then p4='BLUE'
if p3=61952 then p4='RED'
if p3=62208 then p4='PINK'
if p3=62464 then p4='GREEN'
if p3=62720 then p4='TURQ'
if p3=62976 then p4='YELLOW'
if p3=63232 then p4='WHITE'
return
/* ---------------------------------------------------------------------
 * Reset Color of first line and Header Lines
 * ---------------------------------------------------------------------
 */
resetTopColor:
  if arg(1)=1 then do
    'SET COLOR LINEA.1' colortop1
    'SET COLOR #$_BUF._#BNO.1' colortop2
  end
  else do
    'SET COLOR LINEA.1' colorlist1
    'SET COLOR #$_BUF._#BNO.1' colorlist2
  end
  if #$_BUF._#BNO.$HDR2=0 & arg(1) <> 'FORCE' then return
 'SET COLOR LINEA.2' colorlist1
 'SET COLOR #$_BUF._#BNO.2' colorlist2
  topcolor=0
return
/* ---------------------------------------------------------------------
 * Reset Color of first line and Header Lines
 * ---------------------------------------------------------------------
 */
setTopColor:
parse arg tlino
  'SET COLOR LINEA.'tlino colortop1
  'SET COLOR #$_BUF._#BNO.'tlino colortop2
   topcolor=100
return
/* ---------------------------------------------------------------------
 * Reset Color of Last Line
 * ---------------------------------------------------------------------
 */
resetBotColor:
  botcolor=arg(1)
 'SET COLOR LINEA.'botcolor colorlist1
 'SET COLOR #$_BUF._#BNO.'botcolor colorlist2
  botcolor=0
return
/* ---------------------------------------------------------------------
 * Set Color of Last Line
 * ---------------------------------------------------------------------
 */
setBotColor:
  botcolor=arg(1)
 'SET COLOR LINEA.'botcolor colorbot1
 'SET COLOR #$_BUF._#BNO.'botcolor colorbot2
return
/* ---------------------------------------------------------------------
 * Set Error Message  short/long ZERRSM/ZERRLM
 * ---------------------------------------------------------------------
 */
zerrsm:
  parse arg _msg
 'SET FIELD STATS _msg'
  msgset=1
return
/* Set Error Message Long (ZERRLM) */
zerrlm:
  if msglong<>1 then return
  call fssZERRLM arg(1)
  msgset=1
return
/* ---------------------------------------------------------------------
 * Instruction Log SET
 * ---------------------------------------------------------------------
 */
InstructionLogA:
  instrctA=SYSVAR('RXINSTRC')
  instrelp=time('e')
return
InstructionLogB:
  instrCtB=SYSVAR('RXINSTRC')
  instrelp=trunc(time('e')-instrelp,3)
  _cmdl=overlay(';'instrctb-instrctA'/'instrctb'/'instrelp,_clearcmd,25)
  'SET FIELD CMD  _cmdl'
return
/* ---------------------------------------------------------------------
 * Write Header Lines (line area and line content)
 * ---------------------------------------------------------------------
 */
headerAll:
  #line=substr(#$_BUF._#BNO.$HEADER,scol,#lstwidth)
  if colortop1='' then say 'Colortop1 Lost'
  if colortop2='' then say 'Colortop2 Lost'
  call setLine 1,#laperd,#line
  if colortop1<>'' then call setTopColor 1
  if #$_BUF._#BNO.$HDR2=0 then return 1
  #line=substr(#$_BUF._#BNO.$HEADER2,scol,#lstwidth)
  call setLine 2,#laperd,#line
  if colortop2<>'' then call setTopColor 2
return 2
/* ---------------------------------------------------------------------
 * Write Buffer Line
 * ---------------------------------------------------------------------
 */
BufLine:
  #line=substr(#$_BUF._#BNO.lini,scol,#lstwidth)
  if #lch=='' then  ,
     call setline i,right(lini,#lal,'0'),#line
  else call setline i,#labnch,#line
  if #$_BUF.xbufcolindx1._#bno.lini>0 then do
    'SET COLOR LINEA.'i #$_BUF.XBUFCOLINDX1._#bno.lini
     if #$_BUF.XBUFCOLINDX2._#bno.lini>0 then ,
    'SET COLOR #$_BUF._#BNO.'i #$_BUF.XBUFCOLINDX2._#bno.lini
     #$_BUF.XBUFCOLOR._#bno.i=1
  end
  lini=lini+1                         /* set to next buffer line */
return
/* ---------------------------------------------------------------------
 * Write End of Data Line
 * ---------------------------------------------------------------------
 */
dummyLine:
 'SET FIELD LINEA.'i' _blk0'
 'SET FIELD #$_BUF._#BNO.'i' _blk0'
  _linArea.i=-1
return
/* ---------------------------------------------------------------------
 * Write Empty line into Screen (after End of Data)
 * ---------------------------------------------------------------------
 */
EndLine:
  botl=center(' End of Data ',#LSTWIDTH,'*')
  call setLine i,#lastar,botl
  if colorbot1<>'' then call setBotColor i
  botindx=i   /* bottom line is in screen line number ...  */
  lini=linc+9 /* Set beyond line count to init empty lines */
return
/* ---------------------------------------------------------------------
 * Set single Line in Buffer
 * ---------------------------------------------------------------------
 */
SetLine:
  parse arg indx,_la,_lc
 'SET FIELD LINEA.'indx' _la'
 'SET FIELD #$_BUF._#BNO.'indx' _lc'
  _linArea.indx=_la
  if #$_BUF.xbufcolor._#bno.indx>0 then do
    'SET COLOR LINEA.'indx colorlist1
    'SET COLOR #$_BUF._#BNO.'indx colorlist2
     #$_BUF.xbufcolor._#bno.indx=0
  end
return indx
/* ---------------------------------------------------------------------
 * Returning from a higher Buffer requires recovery from the current one
 * ---------------------------------------------------------------------
 */
RecoverScreen:
  parse arg usebuf,rmode   /* Buffer which has been left */
  if #bufactive=1 then nop
  else do
     CALL FSSCLOSE
     CALL FSSINIT 'FMTLIST'
  end
  call SCREENINIT rmode   /* re-Setup current Screen Definitions */
  if usebuf>0 then do
     do jb=1 to #$_BUF.$stack /* test for at least one valid buffer */
        if #$_BUF.jb.0>=0 then leave
     end
     if jb<=#$_BUF.$stack then do
        do until #$_BUF.usebuf.0>=0
           usebuf=usebuf-1
           if usebuf<1 then usebuf=#$_BUF.$stack
        end
       _#bno=usebuf   /* Set to this Buffer */
       parse var #$_BUF._#bno.0 linc'/'lino
       if lino='' then lino=1
     end
  end
  #lstWidth=#$_BUF._#BNO.$LWIDTH  /* Recover some screen metrics    */
  #lstHeigh=#$_BUF._#BNO.$LHEIGHT /* Recover some screen metrics    */
  if msglong=1 then Call FSSMessage #scrheight-1
  if botline<>'' then Call FSSFooter #$_BUF._#BNO.$Footer
  lino=Display(lino,scol,'FORCE') /* Force re-display of old Buffer */
return
/* ---------------------------------------------------------------------
 * Enter Key was pressed on LIST Screen
 * ---------------------------------------------------------------------
 */
enterkey:
  if LinecommandAPPL<>'' then if checkLineCommands()>0 then return 0
 'SET CURSOR CMD'
/* */
  wcmd=word(command,1)
  poscmd=pos(';',command)
  if poscmd>0 then command=substr(command,1,poscmd-1)
  if command<>'' then nop /* command provided */
     else return 0        /* no command provided */
  if wcmd='TOP' then lino=display(1,scol)
  else if abbrev('BOTTOM',wcmd,3) then lino=display(99999,scol)
  else if abbrev('START',wcmd,5) then ,
       if startx()=-16 then return -16 ; else nop
  else if abbrev('SPLIT',wcmd,5) then ,
       if startx()=-16 then return -16 ; else nop
  else if abbrev('SWAP',wcmd,4) then call swapx word(command,2)
  else if abbrev('QUIT',wcmd,4) then return -4
  else if abbrev('XQUIT',wcmd,2) then return -4
  else if abbrev('LOOKASIDE',wcmd,4)=1 then do
     if wordindex(command,2)=0 then return startx()
     if runrexx(substr(command,wordindex(command,2)))=4 then return 0
     if callError=1 then return 0
     if buffer.0>0 then if lookaside()=-16 then return -16
     return 0
  end
  else if _screen.lookaside_retention=1 then do
     if runRexx(command)=4 then return 0 /* recovered,no display */
     if callError=1 then return 0
     if buffer.0>0 then if lookaside()=-16 then return -16
     return 0
  end
  else if wcmd<>'REXX' & wcmd<>'RX' then do
     if runRexx(command)=4 then return 0 /* recovered,no display */
     if callError=1 then return 0
     if buffer.0>0 then do
        #$_BUF._#BNO.$HDR1=0
        #$_BUF._#BNO.$HDR2=0
        if topcolor>0 then call resetTopColor 'FORCE'
        call resetColors
        call fetchbuffer
        #$_BUF._#BNO.$COMMAND=called_rexx
        nxtbuf=_#bno+1    /* correct exec name, as it is no lookaside */
        #$_BUF.nxtbuf.$COMMAND=''
        lino=Display(1,1,'FORCE') /* Force display of new Buffer */
     end
     return 0
  end
  else do   /* Perform Call from Command Line */
     if runrexx(substr(command,wordindex(command,2)))=4 then return 0
     if callError=1 then return 0
     if lookaside()=-16 then return -16
     call zerrsm word(command,2)' command executed'
  end
return 0
/* ---------------------------------------------------------------------
 * Run REXX which was requested from the Command line
 * ---------------------------------------------------------------------
 */
runrexx:
  parse arg called_rexx exparms
  parse var exparms "'"nexparms"'"
  if nexparms='' then do
     parse var exparms '"'nexparms'"'
     if nexparms='' then nexparms=exparms
  end
  #$_BUF._#bno.0=linc'/'lino
  callError=0
  oldbuf=buffer.0
  buffer.0=0
/* ...... Prepare calling of REXX .................................. */
  _screen.FSSHELL=1   /* run as shell, in case FSSMENU/FSSCOLUM used */
  signal on syntax name nofunc
  nxtbuf=_#bno+1       /* save exec name in next buffer, even if not  */
  #$_BUF.nxtbuf.$COMMAND=called_rexx  /* lookaside, correct later     */
  lastScreen_appl=_SCREEN.$_SCREENAPPL
/* ...... Now call REXX ............................................ */
  if #$_BUF._#BNO.$CMDPREF='CMD' then  ,
     interpret "call "called_rexx" '"nexparms"'"
   else do
     interpret "call "#$_BUF._#BNO.$CMDPREF' 'called_rexx" '"nexparms"'"
     if result=-128 then interpret "call "called_rexx" '"nexparms"'"
  end
/* ...... Check if FSS was used .................................... */
  rexxrc=result
  signal off syntax
  _screen.FSSHELL=0
  ADDRESS FSS
  apc=applstat(lastScreen_appl,_SCREEN.$_SCREENAPPL)
  if apc=0 then return 0              /* same screen,     no recovery */
  if apc=1 then call recoverScreen ,0 /* both FMTLIST, short recovery */
  else do
     #bufactive=0      /* Buffer has been overwritten by other screen */
     call RecoverScreen ,1 /* full recovery */
     if topcolor>0 then call resettopColor lino
     #bufactive=1
     return 4    /* Screen was recovered and displayed */
  end
return 0
/* Error Exit, if called Rexx was not available  */
nofunc:
  buffer.0=oldbuf
  signal off syntax
  call zerrsm wcmd' invalid command'
  call zerrlm wcmd' is an invalid or unsupported command'
  callError=1
return 0
/* ---------------------------------------------------------------------
 * Open new Buffer and recover screen at the end
 * ---------------------------------------------------------------------
 */
Lookaside:
 #$_BUF.subbuf=#bufactive
 if botcolor>0 & botindx>0 then call resetBotColor botindx
 if topcolor>0 then call resettopColor 'FORCE'
 call resetColors
 lastScreen_appl=_SCREEN.$_SCREENAPPL
 fmbuf=FMTLIST(lineareaLen,LineareaChar,,,LinecommandAPPL)
 if fmbuf=-16 then return -16
 apc=applstat(lastScreen_appl,_SCREEN.$_SCREENAPPL)
 if apc=0 then return 0              /* same screen,     no recovery */
 if apc=1 then call recoverScreen ,0 /* both FMTLIST, short recovery */
    else call RecoverScreen fmbuf,1   /* changed type, full recovery */
return 0
/* ---------------------------------------------------------------------
 * Check on return if other screen was overlaying current one
 * ---------------------------------------------------------------------
 */
applstat:
  if arg(1)=arg(2) then return 0
  parse value arg(1)','arg(2) with fappl' 'fdat','sappl' 'sdat
  if fappl=sappl then return 1
return 2
/* ---------------------------------------------------------------------
 * Check if there was a line command issued
 * ---------------------------------------------------------------------
 */
checkLineCommands:
  _licmdindx=0
  do for 80 /* until _licmdindx<0 */
     licmd=getLineCmd(_licmdindx+1)
     if _licmdindx<0 then leave
     setcolor1=0
     setcolor2=0
     rc=LineCMDLocal(LinecommandAPPL,licmd)
     if rc>4 then rc=LineCMD(LinecommandAPPL,licmd)
  /* _linecmd and buflino is set in LineCMD() */
     if rc>4 then do    /* _linecmd comes from getLineCMD */
        call zerrsm 'Invalid Line Command '_lineCmd
        if msglong=1 then ,
      call zerrLM _lineCMD' is an invalid Line Command (see LINECMD() )'
       'SET CURSOR LINEA.'_licmdindx
        return 4
     end
     if rc<=4 then 'SET FIELD LINEA.'_licmdindx _linArea._licmdindx
     if rc=4 then do
       'SET FIELD #$_BUF._#BNO.'_licmdindx' newline'
        #$_BUF._#BNO.buflino=newline
     end
     if setcolor1>0 then do
       'SET COLOR LINEA.'_licmdindx setcolor1
        #$_BUF.xbufcolindx1._#bno.buflino=setcolor1
        #$_BUF.xbufcolor._#bno._licmdindx=1
     end
     if setcolor2>0 then do
       'SET COLOR #$_BUF._#BNO.'_licmdindx setcolor2
        #$_BUF.xbufcolindx2._#bno.buflino=setcolor2
        #$_BUF.xbufcolor._#bno._licmdindx=1
     end
  end
return 0
/* ---------------------------------------------------------------------
 * Retrieve Last Command
 * ---------------------------------------------------------------------
 */
LastCommand:
  llcm=lastCommand.LCMptr
  if llcm<1 then llcm=lastCommand.0
  if llcm=0 then return
  _cml=overlay(lastCommand.llcm,_clearcmd,1)
 'SET FIELD CMD _cml'
  lastCommand.LCMPTR=llcm-1
  return
/* ---------------------------------------------------------------------
 * Check Local line Command in rexx calling FMTLIST
 * ---------------------------------------------------------------------
 */
LineCMDLocal: Procedure expose zerrsm zerrlm newline refresh
 parse arg appl,licmd
  zerrsm=''
  zerrlm=''
  parse value licmd with linecmd';'lino';'licmd
  signal on syntax name nolincmd
  interpret 'lrc='appl'_'linecmd'(licmd)'
  signal off syntax
return lrc
noLincmd:
  signal off syntax
return 8
/* ---------------------------------------------------------------------
 * Calculate Scroll Amount
 * ---------------------------------------------------------------------
 */
scroll:
  parse arg incr' 'ign
  if incr='M' then incr=99999
  if datatype(incr)<>'NUM' then do
     if lino<=1 then incr=#lstheight-1
     else do
        incr=#LSTHEIGHT
        if #$_BUF._#BNO.$HDR1=1 then incr=incr-1
     end
     if #$_BUF._#BNO.$HDR2=1 then incr=incr-1
  end
return incr
/* ---------------------------------------------------------------------
 * Create New Buffer on top of the other
 * ---------------------------------------------------------------------
 */
startx:
  #$_BUF._#bno.0=linc'/'lino
  buffer.0=3
  buffer.1='Split Screen Buffer '_fssinit+1' opened'
  buffer.2='Run Command from Command Line'
  buffer.3='-----------------------------'
  if lookaside()=-16 then return -16
  call zerrsm word(command,2)' Split Screen ended'
return 0
/* ---------------------------------------------------------------------
 * Swap to a existing Buffer
 * ---------------------------------------------------------------------
 */
swapx:
 parse arg bufn
  if bufn='' then bufn='NEXT'     /* Default SWAP parm is NEXT       */
  #$_BUF._#bnO.0=linc'/'lino      /* save current Buffer information */
  _bufno=#$_BUF.$stack            /* number of avaialable buffers    */
  if bufn='NEXT' then bufn=_#bno  /* Default SWAP parm is NEXT       */
  else if abbrev('LAST',bufn,2)=1 then bufn=_bufno-1
  else if abbrev('FIRST',bufn,2)=1 then bufn=0
  else if datatype(bufn)='NUM' then bufn=bufn-1 /* for number -1     */
  else bufn=_#bno                 /* for nonsense use NEXT           */
  if bufn>_bufno then bufn=_bufno-1 /* some boundary checks          */
  if bufn<1      then bufn=0       /* set current Buffer number      */
/* buffer sits on current buffer, or buffer to display -1            */
  _#bno=bufn
  do for _bufno until #$_BUF._#bnO.0>=0
     _#bno=_#bno+1
     if _#bno>_bufno then _#bno=1 /* Buffer number > max, then circle*/
  end
  parse var #$_BUF._#bnO.0 linc'/'lino
  if lino='' then lino=1
  call resetColors
/* Display requested Buffer */
  lino=Display(lino,scol,'FORCE') /* Force re-display of old Buffer */
return
/* ---------------------------------------------------------------------
 * Reset Colors if new or swap screen is displayed
 * ---------------------------------------------------------------------
 */
resetColors:
/* Reset line area and line colors */
  do ixt=1 to  #lstheight
    'SET COLOR LINEA.'ixt colorlist1
    'SET COLOR #$_BUF._#BNO.'ixt colorlist2
  end
return
/* ---------------------------------------------------------------------
 * Create Panel Text Field
 * ---------------------------------------------------------------------
 */
fsstextl:
  parse arg row,col,attr,txt
  col=col+1
  _txt=txt
 'TEXT 'row col attr' _txt'
return 1
/* ---------------------------------------------------------------------
 * Create Panel Input Field
 * ---------------------------------------------------------------------
 */
fssfieldL:
  parse arg row,col,attr,field,vlen,vinit
  len=length(vinit)
  if len=0 then vinit=' '
  if len<=1 then vinit=copies(vinit,vlen)
     else vinit=Left(vinit,vlen)
  col=col+1
 'FIELD  'row col attr field vlen ' vinit'
return 1
/* ---------------------------------------------------------------------
 * Fetch Values of all Input Fields
 * ---------------------------------------------------------------------
 */
GetFields:
 'GET AID AID'
 'GET FIELD CMD _CMD'
 'GET FIELD STATS _STATS'
  command=strip(strip(translate(_cmd,,'_')))
  ppos=pos(';',command)
  if ppos>0 then command=strip(substr(command,1,ppos-1))
  if command<>'' then do
     lastcommand.0=lastcommand.0+1
     lcm=lastcommand.0
     lastcommand.lcm=command
     lastCommand.LCMptr=lcm
     command=translate(command)
  end
 'SET FIELD CMD  _clearcmd'
  if _screen.ftrace=1 then call fssFtrace "Command Line '"command"'"
return
/* ---------------------------------------------------------------------
 * Check for Line Commands
 * ---------------------------------------------------------------------
 */
GetLineCMD:
  if header2<>''        then buflino=lino-3
     else if header<>'' then buflino=lino-2
     else if lino=1     then buflino=-1 /* no header, top line */
     else buflino=lino-1              /* no header */
  do _licmdindx=arg(1) to #lstheight
     buflino=buflino+1
    'GET FIELD LINEA.'_LICMDINDX' _LINA'
     if _lina==_linarea._licmdindx then iterate
     if _lina=='' then leave
     _linecmd=extractlincmd(_lina, _linarea._licmdindx)
     if strip(_linecmd)=='' then do
       'set field linea.'_licmdindx _linarea._licmdindx
        iterate
     end
    'GET FIELD #$_BUF._#BNO.'_LICMDINDX' _LINE'
     return _linecmd';'buflino';'_line
  end
  _licmdindx=-1
return ''
/* ---------------------------------------------------------------------
 * Extract Line Command from Line Area
 * ---------------------------------------------------------------------
 */
extractLincmd:
  selcmd=''
  do _li=1 to #lal
     st1=substr(arg(1),_li,1)
     st2=substr(arg(2),_li,1)
     if st1==st2 then iterate
     selcmd=selcmd''st1
  end
  if datatype(selcmd)='NUM' then return ''
return translate(strip(selcmd))
/* ---------------------------------------------------------------------
 * Move BUFFER Stem into internal Buffer
 * ---------------------------------------------------------------------
 */
fetchBuffer:
  drop #$_BUF._#BNO.
  if datatype(buffer.0)<>'NUM' then do
     buffer.0=2
     buffer.1='BUFFER.0 is not set, number of entries necessary'
     buffer.2='FMTLIST does not show Buffer content'
  end
  do k=0 to BUFFER.0
     #$_BUF._#BNO.k=buffer.k
  end
  linc=#$_BUF._#BNO.0
  if datatype(help.0)='NUM' & help.0>0 then return
  call fetchhelp
return
/* ---------------------------------------------------------------------
 * INIT FSS and setup List Screen
 * ---------------------------------------------------------------------
 */
SCREENINIT:
  cmdpref=#$_BUF._#BNO.$cmdPref' ==>'
  cmdoffs=length(cmdpref)+2
  statsoffset=#scrwidth-28-1
  cmdlen=statsoffset-cmdoffs
/* FSS requires offset as real offset of text or fileld
   as this is not easy readable we re-calculate in the FSSTEXT/FSSFIELD
   function. The call has now real offset, which means the 1. byte
   contains attribute byte, byte starts with real output value
 */
  topdata=center(' Top of Data ',#LSTWIDTH,'*')
  toplina=#lastar
  blk=copies(' ',#LSTWIDTH)
  loff=2+#lal
  if #bufactive=1 then do
    'SET CURSOR CMD'
     return
  end
  call fsstextL  1,1,  #PROT+#HI+COLORCMD,cmdpref
  call fssfieldL 1,cmdoffs,  #HI+COLORCMD,CMD,cmdlen,"_"
  call fssfieldL 1,STATSOFFSET,#PROT+#HI+colorstats,stats,28," "
  do j=1 to #lstheight
     call fssfieldL j+1,1, colorlist1,'LINEA.'j,#LAL,#lablnk
     call fssfieldL j+1,loff,#prot+#hi+colorlist2,'#$_BUF._#BNO.'j,#LSTWIDTH,blk
  end
 'SET CURSOR CMD'
return
/* ---------------------------------------------------------------------
 * Display Help Information
 * ---------------------------------------------------------------------
 */
displayhelp:
do hi=0 to help.0
   buffer.hi=help.hi
end
return
/* ---------------------------------------------------------------------
 * Load Default Help, if nothing is provided in global var HELP.
 * ---------------------------------------------------------------------
 */
fetchhelp:
drop help.
help.1=copies('-',#LSTWIDTH)
help.2='Display REXX Results in Formatted List Screens'
help.3=copies('-',#LSTWIDTH)
help.4='The results to be displayed need to be stored in the'
help.5='Stem Variable BUFFER.n, n is line number. BUFFER.0 must'
help.6='contain the number of lines stored in Buffer.'
help.7=' '
help.8='To Display the Buffer CALL FMTLIST.'
help.9=' '
help.10='FMTLIST supports the following PF Keys:'
help.11='   PF1    Display Help '
help.12='   PF3    Return to previous FMTLIST Buffer if any, or leave'
help.13='   PF7    Scroll Forward, full page or lines given in Command   Line'
help.14='   PF8    Scroll Backward, full page or lines given in Comman  d Line'
help.15='   PF10   Show Output shifted 50 Bytes to the left'
help.16='   PF11   Show Output shifted 50 Bytes to the right'
help.17='   PF12   Recall last command'
help.0=17
return
/* ---------------------------------------------------------------------
 * INIT Environment, set 3270 screen size
 * ---------------------------------------------------------------------
 */
fmtLInit:
/* --- #bufactive=1: all subsequent FMTLIST Calls use same buffer --- */
  #bufactive=0
  if _screen.FSSHELL=1 then _screen.FSSHELL=0
  else if datatype(#$_BUF.SUBBUF)='NUM' then do
    'GET FIELD #$_BUF._#BNO.1 isdefined'   /* is field really defined */
     if isdefined<>'' then #bufactive=1
  end
  #$_BUF.SUBBUF=0
  if #bufactive=1 then call fssFastInit 'FMTLIST'
  else do
     #bufactive=0
     _FSSFUNCKEYS.INIT=0       /* Init STEM for EXPOSE */
     CALL FSSCLOSE
     CALL FSSINIT 'FMTLIST'
  end
  if symbol('LastCommand.0')<>'VAR' then do
     LastCommand.0=0
     LastCommand.LCMptr=0
  end
  #lal=5
  _clearcmd=Copies('_',54)
  lastinstr=SYSVAR('RXINSTRC')
  if datatype(lineareaLen)='NUM' then #LAL=LineareaLen
  if #lal>12 then #lal=12
  if #lal<1 then #lal=1
  #lal=trunc(#lal)
  #lastar=copies('*',#LAL)
  #laperd=copies('.',#LAL)
  #lablnk=copies(' ',#LAL)
  _blk0=''
  msgset=0
/* ----- Screen Dimensions and Definitions --- */
  #scrHeight=FSSHeight()     /* Number of lines   in 3270  screen */
  #scrWidth=FSSWidth()       /* Number of columns in 3270  screen */
  if #scrHeight*#scrWidth > 4096 then do
     _twidth=trunc(4096/#scrheight)
     _theight=#scrheight
     if _twidth<80 then do
        _twidth=80
        _theight=trunc(4096/_twidth)
        if _theight>#scrheight then _theight=#scrheight
     end
     #scrwidth=_twidth
     #scrheight=_theight
  end
  #lstWidth =#scrwidth-3-#LAL  /* Number of Columns in list area  */
  #lstHeight=#scrHeight-1      /* Number of Lines   in list area  */
  #lablln=copies(' ',#LSTWIDTH)
  if LineareaChar==''then #lch='' /* Line Area default is numbering */
  else do
     #lch=substr(LineareaChar,1,1)
     #labnch=copies(#lch,#lal)
  end
/* ----- List Area Color Setting ------------- */
  _screen.FMTLIST=1  /* first setup  _screen. variable! */
  colorbot1 =GetScrIni('Color.bot1',#red)
  colorbot2 =GetScrIni('Color.bot2',#blue)
  colortop1 =GetScrIni('Color.top1',#red)
  colortop2 =GetScrIni('Color.top2',#blue)
  colorlist1=GetScrIni('Color.list1',#white)
  colorlist2=GetScrIni('Color.list2',#green)
  colorcmd  =GetScrIni('Color.cmd',#red)
  colorstats=GetScrIni('Color.stats',#white)
  botcolor=0
  topcolor=0
/* ----- Others ------------------------------ */
  if LinecommandAPPL='' then LinecommandAPPL=_screen.FMTLIST.APPLname
     else _screen.FMTLIST.APPLname=LinecommandAPPL
  botline=GetScrIni('Footer','')
  if botline<>'' then #lstHeight=#lstHeight-1
  msglong=GetScrIni('Message',0)
  if msglong=1 then #lstHeight=#lstHeight-1
  if datatype(buffer.0)<>'NUM' then call fetchHelp
  if datatype(_fssinit)<>'NUM' then _fssinit=1
     else _fssinit=_fssinit+1
/* ----- Headers, Buffer Number, etc. -------- */
  _#BNO=_fssinit        /* keep it as local variable */
  if header='' then #$_BUF._#BNO.$HDR1=0
  else do
     #$_BUF._#BNO.$HDR1=1
     #$_BUF._#BNO.$HEADER=header
  end
  if header2='' then #$_BUF._#BNO.$HDR2=0
  else do
     #$_BUF._#BNO.$HEADER2=header2
     #$_BUF._#BNO.$hdr2=1
  end
  #$_BUF._#BNO.$Footer=botline
  if datatype(#$_BUF.$stack)<>'NUM' then #$_BUF.$stack=1
     else #$_BUF.$stack=#$_BUF.$stack+1
  #$_BUF._#BNO.$LWIDTH =#lstWidth      /* Save some screen metrics    */
  #$_BUF._#BNO.$LHEIGHT=#lstHeight     /* Save some screen metrics    */
  #$_BUF._#BNO.$CMDPREF=  ,
     translate(GetscrIni('CommandPrefix','CMD','RESET'))
  #$_BUF._#BNO.$APPLID=_SCREEN.$_SCREENAPPL
/* ----- INIT Screen definitions ------------------------------------ */
  call SCREENINIT
/* ----- Now we can define Text and Fields -------------------------- */
  if msglong=1 then Call FSSMessage #scrheight-1
  if botline<>'' then Call FSSFooter botline
  #bufactive=1   /* Now buffer is active */
return
