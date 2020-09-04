         COPY  REGS
         GBLA  &WTOMSG      ADDITIONAL WTO TRACING WANTED
         GBLA  &NJETRC      ADDITIONAL TRACE FOR NJE38 FUNCTIONS
&WTOMSG  SETA  0            0=NO, 1=YES
&NJETRC  SETA  0            0=NO, 1=YES
* =====================================================================
*   DECIDE IF ADDITIONAL REXX VARIABLES SHOULD BE SET FOR TESTING
* =====================================================================
NJE38DIR PPROC TITLE='NJE38 NETSPOOL INTERFACE',PGMREG=(RC,RA)
* .... INIT PROGRAM ...................................................
         XC    NCB1,NCB1           Init NCB
         LA    R8,NCB1             -> NCB area
         RXPUT VAR=NJE38MSG_000000,VALUE='0',VALLEN=6
         BLANK MTEXT
         USING NCB,R8
* .... GET MODE .......................................................
         RXGET VAR=NJEMODE,INTO=NJEMODE,FEX=EXIT
         RXPUT VAR=NJE38TEMP,VALFLD=NJEMODE,VALLEN=6
         CLC   =CL3'DIR',NJEMODE
         BE    NJEDIR
         CLC   =CL3'CAN',NJEMODE
         BE    NJECAN
         CLC   =CL3'INF',NJEMODE
         BE    NJEINF
         CLC   =CL4'ECHO',NJEMODE
         BE    NJEECHO
         B     NOPARM
* ---------------------------------------------------------------------
* .... MAIN PROGRAM NJE38DIR DISPLAY DIRECTORY ........................
* ---------------------------------------------------------------------
NJEDIR   DS    0H
         BAL   RE,NJEOPEN
         ST    RF,RXRETURN
         B     *+4(RF)
         B     RCONT
         B     EXIT
RCONT    BAL   RE,NJECONT
         ST    RF,RXRETURN
         B     *+4(RF)
         B     EXIT      rf=0
         B     EXIT      rf=4
         B     EXIT      rf=8
* ---------------------------------------------------------------------
* .... MAIN PROGRAM RXNJE38 Purge Entry ...............................
* ---------------------------------------------------------------------
NJECAN   DS    0H
         RXGET VAR=NJEFILE,INTO=FILENO,FEX=NOFILE
         BAL   RE,NJEPURGE
         ST    RF,RXRETURN
         B     *+4(RF)
         B     EXIT      rf=0
         B     EXIT      rf=4
         B     EXIT      rf=8
NOFILE   MVC   MTEXT(L'CMSG16A),CMSG16A
         RXPUT VAR=NJE38MSG_000000,VALUE='1',VALLEN=6
         BAL   RE,ISSUE000
         B     EXIT
* ---------------------------------------------------------------------
* .... MAIN PROGRAM ECHO ..............................................
* ---------------------------------------------------------------------
NJEECHO  MVC   MTEXT(6),=C'ECHO: '
         RXGET VAR=NJETEXT,INTO=NJETEXT,FEX=EXIT
         MVC   MTEXT+6(32),NJETEXT
         RXPUT VAR=NJE38MSG_000000,VALUE='1',VALLEN=6
         BAL   RE,ISSUE000
         LA    RF,0
         ST    RF,RXRETURN
         B     EXIT
* ---------------------------------------------------------------------
* .... MAIN PROGRAM NJE38DIR DISPLAY File Info ........................
* ---------------------------------------------------------------------
NJEINF   DS    0H
         RXGET VAR=NJEFILE,INTO=FILENO,FEX=NOFILE
         BAL   RE,NJEINFO
         ST    RF,RXRETURN
         B     *+4(RF)
         B     EXIT      rf=0
         B     EXIT      rf=4
         B     EXIT      rf=8
* ---------------------------------------------------------------------
* .... NO PARM ERROR ..................................................
* ---------------------------------------------------------------------
NOPARM   MVC   MTEXT(29),=C'NO OR WRONG PARM IN NJEMODE: '
         MVC   MTEXT+29(8),NJEMODE
         RXPUT VAR=NJE38MSG_000000,VALUE='1',VALLEN=6
         BAL   RE,ISSUE000
         LA    RF,8
         ST    RF,RXRETURN
         B     EXIT
* ---------------------------------------------------------------------
* ..... EXIT HANDLING .................................................
* ---------------------------------------------------------------------
FMT000   LA    RF,12
         ST    RF,RXRETURN
         B     EXIT
U0039    LA    RF,16
         ST    RF,RXRETURN
EXIT     DS    0H
         BIN2CHR STRNUM,NJECNT
         RXPUT VAR=NJE38MSG_000000,VALFLD=STRNUM+10,VALLEN=6
         BIN2CHR STRNUM,RXRETURN
         RXPUT VAR=NJE38RC,VALFLD=STRNUM+12,VALLEN=4
         SRETURN RC=(RF)
* ---------------------------------------------------------------------
*      Open NJE Spool Dataset
* ---------------------------------------------------------------------
NJEOPEN  DS    0H
         ST    RE,SAVE02
         BLANK MTEXT
         XC    NJECNT,NJECNT
         NSIO  TYPE=OPEN,          Open dataset                        x
               NCB=(R8)
         LTR   R15,R15             Any errors?
         BZ    OPEN00              NO
         MVC   MTEXT(25),=CL25'NETSPOOL Open Error'
         BAL   RE,ISSUE000        GO STACK THE MESSAGE
         LA    RF,4
         B     OPEN08              ABEND ON VSAM ERROR
OPEN00   LA    RF,0
OPEN08   L     RE,SAVE02
         BR    RE
* ---------------------------------------------------------------------
*      Get Content (Close Dataset before preparing Output
* ---------------------------------------------------------------------
NJECONT  DS    0H
         ST    RE,SAVE01
         XR    R5,R5
*
CMD255   EQU   *
         NSIO  TYPE=CONTENTS,      get directory contents              x
               NCB=(R8)
         LTR   R15,R15             Any errors?
         BZ    CMD260
         ICM   R5,3,NCBRTNCD       Save error codes for now
*
CMD260   EQU   *         ANALYSE CONTENT
         NSIO  TYPE=CLOSE,         Close dataet                        x
               NCB=(R8)
*
         CLM   R5,3,=AL1(12,6)     Were no directory entries returned?
         BE    CMD280              Correct
         CLM   R5,2,=AL1(0)        Were there any error codes?
         BZ    CMD265              No
         STCM  R5,3,NCBRTNCD       Restore codes for formatting    v110
         BAL   RE,ISSUE000        Go stack the message
         LA    RF,8
         B     CMD290
*
CMD265   DS    0H
         BAL   RE,HEADER           CREATE OUTPUT HEADER
         L     R6,NCBAREA          -> returned directory entries
         USING NSDIR,R6
         SR    R5,R5
         ICM   R5,3,NCBRECCT       # of returned entries
*  ... LOOP THROUGH ALL ENTRIES
CMD270   DS    0H
         BAL   RE,PREPARE
         AH    R6,NCBRECLN         -> next directory entry
         BCT   R5,CMD270           Loop through entries
*  ... Loop End
         DROP  R6                  NSDIR
         LA    RF,0
         B     CMD290
*
CMD280   EQU   *                   No files queued
         BAL   RE,NODIR
         LA    RF,4
CMD290   DS    0H
         L     RE,SAVE01
         BR    RE
* ---------------------------------------------------------------------
*      Purge File
* ---------------------------------------------------------------------
NJEPURGE DS    0H
         ST    RE,SAVE01
         MVC   MTEXT,BLANKS        Clear work area
         L     R5,FILENO
         LR    R6,R5
         AIF   ('&NJETRC' NE '1').NOP1
         CVD   R5,DBLE             CONVERT FILE #
         UNPK  TWRK(4),DBLE        Add zones
         OI    TWRK+3,X'F0'        Fix sign
         MVC   MTEXT(19),=C'Purge request for: '
         MVC   MTEXT+19(4),TWRK
         BAL   RE,ISSUE000
.NOP1    ANOP
*   R5  contains starting job number
*   R6  contains ending job number
*
         MVC   MTEXT,BLANKS        Clear work area
         XC    NCB1,NCB1           Init NCB
         LA    R2,NCB1             -> NCB area
         USING NCB,R2
*
         NSIO  TYPE=OPEN,          Open dataset                        x
               NCB=(R2)
         LTR   R15,R15             Any errors?
         BZ    CMD320              No
         BAL   R14,FMT000          Display error
         B     U0039               Abend on VSAM error
*
CMD320   EQU   *
         AIF   ('&WTOMSG' NE '1').NOP2
         WTO   'Open Purge Successful'
.NOP2    ANOP
         NSIO  TYPE=CONTENTS,      get directory contents              x
               NCB=(R2)
         LTR   R15,R15             Any errors?
         BZ    CMD330              No
         CLC   NCBRTNCD(2),=AL1(12,6) No files in spool?           v110
         BE    CMD370              True                            v110
         BAL   R14,FMT000          Display error
         B     U0039               Abend on VSAM error
*
CMD330   EQU   *
         L     R3,NCBAREA          -> returned directory entries
         USING NSDIR,R3
         SR    R4,R4
         ICM   R4,3,NCBRECCT       # of returned entries
*
CMD340   EQU   *
         LH    R14,NSID            Get a file number
         AIF   ('&WTOMSG' NE '1').NOP3
         WTO   'Fetch Directory Entry'
.NOP3    ANOP
         CR    R14,R5              Is file number in cancel range?
         BL    CMD360              N, get next
         CR    R14,R6              Is file number in cancel range?
         BH    CMD360              N, get next
*
         AIF   ('&WTOMSG' NE '1').NOP4
         WTO   'Directory Entry Match'
.NOP4    ANOP
         TM    NJFL1,NJF1AUTH      Is issuing user cmd authorized?
         BO    CMD348              Yes, continue
*
*-- See if file originated from command issuing user. YES=ALLOW
*        CLC   CMDLINK,NSINLOC     Is file here on issuer's node?
*        BNE   CMD344              Nope cant cncl files on other nodes
*        CLC   CMDVMID,NSINVM      Does userid match issuer's ?
*        BE    CMD348              Yes, allow the cancel
*
*-- See if file was destined for command issuing user.  YES=ALLOW
CMD344   EQU   *
*        CLC   CMDLINK,NSTOLOC     Was file dest = cmd issuer's node?
*        BNE   CMD360              Nope cant cncl files on other nodes
*        CLC   CMDVMID,NSTOVM      Does userid match issuer's ?
*        BNE   CMD360              No, disallow the cancel
*
CMD348   EQU   *
         AIF   ('&WTOMSG' NE '1').NOP5
         WTO   'Prepare Purge'
.NOP5    ANOP
         LA    R15,TDATA           -> tag data area
         USING TAG,R15
         STH   R14,TAGID           Save file id in tag data
         DROP  R15                 TAG
*
         NSIO  TYPE=PURGE,         Purge the file by file #            x
               NCB=(R2),                                               x
               TAG=(R15)
         LTR   R15,R15             Any errors?
         BZ    CMD350              No
         AIF   ('&WTOMSG' NE '1').NOP6
         WTO   'Some Error(s) during Purge'
.NOP6    ANOP
         CLC   NCBRTNCD(2),=AL1(12,4) Was file # not found in NETSPOOL?
         BE    CMD360              True
         BAL   R14,FMT000          Display other error
         B     U0039               Abend on VSAM error
*
CMD350   EQU   *
         AIF   ('&WTOMSG' NE '1').NOP7
         WTO   'Purge Successful'
.NOP7    ANOP
         OI    NJFL1,NJF1CNCL      Indic at least one file purged
         LH    R1,NSID             Get the file number
         CVD   R1,DBLE             Convert file #
         UNPK  TWRK(4),DBLE        Add zones
         OI    TWRK+3,X'F0'        Fix sign
         MVC   MTEXT,BLANKS        Clear work area
         MVC   MTEXT(L'CMSG14),CMSG14  Move msg
         MVC   MTEXT+14(4),TWRK    Insert file number
         LA    R1,L'CMSG14         Length of message
         BAL   R14,ISSUE000        Go stack the message
*
CMD360   EQU   *
         LA    R3,NSDIRLN(,R3)     -> next dir entry
         BCT   R4,CMD340           Keep scanning for files to purge
         DROP  R3                  NSDIR
*
CMD370   EQU   *
         NSIO  TYPE=CLOSE,         Done with dataset                   x
               NCB=(R2)
*
         LM    R0,R1,NCBAREAL      Get list length and address
         LTR   R1,R1               Was an area returned?           v110
         BZ    CMD380              No; avoid freemain              v110
         XC    NCBAREA,NCBAREA     Clear obsolete ptr
         FREEMAIN RU,LV=(0),A=(1)
         DROP  R2                  NCB
*
         TM    NJFL1,NJF1CNCL      Were any files successfully purged?
         BO    XITCMG00            Yes, done with command
*
CMD380   EQU   *                   File was not found
         MVC   MTEXT,BLANKS        Clear work area
         MVC   MTEXT(L'CMSG15),CMSG15  Move msg
         LA    R1,L'CMSG15         Length of message
         BAL   R14,ISSUE000        Go stack the message
         LA    RF,4
         B     XITCMG08            Exit command function completed
*
CMD390   EQU   *                   Invalid file # specified
         MVC   MTEXT,BLANKS        Clear work area
         MVC   MTEXT(L'CMSG16),CMSG16  Move msg
         LA    R1,L'CMSG16         Length of message
         BAL   R14,ISSUE000        Go stack the message
         LA    RF,8
         B     XITCMG08            Exit command function completed
*
XITCMG00 DS    0H
         LA    RF,0
XITCMG08 DS    0H
         L     RE,SAVE01
         BR    RE
*
* ---------------------------------------------------------------------
*      Pepare Output Line and output it
* ---------------------------------------------------------------------
PREPARE  DS    0H
         ST    RE,SAVE02
         MVC   MTEXT,BLANKS        Clear work area
         USING NSDIR,R6
         LH    R1,NSID             Get file id number
         CVD   R1,DBLE             Convert
         UNPK  MTEXT(4),DBLE
         OI    MTEXT+3,X'F0'
         MVC   VNUM,MTEXT
         MVC   MTEXT+06(8),NSINLOC  Origin node
         MVC   MTEXT+15(8),NSINVM   Origin userid
         MVC   MTEXT+25(8),NSTOLOC  Destination node
         MVC   MTEXT+34(8),NSTOVM   Destination userid
         MVC   MTEXT+44(1),NSCLASS  Class
*
         MVC   MTEXT+45(10),=X'40206B2020206B202120'
         L     R1,NSRECNM          Get # of records in file
         CVD   R1,DBLE             Convert
         ED    MTEXT+45(10),DBLE+4 Edit result
         BAL   R14,ISSUE000        Go stack the message
*
         DROP  R6
         L     RE,SAVE02
         BR    RE
* ---------------------------------------------------------------------
*      CREATE HEADER OF OUTPUT
* ---------------------------------------------------------------------
HEADER   DS    0H
         ST    RE,SAVE02
*
         MVC   MTEXT,BLANKS        Clear work area
         MVC   MTEXT(L'CMSG10),CMSG10 Move msg
         LA    R1,L'CMSG10         Length of message
         BAL   R14,ISSUE000        Go stack the message
*
         MVC   MTEXT,BLANKS        Clear work area
         MVC   MTEXT(L'CMSG11),CMSG11 Move msg
         LA    R1,L'CMSG11         Length of message
         BAL   R14,ISSUE000        Go stack the message
*
         L     RE,SAVE02
         BR    RE
* ---------------------------------------------------------------------
*      NO DIRECTORY FOUND GE
* ---------------------------------------------------------------------
NODIR    DS    0H
         ST    RE,SAVE02
         MVC   MTEXT,BLANKS        Clear work area
         MVC   MTEXT(L'CMSG9),CMSG9  Move msg
         L     R6,ALINKS           -> first LINKTABL entry         v102
         USING LINKTABL,R6                                         v102
         MVC   MTEXT+L'CMSG9(8),LINKID  Plug local node name to msgv102
         DROP  R6                                                  v102
         LA    R1,L'CMSG9+8        Length of message               v102
         BAL   R14,ISSUE000        Go stack the message
         MVC   MTEXT,BLANKS        Clear work area
         MVC   MTEXT(L'CMSG13),CMSG13  Move msg
         LA    R1,L'CMSG13         Length of message
         BAL   R14,ISSUE000        Go stack the message
         L     RE,SAVE02
         BR    RE
* ---------------------------------------------------------------------
*      Stack Output Message in REXX Variable
* ---------------------------------------------------------------------
ISSUE000 DS    0H
         ST    RE,SAVE03
         L     R1,NJECNT
         LA    R1,1(R1)
         ST    R1,NJECNT
         AIF   ('&WTOMSG' EQ '0').NOWTO
         MVC   WTOMSG,MTEXT
         MVC   WTOFILLR,=AL2(0)  CLEAR NEXT 2 BYTES
         MVC   WTOMSGLN,=AL2(80)
         WTO   MF=(E,WTOCB)      SEND MESSAGE TO CONSOLE
.NOWTO   ANOP
         RXPUT VAR=NJE38MSG_,INDEX=NJECNT,VALFLD=MTEXT,VALLEN=80
         L     RE,SAVE03
         BR    RE
         EJECT
* ---------------------------------------------------------------------
*-- Display filenum
*     Entry:  R3 = file number
*   File is already opened
* ---------------------------------------------------------------------
NJEINFO  EQU   *
         ST    RE,SAVE01
         XC    NCB1,NCB1           Init NCB
         LA    R2,NCB1             -> NCB area
         USING NCB,R2
*
         NSIO  TYPE=OPEN,          Open dataset                        x
               NCB=(R2)
         LTR   R15,R15             Any errors?
         BZ    DNUM020             No
         BAL   R14,FMT000          Display error
         B     U0039               Abend on VSAM error
*
DNUM020  EQU   *
         L     R3,FILENO
         LA    R6,TDATA            -> tag data area
         USING TAG,R6
         STH   R3,TAGID            Set file # to find
*
         NSIO  TYPE=FIND,          get directory entry                 x
               NCB=(R2),                                               x
               TAG=(R6)            Where to place tag data
         LTR   R15,R15             Any errors?
         BZ    DNUM040
         CLC   NCBRTNCD(2),=AL1(12,4) Was specified file id not found?
         BE    DNUM900             Yes
         BAL   R14,FMT000          Otherwise, display error
         B     U0039               Abend on VSAM error
*
DNUM040  EQU   *
*
*
DNUM050  EQU   *
         MVC   MTEXT,BLANKS        Clear work area
         LH    R1,TAGID            Get file id number
         CVD   R1,DBLE             Convert
         UNPK  MTEXT(4),DBLE
         OI    MTEXT+3,X'F0'
         MVC   MTEXT+06(8),TAGINLOC  Origin node
         MVC   MTEXT+15(8),TAGINVM   Origin userid
         MVC   MTEXT+25(8),TAGTOLOC  Destination node
         MVC   MTEXT+34(8),TAGTOVM   Destination userid
         MVC   MTEXT+44(1),TAGCLASS  Class
*
         MVC   MTEXT+45(10),=X'40206B2020206B202120'
         L     R1,TAGRECNM         Get # of records in file
         CVD   R1,DBLE             Convert
         ED    MTEXT+45(10),DBLE+4 Edit result
*
         LA    R1,L'N026C          Length of msg
         LM    R14,R15,TAGINTOD  TOD CLOCK UNITS
         SRDL  R14,12            MICROSECONDS SINCE JAN 1, 1900
* .... SUBTRACT date range 1.1.1900 - 1.1.1970 ,
         SL    R15,D2EPOCH+4    - Right Half
         BC    11,*+6             BRANCH ON NO BORROW
         BCTR  R14,R0             -1 FOR BORROW
         SL    R14,D2EPOCH      - LEFT HALF
         D     R14,=F'1000000'    SECONDS SINCE JAN 1, 1970
         ST    R15,NJEDATE        Store result
         BIN2CHR STRNUM,NJEDATE
         RXPUT VAR=NJE38DATE,VALFLD=STRNUM+6,VALLEN=10
*                                  LENGTH OF MSG
         BAL   R14,ISSUE000        Stack it
*
         TM    TAGINDEV,TYPPRT     Is it PRINT data?
         BO    DNUM060             Y, don't need to check for NETDATA
*
         L     R15,=A(NJECME)      NETDATA examination routine
         BALR  R14,R15             Go look for NETDATA
         LTR   R15,R15             Check RC
         BZ    DNUM070             All is well, we have NETDATA
*
DNUM060  EQU   *
         OI    NJFL1,NJF1NYET      No NETDATA or PRINT file
*
DNUM070  EQU   *
         MVC   MTEXT,BLANKS        Clear work area
         MVC   MTEXT(L'N026D),N026D  Move model msg
         LA    R1,MTEXT+L'N026D    -> end of model
         MVC   0(12,R1),TAGNAME    Move file name
*
         TRT   0(13,R1),BLANK      Look for end of file name
         LA    R1,1(,R1)           Skip blank
         MVC   0(12,R1),TAGTYPE    Move file type
*
         TRT   0(13,R1),BLANK      Look for end of file type
         LA    R1,3(,R1)           Skip 3 blanks
         MVC   0(11,R1),=C'Type: PRINT'  Assume print data
         LA    R1,6(,R1)           -> where to put format type
         TM    TAGINDEV,TYPPRT     Was it actually PRINT type?
         BO    DNUM080             Yes, display PRINT attr
*
         MVC   0(5,R1),=C'PUNCH'   Assume PUNCH unless its NETDATA
         TM    NJFL1,NJF1NYET      Was it NETDATA or PRINT file
         BO    DNUM100             No, display PUNCH attr
         MVC   0(7,R1),=C'NETDATA' Yes
         B     DNUM200             Display NETDATA attr
*
*-- Display for flat PRINT type file
*
DNUM080  EQU   *
         LA    R1,7(,R1)           -> end of message
         LA    R0,MTEXT            -> Start
         SR    R1,R0               compute length of msg
         BAL   R14,ISSUE000        Stack msg N026D
*
         MVC   MTEXT,BLANKS        Clear work area
         MVC   MTEXT(L'N026E),N026E  Move model msg
         LA    R1,MTEXT+L'N026E    -> end of model
         MVC   0(8,R1),=C'132/F/PS' Display all we know
         LA    R1,8(,R1)           Bump length
         BAL   R14,ISSUE000        Stack msg N026E
         B     DNUM990             Command function completed
*
*-- Display for flat PUNCH type file
*
DNUM100  EQU   *
         LA    R1,7(,R1)           -> end of message
         LA    R0,MTEXT            -> Start
         SR    R1,R0               compute length of msg
         BAL   R14,ISSUE000        Stack msg N026D
*
         MVC   MTEXT,BLANKS        Clear work area
         MVC   MTEXT(L'N026E),N026E  Move model msg
         LA    R1,MTEXT+L'N026E    -> end of model
         MVC   0(7,R1),=C'80/F/PS' Display all we know
         LA    R1,7(,R1)           Bump length
         BAL   R14,ISSUE000        Stack msg N026E
         B     DNUM990             Command function completed
*
*-- Display for NETDATA files
*
DNUM200  EQU   *
         LA    R1,7(,R1)           -> end of message
         LA    R0,MTEXT            -> Start
         SR    R1,R0               compute length of msg
         BAL   R14,ISSUE000        Stack msg N026D
*
         CLI   FFM,X'00'           Was a file mode present?
         BE    DNUM300             Its 0, so this is OS NETDATA
*
*-- Display for VM-based NETDATA files
*
DNUM210  EQU   *
         MVC   MTEXT,BLANKS        Clear work area
         MVC   MTEXT(L'N026E),N026E  Move model msg
         LA    R1,MTEXT+L'N026E    -> end of model
*
*-- Dont display BLKSIZE for VM files; it is meaningless
*        L     R4,BLKSIZE          Get the blocksize  value
*        CVD   R4,DBLE             Convert
*        BAL   R14,DSPNUM          Make number displayable
*        MVI   0(R1),C'/'
*        LA    R1,1(,R1)
*
         L     R4,LRECL            Get the lrecl value
         CVD   R4,DBLE             Convert
         BAL   R14,DSPNUM          Make number displayable
         MVI   0(R1),C'/'
         LA    R1,1(,R1)
*
         BAL   R14,DSPRECFM        Format the RECFM value
         MVI   0(R1),C'/'
         LA    R1,1(,R1)
*
         BAL   R14,DSPORG          Format the DSORG value
*
         LA    R1,4(,R1)           Skip some space in msg
         MVC   0(5,R1),=C'Size:'
         LA    R1,6(,R1)
         LM    R4,R5,FILESIZE      Get approx file size
         LA    R3,8                Max length of file size value
         LH    R0,FSIZELEN         Get length from NETDATA key
         SR    R3,R0               Compute # bytes of shift
         SLA   R3,3                Turn # bytes into # bits
         SRDL  R4,0(R3)            Right justify the filesize
         SRL   R5,10               divide by 1024 to get kilobytes
         LA    R5,1(,R5)           Always round up
         CVD   R5,DBLE             Convert
         BAL   R14,DSPNUM          Make number displayable
         MVC   1(2,R1),=C'KB'
         LA    R1,3(,R1)           -> end of msg
*
         LA    R0,MTEXT            -> Start
         SR    R1,R0               compute length of msg
         BAL   R14,ISSUE000        Stack msg N026E
         B     DNUM990
*
*-- Display for OS-based NETDATA files
*
DNUM300  EQU   *
         MVC   MTEXT,BLANKS        Clear work area
         MVC   MTEXT(L'N026E),N026E  Move model msg
         LA    R1,MTEXT+L'N026E    -> end of model
*
         L     R4,BLKSIZE          Get the blocksize  value
         CVD   R4,DBLE             Convert
         BAL   R14,DSPNUM          Make number displayable
         MVI   0(R1),C'/'
         LA    R1,1(,R1)
*
         TM    RECFM,DCBRECU       Is this a RECFM=U dataset?
         BO    DNUM310             Y, don't format LRECL
*
         L     R4,LRECL            Get the lrecl value
         CVD   R4,DBLE             Convert
         BAL   R14,DSPNUM          Make number displayable
         MVI   0(R1),C'/'
         LA    R1,1(,R1)
*
DNUM310  EQU   *
         BAL   R14,DSPRECFM        Format the RECFM value
         MVI   0(R1),C'/'
         LA    R1,1(,R1)
*
         BAL   R14,DSPORG          Format the DSORG value
*
         CLI   DSORG,X'02'         Is this DSORG=PO?
         BNE   DNUM330             No, skip dir blks
         LA    R1,3(,R1)           Skip some space in msg
         MVC   0(8,R1),=C'DIRBLKS:'
         LA    R1,9(,R1)
         LM    R4,R5,DIRBLKS       Get approx file size
         LA    R3,8                Max length of value
         LH    R0,DIRBLKLN         Get length from NETDATA key
         SR    R3,R0               Compute # bytes of shift
         SLA   R3,3                Turn # bytes into # bits
         SRDL  R4,0(R3)            Right justify the # dir blks
         CVD   R5,DBLE             Convert
         BAL   R14,DSPNUM          Make number displayable
*
DNUM330  EQU   *
         LA    R1,3(,R1)           Skip some space in msg
         MVC   0(5,R1),=C'Size:'
         LA    R1,6(,R1)
         LM    R4,R5,FILESIZE      Get approx file size
         LA    R3,8                Max length of file size value
         LH    R0,FSIZELEN         Get length from NETDATA key
         SR    R3,R0               Compute # bytes of shift
         SLA   R3,3                Turn # bytes into # bits
         SRDL  R4,0(R3)            Right justify the filesize
         SRL   R5,10               divide by 1024 to get kilobytes
         LA    R5,1(,R5)           Always round up
         CVD   R5,DBLE             Convert
         BAL   R14,DSPNUM          Make number displayable
         MVC   1(2,R1),=C'KB'
         LA    R1,3(,R1)           -> end of msg
*
         LA    R0,MTEXT            -> Start
         SR    R1,R0               compute length of msg
         BAL   R14,ISSUE000        Stack msg N026E
*
DNUM350  EQU   *
         MVC   MTEXT,BLANKS        Clear work area
         MVC   MTEXT(L'N026F),N026F  Move model msg
         LA    R1,MTEXT+L'N026F    -> end of model
         MVC   0(44,R1),DSNAME     Move DSNAME to msg
         LA    R1,L'N026F+44       Length of MSG + DSNAME          V110
         BAL   R14,ISSUE000        Stack msg N026F
         B     DNUM990
*
*-- Format a number to remove leading blanks and insert into msg line
*   Entry:  R1 -> where to place result
*   Exit :  R1 -> next available byte after result
*
DSPNUM   EQU   *
         LR    R15,R1              Save msg line position
         MVC   TWRK(8),=X'4020202020202120'
         LA    R1,TWRK+7           -> last digit area
         LR    R3,R1               Save a copy
         EDMK  TWRK(8),DBLE+4      Edit the number
         SR    R3,R1               Compute number's length
         EX    R3,DSPMVC           Move number to msg line
         LA    R1,1(R3,R15)        Compute next msg line byte
         BR    R14
DSPMVC   MVC   0(0,R15),0(R1)      executed instr
*
*-- Format the RECFM value
*   Entry:  Field 'RECFM' contains the record format bits
*   Exit :  R1 -> next available byte after result
*
DSPRECFM EQU   *
         MVI   0(R1),C'?'        Assume unknown RECFM              v130
         TM    RECFM+1,X'03'     Using shortened variable formats? v130
         BNZ   DSPV              Yes, start with V                 v130
         TM    RECFM,DCBRECF     FIXED?
         BZ    *+8
         MVI   0(R1),C'F'
         TM    RECFM,DCBRECV     VARIABLE?
         BZ    *+8
*
DSPV     EQU   *                                                   v130
         MVI   0(R1),C'V'
         TM    RECFM,DCBRECU     UNDEFINED?
         BNO   *+8
         MVI   0(R1),C'U'
         LA    R1,1(,R1)
*
         TM    RECFM,DCBRECBR    BLOCKED?
         BZ    *+12
         MVI   0(R1),C'B'
         LA    R1,1(,R1)
*
         TM    RECFM,DCBRECSB    SPANNED?
         BZ    *+12
         MVI   0(R1),C'S'
         LA    R1,1(,R1)
         TM    RECFM,DCBRECTO    TRACK OVERFLOW?
         BZ    *+12
         MVI   0(R1),C'T'
         LA    R1,1(,R1)
*
         TM    RECFM,DCBRECCA    ASA CONTROL CHAR?
         BZ    *+12
         MVI   0(R1),C'A'
         LA    R1,1(,R1)
         TM    RECFM,DCBRECCM    MACHINE CONTROL CHAR?
         BZ    *+12
         MVI   0(R1),C'M'
         LA    R1,1(,R1)
         BR    R14
*
*-- Format the DSORG value
*   Entry:  Field 'DSORG' contains the organization bits
*   Exit :  R1 -> next available byte after result
*
DSPORG   EQU   *
         MVC   0(2,R1),=C'? '    Assume unknown DSORG
         CLC   DSORG,=X'4000'    DSORG=PS?
         BNE   *+10
         MVC   0(2,R1),=C'PS'
         CLC   DSORG,=X'0200'    DSORG=PO?
         BNE   *+10
         MVC   0(2,R1),=C'PO'
         CLC   DSORG,=X'0008'    DSORG=VS?
         BNE   *+10
         MVC   0(2,R1),=C'VS'
         LA    R1,2(,R1)         -> next available byte
         BR    R14
*
DNUM900  EQU   *                ** Here if file not found
         LH    R1,TAGID            Get the file number
         CVD   R1,DBLE             Convert file #
         UNPK  TWRK(4),DBLE        Add zones
         OI    TWRK+3,X'F0'        Fix sign
         MVC   MTEXT,BLANKS        Clear work area
         MVC   MTEXT(L'NJE027E),NJE027E Move msg
         MVC   MTEXT+14(4),TWRK    Insert file number
         LA    R1,L'NJE027E        Length of message
         BAL   R14,ISSUE000        Go stack the message
*
*
DNUM990  EQU   *
         LA    R2,NCB1             -> NCB area
         NSIO  TYPE=CLOSE,         Close spool dataset                 x
               NCB=(R2)
         DROP  R6                  TAG
         L     RE,SAVE01
         BR    RE
* =====================================================================
* SHVBLOCK:  LAYOUT OF SHARED-VARIABLE PLIST ELEMENT
* =====================================================================
         LTORG
         DS    0F
D2EPOCH  DC    FL8'2208902400000000'
BLANKS   DC    CL120' '
NONBLANK DC    64X'FF',X'00',191X'FF'  TR Table to locate nonblank
BLANK    DC    64X'00',X'FF',191X'00'  TR Table to locate blanks
CMSG9  DC C'NJE014I  File status for node '
CMSG10 DC C'File  Origin   Origin    Dest     Dest'
CMSG11 DC C' ID   Node     Userid    Node     Userid    CL  Records'
CMSG13 DC C'No files queued'
*-- C #### RESPONSE MODELS:
CMSG14   DC    C'NJE015I  FILE(XXXX) PURGED'
CMSG15   DC    C'NJE016E  NO ELIGIBLE FILE FOUND'
CMSG16   DC    C'NJE017E  INVALID FILE NUMBER SPECIFIED'
CMSG16A  DC    C'NJE017E  NO FILE NUMBER SPECIFIED'
*
NJE026I  DC    C'NJE026I  File status for node '
N026A  DC C'File  Origin   Origin    Dest     Dest'
N026B  DC C' ID   Node     Userid    Node     Userid    CL  Records'
N026C  DC C'xxxx  xxxxxxxx xxxxxxxx  xxxxxxxx xxxxxxxx  c x,xxx,xxx'
N026D  DC C'Tagged name: '
N026E  DC C'Attributes: '
N026F  DC C'Origin DSN='
NJE027E  DC    C'NJE027E  File(xxxx) does not exist'  used by E ### too
         EJECT
*********************
*  N J E C M E      *               NJECME determines if NETDATA
*                   *               exists in a spool file and
*  Examine NETDATA  *               examines the INMR02 control
*                   *               record for attributes.
*********************               Entire CSECT added             v110
*
NJECME   CSECT
         B     28(,R15)               BRANCH AROUND EYECATCHERS
         DC    AL1(23)                LENGTH OF EYECATCHERS
         DC    CL9'NJECME'
         DC    CL9'&SYSDATE'
         DC    CL5'&SYSTIME'
*
         STM   R14,R12,12(R13)         Save Regs
         LR    R12,R15                 Base
         USING NJECME,R12
* !!!    USING NJEWK,R10
         ST    R13,CMESA+4             SAVE prv S.A. ADDR
         LA    R1,CMESA                -> my save area
         ST    R1,8(,R13)              Plug it into prior SA
         LR    R13,R1
*
*
         LA    R0,2                    # of bytes to get
         BAL   R14,GETBYTES            Get length and desc of segment
*
         TM    1(R1),X'20'             Is this a control record?
         BZ    XITCME04                No, its not NETDATA
*
         SR    R0,R0
         IC    R0,0(,R1)               Get segment length byte
         S     R0,=F'2'                Less 2 we already retrieved
         BAL   R14,GETBYTES            Get control record
*
         CLC   0(6,R1),INMR01          NETDATA?
         BNE   XITCME04                Not NETDATA
*
         LA    R0,2                    # of bytes to get
         BAL   R14,GETBYTES            Get length and desc of segment
*
         TM    1(R1),X'20'             Is this a control record?
         BZ    XITCME04                No, its not NETDATA
*
         SR    R0,R0
         IC    R0,0(,R1)               Get segment length byte
         S     R0,=F'2'                Less 2 we already retrieved
         LR    R3,R0                   Copy length of control record
         BAL   R14,GETBYTES            Get control record
*
         CLC   0(6,R1),INMR02          NETDATA?
         BNE   XITCME04                Not NETDATA
*
         LA    R15,10                  Len of "INMR02"+file number word
         AR    R1,R15                  Skip over those fields
*
CTL000   EQU   *
         SR    R3,R15                  Reduce remaining length
         BNP   XITCME00                Done with control record
*
*-- Look for supported keys
*
         CLC   0(2,R1),INMUTILN        Utility name?
         BE    UTL000                  Y
         CLC   0(2,R1),INMSIZE         File size?
         BE    FSZ000                  Y
         CLC   0(2,R1),INMDSORG        DSORG?
         BE    DSG000                  Y
         CLC   0(2,R1),INMBLKSZ        BLKSIZE?
         BE    BLK000                  Y
         CLC   0(2,R1),INMLRECL        LRECL?
         BE    LRL000                  Y
         CLC   0(2,R1),INMRECFM        RECFM?
         BE    RFM000                  Y
         CLC   0(2,R1),INMFFM          File mode number?
         BE    FFM000                  Y
         CLC   0(2,R1),INMDIR          # directory blocks?
         BE    DIR000                  Y
         CLC   0(2,R1),INMDSNAM        DSNAME?
         BE    DSN000                  Y
*
*-- Skip over unsupported/unrecognized keys
*
         LA    R1,2(,R1)               Skip over unrecognized key
         LA    R15,2                   Remaining length adjust
         SR    R0,R0                   Clear for IC
         ICM   R0,3,0(R1)              Get # value
         LA    R1,2(,R1)               Skip over # value
         LA    R15,2(,R15)             Remaining length adjust
         BZ    CTL000                  # was 0; no lengths
         SR    R14,R14                 Clear for ICM
*
CTL020   EQU   *
         ICM   R14,3,0(R1)             Get length field
         LA    R1,2(R14,R1)            Skip over length and data
         LA    R15,2(R14,R15)          Remaining length adjust
         BCT   R0,CTL020               Do next len/data field pair
         B     CTL000                  Resume
*
*-- Handle keys we support
*
*- Utility name
UTL000   EQU   *                       Get utility name
         MVC   UTLNAME,BLANKS          Init receiving field
         LA    R6,UTLNAME              -> receiving field
         B     KEY000                  Go handle the key
*
*- File size
FSZ000   EQU   *                       File size
         MVC   FSIZELEN,4(R1)          Save length of file size value
         LA    R6,FILESIZE             -> receiving field
         B     KEY000                  Go handle the key
*
*- DSORG
DSG000   EQU   *                       DSORG
         LA    R6,DSORG                -> receiving field
         B     KEY000                  Go handle the key
*- BLKSIZE
BLK000   EQU   *                       BLKSIZE
         LA    R6,BLKSIZE              -> receiving field
         B     KEY000                  Go handle the key
*
*- LRECL
LRL000   EQU   *                       LRECL
         LA    R6,LRECL                -> receiving field
         B     KEY000                  Go handle the key
*
*- RECFM
RFM000   EQU   *                       RECFM
         LA    R6,RECFM                -> receiving field
         B     KEY000                  Go handle the key
*
*- # directory blocks
DIR000   EQU   *                       File size
         MVC   DIRBLKLN,4(R1)          Save length of dirblk siz value
         LA    R6,DIRBLKS              -> receiving field
         B     KEY000                  Go handle the key
*
*- FFM
FFM000   EQU   *                       File mode number
         LA    R6,FFM                  -> receiving field
         B     KEY000                  Go handle the key
*
*- DSNAME
DSN000   EQU   *                       DSNAME
         MVC   DSNAME,BLANKS           Init receiving field
         LA    R6,DSNAME               -> receiving field
         LA    R1,2(,R1)               Skip over key
         LA    R15,2                   Remaining length adjust
         SR    R0,R0                   Clear for IC
         ICM   R0,3,0(R1)              Get # value
         LA    R1,2(,R1)               Skip over # value
         LA    R15,2(,R15)             Remaining length adjust
         BZ    CTL000                  # was 0; no lengths
         SR    R14,R14                 Clear for ICM
*
DSN020   EQU   *
         ICM   R14,3,0(R1)             Get length field
         BCT   R14,DSN030              Adjust for execute
         MVC   0(0,R6),2(R1)           executed instr
DSN030   EX    R14,*-6                 Move name to receiving field
         LA    R1,3(R14,R1)            Skip over length and data
         LA    R15,3(R14,R15)          Remaining length adjust
         LA    R6,1(R14,R6)            Bump to next qualifier area
         MVI   0(R6),C'.'              Add qualifier dot
         LA    R6,1(,R6)               -> next qualifier area
         BCT   R0,DSN020               Do next len/data field pair
         BCTR  R6,0                    -> last byte of DSNAME
         MVI   0(R6),C' '              Remove trailing dot
         BCTR  R6,0                    -> prior to trailing '.'
         LA    R0,DSNAME               -> start of DSNAME
         SR    R6,R0                   Compute DSN length
         STH   R6,DSNAMELN             Save it
         B     CTL000                  get next key
*
*-- Common routine to break part key/#/len/data elements that have #=1
*
KEY000   EQU   *
         LA    R1,4(,R1)               Skip over key, #
         LA    R15,4                   Remaining length accum
         SR    R5,R5                   Clear for IC
         ICM   R5,3,0(R1)              Get length of name
         BCT   R5,KEY010               Adjust for execute
         MVC   0(0,R6),2(R1)           executed instr
KEY010   EX    R5,*-6                  Move name to receiving field
         LA    R1,3(R5,R1)             -> next text unit key
         LA    R15,3(R5,R15)           Accum length adjustment
         B     CTL000                  Get next key
*
*
*
GETBYTES EQU   *
         ST    R14,SV14GB              Save return addr
         L     R5,GBREM                Get # bytes remaining in rec buf
         LA    R1,BUFF                 Point to getbytes buffer
         ST    R1,GBPOS                Set starting position
         LR    R8,R0                   Requested amount to R8
*
*
GB010    EQU   *
         LTR   R5,R5                   Any bytes left in phy record?
         BP    GB040                   Yes, use them first
*
         LA    R2,NCB1                 -> active NCB for spool file
         NSIO  TYPE=GET,               TAG data contains file #        x
               NCB=(R2),               Get a spool file record         x
               AREA=REC,               -> where to place record        x
               EODAD=XITCME04          if EOF, then NETDATA isnt valid
         LTR   R15,R15                 Any errors?
         BZ    GB020                   No
*        BAL   R14,FMT000              Display error
*        B     U0039                   And abend
*
GB020    EQU   *
         LA    R5,80                   Num bytes read
         LA    R1,REC                  -> input buffer
*
GB030    EQU   *
         ST    R1,GBRPS                Reset start of record position
*
GB040    EQU   *
         LR    R7,R8                   Assume requested amt avail
         LR    R15,R8                  Same
*
         CR    R5,R8                   Have more than we need?
         BH    GB050                   Yes, just move requested
         LR    R7,R5                   Else move entire rec
         LR    R15,R5                  Same
*
GB050    EQU   *
         LR    R0,R7                   Save copy of length to move
         L     R14,GBPOS               -> GB buffer position
         L     R6,GBRPS                -> input record curr position
         MVCL  R14,R6                  Move
*
         ST    R14,GBPOS               New GB position
         ST    R6,GBRPS                New phys record curr position
*
         SR    R5,R0                   Reduce bytes left in phy record
         SR    R8,R0                   Reduce requested amt
         BP    GB010                   We need more, go get it
*
         ST    R5,GBREM                Remember whats left in phy rec
*
         LA    R1,BUFF                 Point to the requested bytes
         L     R14,SV14GB              Load  return addr
         BR    R14                     Return from getbytes
*
         LTORG
*
INMR01   DC    C'INMR01'               Control record
INMR02   DC    C'INMR02'               Control record
*
*- Keys
INMUTILN DC    X'1028'                 Utility name
INMSIZE  DC    X'102C'                 File size in bytes
INMDSORG DC    X'003C'                 DSORG
INMLRECL DC    X'0042'                 LRECL
INMBLKSZ DC    X'0030'                 BLKSIZE
INMRECFM DC    X'0049'                 RECFM
INMDSNAM DC    X'0002'                 DSNAME
INMDIR   DC    X'000C'                 # directory blocks
INMFFM   DC    X'102D'                 File mode number
*
*
*
*-- Exit NETDATA examination processing
*
*
XITCME00 EQU   *
         SR    R15,R15             Set RC=0; NETDATA info filled
         B     XITCME
*
XITCME04 EQU   *
         LA    R15,4               Set RC=4; File contains no NETDATA
*
XITCME   EQU   *
         L     R13,4(,R13)         -> prev s.a.
         ST    R15,16(,R13)        Set RC
         LM    R14,R12,12(R13)     Reload callers regs
         BR    R14                 Return with RC
*
         LTORG
         SHVCB DSECT
         WORKAREA
RXRETURN DS    A                   Return Code before returning to REXX
FILENO   DS    A                   FILE NUMBER OF A REQUEST
STRDPCK  DS    0D                  STRPACK ON DOUBLE FOR BIN CONVERSION
STRPACK  DS    PL8                 MAXIMUM 999,999,999,999,999
STRNUM   DS    CL16                BIN2CHR DESTINATION FIELD
NJECNT   DS    A                   Index Count of Output
NJEMODE  DS    CL8                 REQUESTED MODE
NJETEXT  DS    CL32                INPUT LINE
IRXADDR  DS    A
NCB1     DS    XL48                NCB
         DS    CL64                Filler
MTEXT    DS    CL120               Message text work area
VARN     DS    0CL10
VAR      DC    CL6'NJEMSG'
VNUM     DS    CL4
DBLE     DS    D                   Work area
TWRK     DS    2D                  WORK AREA
ALINKS   DS    A  16                -> first LINKTABL entry
* .... Info Area
FILESIZE DS    2F                     File size in bytes           v110
DIRBLKS  DS    2F                     #directory blocks            v110
BLKSIZE  DS    F                      BLKSIZE                      v110
LRECL    DS    F                      LRECL                        v110
RECFM    DS    XL2                    RECFM                        v110
DSORG    DS    XL2                    DSORG                        v110
FFM      DS    C                      File mode number             v110
         DS    X                      available                    v110
DIRBLKLN DS    H                      Length of dir blks value     v110
FSIZELEN DS    H                      Length of file size value    v110
DSNAMELN DS    H                      Length of DSNAME             v110
DSNAME   DS    CL44                   DSNAME                       v110
* .... WTO Area
WTOCB    DS    0H
WTOMSGLN DS    AL2
WTOFILLR DS    CL2
WTOMSG   DS    CL80
WTOMSEND DS    0H
* .... Time to Convert TOD Clock Time
NJEDATE  DS    F
*
TDATA    DS    0XL108
BLNKDASH DS    0CL256
ASIDTAB  DS    24CL24
TARGET   DS    X                   CODE FOR WHO GETS THE CMD RESPONSE
TGTUSER  EQU   0                    REMOTE USER
TGTCONS  EQU   4                    MVS SYSTEM CONSOLE
TYPPRT   EQU   X'40'                PRT dev
*
NJFL1    DS    X                   FLAG BITS
NJF1MULT EQU   X'80'   1... ....    MULTI-FILE CANCEL COMMAND
NJF1CNCL EQU   X'40'   .1.. ....    A FILE WAS DELETED BY COMMAND
NJF1DATH EQU   X'20'   ..1. ....    AT LEAST 1 AUTH USER DISPLAYED
NJF1NYET EQU   X'10'   ...1 ....    NO USABLE NETDATA FOUND IN FILEV110
NJF1VSER EQU   X'02'   .... ..1.    NETSPOOL VSAM ERROR OCCURRED
NJF1AUTH EQU   X'01'   .... ...1    CMD ISSUER IS CMD AUTHORIZED
*
SV14GB   DS    A                      R14 save area                v110
GBREM    DC    F'0'                   # bytes remaining in phys recv110
GBPOS    DS    A                      -> cur position in BUFF      v110
GBRPS    DS    A                      -> cur position in phys rec  v110
*                                                                  v110
UTLNAME  DS    CL8                    Utility name                 v110
*
REC      DS    CL80                   Physical record              v110
TRTAB    DS    0CL256                 Translate table              v120
BUFF     DS    CL256                  GB buffer containing key datav110
*
NJESA    DS    18F                     NJECMX OS save area
CMCSA    DS    18F                     NJECMC OS save area         v110
CMGSA    DS    18F                     NJECMG OS save area         v110
CMHSA    DS    18F                     NJECMH OS save area         v110
CMESA    DS    18F                     NJECME OS save area         v110
BALRSAVE DS    16F                     Local rtns register save
*
         DS    0D                      Force doubleword size
         SHVCB DEFINE
*
         WORKEND
         COPY  NETSPOOL
         COPY  TAG
         COPY  LINKTABL
         DCBD  DSORG=PS,DEVD=DA
         END
