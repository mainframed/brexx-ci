#include <stdlib.h>
#include "rexx.h"
#include "rxdefs.h"
#include "rxmvsext.h"
#ifdef __DEBUG__
#include "bmem.h"
#endif

/* internal function prototypes */
int checkNameLength(long lName);
int checkValueLength(long lValue);
int checkVariableBlacklist(const PLstr name);

#define BLACKLIST_SIZE 4
char *RX_VAR_BLACKLIST[BLACKLIST_SIZE] = {"RC", "LASTCC", "SIGL", "RESULT"};

#if defined(__MVS__) && defined(JCC) || defined(__CROSS__)
void R_wto( const int func )
{
    RX_WTO_PARAMS_PTR params;

    char  *msgptr = NULL;
    size_t msglen = 0;
    int      cc     = 0;
    void     *wk;

    if (ARGN != 1) Lerror(ERR_INCORRECT_CALL,0);

    msglen = MIN(strlen(LSTR(*ARG1)),80);

    if (msglen > 0) {
        msgptr = malloc(msglen);
        params = malloc(sizeof(RX_WTO_PARAMS));
        wk     = malloc(256);

        memset(msgptr,0,80);
        memcpy(msgptr,LSTR(*ARG1),msglen);


        params->msgadr       = msgptr;
        params->msgladr      = &msglen;
        params->ccadr        = (unsigned *)&cc;
        params->wkadr        = (unsigned *)wk;

        call_rxwto(params);

        free(wk);
        free(params);
        free(msgptr);
    }

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
    RX_ABEND_PARAMS_PTR params;
    int      ucc     = 0;

    if (ARGN != 1) Lerror(ERR_INCORRECT_CALL,0);

    get_i (1,ucc);

    params = malloc(sizeof(RX_WAIT_PARAMS));

    params->ucc          = ucc;

    call_rxabend(params);

    free(params);
}

/* new function */
void R_dec( const int func )
{
    if (ARGN != 1) Lerror(ERR_INCORRECT_CALL,0);

    Ldec(ARG1);

    Lstrcpy(ARGR,ARG1);
}

void R_inc( const int func )
{
    if (ARGN != 1) Lerror(ERR_INCORRECT_CALL,0);

    Linc(ARG1);

    Lstrcpy(ARGR,ARG1);
}

#ifdef __DEBUG__
void R_magic( const int func )
{
    void *pointer;
    long decAddr;
    int  count;
    char magicstr[64];

    char option='F';

    if (ARGN>1)
        Lerror(ERR_INCORRECT_CALL,0);
    if (exist(1)) {
        L2STR(ARG1);
        option = l2u[(byte)LSTR(*ARG1)[0]];
    }

    option = l2u[(byte)option];

    switch (option) {
        case 'F':
            pointer = mem_first();
            decAddr = (long) pointer;
            sprintf(magicstr,"%ld",decAddr);
            break;
        case 'L':
            pointer = mem_last();
            decAddr = (long) pointer;
            sprintf(magicstr,"%ld",decAddr);
            break;
        case 'C':
            count = mem_count();
            sprintf(magicstr,"%d",count);
            break;
    }

    Lscpy(ARGR,magicstr);
}
#endif

void RxMvsInitialize()
{
    /* MVS specific functions */
    RxRegFunction("WAIT",   R_wait,   0);
    RxRegFunction("WTO",    R_wto ,   0);
    RxRegFunction("ABEND",  R_abend , 0);
    /* new functions */
    RxRegFunction("DEC",    R_dec,    0);
    RxRegFunction("INC",    R_inc,    0);

#ifdef __DEBUG__
    RxRegFunction("MAGIC",  R_magic,  0);
#endif
}

int GetClistVar( const PLstr name, PLstr value)
{
    int rc = 0;
    void *wk;

    RX_IKJCT441_PARAMS_PTR params;

    /* do not handle special vars here */
    if (checkVariableBlacklist(name) != 0)
        return -1;

    /* NAME LENGTH < 1 OR > 252 */
    if (checkNameLength(name->len) != 0)
        return -2;

    params = malloc(sizeof(RX_IKJCT441_PARAMS));
    wk     = malloc(256);

    memset(wk,     0, sizeof(wk));
    memset(params, 0, sizeof(params)),

    params->ecode    = 18;
    params->nameadr  = name->pstr;
    params->namelen  = name->len;
    params->valueadr = 0;
    params->valuelen = 0;
    params->wkadr    = wk;

    rc = call_rxikj441 (params);

    if (value->maxlen < params->valuelen) {
        Lfx(value,params->valuelen);
    }
    strncpy(value->pstr,params->valueadr,params->valuelen);

    value->len    = params->valuelen;
    value->maxlen = params->valuelen;
    value->type   = LSTRING_TY;

    free(wk);
    free(params);

    return rc;

}

int SetClistVar( const PLstr name, PLstr value)
{
    int rc = 0;
    void *wk;

    RX_IKJCT441_PARAMS_PTR params;

    /* convert numeric values to a string */
    if (value->type != LSTRING_TY) {
        L2str(value);
    }

    /* terminate all strings with a binary zero */
    LASCIIZ(*name);
    LASCIIZ(*value);

    /* do not handle special vars here */
    if (checkVariableBlacklist(name) != 0)
        return -1;

    /* NAME LENGTH < 1 OR > 252 */
    if (checkNameLength(name->len) != 0)
        return -2;

    /* VALUE LENGTH < 0 OR > 32767 */
    if (checkValueLength(value->len) != 0)
        return -3;

    params = malloc(sizeof(RX_IKJCT441_PARAMS));
    wk     = malloc(256);

    memset(wk,     0, sizeof(wk));
    memset(params, 0, sizeof(params)),

    params->ecode    = 2;
    params->nameadr  = name->pstr;
    params->namelen  = name->len;
    params->valueadr = value->pstr;
    params->valuelen = value->len;
    params->wkadr    = wk;

    rc = call_rxikj441(params);

    free(wk);
    free(params);

    return rc;
}

/* internal functions */
int checkNameLength(long lName)
{
    int rc = 0;
    if (lName < 1)
        rc = -1;
    if (lName > 252)
        rc =  1;

    return rc;
}

int checkValueLength(long lValue)
{
    int rc = 0;

    if (lValue == 0)
        rc = -1;
    if (lValue > 32767)
        rc =  1;

    return rc;
}

int checkVariableBlacklist(const PLstr name)
{
    int rc = 0;
    int i  = 0;

    for (i; i < BLACKLIST_SIZE; ++i) {
        if (strcmp(name->pstr,RX_VAR_BLACKLIST[i]) == 0)
            return -1;
    }

    return rc;
}
#endif

/* dummy implementations for cross development */
#ifdef __CROSS__
unsigned int call_rxikj441 (RX_IKJCT441_PARAMS_PTR params)
{
    unsigned int rc = 4;
    char * SYSUID = "SYSUID";
    char * VALUE  = "MIG";

    if (params->ecode == 2) {
        printf("FOO> IKJCT441_DUMMY CALLED TO SET A CLIST VARIABLE %s to the value %s\n", params->nameadr, params->valueadr);
    } else if (params->ecode == 18) {
        printf("FOO> IKJCT441_DUMMY CALLED TO GET A CLIST VARIABLE %s\n", params->nameadr);

        if (strcmp(params->nameadr,SYSUID)==0){
            params->valueadr = VALUE;
            params->valuelen = (unsigned int) strlen(VALUE);
            rc = 0;
        }
    }

    return rc;
}

int call_rxptime (RX_PTIME_PARAMS_PTR params)
{
    printf("FOO> call_rxptime()\n");
}

int call_rxstime (RX_STIME_PARAMS_PTR params)
{
    printf("FOO> call_rxstime()\n");
}

int call_rxwto (RX_WTO_PARAMS_PTR params)
{
    printf("FOO> call_rxwto()\n");
}

int call_rxwait (RX_WAIT_PARAMS_PTR params)
{
    printf("FOO> call_rxwait()\n");
}

unsigned int call_rxabend (RX_ABEND_PARAMS_PTR params)
{
    printf("FOO> call_rxabend()\n");
}

#endif





