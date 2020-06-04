 #ifdef __cplusplus
 extern "C" {
 #endif

 struct   __S99rbx ;
 struct   __S99emparms;

 struct   __DYNstruct {
   char           *__ddname;     /* DDNAME */
   char           *__dsname;     /* DSNAME, dataset name  */
   char            __sysout;     /* system output dataset */
   __pad64(__01__,7)
   char           *__sysoutname; /* program name for sysout */
   char           *__member;     /* member of a PDS */
   char            __status;     /* dataset status  */
   char            __normdisp;   /* dataset's normal disp */
   char            __conddisp;   /* dataset's cond disp */
   __pad64(__02__,5)
   char           *__unit;       /* unit name of dataset */
   char           *__volser;     /* volume serial number */
   short           __dsorg;      /* dataset organization */
   char            __alcunit;    /* unit of space allocation */
   __pad64(__03__,1)
   int             __primary;    /* primary space allocation */
   int             __secondary;  /* secondary space alloc'n  */
   short           __recfm;      /* the record format */
   short           __blksize;    /* the block size */
   unsigned short  __lrecl;      /* record length  */
   __pad64(__04__,6)
   char           *__volrefds;   /* volume serial reference */
   char           *__dcbrefds;   /* dsname for DCB reference */
   char           *__dcbrefdd;   /* ddname for DCB reference */
   unsigned char   __misc_flags; /* attribute flags */
   __pad64(__05__,7)
   char           *__password;   /* password */
   char * __ptr32 * __ptr32 __miscitems; /* all remaining text units */
   short           __infocode;   /* SVC 99 info code  */
   short           __errcode;    /* specifies SVC 99 error code */
   int             __dirblk;     /* number of directory blks */
   int             __avgblk;     /* the average block length */
   char           *__storclass;  /* SMS storage class  */
   char           *__mgntclass;  /* SMS management class  */
   char           *__dataclass;  /* SMS data class  */
   unsigned char   __recorg;     /* Vsam dataset organization */
   __pad64(__06__,1)
   short           __keylength;  /* Vsam key length  */
   short           __keyoffset;  /* Vsam key offset  */
   __pad64(__07__,2)
   char           *__refdd;      /* copy attributes of ref. dd*/
   char           *__like;       /* copy attributes of like dsn */
   unsigned char   __dsntype;    /* type att. of pds or pdse */
   __pad64(__08__,7)
   __pad64(__09__,4)
   struct __S99rbx * __ptr32 __rbx;      /* to the req. block extension */
   __pad64(__10__,4)
   struct __S99emparms * __ptr32 __emsgparmlist; /* @ of error msg parms*/
   char           *__pathname;   /* path name                   */
   unsigned int    __pathopts;   /* path options                */
   unsigned int    __pathmode;   /* path access attributes      */
   char            __pathndisp;  /* path normal disposition   */
   char            __pathcdisp;  /* path abnormal disposition   */
   unsigned int    __reserved1;  /* reserved                    */
   unsigned int    __reserved2;  /* reserved                    */
   unsigned int    __reserved3;  /* reserved                    */
   unsigned int    __reserved4;  /* reserved                    */
   __pad64(__12__,4)
   };

typedef struct __DYNstruct __dyn_t;

    #if defined(__NATIVE_ASCII_F) && \
        (__EA_F >= __EA_F_4102_PQ63405)
      #pragma map (dynalloc,         "\174\174A00385")
      #pragma map (dynfree,          "\174\174A00386")
    #else /* Not __NATIVE_ASCII_F*/
      #ifdef __LIBASCII_F
        #pragma map (dynalloc,"\174\174DYNAL\174")
      #endif /* __LIBASCII_F*/
    #endif  /* __NATIVE_ASCII_F  */

    int   dyninit  (__dyn_t * dyn_parms);
    /* dyninit() zeros out the __dyn_t structure and places the version */
    /* number of the structure into the byte right after __sysout.      */
    #define dyninit(__dynp) \
       (\
        memset((char *)__dynp,'\0',sizeof(__dyn_t)),   \
        *((char *)__dynp+1+(2*sizeof(void *))) = 0x01, 0 \
       )

    int __dynall   (__dyn_t *);
    int   dynalloc (__dyn_t *);

    int __dynfre  (__dyn_t *);
    int   dynfree (__dyn_t *);

    #ifdef __AE_BIMODAL_F
      #pragma map (__dynalloc_a,     "\174\174A00385")
      #pragma map (__dynalloc_e,     "\174\174DYNALL")
      #pragma map (__dynfree_a,      "\174\174A00386")
      #pragma map (__dynfree_e,      "\174\174DYNFRE")

      __new4102( int, __dynalloc_a,(__dyn_t *));
      __new4102( int, __dynalloc_e,(__dyn_t *));
      __new4102( int, __dynfree_a,(__dyn_t *));
      __new4102( int, __dynfree_e,(__dyn_t *));

    #endif /* __AE_BIMODAL_F*/


    /* for sysout */

    #define __DEF_CLASS '-'

    /* for alcunit */
    #define __CYL               '\x01'
    #define __TRK               '\x02'

    /* for status: */

    #define __DISP_OLD          0x01
    #define __DISP_MOD          0x02
    #define __DISP_NEW          0x04
    #define __DISP_SHR          0x08

    /* for normdisp, conddisp: */

    #define __DISP_UNCATLG      0x01
    #define __DISP_CATLG        0x02
    #define __DISP_DELETE       0x04
    #define __DISP_KEEP         0x08

    /* for dsorg: */

    #define __DSORG_unknown     0x0000
    #define __DSORG_VSAM        0x0008
    #define __DSORG_GS          0x0080
    #define __DSORG_PO          0x0200
    #define __DSORG_POU         0x0300
    #define __DSORG_DA          0x2000
    #define __DSORG_DAU         0x2100
    #define __DSORG_PS          0x4000
    #define __DSORG_PSU         0x4100
    #define __DSORG_IS          0x8000
    #define __DSORG_ISU         0x8100

    /* for recfm: */

    #define _M_                 0x02
    #define _A_                 0x04
    #define _S_                 0x08
    #define _B_                 0x10
    #define _D_                 0x20
    #define _V_                 0x40
    #define _F_                 0x80
    #define _U_                 0xc0
    #define _FB_                0x90
    #define _VB_                0x50
    #define _FBS_               0x98
    #define _VBS_               0x58

    /* for miscflags: */

    #define __CLOSE             0x01
    #define __RELEASE           0x02
    #define __PERM              0x04
    #define __CONTIG            0x08
    #define __ROUND             0x10
    #define __TERM              0x20
    #define __DUMMY_DSN         0x40
    #define __HOLDQ             0x80

    /* for vsam record organization: */

    #define __KS                0x80
    #define __ES                0x40
    #define __RR                0x20
    #define __LS                0x10

    #if __EDC_TARGET >= 0X22080000
    /* for vsam record level sharing */

    #define __RLS_NRI           0x80
    #define __RLS_CR            0x40
    #define __RLS_CRE           0x20
    #endif                           /* __EDC_TARGET                */

    /* for pds type attributes: */

    #define __DSNT_HFS          0x10
    #define __DSNT_PIPE         0x20
    #define __DSNT_PDS          0x40
    #define __DSNT_LIBRARY      0x80

    /* for path options */

    #define __PATH_OCREAT       0x00000080
    #define __PATH_OEXCL        0x00000040
    #define __PATH_ONOCTTY      0x00000020
    #define __PATH_OTRUNC       0x00000010
    #define __PATH_OAPPEND      0x00000008
    #define __PATH_ONONBLOCK    0x00000004
    #define __PATH_ORDWR        0x00000003
    #define __PATH_ORDONLY      0x00000002
    #define __PATH_OWRONLY      0x00000001

    /* for path attributes  */

    #define __PATH_SISUID       0x00000800
    #define __PATH_SISGID       0x00000400
    #define __PATH_SIRUSR       0x00000100
    #define __PATH_SIWUSR       0x00000080
    #define __PATH_SIXUSR       0x00000040
    #define __PATH_SIRWXU       0x000001C0
    #define __PATH_SIRGRP       0x00000020
    #define __PATH_SIWGRP       0x00000010
    #define __PATH_SIXGRP       0x00000008
    #define __PATH_SIRWXG       0x00000038
    #define __PATH_SIROTH       0x00000004
    #define __PATH_SIWOTH       0x00000002
    #define __PATH_SIXOTH       0x00000001
    #define __PATH_SIRWXO       0x00000007

 #endif /* _EXT */


 #ifdef __cplusplus
 }
 #endif

                   #pragma checkout(resume)
                   ??=endif   /* __dynit */

