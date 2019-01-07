#ifndef __RXMVSEXT_H
#define __RXMVSEXT_H

#if defined(__MVS__) && defined(JCC) || defined(__CROSS__)

int __CDECL GetClistVar(PLstr name, PLstr value);
int __CDECL SetClistVar(PLstr name, PLstr value);

/* ---------------------------------------------------------- */
/* assembler module RXIKJ441                                  */
/* ---------------------------------------------------------- */
typedef struct trx_ikjct441_params {
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
typedef struct trx_ptime_params {
    unsigned    *wptmadr;
    unsigned    *wptladr;
    unsigned    *wptccadr;
    unsigned    *wptwkadr;
} RX_PTIME_PARAMS, *RX_PTIME_PARAMS_PTR;

/* ---------------------------------------------------------- */
/* assembler module RXSTIME                                   */
/* ---------------------------------------------------------- */
typedef struct t_rx_stime_params {
    unsigned    *wstmadr;
    unsigned    *wstladr;
    unsigned    *wstccadr;
    unsigned    *wstwkadr;
} RX_STIME_PARAMS, *RX_STIME_PARAMS_PTR;

/* ---------------------------------------------------------- */
/* assembler module RXWTO                                     */
/* ---------------------------------------------------------- */
typedef struct t_rx_wto_params {
    char        *msgadr;
    unsigned    *msgladr;
    unsigned    *ccadr;
    unsigned    *wkadr;
} RX_WTO_PARAMS, *RX_WTO_PARAMS_PTR;

/* ---------------------------------------------------------- */
/* assembler module RXAIT                                     */
/* ---------------------------------------------------------- */
typedef struct t_rx_wait_params {
    unsigned    *timeadr;
    unsigned    *ccadr;
    unsigned    *wkadr;
} RX_WAIT_PARAMS, *RX_WAIT_PARAMS_PTR;

/* ---------------------------------------------------------- */
/* assembler module RXABEND                                   */
/* ---------------------------------------------------------- */
typedef struct trx_abend_params {
    int         ucc;
} RX_ABEND_PARAMS, *RX_ABEND_PARAMS_PTR;

#endif

#if defined(__MVS__) && defined(JCC)
extern unsigned int call_rxikj441 (RX_IKJCT441_PARAMS_PTR params);
extern int call_rxptime (RX_PTIME_PARAMS_PTR params);
extern int call_rxstime (RX_STIME_PARAMS_PTR params);
extern int call_rxwto (RX_WTO_PARAMS_PTR params);
extern int call_rxwait (RX_WAIT_PARAMS_PTR params);
extern unsigned int call_rxabend (RX_ABEND_PARAMS_PTR params);
#elif defined(__CROSS__)
unsigned int call_rxikj441 (RX_IKJCT441_PARAMS_PTR params);
int call_rxptime (RX_PTIME_PARAMS_PTR params);
int call_rxstime (RX_STIME_PARAMS_PTR params);
int call_rxwto (RX_WTO_PARAMS_PTR params);
int call_rxwait (RX_WAIT_PARAMS_PTR params);
unsigned int call_rxabend (RX_ABEND_PARAMS_PTR params);
#endif

#endif