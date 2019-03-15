#ifndef __RXMVSEXT_H
#define __RXMVSEXT_H

/* TODO: should be moved to rxmvs.h */
int __CDECL isTSO();
int __CDECL isTSOFG();
int __CDECL isTSOBG();
int __CDECL isEXEC();
int __CDECL isIPSF();

int __CDECL GetClistVar(PLstr name, PLstr value);
int __CDECL SetClistVar(PLstr name, PLstr value);

/* ---------------------------------------------------------- */
/* environment context RXENVCTX                               */
/* ---------------------------------------------------------- */
typedef  struct trx_env_ctx
{
    /* **************************/
    /* SYSVARS                  */
    /* **************************/

    /* User Information */
    char    SYSPREF[9];
    char    SYSUID[9];
    /* Terminal Information */
    /* Exec Information */
    char    SYSENV[5];
    char    SYSISPF[11];
    /* System Information */
    /* Language Information */

    /* **************************/
    /* MVSVARS                  */
    /* **************************/

    char    FILLER01[2];

    /* **************************/
    /* FLAG FIELD               */
    /* **************************/

    unsigned char flags1;  /* allocations */
    unsigned char flags2;  /* environment */
    unsigned char flags3;  /* unused */
    unsigned char flags4;  /* unused */

} RX_ENVIRONMENT_CTX, *RX_ENVIRONMENT_CTX_PTR;

/* ---------------------------------------------------------- */
/* assembler module RXIKJ441                                  */
/* ---------------------------------------------------------- */
typedef struct trx_init_params
{
    unsigned   *rxctxadr;
    unsigned   *wkadr;
} RX_INIT_PARAMS, *RX_INIT_PARAMS_PTR;

/* ---------------------------------------------------------- */
/* assembler module RXIKJ441                                  */
/* ---------------------------------------------------------- */
typedef struct trx_ikjct441_params
{
    unsigned    ecode;
    size_t      namelen;
    char       *nameadr;
    size_t      valuelen;
    char       *valueadr;
    unsigned   *wkadr;
} RX_IKJCT441_PARAMS, *RX_IKJCT441_PARAMS_PTR;

/* ---------------------------------------------------------- */
/* assembler module RXPTIME                                   */
/* ---------------------------------------------------------- */
typedef struct trx_ptime_params
{
    unsigned    *wptmadr;
    unsigned    *wptladr;
    unsigned    *wptccadr;
    unsigned    *wptwkadr;
} RX_PTIME_PARAMS, *RX_PTIME_PARAMS_PTR;

/* ---------------------------------------------------------- */
/* assembler module RXSTIME                                   */
/* ---------------------------------------------------------- */
typedef struct t_rx_stime_params
{
    unsigned    *wstmadr;
    unsigned    *wstladr;
    unsigned    *wstccadr;
    unsigned    *wstwkadr;
} RX_STIME_PARAMS, *RX_STIME_PARAMS_PTR;

/* ---------------------------------------------------------- */
/* assembler module RXWTO                                     */
/* ---------------------------------------------------------- */
typedef struct t_rx_wto_params
{
    char        *msgadr;
    unsigned    *msgladr;
    unsigned    *ccadr;
    unsigned    *wkadr;
} RX_WTO_PARAMS, *RX_WTO_PARAMS_PTR;

/* ---------------------------------------------------------- */
/* assembler module RXAIT                                     */
/* ---------------------------------------------------------- */
typedef struct t_rx_wait_params
{
    unsigned    *timeadr;
    unsigned    *ccadr;
    unsigned    *wkadr;
} RX_WAIT_PARAMS, *RX_WAIT_PARAMS_PTR;

/* ---------------------------------------------------------- */
/* assembler module RXABEND                                   */
/* ---------------------------------------------------------- */
typedef struct trx_abend_params
{
    int         ucc;
} RX_ABEND_PARAMS, *RX_ABEND_PARAMS_PTR;

#ifdef __CROSS__
typedef struct trx_ikj441_dummy_dict
{
    char *key;
    char *value;
    struct trx_ikj441_dummy_dict *next;
} RX_IKJ441_DUMMY_DICT, *RX_IKJ441_DUMMY_DICT_PTR;

typedef struct trx_ikj441_dummy_dict_head
{
    RX_IKJ441_DUMMY_DICT *first;
} RX_IKJ441_DUMMY_DICT_HEAD;
#endif

#ifdef __CROSS__
int call_rxinit(RX_INIT_PARAMS_PTR params);
unsigned int call_rxikj441 (RX_IKJCT441_PARAMS_PTR params);
int call_rxptime (RX_PTIME_PARAMS_PTR params);
int call_rxstime (RX_STIME_PARAMS_PTR params);
int call_rxwto (RX_WTO_PARAMS_PTR params);
int call_rxwait (RX_WAIT_PARAMS_PTR params);
unsigned int call_rxabend (RX_ABEND_PARAMS_PTR params);
#else
extern int call_rxinit(RX_INIT_PARAMS_PTR params);
extern unsigned int call_rxikj441 (RX_IKJCT441_PARAMS_PTR params);
extern int call_rxptime (RX_PTIME_PARAMS_PTR params);
extern int call_rxstime (RX_STIME_PARAMS_PTR params);
extern int call_rxwto (RX_WTO_PARAMS_PTR params);
extern int call_rxwait (RX_WAIT_PARAMS_PTR params);
extern unsigned int call_rxabend (RX_ABEND_PARAMS_PTR params);
#endif

struct psa {
    char    psastuff[548];      /* 548 bytes before ASCB ptr */
    struct  ascb *psaaold;
};

struct ascb {
    char    ascbascb[4];        /* acronym in ebcdic -ASCB- */
    char    ascbstuff[104];     /* 104 byte to the ASXB ptr */
    struct  asxb *ascbasxb;
};

struct asxb {
    char    asxbasxb[4];        /* acronym in ebcdic -ASXB- */
    char    asxbstuff[16];         /* 16 bytes to the lwa ptr */
    struct lwa *asxblwa;
};

struct lwa {
    int     lwapptr;          /* address of the logon work area */
    char    lwalwa[8];        /* ebcdic ' LWA '
    char    lwastuff[12];     /* 12 byte to the PSCB ptr */
    struct  pscb *lwapscb;
};

struct pscb {
    char    pscbstuff[52];        /* 52 byte before UPT ptr */
    struct  upt *pscbupt;
};

struct upt {
    char    uptstuff[16];         /* 16 byte before UPTPREFX */
    char    uptprefx[7];        /* dsname prefix */
    char    uptprefl;        /* length of dsname prefix */
};

#endif