#include <stdlib.h>
#include "rexx.h"
#include "rxdefs.h"
#include "rxmvsext.h"

/* internal function prototypes */
int checkNameLength(long lName);
int checkValueLength(long lValue);

#if defined(__MVS__) && defined(JCC)
void R_putvar( const int func )
{
    int rc = 0;
    char *name;
    int   namelen;
    char *value;
    int   valuelen;
    void *wk;

    RX_IKJCT441_PARAMS_PTR params;

    if (ARGN != 2) Lerror(ERR_INCORRECT_CALL,0);

    namelen  = LLEN(*ARG1);
    valuelen = LLEN(*ARG2);

    if (checkNameLength(namelen) != 0
        ||
        checkValueLength(valuelen) != 0)
    {
        rc = -1;
    }

    if ( rc == 0)
    {
        params = malloc(sizeof(RX_IKJCT441_PARAMS));
        wk     = malloc(256);

        memset(wk,     0, sizeof(wk));
        memset(params, 0, sizeof(params)),

        params->ecode    = 2;
        params->nameadr  = LSTR(*ARG1);
        params->namelen  = namelen;
        params->valueadr = LSTR(*ARG2);
        params->valuelen = valuelen;
        params->wkadr    = wk;

        rc = call_rxikj441(params);

        free(wk);
        free(params);
    }
}

void R_getvar( const int func )
{
    int rc = 0;
    char *name;
    int   namelen;
    char *value;
    int   valuelen;
    void *wk;

    RX_IKJCT441_PARAMS_PTR params;

    if (ARGN != 1) Lerror(ERR_INCORRECT_CALL,0);

    namelen  = LLEN(*ARG1);

    if (checkNameLength(namelen) != 0)
    {
        rc = -1;
    }

    if ( rc == 0)
    {
        params = malloc(sizeof(RX_IKJCT441_PARAMS));
        wk     = malloc(256);
/*      value  = malloc(32767);    */

        memset(wk,     0, sizeof(wk));
        memset(params, 0, sizeof(params)),

        params->ecode    = 18;
        params->nameadr  = LSTR(*ARG1);
        params->namelen  = namelen;
        params->valueadr = value;
        params->valuelen = 0;
        params->wkadr    = wk;

        rc = call_rxikj441 (params);

        if ( rc == 0)
        {
            Lscpy(ARGR,params->valueadr);
        }

        free(wk);
/*      free(value);               */
        free(params);
    }
}

void R_wto( const int func )
{
    int rc = 0;

    RX_WTO_PARAMS_PTR params;
    char     *msgptr;
    unsigned msglen = 0;
    int      cc     = 0;
    void     *wk;

    if (ARGN != 1) Lerror(ERR_INCORRECT_CALL,0);

    msglen = MIN(strlen(LSTR(*ARG1)),80);
    memset(msgptr,0,80);
    memcpy(msgptr,LSTR(*ARG1),msglen);

    params = malloc(sizeof(RX_WTO_PARAMS));
    wk     = malloc(256);

    params->msgadr       = msgptr;
    params->msgladr      = &msglen;
    params->ccadr        = (unsigned *)&cc;
    params->wkadr        = (unsigned *)wk;

    rc = call_rxwto(params);

    free(wk);
    free(params);
}

void R_wait( const int func )
{
    int rc = 0;

    RX_WAIT_PARAMS_PTR params;
    unsigned time   = 1;
    int      cc     = 0;
    void     *wk;

    if (ARGN != 1) Lerror(ERR_INCORRECT_CALL,0);

    get_i (1,time);

    params = malloc(sizeof(RX_WAIT_PARAMS));
    wk     = malloc(256);

    params->timeadr      = &time;
    params->ccadr        = (unsigned *)&cc;
    params->wkadr        = (unsigned *)wk;

    rc = call_rxwait(params);

    free(wk);
    free(params);
}

void R_abend( const int func )
{
    int rc = 0;

    RX_ABEND_PARAMS_PTR params;
    int      ucc     = 0;

    if (ARGN != 1) Lerror(ERR_INCORRECT_CALL,0);

    get_i (1,ucc);

    params = malloc(sizeof(RX_WAIT_PARAMS));

    params->ucc          = ucc;

    rc = call_rxabend(params);

    free(params);
}

void RxMvsInitialize()
{
    RxRegFunction("PUTVAR", R_putvar, 0); /* temporary workaround */
    RxRegFunction("GETVAR", R_getvar, 0); /* temporary workaround */
    RxRegFunction("WAIT",   R_wait,   0);
    RxRegFunction("WTO",    R_wto ,   0);
    RxRegFunction("ABEND",  R_abend , 0);
}

/* internal functions */

/* NAME LENGTH < 1 OR > 252 */
int checkNameLength(long lName)
{
    int rc = 0;
    if (lName < 1)
        rc = -1;
    if (lName > 252)
        rc =  1;

    return rc;
}

/* VALUE LENGTH < 0 OR > 32767 */
int checkValueLength(long lValue)
{
    int rc = 0;

    if (lValue == 0)
        rc = -1;
    if (lValue > 32767)
        rc =  1;

    return rc;
}
#endif