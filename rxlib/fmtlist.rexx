/* ---------------------------------------------------------------------
 * Display Buffer
 *   Content must be stored in STEM BUFFER.
 *   Buffer.0    contains number of lines
 *   Buffer.n    each line in stem as single entry n=1,2,...,maximum
 * .................................. Created by PeterJ on 25. July 2019
 * ---------------------------------------------------------------------
 */
fmtlist: procedure  ,
         expose LastCommand. buffer. _screen. _refresh _#bno (public)
signal off syntax
parse arg lineareaLen,LineareaChar,header,header2 ,
         ,LinecommandAPPL
  signal off syntax
  call import FSSAPI
  Address FSS
  call fmtLinit             /* init and set size of 3270 screen  */
  if buffer.0='STACK' then call fetchQueue
     else call fetchBuffer  /* Copy Buffer to internal buffer    */
/* ..... Display first Buffer ..... */
  lino=display(1,1)
/* .....................................................................
 * Screen Handler manages Buffer until PF3/PF4
 * ...........*/
  _rc=screenHandler()   /* Screen Handler manages input keys */
  _#bno=_#bno-1
  _refresh=1
return 0
/* ---------------------------------------------------------------------
 * Screen Handler
 *   handles ENTER and PF Keys
 * ---------------------------------------------------------------------
 */
screenHandler:
  do forever
     call InstructionLogA
     callerror=0
     _pfkey=fssrefresh('CHAR') /* Display Screen, return PF-key/Enter */
     call wait 2
     if msgset=1 then call msgreset
     if _pfkey='PF12' then do ; call LastCommand ; iterate ; end
     call getFields
     if _pfkey='PF03' then leave;
        else if _pfkey='PF15' then leave;
        else if _pfkey='PF04' then return -16
        else if _pfkey='PF16' then return -16
        else if _pfkey='PF01' then do ; call DisplayHelp ; iterate ; end
/*  else if _pfkey='ENTER' then do */
/* Run always Enterkey processing in case there are (line) commands */
     $erc=enterKey()
     if $erc=-4  then leave
        else if $erc=-12 then leave  /* PF03 pressed in enterkey proc */
        else if $erc=-16 then return -16
        else if $erc=-20 then do ; call DisplayHelp ; iterate ; end
        else call check4Recovery
     if _pfkey='PF08' then lino=display(lino+scroll(command),scol)
     else if _pfkey='PF07' then lino=display(lino-scroll(command),scol)
     else if _pfkey='PF11' then lino=display(lino,scol+50)
     else if _pfkey='PF10' then lino=display(lino,scol-50)
/*  call InstructionLogB */
  end
return 0
/* ---------------------------------------------------------------------
 * Check if Recovery is needed
 * ---------------------------------------------------------------------
 */
check4Recovery:
  if _refresh>0 & datatype(_refresh)='NUM' then do
     if _refresh>=100 then do
        _refresh=_refresh%100
        call Fetchbuffer
     end
     if _refresh=10 then call scrRecover 'KEEP'
        else call scrRecover
     lino=display(lino,scol,'FORCE')
     _refresh=0
  end
  else if _refreshLA=1 then lino=display(lino,scol,'FORCE')
return
InstructionLogA:
  instrctA=SYSVAR('RXINSTRC')
  instrelp=time('e')
return
InstructionLogB:
  instrCtB=SYSVAR('RXINSTRC')
  instrelp=trunc(time('e')-instrelp,3)
  _cmdl=overlay(';'instrctb-instrctA'/'instrctb'/'instrelp,_clearcmd,25)
  call zerrlm _cmdl
return
/* ---------------------------------------------------------------------
 * Reset Screen Messages
 * ---------------------------------------------------------------------
 */
msgreset:
  call statspart
  if msglong=1 then call ZERRLM ' '
  msgset=0
return
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
  if lini<=1 then do
     i=setLine(1,#lastar,topdata)
     call SetTopColor 1
  end
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
setTopColor:
parse arg tlino
  'SET COLOR LINEA.'tlino colortop1
  'SET COLOR _LIST.'tlino colortop2
   topcolor=100
return
/* ---------------------------------------------------------------------
 * Reset Color of first line and Header Lines
 * ---------------------------------------------------------------------
 */
resetTopColor:
 'SET COLOR LINEA.1' colorlist1
 'SET COLOR _LIST.1' colorlist2
  topcolor=0
return
/* ---------------------------------------------------------------------
 * Reset Color of List Line(s)
 * ---------------------------------------------------------------------
 */
resetBotColor:
  botcolor=arg(1)
 'SET COLOR LINEA.'botcolor colorlist1
 'SET COLOR _LIST.'botcolor colorlist2
  botcolor=0
return
/* ---------------------------------------------------------------------
 * Set Color of Last Line
 * ---------------------------------------------------------------------
 */
setBotColor:
  botcolor=arg(1)
 'SET COLOR LINEA.'botcolor colorbot1
 'SET COLOR _LIST.'botcolor colorbot2
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
 * Write Buffer Line
 * ---------------------------------------------------------------------
 */
BufLine:
  #line=substr(_LIST.lini,scol,#lstwidth)
  if #lch=='' then call setline i,right(lini,#lal,'0'),#line
    else call setline i,#labnch,#line
  if _LIST.xbufcolindx1.lini>0 then do
    'SET COLOR LINEA.'i _LIST.XBUFCOLINDX1.lini
     if _LIST.XBUFCOLINDX2.lini>0 then ,
    'SET COLOR _LIST.'i _LIST.XBUFCOLINDX2.lini
     _LIST.XBUFCOLOR.i=1
  end
  lini=lini+1                         /* set to next buffer line */
return
/* ---------------------------------------------------------------------
 * Write End of Data Line
 * ---------------------------------------------------------------------
 */
dummyLine:
 'SET FIELD LINEA.'i' _blk0'
 'SET FIELD _LIST.'i' _blk0'
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
  if _savela.indx='' then 'SET FIELD LINEA.'indx' _la'
  else do
    'SET FIELD LINEA.'indx' _savela.indx'
     _savela.indx=''
  end
 'SET FIELD _LIST.'indx' _lc'
  _linArea.indx=_la
  if _LIST.xbufcolor.indx>0 then do
    'SET COLOR LINEA.'indx colorlist1
    'SET COLOR _LIST.'indx colorlist2
     _LIST.xbufcolor.indx=0
  end
return indx
/* ---------------------------------------------------------------------
 * Enter Key was pressed on LIST Screen
 * ---------------------------------------------------------------------
 */
enterkey:
  if LinecommandAPPL<>'' then do
     lcr=checkLineCommands()
     if lcr=-12 then return -12  /* PF03 */
     if lcr=-16 then return -16  /* PF04 */
     if lcr=-20 then return -20  /* PF01 */
  end
 'SET CURSOR CMD'
/* */
  wcmd=word(command,1)
  if command<>'' then nop /* command provided */
     else return 0        /* no command provided */
  if wcmd='TOP' then lino=display(1,scol)
  else if abbrev('BOTTOM',wcmd,3) then lino=display(99999,scol)
  else if abbrev('QUIT',wcmd,4) then return -4
  else if abbrev('HELP',wcmd,3) then call DisplayHelp
  else if abbrev('RESET',wcmd,3) then do
     call colorreset
     return 0
  end
  else do
     rrc=runRexx(command)
     if callError=1 then return 0
     if rrc>0 then do
        call zerrsm wcmd' invalid command'
        call zerrlm wcmd' is an invalid or unsupported primary command'
     end
     call check4Recovery
  end
return 0
/* ---------------------------------------------------------------------
 * Run REXX which was requested from the Command line
 * ---------------------------------------------------------------------
 */
runrexx:
  parse arg called_rexx exparms
  parse var exparms "'"nexparms"'"
  if nexparms='' then parse var exparms '"'nexparms'"'
  if nexparms='' then nexparms=exparms
  _rebuild=0
/* ...... Now call REXX ............................................ */
  signal on syntax name nofunc
  if #CMDPREF='CMD' then interpret "call "called_rexx" '"nexparms"'"
     else interpret "call "#CMDPREF'_primary 'called_rexx" '"nexparms"'"
  if _rebuild=1 then call resetcolors
return result
/* Error Exit, if called Rexx was not available  */
nofunc:
  signal off syntax
  call zerrsm wcmd' invalid line command'
  call zerrlm wcmd' is an invalid or unsupported line command'
  callError=1
return 0
/* ---------------------------------------------------------------------
 * Reset Colors of entire Buffer
 * ---------------------------------------------------------------------
 */
colorreset:
  do kli=1 to linc
     if _LIST.xbufcolindx1.kli>0 then _LIST.xbufcolindx1.kli=0
     if _LIST.xbufcolindx2.kli>0 then _LIST.xbufcolindx2.kli=0
  end
  _refreshLA=1
return
/* ---------------------------------------------------------------------
 * Check if there was a line command issued
 * ---------------------------------------------------------------------
 */
checkLineCommands:
  _licmdindx=0
  _refresh=0
  _refreshLA=0
  do for 80 /* until _licmdindx<0 */
     licmd=getLineCmd(_licmdindx+1)
     if _licmdindx<0 then leave /*_licmdindx contains list # in screen*/
     setcolor1=0
     setcolor2=0
     rc=8
     #action=''
     rc=LineCMDLocal(LinecommandAPPL,licmd)
     if #action='PF03' then return -12
     if #action='PF04' then return -16
     if #action='PF01' then return -20
     if rc>4 then do    /* _linecmd comes from getLineCMD */
        call zerrsm 'Invalid Line Command '_lineCmd
        if msglong=1 then ,
      call zerrLM _lineCMD' is an invalid Line Command'
       'SET CURSOR LINEA.'_LICMDINDX
     end
     if rc<=4 then do
       'SET FIELD LINEA.'_LICMDINDX _linArea._LICMDINDX
        if zerrsm<>'' then call zerrsm zerrsm
        if msglong=1 & zerrlm<>'' then call zerrLM zerrlm
     end
     if rc=4 &newline<>'' then do
       'SET FIELD _LIST.'_LICMDINDX' newline'
        _LIST.buflino=newline
     end
    'GET FIELD _LIST.'_LICMDINDX' _LINE'
     if setcolor1>0 then do
       'SET COLOR LINEA.'_LICMDINDX setcolor1
        _LIST.xbufcolindx1.buflino=setcolor1
        _LIST.xbufcolor._licmdindx=1
     end
     if setcolor2>0 then do
       'SET COLOR _LIST.'_LICMDINDX setcolor2
        _LIST.xbufcolindx2.buflino=setcolor2
        _LIST.xbufcolor._licmdindx=1
     end
     if _refresh>0 then return 4
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
 * Retrieve Line Command
 *   _licmdindx    List line number in screen (not in buffer)
 *   #lstrow       List line number in screen kept for recall
 *   #fssrow       line number in screen (not just in LIST)
 *   lino          line number in buffer
 *   linc          maximum line number in buffer
 * ---------------------------------------------------------------------
 */
LineCMDLocal:
  do ksi=1 to _licmdindx    /* save previous line commands, if any */
     _savela.ksi=''
  end
  xlino=lino
  do ksi=_licmdindx+1 to #lstHeight    /* save remaining line area */
    xlino=xlino+1
    if xlino>linc then leave
   'GET FIELD LINEA.'ksi' _savela.ksi'
  end
PrimCMDLocal:
/* try to perform line command, in arg(1) is prefix for linecmd */
  applid=arg(1)
/*  if datatype('_licmdindx')<>'NUM' then _licmdindx=0 */
  #lstrow=_licmdindx     /* List Row selected, refers to Screen Row   */
  #fssrow=#lstrow+#lstOFF /* physical row on screen, needed 4 lineupd */
  lrc=LineCMDLocalR(applid,arg(2))  /* complete line */
return lrc
/* ---------------------------------------------------------------------
 * Perform Line Command, embedded in Procedure to keep local Variables
 * ---------------------------------------------------------------------
 */
LineCMDLocalR: Procedure expose  ,
   zerrsm zerrlm msglong FSSPARMS._#VAR.#ZERRLM newline #action ,
   _refresh _#bno setcolor1 setcolor2 (public) #fssrow #lstrow #lal
 parse arg appl,licmd
  zerrsm=''
  zerrlm=''
  newline=''
  parse value licmd with linecmd';'llino';'licmd
  signal on syntax name nolincmd
  interpret 'lrc='appl'_'linecmd'(licmd,llino)'
  signal off syntax
return lrc
noLincmd:
  signal off syntax
  callError=1
  setcolor1=61952
/* say 'ERROR' llino appl'_'linecmd licmd */
return 8
/* ---------------------------------------------------------------------
 * Display input line for certain Lino
 *   #lstrow       List row number in screen kept for recall
 *   #fssrow       row number in screen (not just in LIST)
 * ---------------------------------------------------------------------
 */
lineEdit:
   parse arg _edln
   flino=#fssrow     /* #fssrow is the physical row on screen  */
   _linal=#lal+3     /* offset in row=line area length +3      */
 ADDRESS FSS         /* open EDIT mask on it and set cursor    */
   if datatype(_edln)<>'NUM' then _edln=50
   _maxed=FSSWidth()-_linal+1   /* max available input field   */
   if _edln>_maxed then _edln=_maxed
  'GET FIELD _LIST.'#lstrow' _curl'
  'FIELD  'flino _linal '61952 EXPDSN '_edln' 'copies('_',_edln)
  'SET CURSOR EXPDSN'
   if zerrlm<>'' then call zerrlm zerrlm
  'REFRESH'
   #action=fsskey('CHAR')          /* wait for editing */
   if #action<>'ENTER' then return ''
  'GET FIELD _LIST.'#lstrow' _lc'
   if _lc=_curl then return ''
   _lc=strip(translate(left(_lc,_edln),,'_'))
return translate(_lc)
/* ---------------------------------------------------------------------
 * Calculate Scroll Amount
 * ---------------------------------------------------------------------
 */
scroll:
  parse arg incr' 'ign
  if incr='M' then incr=99999
  if datatype(incr)<>'NUM' then do
     if lino<=0 then incr=#lstheight-1
        else incr=#LSTHEIGHT
  end
return incr
/* ---------------------------------------------------------------------
 * Reset Colors if new or swap screen is displayed
 * ---------------------------------------------------------------------
 */
resetColors:
/* Reset line area and line colors */
  do ixt=1 to  #lstheight
    'SET COLOR LINEA.'ixt colorlist1
    'SET COLOR _LIST.'ixt colorlist2
     _list.xbufcolindx1.ixt=0
     _list.xbufcolindx2.ixt=0
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
return
/* ---------------------------------------------------------------------
 * Check for Line Commands
 * ---------------------------------------------------------------------
 */
GetLineCMD:
  if lino<=1 then buflino=lino-2 /* just if TOP OF DATA is displayed */
     else buflino=lino-1    /* bufno is index in Buffer. stem */
  do _licmdindx=arg(1) to #lstheight
     buflino=buflino+1
    'GET FIELD LINEA.'_LICMDINDX' _LINA'
     if _lina==_linarea._licmdindx then iterate
     if _lina=='' then leave
     _linecmd=extractlincmd(_lina, _linarea._licmdindx)
     _linecm2=filter(_linecmd,'.*-+=')
     if strip(_linecm2)=='' then do
       'set field linea.'_licmdindx _linarea._licmdindx
        _refreshLA=1
        iterate
     end
    'GET FIELD _LIST.'_LICMDINDX' _LINE'
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
  if strip(selcmd)='' then return ''
  if datatype(selcmd)='NUM' then return ''
return translate(strip(selcmd))
/* ---------------------------------------------------------------------
 * Move BUFFER Stem into internal Buffer
 * ---------------------------------------------------------------------
 */
fetchBuffer:
/*drop _LIST.  */
  if datatype(buffer.0)<>'NUM' then do
     buffer.0=2
     buffer.1='BUFFER.0 is not set, number of entries necessary'
     buffer.2='FMTLIST does not show Buffer content'
  end
  do k=0 to BUFFER.0
     _LIST.k=buffer.k
  end
  linc=_LIST.0
return
/* ---------------------------------------------------------------------
 * Move BUFFER Queue into internal Buffer
 * ---------------------------------------------------------------------
 */
fetchQueue:
  linc=queued('T')
  if linc=0 then do
     _list.0=2
     _list.1='BUFFER.0 is not set, number of entries necessary'
     _list.2='FMTLIST does not show Buffer content'
     return
  end
  do k=1 to linc
     PULL bline
     _LIST.k=bline
  end
  _LIST.0=linc
return
/* ---------------------------------------------------------------------
 * Recover Screen if returning from another FMT Screen
 * ---------------------------------------------------------------------
 */
SCRRECOVER:
  CALL FSSCLOSE
  CALL FSSINIT 'FMTLIST'
  call screeninit
  if botline<>'' then Call FSSFooter botline
  call statspart
  if arg(1)='KEEP' then do
     call zerrsm zerrsm
     call zerrlm zerrlm
  end
return
/* ---------------------------------------------------------------------
 * INIT FSS and setup List Screen
 * ---------------------------------------------------------------------
 */
ScrEENINIT:
   cmdpref=#CMDPREF' ==>'
   cmdoffs=length(cmdpref)+2
   statsoffset=#scrwidth-28-1
   if titleA='' then cmdlen=statsoffset-cmdoffs
      else cmdlen=#scrwidth-cmdoffs-1
 /* FSS requires offset as real offset of text or fileld
    as this is not easy readable we re-calculate in the FSSTEXT/FSSFIELD
    function. The call has now real offset, which means the 1. byte
    contains attribute byte, byte starts with real output value
  */
   topdata=center(' Top of Data ',#LSTWIDTH,'*')
   toplina=#lastar
   blk=copies(' ',#LSTWIDTH)
   loff=2+#lal
   topl=1
   if titleA<>'' then do
     call fsstextL topl,1,#PROT+#HI+#White,center(' 'titleA' ',statsoffset-1,'-')
      topl=topl+1
   end
   do j=1 to #lstheight
      call fssfieldL j+#lstOFF,1, colorlist1,'LINEA.'j,#LAL,#lablnk
      call fssfieldL j+#lstOFF,loff,#prot+#hi+colorlist2,'_LIST.'j,#LSTWIDTH,blk
   end
   call fsstextL  topl,1,  #PROT+#HI+COLORCMD,cmdpref
   call fssfieldL topl,cmdoffs,  #HI+COLORCMD,CMD,cmdlen,"_"
   if _header1=1 then  ,
      call fsstextL topl+1,#lal+2,#PROT+#blue,strip(#header1,'T')
   if _header2=1 then  ,
      call fsstextL topl+2,#lal+2,#PROT+#blue,strip(#header2,'T')
   call fssfieldL 1   ,STATSOFFSET,#PROT+#HI+colorstats,stats,28," "
   if msglong=1 then Call FSSMessage #scrheight-1
   if botline<>'' then Call FSSFooter botline
  'SET CURSOR CMD'
return
/* ---------------------------------------------------------------------
 * Display Help Information
 * ---------------------------------------------------------------------
 */
displayhelp:
  _licmdindx=0   /* Set line command row to 0, it's a primary command */
  rrc=PrimCMDLocal(LinecommandAPPL,'help')
  if callError=1 | rrc>4 then do
     call zerrsm 'Help System not defined'
     call zerrlm 'Help System not defined'
  end
  call check4Recovery
return 0
/* ---------------------------------------------------------------------
 * INIT Environment, set 3270 screen size
 * ---------------------------------------------------------------------
 */
fmtLInit:
  CALL FSSCLOSE
  CALL FSSINIT 'FMTLIST'
  if symbol('LastCommand.0')<>'VAR' then do
     LastCommand.0=0
     LastCommand.LCMptr=0
  end
  #lal=5
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
  #scrrow=2
  #lstoff=1
/* ----- Others ------------------------------ */
  botline=GetScrIni('Footer','')
  if botline<>'' then #lstHeight=#lstHeight-1
  Titlea=GetScrIni('Title1','')
  if titlea<>'' then do
     #lstOff=#lstOff+1
     #lstHeight=#lstHeight-1
     #scrrow=#scrrow+1
  end
  msglong=GetScrIni('Message',0)
  if msglong=1 then #lstHeight=#lstHeight-1
  if datatype(_#BNO)<>'NUM' then _#BNO=1
     else _#BNO=_#BNO+1
/* ----- Headers, Buffer Number, etc. -------- */
  if header='' then _header1=0
  else do
     _header1=1
     #header1=header
     #lstHeight=#lstHeight-1
     #lstOff=#lstOff+1
     #scrrow=#scrrow+1
     if header2='' then _header2=0
     else do
        #header2=header2
        _header2=1
        #lstHeight=#lstHeight-1
        #lstOff=#lstOff+1
        #scrrow=#scrrow+1
     end
  end
  #CMDPREF=LinecommandAPPL
  if #cmdpref='' then #cmdpref='CMD'
/* ----- INIT Screen definitions ------------------------------------ */
  call SCREENINIT
  _clearcmd=Copies('_',cmdlen)
/* ----- Now we can define Text and Fields -------------------------- */
  _savela.=''
return
