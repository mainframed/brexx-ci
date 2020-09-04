/* ---------------------------------------------------------------------
 * This document has been taken from CBT File 089
 *   PL1, Cobol and Fortran related notes have been dropped, as its
 *   usage is for REXX using a wrapper
 * Failing SVC99 operations are reported in details in the console log
 * To suppress this report you need to specify NOPRINT calling the
 * wrapper (default is PRINT)
 * $INTERNAL   Do not ship in BREXX Release
 * ---------------------------------------------------------------------
 */
          OPERATIONS
          __________
          The second parameter to DYNAM describes the operation you
          wish to perform.
               ALLOC     - allocate a dataset
               UNALLOC   - unallocate a dataset
               CONCAT    - concatenate 2 or more DD names
               UNCONCAT  - unconcatenate previously concatenated DD names.
               REMOVE    - remove in-use attribute
               DDALLOC   - allocate a DD name
          ***  INFO      - retrieve information about an allocation
          ***  not implemented in BREXX
 
          OPERANDS
          ________
             The  third  and  subsequent  parameters  supply  operands
          needed to perform dynamic allocation.  Only one operand par-
          ameter is required  but it is sometimes convenient  to use a
          larger number, for example when obtaining allocation parame-
          ters from  a user  at a terminal  in an  interactive manner.
          Normal OS linkage  conventions flag the last  parameter in a
          parameter list so a variable  number of parameters is easily
          allowed if the high level host language supports it. Operand
          parameters must be delimited on the right by a semicolon.
             Operands consist of a keyword and an optional value.   if
          a value  is present it is  separated from the keyword  by an
          equals (=)  sign.  Operands are separated from each other by
          one of more spaces and an operand string is delimited on the
          right by  a semicolon.    If a  keyword requires  a list  of
          values, the values in the list are separated by a comma.
             Keywords may be  abbreviated by specifying enough  of the
          keyword so that it is unambiguous.  In cases where an entire
          keyword is  the same as  the first  few letters of  a longer
          keyword, ambiguity is resolved by picking the first keyword.
          In describing  DYNAM keywords below the  unambiguous portion
          of the keyword is written in uppercase.
             An attempt has  been made to minimize the  number of key-
          words that require values.
 
          ALLOC Operation
          _______________
 
             This operation is equivalent to dataset allocation during
          job step  initialization;  the  parameter list  to DYNAM  is
          equivalent to a DD statement.   You  can request most of the
          JCL services that you can code on a DD statement.   In addi-
          tion you can  specify dataset passwords which do  not have a
          JCL equivalent.   The following is  a list of JCL parameters
          and the equivalent DYNAM keyword.
               DD card parameter      DYNAM keyword
               __ ____ _________      _____ _______
               COPIES=num             COPies=num
               DCB=(*.ddname)         DCBDD=ddname
               DCB=(dsname)           DCBDS=dsname
               DCB=(BLKSIZE=num)      BLKsize=num
               DCB=(BUFALN=key)       BUFAln=key  (key = D|F)
               DCB=(BUFIN=num)        BUFIN=num
               DCB=(BUFL=num)         BUFL=num
               DCB=(BUFMAX=num)       BUFMAX=num
               DCB=(BUFNO=num)        BUFNo=num
               DCB=(BUFOFF=num)       BUFOFf=num
               DCB=(BUFOUT=num)       BUFOUt=num
               DCB=(BUFRQ=num)        BUFRq=num
               DCB=(BUFSIZE=num)      BUFSize=num
               DCB=(BUFTEK=key)       BUFTEK=key  (key = A|E|R|S)
               DCB=(CODE=key)         CODe=key  (key = A|B|C|F|I|N|T)
               DCB=(DEN=2)            D800
               DCB=(DEN=3)            D1600
               DCB=(DEN=4)            D6250
               DCB=(DIAGNS=TRACE)     TRAce
               DCB=(DSORG=key)        DSORG=key  (key = CX|DA|DAU|GS|
                                                        PO|PS|PSU|TQ|
                                                        TX|TCAM|VSAM)
               DCB=(EROPT=ABE)        ABE
               DCB=(EROPT=ACC)        ACC
               DCB=(EROPT=SKP)        SKp
               DCB=(KEYLEN=num)       KEYlen=num
               DCB=(LIMCT=num)        LImct=num
               DCB=(LRECL=num)        LRecl=num
               DCB=(MODE=key)         MODE=key  (key = C|CO|CR|E|EO|ER)
               DCB=(NCP=num)          NCP=num
               DCB=(OPTCD=key)        OPTCD=key  (see note 1 below)
               DCB=(PRTSP=key)        PRTsp=key  (key = 0|1|2|3)
               DCB=(RECFM=key)        RECFM=key  (see note 2 below)
               DCB=(RECFM=F)          F
               DCB=(RECFM=FA)         FA
               DCB=(RECFM=FAS)        FAS
               DCB=(RECFM=FB)         FB
               DCB=(RECFM=FBA)        FBA
               DCB=(RECFM=FBAS)       FBAS
               DCB=(RECFM=FBM)        FBM
               DCB=(RECFM=FBS)        FBMS
               DCB=(RECFM=FM)         FM
               DCB=(RECFM=FMS)        FMS
               DCB=(RECFM=FS)         FS
               DCB=(RECFM=U)          U
               DCB=(RECFM=V)          V
               DCB=(RECFM=VA)         VA
               DCB=(RECFM=VAS)        VAS
               DCB=(RECFM=VB)         VB
               DCB=(RECFM=VBA)        VBA
               DCB=(RECFM=VBAS)       VBAS
               DCB=(RECFM=VBM)        VBM
               DCB=(RECFM=VBMS)       VBMS
               DCB=(RECFM=VBS)        VBS
               DCB=(RECFM=VM)         VM
               DCB=(RECFM=VMS)        VMS
               DCB=(RECFM=VS)         VS
               DCB=(STACK=num)        STACK=num
               DCB=(TRTCH=key)        TRTch=key  (key = C|E|ET|T)
               DISP=(MOD)             MOD
               DISP=(NEW)             NEW
               DISP=(OLD)             OLD
               DISP=(SHR)             SHr
               DISP=(,CATLG)          CAtlg
               DISP=(,DELETE)         DElete
               DISP=(,KEEP)           KEEp
               DISP=(,UNCATLG)        UNCatlg
               DISP=(,,CATLG)         CCatlg
               DISP=(,,DELETE)        CDelete
               DISP=(,,KEEP)          CKeep
               DISP=(,,UNCATLG)       CUncatlg
               DSN=...(name)          MEmber=name
               DSN=dsname             DSN=dsname
               DUMMY                  DUMMY
               FCB=(name)             FORms=name
               FCB=(,ALIGN)           ALIgn
               FCB=(,VERIFY)          VERIFYF
               FREE=CLOSE             CLose
               HOLD=YES               Hold
               LABEL=(num)            DSSeq=num
               LABEL=(EXPDT=yyddd)    Expdt=yyddd
               LABEL=(RETPD=num)      RETpd=num
               LABEL=(,AL)            AL
               LABEL=(,AUL)           AUL
               LABEL=(,BLP)           BLP
               LABEL=(,LTM)           LTM
               LABEL=(,NL)            NL
               LABEL=(,NSL)           NSL
               LABEL=(,SL)            SL
               LABEL=(,SUL)           SUL
               LABEL=(,,IN)           Input
               LABEL=(,,NOPWREAD)     PASSWRite
               LABEL=(,,OUT)          OUTput
               LABEL=(,,PASSWORD)     PASSRead
               MSVGP=name             MSVGP=name
               OUTLIM=num             OUTLim=num
               QNAME=name             QNAME=name
               SPACE=(num)            BLOck=num
               SPACE=(CYL)            CYL
               SPACE=(TRK)            TRK
               SPACE=(,(num))         PRIMary=num
               SPACE=(,(,num))        SECondary=num
               SPACE=(,(,,num))       DIRectory=num
               SPACE=(,,RLSE)         RLse
               SPACE=(,,,ALX)         ALX
               SPACE=(,,,CONTIG)      CONtig
               SPACE=(,,,MXIG)        MXIG
               SPACE=(,,,,ROUND)      ROund
               SYSOUT=name            SYSOUt=name
               SYSOUT=(,name)         SYSOUProg=name
               SYSOUT=(,,name)        SYSOUForms=name
               TERM=TS                TErmfile
               UCS=(,FOLD)            FOLdmode
               UCS=(,,VERIFY)         VERIFYC
               UNIT=name              UNIT=name
               UNIT=(,num)            UNITCount=num
               UNIT=(,P)              PARallel
               VOL=(,,num)            VOLSeq=num
               VOL=(,,,num)           VOLCount=num
               VOL=(,,,,REF=name)     VOLRef=name
               VOL=(,,,,SER=(name))   VOLume=name
               VOL=(PRIVATE)          PRIVate
               note 1:  For a complete listing of possible values
               for the  OPTCD parameter refer  to the  IBM manual
               OS/VS2 JCL.
               note 2:  In  addition to the stand  alone keywords
               for Fixed, Undefined,  and Variable record formats
               others may  be coded by  using the  RECFM=key key-
               word.   For  a complete listing  refer to  the IBM
               manual OS/VS2 JCL.
               Others                 DYNAM keyword
               ______                 _____ _______
               DDNAME on DD card      DD=name
               PASSWORD               PASSWOrd=password
               /*ROUTE dest           REMOTE=dest
               assign the permanently
               allocated attribute to
               this resource          PERManent
               assign the convertible
               attribute to this
               resource               CONVert
               note:  For a complete  explaination of the perman-
               ently  allocated  attribute  and  the  convertible
               attribute refer to SPL: JOB MANAGEMENT.
 
 
          UNALLOC and its subsequent parameters
          _____________________________________
             This operation unallocates a dataset  by DD name or data-
          set name.  The following is a list of JCL parameters and the
          equivalent DYNAM keyword.
               DD card parameter      DYNAM keyword
               __ ____ _________      _____ _______
               DISP=(,CATLG)          CAtlg
               DISP=(,DELETE)         DElete
               DISP=(,KEEP)           KEEp
               DISP=(,UNCATLG)        UNCatlg
               DSN=...(name)          MEmber=name
               DSN=dsname             DSN=dsname
               Others                 DYNAM keyword
               ______                 _____ _______
               DDNAME on DD card      DD=name
               change SYSOUT class    NEWClass=name
               put SYSOUT output
               into the hold queue    NEWHold
               take SYSOUT output
               out of the hold queue  NEWNohold
               change SYSOUT
               routing                NEWRemote=name
               unallocate the
               resource even if
               permanently allocated  UNAlloc
               remove the in-use
               attribute even if
               permanently allocated  REMOVe
 
          CONCAT AND UNCONCAT Operation
          _____________________________
             These two operations concatenate  and unconcatenate data-
          sets.  The datasets can only be identified by using DD names
          of datasets currently  allocated so therefore the  only key-
          word needed in the third parameter to DYNAM is DD=name.   To
          concatentate  you  provide   a  list  of  DD   names,   e.g.
          DD=SYSLIB,FILE2,FILE3.   The contenation  is then identified
          by the  first DD name in  the list.   To  unconcatenate just
          provide DD=name.
 
          EXAMPLES
          ________
               CALL DYNAM(WORK, 'ALLOC ', 'DSN=USERID.DATA NEW CATLG;',
                                          'VOL=USER01;',
                                          'TRK PRIMARY=1 SECONDARY=1;',
                                          'LRECL=80 BLKSIZE=6080 FB;')
               CALL DYNAM(WORK, 'ALLOC ', 'DSN=SYS1.USERLINK;',
                                          'DD=SYSLIB SHR;');
               CALL DYNAM(WORK, 'ALLOC ', 'DSN=SYS1.LINKUMW;',
                                          'DD=FILE2 SHR;');
               CALL DYNAM(WORK, 'CONCAT ', 'DD=SYSLIB,FILE2;');
 
          Return Codes
          ____________
               SVC99 RETURN CODES (SEE SPL:JOB MANAGEMENT PAGE 34)
                     00 SUCCESSFULL COMPLETION
                     04 ENVIRONMENT, RESOURCE FAILURE, SYSTEM ROUTINE
                     08 REQUEST DENIED BY INSTALLATION VALIDATION ROUTINE
                     12 INVALID PARAMETER LIST
                  INTERFACE RETURN CODES
                     16 INVALID VERB
                     20 INVALID KEYWORD
                     24 WORK AREA OVERFLOW
                     28 VALUE NOT FOUND IN SUBTABLE: INVALID VALUE
 
  */
